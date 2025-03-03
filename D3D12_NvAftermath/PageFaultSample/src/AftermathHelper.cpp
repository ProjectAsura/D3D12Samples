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
#include <direct.h>
#include <d3d12.h>
#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>


//-----------------------------------------------------------------------------
//! @brief      大小比較演算子です.
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
//! @brief      大小比較演算子です.
//-----------------------------------------------------------------------------
inline bool operator <
(
    const GFSDK_Aftermath_ShaderBinaryHash& lhs,
    const GFSDK_Aftermath_ShaderBinaryHash& rhs
)
{ return lhs.hash < rhs.hash; }

//-----------------------------------------------------------------------------
//! @brief      大小比較演算子です.
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
//! @brief      16進数文字列を変換します.
//-----------------------------------------------------------------------------
template<typename T>
inline std::string ToHexString(T n)
{
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2 * sizeof(T)) << std::hex << n;
    return stream.str();
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
inline std::string ToString(GFSDK_Aftermath_Result result)
{ return std::string("0x") + ToHexString(static_cast<UINT>(result)); }

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
inline std::string ToString(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& identifier)
{ return ToHexString(identifier.id[0]) + "-" + ToHexString(identifier.id[1]); }

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
inline std::string ToString(const GFSDK_Aftermath_ShaderBinaryHash& hash)
{ return ToHexString(hash.hash); }

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(GFSDK_Aftermath_GraphicsApi api)
{
    switch(api)
    {
    case GFSDK_Aftermath_GraphicsApi_Unknown:
        return "Unknown";

    case GFSDK_Aftermath_GraphicsApi_D3D_10_0:
        return "Direct3D 10.0";

    case GFSDK_Aftermath_GraphicsApi_D3D_10_1:
        return "Direct3D 10.1";

    case GFSDK_Aftermath_GraphicsApi_D3D_11_0:
        return "Direct3D 11.0";

    case GFSDK_Aftermath_GraphicsApi_D3D_11_1:
        return "Direct3D 11.1";

    case GFSDK_Aftermath_GraphicsApi_D3D_11_2:
        return "Direct3D 11.2";

    case GFSDK_Aftermath_GraphicsApi_D3D_12_0:
        return "Direct3D 12.0";

    case GFSDK_Aftermath_GraphicsApi_Vulkan:
        return "Vulkan";

    default:
        return "Undefined";
    }
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(GFSDK_Aftermath_Device_Status status)
{
    switch(status)
    {
    case GFSDK_Aftermath_Device_Status_Active:
        return "Active";

    case GFSDK_Aftermath_Device_Status_Timeout:
        return "Timeout";

    case GFSDK_Aftermath_Device_Status_OutOfMemory:
        return "OutOfMemory";

    case GFSDK_Aftermath_Device_Status_PageFault:
        return "PageFault";

    case GFSDK_Aftermath_Device_Status_Stopped:
        return "Stopped";

    case GFSDK_Aftermath_Device_Status_Reset:
        return "Reset";

    case GFSDK_Aftermath_Device_Status_Unknown:
        return "Unknown";

    case GFSDK_Aftermath_Device_Status_DmaFault:
        return "DmaFault";

    case GFSDK_Aftermath_Device_Status_DeviceRemovedNoGpuFault:
        return "DeviceRemovedNoGpuFault";

    default:
        return "Undefined";
    }
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(GFSDK_Aftermath_Context_Status status)
{
    switch(status)
    {
    case GFSDK_Aftermath_Context_Status_NotStarted:
        return "NotStarted";

    case GFSDK_Aftermath_Context_Status_Executing:
        return "Executing";

    case GFSDK_Aftermath_Context_Status_Finished:
        return "Finished";

    case GFSDK_Aftermath_Context_Status_Invalid:
        return "Invalid";

    default:
        return "Undefined";
    }
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(GFSDK_Aftermath_FaultType type)
{
    switch(type)
    {
    case GFSDK_Aftermath_FaultType_Unknown:
        return "Unknown";

    case GFSDK_Aftermath_FaultType_AddressTranslationError:
        return "AddressTranslationError";

    case GFSDK_Aftermath_FaultType_IllegalAccessError:
        return "IllegalAccessError";

    default:
        return "Undefined";
    }
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(GFSDK_Aftermath_AccessType type)
{
    switch(type)
    {
    case GFSDK_Aftermath_AccessType_Unknown:
        return "Unknown";

    case GFSDK_Aftermath_AccessType_Read:
        return "Read";

    case GFSDK_Aftermath_AccessType_Write:
        return "Write";

    case GFSDK_Aftermath_AccessType_Atomic:
        return "Atomic";

    default:
        return "Undefined";
    }
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(GFSDK_Aftermath_Engine engine)
{
    switch(engine)
    {
    case GFSDK_Aftermath_Engine_Unknown:
        return "Unknown";

    case GFSDK_Aftermath_Engine_Graphics:
        return "Graphics";

    case GFSDK_Aftermath_Engine_GraphicsCompute:
        return "GraphicsCompute";

    case GFSDK_Aftermath_Engine_Display:
        return "Display";

    case GFSDK_Aftermath_Engine_CopyEngine:
        return "CopyEngine";

    case GFSDK_Aftermath_Engine_VideoDecoder:
        return "VideoDecoder";

    case GFSDK_Aftermath_Engine_VideoEncoder:
        return "VideoEncoder";

    case GFSDK_Aftermath_Engine_Other:
        return "Other";

    default:
        return "Undefined";
    }
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(GFSDK_Aftermath_Client client)
{
    switch(client)
    {
    case GFSDK_Aftermath_Client_Unknown:
        return "Unknown";

    case GFSDK_Aftermath_Client_HostInterface:
        return "HostInterface";

    case GFSDK_Aftermath_Client_FrontEnd:
        return "FrontEnd";

    case GFSDK_Aftermath_Client_PrimitiveDistributor:
        return "PrimitiveDistributor";

    case GFSDK_Aftermath_Client_GraphicsProcessingCluster:
        return "GraphicsProcessingCluster";

    case GFSDK_Aftermath_Client_PolymorphEngine:
        return "PolymorphEngine";

    case GFSDK_Aftermath_Client_RasterEngine:
        return "RasterEngine";

    case GFSDK_Aftermath_Client_Rasterizer2D:
        return "Rasterizer2D";

    case GFSDK_Aftermath_Client_RenderOutputUnit:
        return "RenderOutputUnit";

    case GFSDK_Aftermath_Client_TextureProcessingCluster:
        return "TextureProcessingCluster";

    case GFSDK_Aftermath_Client_CopyEngine:
        return "CopyEngine";

    case GFSDK_Aftermath_Client_VideoDecoder:
        return "VideoDecoder";

    case GFSDK_Aftermath_Client_VideoEncoder:
        return "VideoEncoder";

    case GFSDK_Aftermath_Client_Other:
        return "Other";

    default:
        return "Undefined";
    }
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(GFSDK_Aftermath_ResourceResidency residency)
{
    switch(residency)
    {
    case GFSDK_Aftermath_ResourceResidency_Unknown:
        return "Unknown";

    case GFSDK_Aftermath_ResourceResidency_FullyResident:
        return "FullyResident";

    case GFSDK_Aftermath_ResourceResidency_Evicted:
        return "Evicted";

    default:
        return "Undefined";
    }
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(GFSDK_Aftermath_ShaderType type)
{
    switch(type)
    {
    case GFSDK_Aftermath_ShaderType_Unknown:
        return "Unknown";

    case GFSDK_Aftermath_ShaderType_Vertex:
        return "Vertex";

    case GFSDK_Aftermath_ShaderType_Hull:
        return "Hull";

    case GFSDK_Aftermath_ShaderType_Domain:
        return "Domain";

    case GFSDK_Aftermath_ShaderType_Geometry:
        return "Geometry";

    case GFSDK_Aftermath_ShaderType_Pixel:
        return "Pixel";

    case GFSDK_Aftermath_ShaderType_Compute:
        return "Comptue";

    case GFSDK_Aftermath_ShaderType_RayTracing_RayGeneration:
        return "RayTracing Ray Generation";

    case GFSDK_Aftermath_ShaderType_RayTracing_Miss:
        return "RayTracing Miss";

    case GFSDK_Aftermath_ShaderType_RayTracing_Intersection:
        return "RayTracing Intersection";

    case GFSDK_Aftermath_ShaderType_RayTracing_AnyHit:
        return "RayTracing AnyHit";

    case GFSDK_Aftermath_ShaderType_RayTracing_ClosestHit:
        return "RayTracing ClosestHit";

    case GFSDK_Aftermath_ShaderType_RayTracing_Callable:
        return "RayTracing Callable";

    case GFSDK_Aftermath_ShaderType_RayTracing_Internal:
        return "RayTracing Internal";

    case GFSDK_Aftermath_ShaderType_Mesh:
        return "Mesh";

    case GFSDK_Aftermath_ShaderType_Task:
        return "Task";

    default:
        return "Undefined";
    }
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(GFSDK_Aftermath_Context_Type type)
{
    switch(type)
    {
    case GFSDK_Aftermath_Context_Type_Invalid:
        return "Invalid";

    case GFSDK_Aftermath_Context_Type_Immediate:
        return "Immedidate";

    case GFSDK_Aftermath_Context_Type_CommandList:
        return "CommandList";

    case GFSDK_Aftermath_Context_Type_Bundle:
        return "Bundle";

    case GFSDK_Aftermath_Context_Type_CommandQueue:
        return "CommandQueue";

    default:
        return "Undefined";
    }
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(GFSDK_Aftermath_EventMarkerDataOwnership ownership)
{
    switch(ownership)
    {
    case GFSDK_Aftermath_EventMarkerDataOwnership_User:
        return "User";

    case GFSDK_Aftermath_EventMarkerDataOwnership_Decoder:
        return "Decoder";

    default:
        return "Undefined";
    }
}

//-----------------------------------------------------------------------------
//! @brief      文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(DXGI_FORMAT format)
{
    switch(format)
    {
        case DXGI_FORMAT_UNKNOWN	                : return "UNKNOWN";
        case DXGI_FORMAT_R32G32B32A32_TYPELESS      : return "R32G32B32A32_TYPELESS";
        case DXGI_FORMAT_R32G32B32A32_FLOAT         : return "R32G32B32A32_FLOAT";
        case DXGI_FORMAT_R32G32B32A32_UINT          : return "R32G32B32A32_UINT";
        case DXGI_FORMAT_R32G32B32A32_SINT          : return "R32G32B32A32_SINT";
        case DXGI_FORMAT_R32G32B32_TYPELESS         : return "R32G32B32_TYPELESS";
        case DXGI_FORMAT_R32G32B32_FLOAT            : return "R32G32B32_FLOAT";
        case DXGI_FORMAT_R32G32B32_UINT             : return "R32G32B32_UINT";
        case DXGI_FORMAT_R32G32B32_SINT             : return "R32G32B32_SINT";
        case DXGI_FORMAT_R16G16B16A16_TYPELESS      : return "R16G16B16A16_TYPELESS";
        case DXGI_FORMAT_R16G16B16A16_FLOAT         : return "R16G16B16A16_FLOAT";
        case DXGI_FORMAT_R16G16B16A16_UNORM         : return "R16G16B16A16_UNORM";
        case DXGI_FORMAT_R16G16B16A16_UINT          : return "R16G16B16A16_UINT";
        case DXGI_FORMAT_R16G16B16A16_SNORM         : return "R16G16B16A16_SNORM";
        case DXGI_FORMAT_R16G16B16A16_SINT          : return "R16G16B16A16_SINT";
        case DXGI_FORMAT_R32G32_TYPELESS            : return "R32G32_TYPELESS";
        case DXGI_FORMAT_R32G32_FLOAT               : return "R32G32_FLOAT";
        case DXGI_FORMAT_R32G32_UINT                : return "R32G32_UINT";
        case DXGI_FORMAT_R32G32_SINT                : return "R32G32_SINT";
        case DXGI_FORMAT_R32G8X24_TYPELESS          : return "R32G8X24_TYPELESS";
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT       : return "D32_FLOAT_S8X24_UINT";
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS   : return "R32_FLOAT_X8X24_TYPELESS";
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT    : return "X32_TYPELESS_G8X24_UINT";
        case DXGI_FORMAT_R10G10B10A2_TYPELESS       : return "R10G10B10A2_TYPELESS";
        case DXGI_FORMAT_R10G10B10A2_UNORM          : return "R10G10B10A2_UNORM";
        case DXGI_FORMAT_R10G10B10A2_UINT           : return "R10G10B10A2_UINT";
        case DXGI_FORMAT_R11G11B10_FLOAT            : return "R11G11B10_FLOAT";
        case DXGI_FORMAT_R8G8B8A8_TYPELESS          : return "R8G8B8A8_TYPELESS";
        case DXGI_FORMAT_R8G8B8A8_UNORM             : return "R8G8B8A8_UNORM";
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB        : return "R8G8B8A8_UNORM_SRGB";
        case DXGI_FORMAT_R8G8B8A8_UINT              : return "R8G8B8A8_UINT";
        case DXGI_FORMAT_R8G8B8A8_SNORM             : return "R8G8B8A8_SNORM";
        case DXGI_FORMAT_R8G8B8A8_SINT              : return "R8G8B8A8_SINT";
        case DXGI_FORMAT_R16G16_TYPELESS            : return "R16G16_TYPELESS";
        case DXGI_FORMAT_R16G16_FLOAT               : return "R16G16_FLOAT";
        case DXGI_FORMAT_R16G16_UNORM               : return "R16G16_UNORM";
        case DXGI_FORMAT_R16G16_UINT                : return "R16G16_UINT";
        case DXGI_FORMAT_R16G16_SNORM               : return "R16G16_SNORM";
        case DXGI_FORMAT_R16G16_SINT                : return "R16G16_SINT";
        case DXGI_FORMAT_R32_TYPELESS               : return "R32_TYPELESS";
        case DXGI_FORMAT_D32_FLOAT                  : return "D32_FLOAT";
        case DXGI_FORMAT_R32_FLOAT                  : return "R32_FLOAT";
        case DXGI_FORMAT_R32_UINT                   : return "R32_UINT";
        case DXGI_FORMAT_R32_SINT                   : return "R32_SINT";
        case DXGI_FORMAT_R24G8_TYPELESS             : return "R24G8_TYPELESS";
        case DXGI_FORMAT_D24_UNORM_S8_UINT          : return "D24_UNORM_S8_UINT";
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS      : return "R24_UNORM_X8_TYPELESS";
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT       : return "X24_TYPELESS_G8_UINT";
        case DXGI_FORMAT_R8G8_TYPELESS              : return "R8G8_TYPELESS";
        case DXGI_FORMAT_R8G8_UNORM                 : return "R8G8_UNORM";
        case DXGI_FORMAT_R8G8_UINT                  : return "R8G8_UINT";
        case DXGI_FORMAT_R8G8_SNORM                 : return "R8G8_SNORM";
        case DXGI_FORMAT_R8G8_SINT                  : return "R8G8_SINT";
        case DXGI_FORMAT_R16_TYPELESS               : return "R16_TYPELESS";
        case DXGI_FORMAT_R16_FLOAT                  : return "R16_FLOAT";
        case DXGI_FORMAT_D16_UNORM                  : return "D16_UNORM";
        case DXGI_FORMAT_R16_UNORM                  : return "R16_UNORM";
        case DXGI_FORMAT_R16_UINT                   : return "R16_UINT";
        case DXGI_FORMAT_R16_SNORM                  : return "R16_SNORM";
        case DXGI_FORMAT_R16_SINT                   : return "R16_SINT";
        case DXGI_FORMAT_R8_TYPELESS                : return "R8_TYPELESS";
        case DXGI_FORMAT_R8_UNORM                   : return "R8_UNORM";
        case DXGI_FORMAT_R8_UINT                    : return "R8_UINT";
        case DXGI_FORMAT_R8_SNORM                   : return "R8_SNORM";
        case DXGI_FORMAT_R8_SINT                    : return "R8_SINT";
        case DXGI_FORMAT_A8_UNORM                   : return "A8_UNORM";
        case DXGI_FORMAT_R1_UNORM                   : return "R1_UNORM";
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP         : return "R9G9B9E5_SHAREDEXP";
        case DXGI_FORMAT_R8G8_B8G8_UNORM            : return "R8G8_B8G8_UNORM";
        case DXGI_FORMAT_G8R8_G8B8_UNORM            : return "G8R8_G8B8_UNORM";
        case DXGI_FORMAT_BC1_TYPELESS               : return "BC1_TYPELESS";
        case DXGI_FORMAT_BC1_UNORM                  : return "BC1_UNORM";
        case DXGI_FORMAT_BC1_UNORM_SRGB             : return "BC1_UNORM_SRGB";
        case DXGI_FORMAT_BC2_TYPELESS               : return "BC2_TYPELESS";
        case DXGI_FORMAT_BC2_UNORM                  : return "BC2_UNORM";
        case DXGI_FORMAT_BC2_UNORM_SRGB             : return "BC2_UNORM_SRGB";
        case DXGI_FORMAT_BC3_TYPELESS               : return "BC3_TYPELESS";
        case DXGI_FORMAT_BC3_UNORM                  : return "BC3_UNORM";
        case DXGI_FORMAT_BC3_UNORM_SRGB             : return "BC3_UNORM_SRGB";
        case DXGI_FORMAT_BC4_TYPELESS               : return "BC4_TYPELESS";
        case DXGI_FORMAT_BC4_UNORM                  : return "BC4_UNORM";
        case DXGI_FORMAT_BC4_SNORM                  : return "BC4_SNORM";
        case DXGI_FORMAT_BC5_TYPELESS               : return "BC5_TYPELESS";
        case DXGI_FORMAT_BC5_UNORM                  : return "BC5_UNORM";
        case DXGI_FORMAT_BC5_SNORM                  : return "BC5_SNORM";
        case DXGI_FORMAT_B5G6R5_UNORM               : return "B5G6R5_UNORM";
        case DXGI_FORMAT_B5G5R5A1_UNORM             : return "B5G5R5A1_UNORM";
        case DXGI_FORMAT_B8G8R8A8_UNORM             : return "B8G8R8A8_UNORM";
        case DXGI_FORMAT_B8G8R8X8_UNORM             : return "B8G8R8X8_UNORM";
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM : return "R10G10B10_XR_BIAS_A2_UNORM";
        case DXGI_FORMAT_B8G8R8A8_TYPELESS          : return "B8G8R8A8_TYPELESS";
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB        : return "B8G8R8A8_UNORM_SRGB";
        case DXGI_FORMAT_B8G8R8X8_TYPELESS          : return "B8G8R8X8_TYPELESS";
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB        : return "B8G8R8X8_UNORM_SRGB";
        case DXGI_FORMAT_BC6H_TYPELESS              : return "BC6H_TYPELESS";
        case DXGI_FORMAT_BC6H_UF16                  : return "BC6H_UF16";
        case DXGI_FORMAT_BC6H_SF16                  : return "BC6H_SF16";
        case DXGI_FORMAT_BC7_TYPELESS               : return "BC7_TYPELESS";
        case DXGI_FORMAT_BC7_UNORM                  : return "BC7_UNORM";
        case DXGI_FORMAT_BC7_UNORM_SRGB             : return "BC7_UNORM_SRGB";
        case DXGI_FORMAT_AYUV                       : return "AYUV";
        case DXGI_FORMAT_Y410                       : return "Y410";
        case DXGI_FORMAT_Y416                       : return "Y416";
        case DXGI_FORMAT_NV12                       : return "NV12";
        case DXGI_FORMAT_P010                       : return "P010";
        case DXGI_FORMAT_P016                       : return "P016";
        case DXGI_FORMAT_420_OPAQUE                 : return "420_OPAQUE";
        case DXGI_FORMAT_YUY2                       : return "YUY2";
        case DXGI_FORMAT_Y210                       : return "Y210";
        case DXGI_FORMAT_Y216                       : return "Y216";
        case DXGI_FORMAT_NV11                       : return "NV11";
        case DXGI_FORMAT_AI44                       : return "AI44";
        case DXGI_FORMAT_IA44                       : return "IA44";
        case DXGI_FORMAT_P8                         : return "P8";
        case DXGI_FORMAT_A8P8                       : return "A8P8";
        case DXGI_FORMAT_B4G4R4A4_UNORM             : return "B4G4R4A4_UNORM";

        case DXGI_FORMAT_P208                       : return "P208";
        case DXGI_FORMAT_V208                       : return "V208";
        case DXGI_FORMAT_V408                       : return "V408";


        case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE         : return "SAMPLER_FEEDBACK_MIN_MIP_OPAQUE";
        case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE : return "SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE";

        case DXGI_FORMAT_A4B4G4R4_UNORM             : return "A4B4G4R4_UNORM";

        case DXGI_FORMAT_FORCE_UINT                 : return "FORCE_UINT";

        default:
            return "Undefined";
    }
}

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

//-----------------------------------------------------------------------------
//! @brief      フルパスに変換します.
//-----------------------------------------------------------------------------
std::string ToFullPath(const char* path)
{
    char fullPath[MAX_PATH];
    GetFullPathNameA(path, MAX_PATH, fullPath, nullptr);
    return std::string(fullPath);
}

std::string ToFullPath(const std::string& path)
{
    char fullPath[MAX_PATH];
    GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, nullptr);
    return std::string(fullPath);
}

//-----------------------------------------------------------------------------
//! @brief      ファイルパスからディレクトリ名を削除します.
//-----------------------------------------------------------------------------
std::string RemoveDirectoryPathA(const char* filePath)
{
    std::string path = filePath;
    auto idx = path.find_last_of("\\");
    if ( idx != std::string::npos )
    {
        auto result = path.substr( idx + 1 );
        return result;
    }

    idx = path.find_last_of("/");
    if ( idx != std::string::npos )
    {
        auto result = path.substr( idx + 1 );
        return result;
    }

    return path;
}

///////////////////////////////////////////////////////////////////////////////
// BinaryInfo structure
///////////////////////////////////////////////////////////////////////////////
struct BinaryInfo
{
    std::vector<uint8_t>    Binary;
    std::string             Path;

    void swap(BinaryInfo& pair)
    {
        std::swap(Binary, pair.Binary);
        std::swap(Path, pair.Path);
    }
};

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
                // 相対パスが混じっていたり，区切り文字がごちゃごちゃだと検索に失敗するため，フルパスに変換する.
                const auto path = ToFullPath(shaderBinaryDirs[i] + "/" + findData.cFileName);
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
                    // 相対パスが混じっていたり，区切り文字がごちゃごちゃだと検索に失敗するため，フルパスに変換する.
                    const auto path = ToFullPath(shaderPdbDirs[i] + "/" + findData.cFileName);
                    AddPdb(path.c_str());
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
    bool FindBinary(const GFSDK_Aftermath_ShaderBinaryHash& hash, BinaryInfo& shader) const
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
    bool FindPdb(const GFSDK_Aftermath_ShaderDebugName& pdbName, BinaryInfo& pdb) const
    {
        // 相対パスが混じっていたり，区切り文字がごちゃごちゃだと検索に失敗するため，フルパスに変換する.
        GFSDK_Aftermath_ShaderDebugName fullName = {};
        {
            auto fullPath = ToFullPath(pdbName.name);
            strncpy_s(fullName.name, fullPath.c_str(), sizeof(fullName.name) - 1);
        }

        auto itr = m_Pdbs.find(fullName);
        if (itr == m_Pdbs.end())
            return false;

        pdb = itr->second;
        return true;
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::map<GFSDK_Aftermath_ShaderBinaryHash, BinaryInfo> m_Binaries;    //!< シェーダバイナリ.
    std::map<GFSDK_Aftermath_ShaderDebugName,  BinaryInfo> m_Pdbs;        //!< シェーダPDB.

    //-------------------------------------------------------------------------
    //! @brief      シェーダバイナリを登録します.
    //-------------------------------------------------------------------------
    void AddBinary(const char* path)
    {
        BinaryInfo info;
        info.Path = path;
        if (!ReadFile(path, info.Binary))
        { return; }

        const D3D12_SHADER_BYTECODE shader{ info.Binary.data(), info.Binary.size() };
        GFSDK_Aftermath_ShaderBinaryHash hash;
        auto ret = GFSDK_Aftermath_GetShaderHash(
            GFSDK_Aftermath_Version_API,
            &shader,
            &hash);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        { return; }

        m_Binaries[hash].swap(info);
    }

    //-------------------------------------------------------------------------
    //! @brief      シェーダPDBを登録します.
    //-------------------------------------------------------------------------
    void AddPdb(const char* path)
    {
        BinaryInfo pair;
        pair.Path = path;
        if (!ReadFile(path, pair.Binary))
        { return; }

        GFSDK_Aftermath_ShaderDebugName pdbName;
        strncpy_s(pdbName.name, path, sizeof(pdbName.name) - 1);

        m_Pdbs[pdbName].swap(pair);
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
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    GpuCrashTracker()
    : m_Initialized     (false)
    , m_Mutex           ()
    , m_DebugInfos      ()
    , m_ShaderDataBase  ()
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~GpuCrashTracker()
    { Term(); }

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //! 
    //! @param[in]      appName             アプリケーション名.
    //! @param[in]      appVersion          アプリケーションバージョン.
    //! @param[in]      outputDir           ダンプ出力ディレクトリ.
    //! @param[in]      shaderBinaryDirs    シェーダバイナリディレクトリのリスト.
    //! @param[in]      shaderPdbDirs       シェーダPDBディレクトリのリスト.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init
    (
        const char* appName,
        const char* appVersion,
        const char* outputDir,
        const std::vector<std::string>& shaderBinaryDirs,
        const std::vector<std::string>& shaderPdbDirs
    )
    {
        m_AppName    = appName;
        m_AppVersion = appVersion;
        m_OutputDir  = outputDir;

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
    //! @brief      終了処理を行います.
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
            m_OutputDir .clear();
        }
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    bool                m_Initialized = false;      //!< 初期化済みフラグ.
    mutable std::mutex  m_Mutex;                    //!< 同期オブジェクト.
    ShaderDataBase      m_ShaderDataBase;           //!< シェーダデータベース.
    DebugInfoMap        m_DebugInfos;               //!< デバッグ情報辞書.
    std::string         m_AppName;                  //!< アプリ名.
    std::string         m_AppVersion;               //!< アプリバージョン.
    std::string         m_OutputDir;                //!< 出力ディレクトリ.

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      GPUクラッシュダンプ情報を書き込みます.
    //! 
    //! @param[in]      pCrashDump      クラッシュダンプバイナリ.
    //! @param[in]      crashDumpSize   クラッシュダンプサイズ.
    //-------------------------------------------------------------------------
    void WriteDump(const void* pCrashDump, uint32_t crashDumpSize)
    {
        // ディレクトリ作成.
        {
            _mkdir(m_OutputDir.c_str());

            auto shader_bin = m_OutputDir + "/shader_bin";
            _mkdir(shader_bin.c_str());

            auto shader_pdb = m_OutputDir + "/shader_pdb";
            _mkdir(shader_pdb.c_str());

            auto nvdbg = m_OutputDir + "/nvdbg";
            _mkdir(nvdbg.c_str());
        }

        GFSDK_Aftermath_GpuCrashDump_Decoder decoder = {};
        auto ret = GFSDK_Aftermath_GpuCrashDump_CreateDecoder(
            GFSDK_Aftermath_Version_API,
            pCrashDump,
            crashDumpSize,
            &decoder);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            OutputLog("GFSDK_Aftermath_GpuCrashDump_CreateDecoder() Failed. errcode = 0x%x", ret);
            return;
        }

        GFSDK_Aftermath_GpuCrashDump_BaseInfo baseInfo = {};
        ret = GFSDK_Aftermath_GpuCrashDump_GetBaseInfo(decoder, &baseInfo);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            OutputLog("GFSDK_Aftermath_GpuCrashDump_GetBaseInfo() Failed. errcode = 0x%x", ret);
            return;
        }

        uint32_t appNameLength = 0;
        ret = GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize(
            decoder,
            GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,
            &appNameLength);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            OutputLog("GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize() Failed. errcode = 0x%x", ret);
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
            OutputLog("GFSDK_Aftermath_GpuCrashDump_GetDescription() Failed. errcode = 0x%x", ret);
            return;
        }

        static int count = 0;
        const std::string baseFileName = m_OutputDir
            + "/"
            + std::string(appName.data())
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
            OutputLog("GFSDK_Aftermath_GpuCrashDump_GenerateJSON() Failed. errcode = 0x%x", ret);
            return;
        }

        std::vector<char> json(jsonSize);
        ret = GFSDK_Aftermath_GpuCrashDump_GetJSON(
            decoder,
            uint32_t(json.size()),
            json.data());
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            OutputLog("GFSDK_Aftermath_GpuCrashDump_GetJSON() Failed. errcode = 0x%x", ret);
            return;
        }

        const std::string jsonFileName = crashDumpFileName + ".json";
        std::ofstream jsonStream(jsonFileName, std::ios::out | std::ios::binary);
        if (jsonStream)
        {
            jsonStream.write(json.data(), json.size() - 1);
            jsonStream.close();
        }

        CustomOutput(decoder);

        ret = GFSDK_Aftermath_GpuCrashDump_DestroyDecoder(decoder);
        if (!GFSDK_Aftermath_SUCCEED(ret))
        {
            OutputLog("GFSDK_Aftermath_GpuCrashDump_DestoryDecoder() Failed. errcode = 0x%x", ret);
            return;
        }

        OutputLog("CrashDump Output Succeeded!");
    }

    //-------------------------------------------------------------------------
    //! @brief      NVIDIAシェーダデバッグ情報ファイルを出力します.
    //-------------------------------------------------------------------------
    void WriteDebugInfo
    (
        GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier,
        const void* pDebugInfo,
        uint32_t    debugInfoSize
    )
    {
        const std::string path = m_OutputDir + "/nvdbg/" + "shader-" + ToString(identifier) + ".nvdbg";

        std::ofstream stream(path, std::ios::out | std::ios::binary);
        if (stream)
        {
            stream.write(reinterpret_cast<const char*>(pDebugInfo), debugInfoSize);
            stream.close();
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      クラッシュダンプの処理です.
    //-------------------------------------------------------------------------
    void OnCrashDump(const void* pCrashDump, uint32_t crashDumpSize)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        WriteDump(pCrashDump, crashDumpSize);
    }

    //-------------------------------------------------------------------------
    //! @brief      シェーダデバッグ情報の処理です.
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
            OutputLog("GFSDK_Aftermath_GetShaderDebugInfoIndentifier() Failed. errcode = 0x%x", ret);
            return;
        }

        std::vector<uint8_t> binary((uint8_t*)pDebugInfo, (uint8_t*)pDebugInfo + debugInfoSize);
        m_DebugInfos[identifier].swap(binary);

        WriteDebugInfo(identifier, pDebugInfo, debugInfoSize);
    }

    //-------------------------------------------------------------------------
    //! @brief      概要の処理です.
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
    //! @brief      マーカーの解決処理です.
    //-------------------------------------------------------------------------
    void OnResolveMarker
    (
        const void* pMarkerData,
        uint32_t    markerDataSize,
        void**      ppResolvedMarkerData,
        uint32_t*   pResolveMarkerDataSize
    )
    {
        // TODO : Implementation.
    }

    //-------------------------------------------------------------------------
    //! @brief      シェーダデバッグ情報の参照処理です.
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
    //! @brief      シェーダの参照処理です.
    //-------------------------------------------------------------------------
    void OnShaderLookup
    (
        const GFSDK_Aftermath_ShaderBinaryHash& shaderHash,
        PFN_GFSDK_Aftermath_SetData             setShaderBinary
    ) const
    {
        BinaryInfo info;
        if (!m_ShaderDataBase.FindBinary(shaderHash, info))
            return;

        setShaderBinary(info.Binary.data(), uint32_t(info.Binary.size()));
    }

    //-------------------------------------------------------------------------
    //! @brief      シェーダPDBの参照処理です.
    //-------------------------------------------------------------------------
    void OnShaderPdbLookup
    (
        const GFSDK_Aftermath_ShaderDebugName&  shaderDebugName,
        PFN_GFSDK_Aftermath_SetData             setShaderBinary
    ) const
    {
        BinaryInfo info;
        if (!m_ShaderDataBase.FindPdb(shaderDebugName, info))
            return;

        setShaderBinary(info.Binary.data(), uint32_t(info.Binary.size()));
    }

    //-------------------------------------------------------------------------
    //! @brief      GPUクラッシュコールバック関数です.
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
    //! @brief      シェーダデバッグ情報コールバック関数です.
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
    //! @brief      クラッシュダンプ概要コールバック関数です.
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
    //! @brief      マーカー解決コールバック関数です.
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
    //! @brief      シェーダデバッグ情報参照コールバック関数.
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
    //! @brief      シェーダ参照コールバック関数です.
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
    //! @brief      シェーダPBD参照コールバック関数です.
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

    //-------------------------------------------------------------------------
    //! @brief      カスタムのログ出力を行います.
    //-------------------------------------------------------------------------
    void CustomOutput(const GFSDK_Aftermath_GpuCrashDump_Decoder& decoder)
    {
        // 基本情報.
        {
            GFSDK_Aftermath_GpuCrashDump_BaseInfo baseInfo;
            auto ret = GFSDK_Aftermath_GpuCrashDump_GetBaseInfo(decoder, &baseInfo);
            if (GFSDK_Aftermath_SUCCEED(ret))
            {
                OutputLog("------------------------------");
                OutputLog("    Basic Info");
                OutputLog("------------------------------");
                OutputLog("Application Name     : %s", baseInfo.applicationName);
                OutputLog("Creation Date        : %s", baseInfo.creationDate);
                OutputLog("Createion Tick Count : %u", baseInfo.creationTickCount);
                OutputLog("Process ID           : %u", baseInfo.pid);
                OutputLog("Graphics API         : %s", ToString(baseInfo.graphicsApi));
                OutputLog("\n");
            }
        }

        // デバイス情報.
        {
            GFSDK_Aftermath_GpuCrashDump_DeviceInfo deviceInfo;
            auto ret = GFSDK_Aftermath_GpuCrashDump_GetDeviceInfo(decoder, &deviceInfo);
            if (GFSDK_Aftermath_SUCCEED(ret))
            {
                OutputLog("------------------------------");
                OutputLog("    Device Info");
                OutputLog("------------------------------");
                OutputLog("Status        : %s", ToString(deviceInfo.status));
                OutputLog("Adapter Reset : %s", deviceInfo.adapterReset ? "Yes" : "No");
                OutputLog("Engine Reset  : %s", deviceInfo.engineReset  ? "Yes" : "No");
                OutputLog("\n");
            }
        }

        // システム情報.
        {
            GFSDK_Aftermath_GpuCrashDump_SystemInfo systemInfo;
            auto ret = GFSDK_Aftermath_GpuCrashDump_GetSystemInfo(decoder, &systemInfo);
            if (GFSDK_Aftermath_SUCCEED(ret))
            {
                OutputLog("------------------------------");
                OutputLog("    System Info");
                OutputLog("------------------------------");
                OutputLog("OS Version             : %s", systemInfo.osVersion);
                OutputLog("Display Driver Version : %u.%u", systemInfo.displayDriver.major, systemInfo.displayDriver.minor);
                OutputLog("\n");
            }
        }

        // GPU情報.
        {
            uint32_t count = 0;
            auto ret = GFSDK_Aftermath_GpuCrashDump_GetGpuInfoCount(decoder, &count);
            if (GFSDK_Aftermath_SUCCEED(ret))
            {
                std::vector<GFSDK_Aftermath_GpuCrashDump_GpuInfo> gpuInfos;
                gpuInfos.resize(count);

                OutputLog("------------------------------");
                OutputLog("    GPU Info");
                OutputLog("------------------------------");
                OutputLog("Count : %u\n", count);

                ret = GFSDK_Aftermath_GpuCrashDump_GetGpuInfo(
                    decoder, count, gpuInfos.data());
                if (GFSDK_Aftermath_SUCCEED(ret))
                {
                    for(size_t i=0; i<gpuInfos.size(); ++i)
                    {
                        const auto& info = gpuInfos[i];
                        OutputLog("GPU [%zu]", i);
                        OutputLog("  Adapter Name    : %s", info.adapterName);
                        OutputLog("  Generation Name : %s", info.generationName);
                        OutputLog("  Adapter LUID    : %llu\n", info.adapterLUID);
                    }
                }
                OutputLog("\n");
            }
        }

        // ページフォルト情報.
        {
            GFSDK_Aftermath_GpuCrashDump_PageFaultInfo pageFaultInfo;
            auto ret = GFSDK_Aftermath_GpuCrashDump_GetPageFaultInfo(decoder, &pageFaultInfo);
            if (GFSDK_Aftermath_SUCCEED(ret))
            {
                OutputLog("------------------------------");
                OutputLog("    Page Fault Info");
                OutputLog("------------------------------");
                OutputLog("Faulting GPU VA : 0x%lx", pageFaultInfo.faultingGpuVA);
                OutputLog("Fault Type      : %s", ToString(pageFaultInfo.faultType));
                OutputLog("Accesss Type    : %s", ToString(pageFaultInfo.accessType));
                OutputLog("Engine          : %s", ToString(pageFaultInfo.engine));
                OutputLog("Client          : %s", ToString(pageFaultInfo.client));
                OutputLog("Resource Count  : %u\n", pageFaultInfo.resourceInfoCount);

                std::vector<GFSDK_Aftermath_GpuCrashDump_ResourceInfo> resourceInfos;
                resourceInfos.resize(pageFaultInfo.resourceInfoCount);

                ret = GFSDK_Aftermath_GpuCrashDump_GetPageFaultResourceInfo(decoder, pageFaultInfo.resourceInfoCount, resourceInfos.data());
                if (GFSDK_Aftermath_SUCCEED(ret))
                {
                    for(size_t i=0; i<resourceInfos.size(); ++i)
                    {
                        const auto& info = resourceInfos[i];
                        OutputLog("Resource Info [%zu]", i);
                        OutputLog("  GPU Virtual Address      : 0x%lx", info.gpuVa);
                        OutputLog("  Size                     : %llu", info.size);
                        OutputLog("  Width                    : %u", info.width);
                        OutputLog("  Height                   : %u", info.height);
                        OutputLog("  Depth                    : %u", info.depth);
                        OutputLog("  Mip Levels               : %u", info.mipLevels);
                        OutputLog("  Format                   : %s", ToString(DXGI_FORMAT(info.format)));
                        OutputLog("  API Resource             : 0x%lx", info.apiResource);
                        OutputLog("  Debug Name               : %s", info.debugName);
                        OutputLog("  Buffer Heap              : %s", info.bIsBufferHeap ? "Yes" : "No");
                        OutputLog("  Static Texture Heap      : %s", info.bIsStaticTextureHeap ? "Yes" : "No");
                        OutputLog("  RTV or DSV Heap          : %s", info.bIsRenderTargetOrDepthStencilViewHeap ? "Yes" : "No");
                        OutputLog("  Placed Resource          : %s", info.bPlacedResource ? "Yes" : "No");
                        OutputLog("  Destroyed                : %s", info.bWasDestroyed ? "Yes" : "No");
                        OutputLog("  Residency                : %s", ToString(info.residency));
                        OutputLog("  Create Destory TickCount : %u\n", info.createDestroyTickCount);
                    }
                }
                OutputLog("\n");
            }
        }

        // アクティブシェーダ情報.
        {
            uint32_t count = 0;
            auto ret = GFSDK_Aftermath_GpuCrashDump_GetActiveShadersInfoCount(decoder, &count);
            if (GFSDK_Aftermath_SUCCEED(ret))
            {
                OutputLog("------------------------------");
                OutputLog("    Active Shader Info");
                OutputLog("------------------------------");
                OutputLog("Count : %u\n", count);

                std::vector<GFSDK_Aftermath_GpuCrashDump_ShaderInfo> shaderInfos;
                shaderInfos.resize(count);

                ret = GFSDK_Aftermath_GpuCrashDump_GetActiveShadersInfo(decoder, count, shaderInfos.data());
                if (GFSDK_Aftermath_SUCCEED(ret))
                {
                    for(size_t i=0; i<shaderInfos.size(); ++i)
                    {
                        const auto& info = shaderInfos[i];
                        OutputLog("Shader [%zu]", i);
                        OutputLog("  Hash        : %lx", info.shaderHash);
                        OutputLog("  Instance    : %llu", info.shaderInstance);
                        OutputLog("  Internal    : %s", info.isInternal ? "Yes" : "No");
                        OutputLog("  Type        : %s", ToString(info.shaderType));

                        GFSDK_Aftermath_ShaderBinaryHash binaryHash;
                        ret = GFSDK_Aftermath_GetShaderHashForShaderInfo(decoder, &info, &binaryHash);
                        if (!GFSDK_Aftermath_SUCCEED(ret))
                            continue;

                        BinaryInfo shaderBinInfo;
                        if (!m_ShaderDataBase.FindBinary(binaryHash, shaderBinInfo))
                            continue;

                        OutputLog("  Binary Path : %s", shaderBinInfo.Path.c_str());

                        // シェーダバイナリを出力.
                        {
                            const std::string path = m_OutputDir + "/shader_bin/shader-" + std::to_string(binaryHash.hash) + ".bin";
                            std::ofstream stream(path, std::ios::out | std::ios::binary);
                            if (stream)
                            {
                                stream.write(reinterpret_cast<char*>(shaderBinInfo.Binary.data()), shaderBinInfo.Binary.size());
                                stream.close();
                            }
                        }

                        D3D12_SHADER_BYTECODE shader = {shaderBinInfo.Binary.data(), shaderBinInfo.Binary.size()};
                        GFSDK_Aftermath_ShaderDebugName debugName;
                        ret = GFSDK_Aftermath_GetShaderDebugName(GFSDK_Aftermath_Version_API, &shader, &debugName);
                        if (!GFSDK_Aftermath_SUCCEED(ret))
                            continue;

                        BinaryInfo pdbInfo;
                        if (!m_ShaderDataBase.FindPdb(debugName, pdbInfo))
                            continue;

                        OutputLog("  PDB Path    : %s", pdbInfo.Path.c_str());

                        // シェーダPDBを出力.
                        {
                            const std::string path = m_OutputDir + "/shader_pdb/shader-" + std::to_string(binaryHash.hash) + ".pdb";
                            std::ofstream stream(path, std::ios::out | std::ios::binary);
                            if (stream)
                            {
                                stream.write(reinterpret_cast<char*>(pdbInfo.Binary.data()), pdbInfo.Binary.size());
                                stream.close();
                            }
                        }
                    }
                }

                OutputLog("\n");
            }
        }

        // イベントマーカー情報.
        {
            uint32_t count = 0;
            auto ret = GFSDK_Aftermath_GpuCrashDump_GetEventMarkersInfoCount(decoder, &count);
            if (GFSDK_Aftermath_SUCCEED(ret))
            {
                OutputLog("------------------------------");
                OutputLog("    EventMarker Info");
                OutputLog("------------------------------");
                OutputLog("Count : %u\n", count);

                std::vector<GFSDK_Aftermath_GpuCrashDump_EventMarkerInfo> markerInfos;
                markerInfos.resize(count);

                ret = GFSDK_Aftermath_GpuCrashDump_GetEventMarkersInfo(decoder, count, markerInfos.data());
                if (GFSDK_Aftermath_SUCCEED(ret))
                {
                    for(size_t i=0; i<markerInfos.size(); ++i)
                    {
                        const auto& info = markerInfos[i];

                        char* name = (char*)malloc(sizeof(char) * info.markerDataSize + 1);
                        strncpy_s(name, info.markerDataSize + 1, reinterpret_cast<const char*>(info.markerData), _TRUNCATE);

                        OutputLog("EventMarker [%zu]", i);
                        OutputLog("  Context ID     : %llu", info.contextId);
                        OutputLog("  Context Status : %s", ToString(info.contextStatus));
                        OutputLog("  Context Type   : %s", ToString(info.contextType));
                        OutputLog("  Ownership      : %s", ToString(info.markerDataOwnership));
                        OutputLog("  Data           : %s", name);
                        OutputLog("  Data Size      : %u", info.markerDataSize);

                        free(name);
                    }
                }

                OutputLog("\n");
            }
        }
    }
};

// GPUクラッシュトラッカーです.
static GpuCrashTracker s_Tracker = {};

} // namespace


//-----------------------------------------------------------------------------
//! @brief      GPUクラッシュトラッカーの初期化処理です.
//-----------------------------------------------------------------------------
void InitGpuCrashTracker
(
    const char* appName,
    const char* appVersion,
    const char* outputDir,
    const std::vector<std::string>& shaderBinaryDirs,
    const std::vector<std::string>& shaderPdbDirs
)
{ s_Tracker.Init(appName, appVersion, outputDir, shaderBinaryDirs, shaderPdbDirs); }

//-----------------------------------------------------------------------------
//! @brief      GPUクラッシュトラッカーの終了処理です.
//-----------------------------------------------------------------------------
void TermGpuCrashTracker()
{ s_Tracker.Term(); }

//-----------------------------------------------------------------------------
//! @brief      NVIDIA Aftermathの初期化処理です.
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
//! @brief      NVIDIA Aftermathの終了処理です.
//-----------------------------------------------------------------------------
void TermAftermath()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//! @brief      GPUクラッシュダンプの処理です.
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
        OutputLog("GFSDK_Aftermath_GetCrashDumpState() errcode = 0x%x", ret);
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
        OutputLog("Unexpected crash dump status: 0x%x", status);
    }
}