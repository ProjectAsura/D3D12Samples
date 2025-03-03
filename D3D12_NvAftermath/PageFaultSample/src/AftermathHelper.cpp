//-----------------------------------------------------------------------------
// File : AftermathHelper.cpp
// Desc : NVIDIA Aftermath Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <cstring>
#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <d3d12.h>
#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>


//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
inline bool operator <
(
    const GFSDK_Aftermath_ShaderDebugInfoIdentifier& lhs,
    const GFSDK_Aftermath_ShaderDebugInfoIdentifier& rhs
)
{
    if (lhs.id[0] == rhs.id[0])
    { return lhs.id[1] < rhs.id[1]; }

    return lhs.id[0] < rhs.id[0];
}

//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
inline bool operator <
(
    const GFSDK_Aftermath_ShaderBinaryHash& lhs,
    const GFSDK_Aftermath_ShaderBinaryHash& rhs
)
{ return lhs.hash < rhs.hash; }

//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
inline bool operator < 
(
    const GFSDK_Aftermath_ShaderDebugName& lhs,
    const GFSDK_Aftermath_ShaderDebugName& rhs
)
{ return strncmp(lhs.name, rhs.name, sizeof(lhs.name)) < 0; }

namespace {

//-------------------------------------------------------------------------
//! @brief      ファイルを読み込みます.
//! 
//! @param[in]      path        ファイルパス.
//! @param[out]     blob        読み込み結果.
//! @retval true    読み込みに成功.
//! @retval false   読み込みに失敗.
//-------------------------------------------------------------------------
static bool ReadFile(const char* path, std::vector<uint8_t>& blob)
{
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    if (!stream)
    { return false; }

    stream.seekg(0, std::ios::end);
    blob.resize(stream.tellg());
    stream.seekg(0, std::ios::beg);
    stream.read(reinterpret_cast<char*>(blob.data()), blob.size());
    stream.close();

    return true;
}

//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
template<typename T>
inline std::string ToHexString(T n)
{
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2 * sizeof(T)) << std::hex << n;
    return stream.str();
}

//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
inline std::string ToString(GFSDK_Aftermath_Result result)
{ return std::string("0x") + ToHexString(static_cast<UINT>(result)); }

//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
inline std::string ToString(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& identifier)
{ return ToHexString(identifier.id[0]) + "-" + ToHexString(identifier.id[1]); }

//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
inline std::string ToString(const GFSDK_Aftermath_ShaderBinaryHash& hash)
{ return ToHexString(hash.hash); }


//-----------------------------------------------------------------------------
//! @brief      ログを出力します.
//-----------------------------------------------------------------------------
void OutputLog(const char* format, ...)
{
    char msg[2048]= {};
    va_list arg = {};

    va_start( arg, format );
    vsprintf_s( msg, format, arg );
    va_end( arg );

    fprintf(stderr, "%s\n", msg );

    OutputDebugStringA( msg );
    OutputDebugStringA("\n");
}

