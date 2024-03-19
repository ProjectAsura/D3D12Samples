//-----------------------------------------------------------------------------
// File : App.cpp
// Desc : Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <App.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>

// For Agility SDK
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 613; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

namespace {

#include "../external/asdx12/res/shaders/Compiled/FullScreenVS.inc"
#include "../res/shaders/Compiled/SimpleRayQueryCS.inc"
#include "../res/shaders/Compiled/SimplePS.inc"

///////////////////////////////////////////////////////////////////////////////
// SceneParam structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) SceneParam
{
    asdx::Matrix    View;
    asdx::Matrix    Proj;
    asdx::Matrix    InvView;
    asdx::Matrix    InvViewProj;
    uint32_t        Width;
    uint32_t        Height;
    asdx::Vector2   InvSize;
};

///////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    asdx::Vector3   Position;
    asdx::Vector4   Color;
};

} // nmaespace

///////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです
//-----------------------------------------------------------------------------
App::App()
: asdx::Application(L"RayQuery Sample", 960, 540, nullptr, nullptr, nullptr)
{
    m_SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
App::~App()
{
}

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool App::OnInit() 
{
    // デバイスを取得.
    auto pDevice = asdx::GetD3D12Device();

    // グラフィックスキューを取得
    m_pQueue = asdx::GetGraphicsQueue();

    // Ray Tracing Tier 1.1 がサポートされているかどうかチェック.
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 options = {};
        auto hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options));
        if (FAILED(hr))
        { return false; }

        // サポートされていなければ終了.
        if (options.RaytracingTier != D3D12_RAYTRACING_TIER_1_1)
        {
            ELOGA("Error : RayTracing Tier 1.1 is not supported.");
            return false;
        }
    }

    auto pCmd = m_GfxCmdList.Reset();

    // 背景画像のロード.
    {
        // https://polyhaven.com/a/symmetrical_garden_02 から拝借.
        std::string path;
        if (!asdx::SearchFilePathA("res/textures/symmetrical_garden_02_2k.hdr", path))
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

        if (!m_Background.Init(pCmd, res))
        {
            res.Dispose();
            ELOGA("Error : Texture::Init() Failed.");
            return false;
        }

        res.Dispose();
    }

    // グラフィックス用のルートシグニチャを生成.
    {
        D3D12_DESCRIPTOR_RANGE  ranges[1] = {};
        asdx::InitRangeAsSRV(ranges[0], 0);

        D3D12_ROOT_PARAMETER params[1] = {};
        asdx::InitAsTable(params[0], 1, ranges, D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.pParameters        = params;
        desc.NumParameters      = _countof(params);
        desc.pStaticSamplers    = asdx::GetStaticSamplers();
        desc.NumStaticSamplers  = asdx::GetStaticSamplerCounts();
        desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        if (!asdx::InitRootSignature(pDevice, &desc, m_GraphicsRootSig.GetAddress()))
        {
            ELOG("Error : InitRootSignature() Failed.");
            return false;
        }
    }

    // グラフィックス用パイプラインステートを生成.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature         = m_GraphicsRootSig.GetPtr();
        desc.VS                     = { FullScreenVS, sizeof(FullScreenVS) };
        desc.PS                     = { SimplePS, sizeof(SimplePS) };
        desc.BlendState             = asdx::Preset::Opaque;
        desc.SampleMask             = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState        = asdx::Preset::CullNone;
        desc.DepthStencilState      = asdx::Preset::DepthNone;
        desc.InputLayout            = asdx::GetQuadLayout();
        desc.PrimitiveTopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets       = 1;
        desc.RTVFormats[0]          = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.DSVFormat              = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count       = 1;
        desc.SampleDesc.Quality     = 0;

        if (!m_GraphicsPipelineState.Init(pDevice, &desc))
        {
            ELOG("Error : GraphicsPipelineState Initialize Failed.");
            return false;
        }
    }

    // コンピュート用のルートシグニチャを生成.
    {
        auto cs = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_DESCRIPTOR_RANGE ranges[4] = {};
        asdx::InitRangeAsSRV(ranges[0], 1);
        asdx::InitRangeAsSRV(ranges[1], 2);
        asdx::InitRangeAsSRV(ranges[2], 3);
        asdx::InitRangeAsUAV(ranges[3], 0);

        D3D12_ROOT_PARAMETER params[6] = {};
        asdx::InitAsCBV(params[0], 0, cs); // SceneBuffer.
        asdx::InitAsSRV(params[1], 0, cs); // SceneTlas
        asdx::InitAsTable(params[2], 1, &ranges[0], cs); // Background.
        asdx::InitAsTable(params[3], 1, &ranges[1], cs); // VertexBuffer
        asdx::InitAsTable(params[4], 1, &ranges[2], cs); // IndexBuffer
        asdx::InitAsTable(params[5], 1, &ranges[3], cs); // Canvas.

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.pParameters        = params;
        desc.NumParameters      = _countof(params);
        desc.pStaticSamplers    = asdx::GetStaticSamplers();
        desc.NumStaticSamplers  = asdx::GetStaticSamplerCounts();

        if (!asdx::InitRootSignature(pDevice, &desc, m_ComputeRootSig.GetAddress()))
        {
            ELOG("Error : InitRootSignature() Failed.");
            return false;
        }
    }

    // コンピュート用パイプラインステートを生成.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};;
        desc.pRootSignature = m_ComputeRootSig.GetPtr();
        desc.CS             = { SimpleRayQueryCS, sizeof(SimpleRayQueryCS) };

        if (!m_ComputePipelineState.Init(pDevice, &desc))
        {
            ELOG("Error : ComputePipelineState Initialize Failed.");
            return false;
        }
    }

    // シーンバッファ初期化.
    {
        if (!m_SceneBuffer.Init(sizeof(SceneParam)))
        {
            ELOGA("Error : ConstantBuffer::Init() Failed.");
            return false;
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
        desc.InitState          = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        if (!m_Canvas.Init1(&desc))
        {
            ELOGA("Error : ComputeTarget::Init() Failed.");
            return false;
        }
    }

    // 頂点データ生成.
    {
        const Vertex vertices[] = {
            { asdx::Vector3( 0.0f,  0.7f, 1.0f), asdx::Vector4(1.0f, 0.0f, 0.0f, 1.0f) },
            { asdx::Vector3(-0.7f, -0.7f, 1.0f), asdx::Vector4(0.0f, 1.0f, 0.0f, 1.0f) },
            { asdx::Vector3( 0.7f, -0.7f, 1.0f), asdx::Vector4(0.0f, 0.0f, 1.0f, 1.0f) },
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

    // BLAS生成.
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

    // TLAS生成.
    {
        auto matrix = asdx::Transform3x4();

        asdx::DXR_INSTANCE_DESC desc = {};
        memcpy(desc.Transform, &matrix, sizeof(matrix));
        desc.InstanceMask           = 0x1;
        desc.AccelerationStructure  = m_BLAS.GetResource()->GetGPUVirtualAddress();

        if (!m_TLAS.Init(pDevice, 1, &desc, asdx::DXR_BUILD_FLAG_PREFER_FAST_TRACE))
        {
            ELOGA("Error : Tlas::Init() Failed.");
            return false;
        }
    }

    // ASビルド.
    {
        m_BLAS.Build(pCmd);
        m_TLAS.Build(pCmd);
    }

    pCmd->Close();
    ID3D12CommandList* pCmds[] = {
        pCmd
    };

    // コマンドを実行.
    m_pQueue->Execute(1, pCmds);

    // 待機点を発行.
    m_WaitPoint = m_pQueue->Signal();

    // 完了を待機.
    m_pQueue->Sync(m_WaitPoint);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理です.
//-----------------------------------------------------------------------------
void App::OnTerm()
{
    m_GraphicsRootSig.Reset();
    m_ComputeRootSig .Reset();

    m_GraphicsPipelineState.Term();
    m_ComputePipelineState .Term();

    m_VB            .Reset();
    m_IB            .Reset();
    m_TLAS          .Term();
    m_BLAS          .Term();
    m_Canvas        .Term();
    m_Background    .Term();
    m_SceneBuffer   .Term();
    m_VertexSRV     .Reset();
    m_IndexSRV      .Reset();

    m_pQueue = nullptr;
}

//-----------------------------------------------------------------------------
//      フレーム描画処理です.
//-----------------------------------------------------------------------------
void App::OnFrameRender(asdx::FrameEventArgs& args)
{
    if (m_pQueue == nullptr)
    { return; }

    auto idx  = GetCurrentBackBufferIndex();
    auto pCmd = m_GfxCmdList.Reset();

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
        res.View        = view;
        res.Proj        = proj;
        res.InvView     = asdx::Matrix::Invert(view);
        res.InvViewProj = asdx::Matrix::Invert(proj) * res.InvView;
        res.Width       = m_Width;
        res.Height      = m_Height;
        res.InvSize.x   = 1.0f / float(m_Width);
        res.InvSize.y   = 1.0f / float(m_Height);

        m_SceneBuffer.SwapBuffer();
        m_SceneBuffer.Update(&res, sizeof(res));
    }

    // コンピュートパイプライン実行.
    {
        m_Canvas.ChangeState(pCmd, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        uint32_t threadX = (m_Width  + 7) / 8;
        uint32_t threadY = (m_Height + 7) / 8;

        pCmd->SetComputeRootSignature(m_ComputeRootSig.GetPtr());
        m_ComputePipelineState.SetState(pCmd);

        pCmd->SetComputeRootConstantBufferView(0, m_SceneBuffer.GetResource()->GetGPUVirtualAddress());
        pCmd->SetComputeRootShaderResourceView(1, m_TLAS.GetResource()->GetGPUVirtualAddress());
        pCmd->SetComputeRootDescriptorTable(2, m_Background.GetView()->GetHandleGPU());
        pCmd->SetComputeRootDescriptorTable(3, m_VertexSRV->GetHandleGPU());
        pCmd->SetComputeRootDescriptorTable(4, m_IndexSRV->GetHandleGPU());
        pCmd->SetComputeRootDescriptorTable(5, m_Canvas.GetUAV()->GetHandleGPU());

        pCmd->Dispatch(threadX, threadY, 1);

        asdx::UAVBarrier(pCmd, m_Canvas.GetResource());
        m_Canvas.ChangeState(pCmd, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    // グラフィックスパイプライン実行.
    {
        D3D12_CPU_DESCRIPTOR_HANDLE pRTVs[] = {
            m_ColorTarget[idx].GetRTV()->GetHandleCPU()
        };

        m_ColorTarget[idx].ChangeState(pCmd, D3D12_RESOURCE_STATE_RENDER_TARGET);
        pCmd->OMSetRenderTargets(1, pRTVs, FALSE, nullptr);
        pCmd->RSSetViewports(1, &m_Viewport);
        pCmd->RSSetScissorRects(1, &m_ScissorRect);
        pCmd->SetGraphicsRootSignature(m_GraphicsRootSig.GetPtr());
        m_GraphicsPipelineState.SetState(pCmd);
        pCmd->SetGraphicsRootDescriptorTable(0, m_Canvas.GetSRV()->GetHandleGPU());

        asdx::DrawQuad(pCmd);
        m_ColorTarget[idx].ChangeState(pCmd, D3D12_RESOURCE_STATE_PRESENT);
    }

    // コマンドリストへの記録を終了.
    pCmd->Close();

    ID3D12CommandList* pCmds[] = {
        pCmd
    };

    // 前フレームの描画の完了を待機.
    if (m_WaitPoint.IsValid())
    { m_pQueue->Sync(m_WaitPoint); }

    // コマンドを実行.
    m_pQueue->Execute(1, pCmds);

    // 待機点を発行.
    m_WaitPoint = m_pQueue->Signal();

    // 画面に表示.
    Present(0);

    // フレーム同期.
    asdx::FrameSync();
}