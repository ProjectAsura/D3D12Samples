//-----------------------------------------------------------------------------
// File : SampleApp.cpp
// Desc : Sample Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SampleApp.h>
#include <asdxLogger.h>
#include <asdxCmdHelper.h>
#include <asdxMisc.h>


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
    auto pDevice = asdx::GfxDevice().GetDevice();
    m_pGraphicsQueue = asdx::GfxDevice().GetGraphicsQueue();

    // DXRがサポートされているかどうかチェック.
    if (!asdx::IsSupportDXR(pDevice))
    {
        ELOGA("Error : DXR Unsupported.");
        return false;
    }

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
            res.Release();
            ELOGA("Error : File Load Failed. path = %s", path.c_str());
            return false;
        }

        if (!m_BackGround.Init(res))
        {
            res.Release();
            ELOGA("Error : Texture::Init() Failed.");
            return false;
        }

        res.Release();
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
                res.Release();
                ELOGA("Error : File Load Failed. path = %s", path.c_str());
                return false;
            }

            if (!m_BaseColor[i].Init(res))
            {
                res.Release();
                ELOGA("Error : Texture::Init() Failed.");
                return false;
            }

            res.Release();
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
        asdx::RootSignatureDesc desc;
        desc.AddTableUAV("RenderTarget", asdx::SV_ALL, 0);
        desc.AddTableSRV("BackGround",   asdx::SV_ALL, 1);
        desc.AddSRV("Scene",        asdx::SV_ALL, 0);
        desc.AddSRV("Vertices",     asdx::SV_ALL, 2);
        desc.AddSRV("Indices",      asdx::SV_ALL, 3);
        desc.AddCBV("SceneParam",   asdx::SV_ALL, 0);
        desc.AddStaticSampler(asdx::SV_ALL, asdx::SST_LINEAR_WRAP, 0);

        if (!m_GlobalRootSig.Init(pDevice, desc))
        {
            ELOGA("Error : RootSignature::Init() Failed");
            return false;
        }
    }

    // ローカルルートシグニチャの生成. 
    // シェーダテーブルごとに異なるものをはこちらで設定する.
    {
        asdx::RootSignatureDesc desc;
        desc.SetFlag(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
        desc.AddTable("BaseColor", asdx::SV_ALL, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);

        if (!m_LocalRootSig.Init(pDevice, desc))
        {
            ELOGA("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // ステートオブジェクトの生成.
    {
        asdx::SubObjects subObjects;

        D3D12_EXPORT_DESC exports[4] = {
            { L"OnGenerateRay", nullptr, D3D12_EXPORT_FLAG_NONE },
            { L"OnClosestHit1", nullptr, D3D12_EXPORT_FLAG_NONE },
            { L"OnClosestHit2", nullptr, D3D12_EXPORT_FLAG_NONE },
            { L"OnMiss",        nullptr, D3D12_EXPORT_FLAG_NONE },
        };

        // グローバルルートシグニチャの設定.
        D3D12_GLOBAL_ROOT_SIGNATURE globalRootSig = {};
        globalRootSig.pGlobalRootSignature = m_GlobalRootSig.GetPtr();
        subObjects.Push(&globalRootSig);

        // ローカルルートシグニチャの設定.
        D3D12_LOCAL_ROOT_SIGNATURE localRootSig = {};
        localRootSig.pLocalRootSignature = m_LocalRootSig.GetPtr();
        subObjects.Push(&localRootSig);

        // DXILライブラリの設定.
        D3D12_DXIL_LIBRARY_DESC dxilLib = {};
        dxilLib.DXILLibrary = { SampleRT, sizeof(SampleRT) };
        dxilLib.NumExports  = _countof(exports);
        dxilLib.pExports    = exports;
        subObjects.Push(&dxilLib);

        // ヒットグループ1の設定.
        D3D12_HIT_GROUP_DESC hitGroup = {};
        hitGroup.ClosestHitShaderImport = L"OnClosestHit1";
        hitGroup.HitGroupExport         = L"HitGroup1";
        hitGroup.Type                   = D3D12_HIT_GROUP_TYPE_TRIANGLES;
        subObjects.Push(&hitGroup);

        // ヒットグループ2の設定.
        D3D12_HIT_GROUP_DESC hitGroup2 = {};
        hitGroup2.ClosestHitShaderImport = L"OnClosestHit2";
        hitGroup2.HitGroupExport         = L"HitGroup2";
        hitGroup2.Type                   = D3D12_HIT_GROUP_TYPE_TRIANGLES;
        subObjects.Push(&hitGroup2);

        // シェーダ設定.
        D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
        shaderConfig.MaxPayloadSizeInBytes   = sizeof(asdx::Vector4) + sizeof(asdx::Vector3);
        shaderConfig.MaxAttributeSizeInBytes = sizeof(asdx::Vector2);
        subObjects.Push(&shaderConfig);

        // パイプライン設定.
        D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
        pipelineConfig.MaxTraceRecursionDepth = 1;
        subObjects.Push(&pipelineConfig);

        // ステート設定取得.
        auto desc = subObjects.GetDesc();

        // ステートオブジェクトを生成.
        auto hr = pDevice->CreateStateObject(&desc, IID_PPV_ARGS(m_StateObject.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateStateObject() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // 高速化機構のビルド.
    {
        asdx::CommandList commandList;
        if (!commandList.Init(pDevice, D3D12_COMMAND_LIST_TYPE_DIRECT))
        {
            ELOGA("Error : CommandList::Init() Failed.");
            return false;
        }

        auto pCmd = commandList.Reset();

        asdx::GfxDevice().SetUploadCommand(pCmd);

        m_BLAS.Build(pCmd);
        m_TLAS.Build(pCmd);

        pCmd->Close();
        ID3D12CommandList* pCmds[] = {
            pCmd
        };

        // コマンドを実行.
        m_pGraphicsQueue->Execute(1, pCmds);

        // 待機点を発行.
        m_FrameWaitPoint = m_pGraphicsQueue->Signal();

        // 完了を待機.
        m_pGraphicsQueue->Sync(m_FrameWaitPoint);

        // コマンドリスト破棄.
        commandList.Term();
    }

    // シェーダテーブルの生成.
    {
        asdx::RefPtr<ID3D12StateObjectProperties> props;
        auto hr = m_StateObject->QueryInterface(IID_PPV_ARGS(props.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12StateObject::QueryInteface() Failed. errcode = 0x%x", hr);
            return false;
        }

        // レイ生成シェーダテーブル.
        {
            asdx::ShaderRecord record;
            record.ShaderIdentifier = props->GetShaderIdentifier(L"OnGenerateRay");

            if (!m_RayGenTable.Init(pDevice, 1, &record))
            {
                ELOGA("Error : ShaderTable::Init() Failed.");
                return false;
            }
        }

        // ミスシェーダテーブル.
        {
            asdx::ShaderRecord record;
            record.ShaderIdentifier = props->GetShaderIdentifier(L"OnMiss");

            if (!m_MissTable.Init(pDevice, 1, &record))
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

            record[0].ShaderIdentifier   = props->GetShaderIdentifier(L"HitGroup1");
            record[0].LocalRootArguments = &param[0];

            record[1].ShaderIdentifier   = props->GetShaderIdentifier(L"HitGroup2");
            record[1].LocalRootArguments = &param[1];

            if (!m_HitGroupTable.Init(pDevice, _countof(record), record, sizeof(LocalParam)))
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
    m_StateObject   .Reset();
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
    auto pCmd = m_GfxCmdList.Reset();

    asdx::GfxDevice().SetUploadCommand(pCmd);

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
        pCmd->SetComputeRootSignature(m_GlobalRootSig.GetPtr());

        // ステートオブジェクト設定.
        pCmd->SetPipelineState1(m_StateObject.GetPtr());

        // リソースをバインド.
        {
            auto u0 = m_GlobalRootSig.Find("RenderTarget");
            auto t0 = m_GlobalRootSig.Find("Scene");
            auto t1 = m_GlobalRootSig.Find("BackGround");
            auto t2 = m_GlobalRootSig.Find("Vertices");
            auto t3 = m_GlobalRootSig.Find("Indices");
            auto b0 = m_GlobalRootSig.Find("SceneParam");
            asdx::SetTable(pCmd, u0, m_Canvas     .GetUAV(), true);
            asdx::SetSRV  (pCmd, t0, m_TLAS  .GetResource(), true);
            asdx::SetTable(pCmd, t1, m_BackGround.GetView(), true);
            asdx::SetSRV  (pCmd, t2, m_VB         .GetPtr(), true);
            asdx::SetSRV  (pCmd, t3, m_IB         .GetPtr(), true);
            asdx::SetCBV  (pCmd, b0, m_SceneCB   .GetView(), true);
        }

        D3D12_DISPATCH_RAYS_DESC desc = {};
        desc.HitGroupTable              = m_HitGroupTable.GetTableView();
        desc.MissShaderTable            = m_MissTable    .GetTableView();
        desc.RayGenerationShaderRecord  = m_RayGenTable  .GetRecordView();
        desc.Width                      = UINT(m_Canvas.GetDesc().Width);
        desc.Height                     = m_Canvas.GetDesc().Height;
        desc.Depth                      = 1;

        pCmd->DispatchRays(&desc);

        // バリアを張っておく.
        asdx::BarrierUAV(pCmd, m_Canvas.GetResource());
    }

    // レイトレ結果をバックバッファにコピー.
    {
        asdx::ScopedTransition b0(pCmd, m_Canvas.GetResource(), asdx::STATE_UAV, asdx::STATE_COPY_SRC);
        asdx::ScopedTransition b1(pCmd, m_ColorTarget[idx].GetResource(), asdx::STATE_PRESENT, asdx::STATE_COPY_DST);
        pCmd->CopyResource(m_ColorTarget[idx].GetResource(), m_Canvas.GetResource());
    }

    pCmd->Close();

    ID3D12CommandList* pCmds[] = {
        pCmd
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
    asdx::GfxDevice().FrameSync();
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