///////////////////////////////////////////////////////////////////////////////
// ShaderDataBase class
///////////////////////////////////////////////////////////////////////////////
class ShaderDataBase
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    ShaderDataBase()
    : m_Binaries()
    , m_Pdbs    ()
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~ShaderDataBase()
    { Term(); }

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //! 
    //! @param[in]      shaderBinaryDirs        シェーダバイナリのディレクトリ.
    //! @param[in]      shaderPdbDirs           シェーダPBDのディレクトリ.
    //-------------------------------------------------------------------------
    void Init
    (
        const std::vector<std::string>& shaderBinaryDirs,
        const std::vector<std::string>& shaderPdbDirs
    )
    {
        // シェーダを追加.
        for(size_t i=0; i<shaderBinaryDirs.size(); ++i)
        {
            // バイナリ検索用パターン.
            const auto pattern = shaderBinaryDirs[i] + "/*.cso";

            WIN32_FIND_DATAA findData = {};
            auto handle = FindFirstFileA(pattern.c_str(), &findData);
            if (handle == INVALID_HANDLE_VALUE)
                continue;

            do
            {
                const auto path = shaderBinaryDirs[i] + "/" + findData.cFileName;
                AddBinary(path.c_str());
            } while (FindNextFileA(handle, &findData) != 0);

            FindClose(handle);
        }

        // PDB(Program Data Base)を追加.
        const char* pdbExts[] = { ".lld", ".pdb" };
        const auto  pdbExtCounts = sizeof(pdbExts) / sizeof(const char*);
        for(size_t i=0; i<shaderPdbDirs.size(); ++i)
        {
            for(size_t j=0; j<pdbExtCounts; ++j)
            {
                const auto pattern = shaderPdbDirs[i] + "/*" + pdbExts[j];

                WIN32_FIND_DATAA findData = {};
                auto handle = FindFirstFileA(pattern.c_str(), &findData);
                if (handle == INVALID_HANDLE_VALUE)
                    continue;

                do
                {
                    const auto path = shaderPdbDirs[i] + "/" + findData.cFileName;
                    AddPdb(path.c_str(), findData.cFileName);
                } while (FindNextFileA(handle, &findData) != 0);

                FindClose(handle);
            }
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term()
    {
        m_Binaries.clear();
        m_Pdbs    .clear();
    }

    //-------------------------------------------------------------------------
    //! @brief      シェーダバイナリを検索します.
    //! 
    //! @param[in]      hash        シェーダバイナリハッシュ.
    //! @param[out]     shader      シェーダバイナリの格納先.
    //! @retval true    検索に成功.
    //! @retval false   検索に失敗.
    //-------------------------------------------------------------------------
    bool FindBinary(const GFSDK_Aftermath_ShaderBinaryHash& hash, std::vector<uint8_t>& shader) const
    {
        auto itr = m_Binaries.find(hash);
        if (itr == m_Binaries.end())
            return false;

        shader = itr->second;
        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      シェーダPDBを検索します.
    //! 
    //! @param[in]      pdbName     シェーダPDB名.
    //! @param[out]     pdb         シェーダPDBの格納先.
    //! @retval true    検索に成功.
    //! @retval false   検索に失敗.
    //-------------------------------------------------------------------------
    bool FindPdb(const GFSDK_Aftermath_ShaderDebugName& pdbName, std::vector<uint8_t>& pdb) const
    {
        auto itr = m_Pdbs.find(pdbName);
        if (itr == m_Pdbs.end())
            return false;

        pdb = itr->second;
        return true;
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::map<GFSDK_Aftermath_ShaderBinaryHash, std::vector<uint8_t>> m_Binaries;    //!< シェーダバイナリ.
    std::map<GFSDK_Aftermath_ShaderDebugName,  std::vector<uint8_t>> m_Pdbs;        //!< シェーダPDB.

    //-------------------------------------------------------------------------
    //! @brief      シェーダバイナリを登録します.
    //-------------------------------------------------------------------------
    void AddBinary(const char* path)
    {
        std::vector<uint8_t> binary;
        if (!ReadFile(path, binary))
        { return; }

        const D3D12_SHADER_BYTECODE shader{ binary.data(), binary.size() };
        GFSDK_Aftermath_ShaderBinaryHash hash;
        auto ret = GFSDK_Aftermath_GetShaderHash(
            GFSDK_Aftermath_Version_API,
            &shader,
            &hash);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        { return; }

        m_Binaries[hash].swap(binary);
    }

    //-------------------------------------------------------------------------
    //! @brief      シェーダPDBを登録します.
    //-------------------------------------------------------------------------
    void AddPdb(const char* path, const char* baseName)
    {
        std::vector<uint8_t> binary;
        if (!ReadFile(path, binary))
        { return; }

        GFSDK_Aftermath_ShaderDebugName pdbName;
        strncpy_s(pdbName.name, path, sizeof(pdbName.name) - 1);

        m_Pdbs[pdbName].swap(binary);
    }
};

///////////////////////////////////////////////////////////////////////////////
// GpuCrashTracker class
///////////////////////////////////////////////////////////////////////////////
class GpuCrashTracker
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    typedef std::map<GFSDK_Aftermath_ShaderDebugInfoIdentifier, std::vector<uint8_t>> DebugInfoMap;

    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    GpuCrashTracker()
    : m_Initialized     (false)
    , m_Mutex           ()
    , m_DebugInfos      ()
    , m_ShaderDataBase  ()
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    ~GpuCrashTracker()
    { Term(); }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    bool Init
    (
        const char* appName,
        const char* appVersion,
        const std::vector<std::string>& shaderBinaryDirs, 
        const std::vector<std::string>& shaderPdbDirs
    )
    {
        m_AppName    = appName;
        m_AppVersion = appVersion;

        m_ShaderDataBase.Init(shaderBinaryDirs, shaderBinaryDirs);

        auto ret = GFSDK_Aftermath_EnableGpuCrashDumps(
            GFSDK_Aftermath_Version_API,
            GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_DX,
            GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks,
            GpuCrashDumpCallback,
            ShaderDebugInfoCallback,
            CrashDumpDescriptionCallback,
            ResolveMarkerCallback,
            this);

        if (GFSDK_Aftermath_SUCCEED(ret))
        {
            m_Initialized = true;
            return true;
        }

        return false;
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void Term()
    {
        if (m_Initialized)
        {
            GFSDK_Aftermath_DisableGpuCrashDumps();
            m_Initialized = false;

            m_ShaderDataBase.Term();

            m_AppName   .clear();
            m_AppVersion.clear();
            m_DebugInfos.clear();
        }
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    bool                m_Initialized = false;
    mutable std::mutex  m_Mutex;
    ShaderDataBase      m_ShaderDataBase;
    DebugInfoMap        m_DebugInfos;
    std::string         m_AppName;
    std::string         m_AppVersion;

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void WriteDump(const void* pCrashDump, uint32_t crashDumpSize)
    {
        GFSDK_Aftermath_GpuCrashDump_Decoder decoder = {};
        auto ret = GFSDK_Aftermath_GpuCrashDump_CreateDecoder(
            GFSDK_Aftermath_Version_API,
            pCrashDump,
            crashDumpSize,
            &decoder);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            return;
        }

        GFSDK_Aftermath_GpuCrashDump_BaseInfo baseInfo = {};
        ret = GFSDK_Aftermath_GpuCrashDump_GetBaseInfo(decoder, &baseInfo);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            return;
        }

        uint32_t appNameLength = 0;
        ret = GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize(
            decoder,
            GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,
            &appNameLength);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            return;
        }

        std::vector<char> appName(appNameLength, '\0');

        ret = GFSDK_Aftermath_GpuCrashDump_GetDescription(
            decoder,
            GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,
            uint32_t(appName.size()),
            appName.data());
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            return;
        }

        static int count = 0;
        const std::string baseFileName = 
            std::string(appName.data())
            + "-"
            + std::to_string(baseInfo.pid)
            + "-"
            + std::to_string(++count);

        const std::string crashDumpFileName = baseFileName + ".nv-gpudmp";
        std::ofstream dumpStream(crashDumpFileName, std::ios::out | std::ios::binary);
        if (dumpStream)
        {
            dumpStream.write(reinterpret_cast<const char*>(pCrashDump), crashDumpSize);
            dumpStream.close();
        }

        uint32_t jsonSize = 0;
        ret = GFSDK_Aftermath_GpuCrashDump_GenerateJSON(
            decoder,
            GFSDK_Aftermath_GpuCrashDumpDecoderFlags_ALL_INFO,
            GFSDK_Aftermath_GpuCrashDumpFormatterFlags_NONE,
            ShaderDebugInfoLookupCallback,
            ShaderLookupCallback,
            ShaderPdbLookupCallback,
            this,
            &jsonSize);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            return;
        }

        std::vector<char> json(jsonSize);
        ret = GFSDK_Aftermath_GpuCrashDump_GetJSON(
            decoder,
            uint32_t(json.size()),
            json.data());
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            return;
        }

        const std::string jsonFileName = crashDumpFileName + ".json";
        std::ofstream jsonStream(jsonFileName, std::ios::out | std::ios::binary);
        if (jsonStream)
        {
            jsonStream.write(json.data(), json.size() - 1);
            jsonStream.close();
        }

        ret = GFSDK_Aftermath_GpuCrashDump_DestroyDecoder(decoder);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            return;
        }
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void WriteDebugInfo
    (
        GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier,
        const void* pDebugInfo,
        uint32_t    debugInfoSize
    )
    {
        const std::string path = "shader-" + ToString(identifier) + ".nvdbg";

        std::ofstream stream(path, std::ios::out | std::ios::binary);
        if (stream)
        { stream.write(reinterpret_cast<const char*>(pDebugInfo), debugInfoSize); }
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnCrashDump(const void* pCrashDump, uint32_t crashDumpSize)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        WriteDump(pCrashDump, crashDumpSize);
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnShaderDebugInfo(const void* pDebugInfo, uint32_t debugInfoSize)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);

        GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier = {};
        auto ret = GFSDK_Aftermath_GetShaderDebugInfoIdentifier(
            GFSDK_Aftermath_Version_API,
            pDebugInfo,
            debugInfoSize,
            &identifier);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            return;
        }

        std::vector<uint8_t> binary((uint8_t*)pDebugInfo, (uint8_t*)pDebugInfo + debugInfoSize);
        m_DebugInfos[identifier].swap(binary);

        WriteDebugInfo(identifier, pDebugInfo, debugInfoSize);
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription)
    {
        addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,    m_AppName.c_str());
        addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationVersion, m_AppVersion.c_str());

        // 付加情報を定義.
        //const char* userData0 = "追加情報サンプル0";
        //const char* userData1 = "追加情報サンプル1";
        //addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 0,    userData0);
        //addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 1,    userData1);
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnResolveMarker
    (
        const void* pMarkerData,
        uint32_t    markerDataSize,
        void**      ppResolvedMarkerData,
        uint32_t*   pResolveMarkerDataSize
    )
    {
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnShaderDebugInfoLookup
    (
        const GFSDK_Aftermath_ShaderDebugInfoIdentifier&    identifier,
        PFN_GFSDK_Aftermath_SetData                         setShaderDebugInfo
    ) const
    {
        auto itr = m_DebugInfos.find(identifier);
        if (itr == m_DebugInfos.end())
            return;

        setShaderDebugInfo(itr->second.data(), uint32_t(itr->second.size()));
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnShaderLookup
    (
        const GFSDK_Aftermath_ShaderBinaryHash& shaderHash,
        PFN_GFSDK_Aftermath_SetData             setShaderBinary
    ) const
    {
        std::vector<uint8_t> binary;
        if (!m_ShaderDataBase.FindBinary(shaderHash, binary))
            return;

        setShaderBinary(binary.data(), uint32_t(binary.size()));
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnShaderPdbLookup
    (
        const GFSDK_Aftermath_ShaderDebugName&  shaderDebugName,
        PFN_GFSDK_Aftermath_SetData             setShaderBinary
    ) const
    {
        std::vector<uint8_t> binary;
        if (!m_ShaderDataBase.FindPdb(shaderDebugName, binary))
            return;

        setShaderBinary(binary.data(), uint32_t(binary.size()));
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    static void GpuCrashDumpCallback
    (
        const void* pCrashDump,
        uint32_t    crashDumpSize,
        void*       pUserData
    )
    {
        auto tracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
        if (tracker != nullptr)
        { tracker->OnCrashDump(pCrashDump, crashDumpSize); }
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    static void ShaderDebugInfoCallback
    (
        const void* pShaderDebugInfo,
        uint32_t    shaderDebugInfoSize,
        void*       pUserData
    )
    {
        auto tracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
        if (tracker != nullptr)
        { tracker->OnShaderDebugInfo(pShaderDebugInfo, shaderDebugInfoSize); }
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    static void CrashDumpDescriptionCallback
    (
        PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription  addDescription,
        void*                                           pUserData
    )
    {
        auto tracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
        if (tracker != nullptr)
        { tracker->OnDescription(addDescription); }
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    static void ResolveMarkerCallback
    (
        const void* pMakerData,
        uint32_t    markerDataSize,
        void*       pUserData,
        void**      ppResolvedMarkerData,
        uint32_t*   pResolvedMarkerDataSize
    )
    {
        auto tracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
        if (tracker != nullptr)
        { tracker->OnResolveMarker(pMakerData, markerDataSize, ppResolvedMarkerData, pResolvedMarkerDataSize); }
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    static void ShaderDebugInfoLookupCallback
    (
        const GFSDK_Aftermath_ShaderDebugInfoIdentifier*    pIdentifier,
        PFN_GFSDK_Aftermath_SetData                         setShaderDebugInfo,
        void*                                               pUserData
    )
    {
        auto tracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
        if (tracker != nullptr)
        { tracker->OnShaderDebugInfoLookup(*pIdentifier, setShaderDebugInfo); }
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    static void ShaderLookupCallback
    (
        const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash,
        PFN_GFSDK_Aftermath_SetData             setShaderBinary,
        void*                                   pUserData
    )
    {
        auto tracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
        if (tracker != nullptr)
        { tracker->OnShaderLookup(*pShaderHash, setShaderBinary); }
    }

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    static void ShaderPdbLookupCallback
    (
        const GFSDK_Aftermath_ShaderDebugName*  pShaderDebugName,
        PFN_GFSDK_Aftermath_SetData             setShaderBinary,
        void*                                   pUserData
    )
    {
        auto tracker = reinterpret_cast<GpuCrashTracker*>(pUserData);
        if (tracker != nullptr)
        { tracker->OnShaderPdbLookup(*pShaderDebugName, setShaderBinary); }
    }
};

static GpuCrashTracker s_Tracker = {};

} // namespace


//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
void InitGpuCrashTracker
(
    const char* appName,
    const char* appVersion,
    const std::vector<std::string>& shaderBinaryDirs,
    const std::vector<std::string>& shaderPdbDirs
)
{ s_Tracker.Init(appName, appVersion, shaderBinaryDirs, shaderPdbDirs); }

//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
void TermGpuCrashTracker()
{ s_Tracker.Term(); }

//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
bool InitAftermath(ID3D12Device* pDevice)
{
    uint32_t flags = GFSDK_Aftermath_FeatureFlags_Minimum;
    flags |= GFSDK_Aftermath_FeatureFlags_EnableMarkers;
    flags |= GFSDK_Aftermath_FeatureFlags_EnableResourceTracking;
    flags |= GFSDK_Aftermath_FeatureFlags_CallStackCapturing;
    flags |= GFSDK_Aftermath_FeatureFlags_GenerateShaderDebugInfo;
#if 0
    //flags |= GFSDK_Aftermath_FeatureFlags_EnableShaderErrorReporting;
#endif

    auto ret = GFSDK_Aftermath_DX12_Initialize(
        GFSDK_Aftermath_Version_API,
        flags,
        pDevice);

    return GFSDK_Aftermath_SUCCEED(ret);
}

//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
void TermAftermath()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//! @brief
//-----------------------------------------------------------------------------
void ReportAftermath()
{
    auto timeout = std::chrono::seconds(3);
    auto start   = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::milliseconds::zero();

    GFSDK_Aftermath_CrashDump_Status status = GFSDK_Aftermath_CrashDump_Status_Unknown;
    auto ret = GFSDK_Aftermath_GetCrashDumpStatus(&status);
    if (!GFSDK_Aftermath_SUCCEED(ret))
    {
        return;
    }

    while(status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed
       && status != GFSDK_Aftermath_CrashDump_Status_Finished
       && elapsed < timeout)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(50));
        auto ret = GFSDK_Aftermath_GetCrashDumpStatus(&status);
        if (!GFSDK_Aftermath_SUCCEED(ret))
            break;

        auto end = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    }

    if (status != GFSDK_Aftermath_CrashDump_Status_Finished)
    {
    }
}