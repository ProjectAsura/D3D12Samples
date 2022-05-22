//-----------------------------------------------------------------------------
// File : SampleApp.cpp
// Desc : Sample Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SampleApp.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <gfx/asdxSampler.h>


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/SampleRT.inc"

///////////////////////////////////////////////////////////////////////////////
// SceneParam structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) SceneParam
{
    asdx::Matrix    InvView;
    asdx::Matrix    InvViewProj;
};

///////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    asdx::Vector3   Position;
    asdx::Vector2   TexCoord;
    asdx::Vector4   Color;
};

} // namespace


///////////////////////////////////////////////////////////////////////////////
// SampleApp class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SampleApp::SampleApp()
: asdx::Application(L"Sample", 960, 540, nullptr, nullptr, nullptr)
{
    m_SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SampleApp::~SampleApp()
{
}

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool SampleApp::OnInit()
{
    auto pDevice = asdx::GetD3D12Device();
    m_pGraphicsQueue = asdx::GetGraphicsQueue();

    // DXRがサポートされているかどうかチェック.
    if (!asdx::IsSupportDXR(pDevice))
    {
        ELOGA("Error : DXR Unsupported.");
        return false;
    }

    asdx::CommandList setupCommandList;
    if (!setupCommandList.Init(pDevice, D3D12_COMMAND_LIST_TYPE_DIRECT))
    {
        ELOGA("Error : CommandList::Init() Failed.");
        return false;
    }

    setupCommandList.Reset();

    // 背景画像のロード.
    {
        // https://hdrihaven.com/hdri/?c=outdoor&h=abandoned_hopper_terminal_04 から拝借.

        std::string path;
        if (!asdx::SearchFilePathA("res/textures/abandoned_hopper_terminal_04_4k.dds", path))
        {
            ELOGA("Error : File Not Found.");
            return false;
        }

        asdx::ResTexture res;
        if (!res.LoadFromFileA(path.c_str()))
        {
            res.Dispose();
            ELOGA("Error : File Load Failed. path = %s", path.c_str());
            return false;
        }

        if (!m_BackGround.Init(setupCommandList, res))
        {
            res.Dispose();
            ELOGA("Error : Texture::Init() Failed.");
            return false;
        }

        res.Dispose();
    }

    // ベースカラーマップのロード.
    {
        const char* kPath[2] = {
            "res/textures/test0.dds",
            "res/textures/test1.dds"
        };

        for(auto i=0; i<2; ++i)
        {
            std::string path;
            if (!asdx::SearchFilePathA(kPath[i], path))
            {
                ELOGA("Error : File Not Found.");
                return false;
            }

            asdx::ResTexture res;
            if (!res.LoadFromFileA(path.c_str()))
            {
                res.Dispose();
                ELOGA("Error : File Load Failed. path = %s", path.c_str());
                return false;
            }

            if (!m_BaseColor[i].Init(setupCommandList, res))
            {
                res.Dispose();
                ELOGA("Error : Texture::Init() Failed.");
                return false;
            }

            res.Dispose();
        }
    }

    // コンピュートターゲット生成.
    {
        asdx::TargetDesc desc;
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = m_Width;
        desc.Height             = m_Height;
        desc.DepthOrArraySize   = 1;
        desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.MipLevels          = 1;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.InitState          = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        if (!m_Canvas.Init(&desc))
        {
            ELOGA("Error : ComputeTarget::Init() Failed.");
            return false;
        }
    }

    // シーンバッファ初期化.
    {
        if (!m_SceneCB.Init(sizeof(SceneParam)))
        {
            ELOGA("Error : ConstantBuffer::Init() Failed.");
            return false;
        }
    }

    // 頂点データ生成.
    {
        const Vertex vertices[] = {
            { asdx::Vector3( 0.0f,  0.7f, 1.0f), asdx::Vector2(0.5f, 0.0f), asdx::Vector4(1.0f, 0.0f, 0.0f, 1.0f) },
            { asdx::Vector3(-0.7f, -0.7f, 1.0f), asdx::Vector2(0.0f, 1.0f), asdx::Vector4(0.0f, 1.0f, 0.0f, 1.0f) },
            { asdx::Vector3( 0.7f, -0.7f, 1.0f), asdx::Vector2(1.0f, 1.0f), asdx::Vector4(0.0f, 0.0f, 1.0f, 1.0f) },
        };

        auto size = sizeof(vertices);
        if (!asdx::CreateUploadBuffer(pDevice, size, m_VB.GetAddress()))
        {
            ELOGA("Error : CreateUploadBuffer() Failed.");
            return false;
        }

        void* ptr = nullptr;
        auto hr = m_VB->Map(0, nullptr, &ptr);
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
            return false;
        }

        memcpy(ptr, vertices, sizeof(vertices));
        m_VB->Unmap(0, nullptr);

        if (!asdx::CreateBufferSRV(pDevice, m_VB.GetPtr(), sizeof(vertices) / 4, 0, m_VertexSRV.GetAddress()))
        {
            ELOGA("Error : CreateBufferSRV() Failed.");
            return false;
        }
    }

    // インデックスデータ生成.
    {
        uint32_t indices[] = { 0, 1, 2 };

        if (!asdx::CreateUploadBuffer(pDevice, sizeof(indices), m_IB.GetAddress()))
        {
            ELOGA("Error : CreateUploadBuffer() Failed.");
            return false;
        }

        void* ptr = nullptr;
        auto hr = m_IB->Map(0, nullptr, &ptr);
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
            return false;
        }

        memcpy(ptr, indices, sizeof(indices));
        m_IB->Unmap(0, nullptr);

        if (!asdx::CreateBufferSRV(pDevice, m_IB.GetPtr(), 3, 0, m_IndexSRV.GetAddress()))
        {
            ELOGA("Error : CreateBufferSRV() Failed.");
            return false;
        }
    }

    // 下位レベル高速化機構の生成.
    {
        asdx::DXR_GEOMETRY_DESC desc = {};
        desc.Type                                   = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        desc.Flags                                  = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
        desc.Triangles.VertexCount                  = 3;
        desc.Triangles.VertexBuffer.StartAddress    = m_VB->GetGPUVirtualAddress();
        desc.Triangles.VertexBuffer.StrideInBytes   = sizeof(Vertex);
        desc.Triangles.VertexFormat                 = DXGI_FORMAT_R32G32B32_FLOAT;
        desc.Triangles.IndexCount                   = 3;
        desc.Triangles.IndexBuffer                  = m_IB->GetGPUVirtualAddress();
        desc.Triangles.IndexFormat                  = DXGI_FORMAT_R32_UINT;
        desc.Triangles.Transform3x4                 = 0;

        if (!m_BLAS.Init(pDevice, 1, &desc, asdx::DXR_BUILD_FLAG_PREFER_FAST_TRACE))
        {
            ELOGA("Error : Blas::Init() Failed.");
            return false;
        }
    }

    // 上位レベル高速化機構の生成.
    {
        asdx::DXR_INSTANCE_DESC desc[2] = {};
        {
            auto transform = asdx::FromMatrix(asdx::Matrix::CreateTranslation(asdx::Vector3(-1.0f, 0.0f, 0.0f)));
    
            memcpy(desc[0].Transform, &transform, sizeof(transform));
            desc[0].InstanceID             = 0;
            desc[0].InstanceMask           = 0xFF;
            desc[0].AccelerationStructure  = m_BLAS.GetResource()->GetGPUVirtualAddress();
            desc[0].InstanceContributionToHitGroupIndex = 0;
        }

        {
            auto transform = asdx::FromMatrix(asdx::Matrix::CreateTranslation(asdx::Vector3(1.0f, 0.0f, 0.0f)));

            memcpy(desc[1].Transform, &transform, sizeof(transform));
            desc[1].InstanceID              = 1;
            desc[1].InstanceMask            = 0xFF;
            desc[1].AccelerationStructure   = m_BLAS.GetResource()->GetGPUVirtualAddress();
            desc[1].InstanceContributionToHitGroupIndex = 1;
        }

        if (!m_TLAS.Init(pDevice, 2, desc, asdx::DXR_BUILD_FLAG_PREFER_FAST_TRACE))
        {
            ELOGA("Error : Tlas::Init() Failed.");
            return false;
        }
    }

    // グローバルルートシグニチャの生成.
    {
        asdx::DescriptorSetLayout<6, 1> layout;
        layout.SetTableUAV(0, asdx::SV_ALL, 0);
        layout.SetTableSRV(1, asdx::SV_ALL, 1);
        layout.SetSRV(2, asdx::SV_ALL, 0);
        layout.SetSRV(3, asdx::SV_ALL, 2);
        layout.SetSRV(4, asdx::SV_ALL, 3);
        layout.SetCBV(5, asdx::SV_ALL, 0);
        layout.SetStaticSampler(0, asdx::SV_ALL, asdx::STATIC_SAMPLER_LINEAR_WRAP, 0);

        if (!m_GlobalRootSig.Init(pDevice, layout.GetDesc()))
        {
            ELOGA("Error : RootSignature::Init() Failed");
            return false;
        }
    }

    // ローカルルートシグニチャの生成. 
    // シェーダテーブルごとに異なるものをはこちらで設定する.
    {
        asdx::DescriptorSetLayout<1, 0> layout;
        layout.SetTableSRV(0, asdx::SV_ALL, 4);
        layout.SetFlags(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);

        if (!m_LocalRootSig.Init(pDevice, layout.GetDesc()))
        {
            ELOGA("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // ステートオブジェクトの生成.
    {
        D3D12_EXPORT_DESC exports[4] = {
            { L"OnGenerateRay", nullptr, D3D12_EXPORT_FLAG_NONE },
            { L"OnClosestHit1", nullptr, D3D12_EXPORT_FLAG_NONE },
            { L"OnClosestHit2", nullptr, D3D12_EXPORT_FLAG_NONE },
            { L"OnMiss",        nullptr, D3D12_EXPORT_FLAG_NONE },
        };

        // ヒットグループ1の設定.
        D3D12_HIT_GROUP_DESC hitGroups[2] = {};
        hitGroups[0].ClosestHitShaderImport = L"OnClosestHit1";
        hitGroups[0].HitGroupExport         = L"HitGroup1";
        hitGroups[0].Type                   = D3D12_HIT_GROUP_TYPE_TRIANGLES;

        // ヒットグループ2の設定.
        hitGroups[1].ClosestHitShaderImport = L"OnClosestHit2";
        hitGroups[1].HitGroupExport         = L"HitGroup2";
        hitGroups[1].Type                   = D3D12_HIT_GROUP_TYPE_TRIANGLES;

        asdx::RayTracingPipelineStateDesc desc = {};
        desc.pGlobalRootSignature   = m_GlobalRootSig.GetPtr();
        desc.pLocalRootSignature    = m_LocalRootSig.GetPtr();
        desc.DXILLibrary            = { SampleRT, sizeof(SampleRT) };
        desc.ExportCount            = _countof(exports);
        desc.pExports               = exports;
        desc.HitGroupCount          = _countof(hitGroups);
        desc.pHitGroups             = hitGroups;
        desc.MaxPayloadSize         = sizeof(asdx::Vector4) + sizeof(asdx::Vector3);
        desc.MaxAttributeSize       = sizeof(asdx::Vector2);
        desc.MaxTraceRecursionDepth = 1;

        if (!m_RayTracingPSO.Init(pDevice, desc))
        {
            ELOGA("Error : RayTracingPipelineState::Init() Failed.");
            return false;
        }
    }

    // 高速化機構のビルド.
    {
        auto pCmd = setupCommandList.GetCommandList();

        m_BLAS.Build(pCmd);
        m_TLAS.Build(pCmd);
    }

    // セットアップコマンド実行.
    {
        setupCommandList.Close();
        ID3D12CommandList* pCmds[] = {
            setupCommandList.GetCommandList()
        };

        // コマンドを実行.
        m_pGraphicsQueue->Execute(1, pCmds);

        // 待機点を発行.
        m_FrameWaitPoint = m_pGraphicsQueue->Signal();

        // 完了を待機.
        m_pGraphicsQueue->Sync(m_FrameWaitPoint);

        // コマンドリスト破棄.
        setupCommandList.Term();
    }

    // シェーダテーブルの生成.
    {
        // レイ生成シェーダテーブル.
        {
            asdx::ShaderRecord record;
            record.ShaderIdentifier = m_RayTracingPSO.GetShaderIdentifier(L"OnGenerateRay");

            asdx::ShaderTable::Desc desc;
            desc.RecordCount    = 1;
            desc.pRecords       = &record;

            if (!m_RayGenTable.Init(pDevice, &desc))
            {
                ELOGA("Error : ShaderTable::Init() Failed.");
                return false;
            }
        }

        // ミスシェーダテーブル.
        {
            asdx::ShaderRecord record;
            record.ShaderIdentifier = m_RayTracingPSO.GetShaderIdentifier(L"OnMiss");

            asdx::ShaderTable::Desc desc;
            desc.RecordCount    = 1;
            desc.pRecords       = &record;

            if (!m_MissTable.Init(pDevice, &desc))
            {
                ELOGA("Error : ShaderTable::Init() Failed.");
                return false;
            }
        }

        // ヒットグループシェーダテーブル
        {
            const auto kGeometryCount = 2;

            struct LocalParam
            {
                D3D12_GPU_DESCRIPTOR_HANDLE Handle;
            } param[kGeometryCount];

            for(auto i=0; i<kGeometryCount; ++i)
            { param[i].Handle = m_BaseColor[i].GetView()->GetHandleGPU(); }

            // シェーダとテクスチャをそれぞれ別割り当てする.
            asdx::ShaderRecord record[kGeometryCount];

            record[0].ShaderIdentifier   = m_RayTracingPSO.GetShaderIdentifier(L"HitGroup1");
            record[0].LocalRootArguments = &param[0];

            record[1].ShaderIdentifier   = m_RayTracingPSO.GetShaderIdentifier(L"HitGroup2");
            record[1].LocalRootArguments = &param[1];

            asdx::ShaderTable::Desc desc;
            desc.RecordCount            = _countof(record);
            desc.pRecords               = record;
            desc.LocalRootArgumentSize  = sizeof(LocalParam);

            if (!m_HitGroupTable.Init(pDevice, &desc))
            {
                ELOGA("Error : ShaderTable::Init() Failed.");
                return false;
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnTerm()
{
    m_GlobalRootSig .Term();
    m_LocalRootSig  .Term();
    m_RayTracingPSO .Term();
    m_VB            .Reset();
    m_IB            .Reset();
    m_TLAS          .Term();
    m_BLAS          .Term();
    m_Canvas        .Term();
    m_BackGround    .Term();
    m_RayGenTable   .Term();
    m_MissTable     .Term();
    m_HitGroupTable .Term();
    m_SceneCB       .Term();
    m_VertexSRV     .Reset();
    m_IndexSRV      .Reset();

    for(auto i=0; i<2; ++i)
    { m_BaseColor[i].Term(); }
}

//-----------------------------------------------------------------------------
//      フレーム描画処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnFrameRender(asdx::FrameEventArgs& args)
{
    if (m_pGraphicsQueue == nullptr)
    { return; }

    auto idx  = GetCurrentBackBufferIndex();
    m_GfxCmdList.Reset();

    // シーンバッファ更新.
    {
        // ビュー行列.
        auto view = asdx::Matrix::CreateLookAt(
            asdx::Vector3(0.0f, 0.0f, -2.0f),
            asdx::Vector3(0.0f, 0.0f, 0.0f),
            asdx::Vector3(0.0f, 1.0f, 0.0f));

        // 射影行列.
        auto proj = asdx::Matrix::CreatePerspectiveFieldOfView(
            asdx::ToRadian(37.5f),
            float(m_Width) / float(m_Height),
            1.0f,
            1000.0f);

        SceneParam res = {};
        res.InvView     = asdx::Matrix::Invert(view);
        res.InvViewProj = asdx::Matrix::Invert(proj) * res.InvView;

        m_SceneCB.SwapBuffer();
        m_SceneCB.Update(&res, sizeof(res));
    }

    // レイトレ実行.
    {
        // グローバルルートシグニチャ設定.
        m_GfxCmdList.SetRootSignature(m_GlobalRootSig.GetPtr(), true);

        // ステートオブジェクト設定.
        m_GfxCmdList.SetStateObject(m_RayTracingPSO.GetStateObject());

        // リソースをバインド.
        {
            m_GfxCmdList.SetTable(0, m_Canvas.GetUAV(), true);
            m_GfxCmdList.SetTable(1, m_BackGround.GetView(), true);
            m_GfxCmdList.SetSRV  (2, m_TLAS.GetResource(), true);
            m_GfxCmdList.SetSRV  (3, m_VB.GetPtr(), true);
            m_GfxCmdList.SetSRV  (4, m_IB.GetPtr(), true);
            m_GfxCmdList.SetCBV  (5, m_SceneCB.GetView(), true);
        }

        D3D12_DISPATCH_RAYS_DESC desc = {};
        desc.HitGroupTable              = m_HitGroupTable.GetTableView();
        desc.MissShaderTable            = m_MissTable    .GetTableView();
        desc.RayGenerationShaderRecord  = m_RayGenTable  .GetRecordView();
        desc.Width                      = UINT(m_Canvas.GetDesc().Width);
        desc.Height                     = m_Canvas.GetDesc().Height;
        desc.Depth                      = 1;

        m_GfxCmdList.DispatchRays(&desc);

        // バリアを張っておく.
        m_GfxCmdList.BarrierUAV(m_Canvas.GetResource());
    }

    // レイトレ結果をバックバッファにコピー.
    {
        asdx::ScopedBarrier b0(m_GfxCmdList.GetCommandList(), m_Canvas.GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
        asdx::ScopedBarrier b1(m_GfxCmdList.GetCommandList(), m_ColorTarget[idx].GetResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
        m_GfxCmdList.CopyResource(m_ColorTarget[idx].GetResource(), m_Canvas.GetResource());
    }

    m_GfxCmdList.Close();

    ID3D12CommandList* pCmds[] = {
        m_GfxCmdList.GetCommandList()
    };

    // 前フレームの描画の完了を待機.
    if (m_FrameWaitPoint.IsValid())
    { m_pGraphicsQueue->Sync(m_FrameWaitPoint); }

    // コマンドを実行.
    m_pGraphicsQueue->Execute(1, pCmds);

    // 待機点を発行.
    m_FrameWaitPoint = m_pGraphicsQueue->Signal();

    // 画面に表示.
    Present(0);

    // フレーム同期.
    asdx::FrameSync();
}

//-----------------------------------------------------------------------------
//      リサイズ処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnResize(const asdx::ResizeEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      キー処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnKey(const asdx::KeyEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      マウス処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnMouse(const asdx::MouseEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnTyping(uint32_t keyCode)
{
}
