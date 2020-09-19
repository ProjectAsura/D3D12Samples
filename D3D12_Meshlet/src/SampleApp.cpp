//-----------------------------------------------------------------------------
// File : SampleApp.cpp
// Desc : Sample Application
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SampleApp.h>
#include <asdxCmdHelper.h>
#include <asdxMath.h>
#include <asdxLogger.h>
#include <asdxMisc.h>


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
#include "../res/Compiled/SampleAS.inc"
#include "../res/Compiled/SampleMS.inc"
#include "../res/Compiled/SamplePS.inc"


///////////////////////////////////////////////////////////////////////////////
// MeshParam structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) MeshParam
{
    asdx::Matrix    World;
};

///////////////////////////////////////////////////////////////////////////////
// SceneParam structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) SceneParam
{
    asdx::Matrix    View;
    asdx::Matrix    Proj;
    asdx::Vector4   Planes[6];
    asdx::Vector3   CameraPos;
    float           Padding;
};

///////////////////////////////////////////////////////////////////////////////
// LightingParam structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) LightingParam
{
    asdx::Vector3   LightDir;
    float           LightIntensity;
};

} // namespace


///////////////////////////////////////////////////////////////////////////////
// SampleApp class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SampleApp::SampleApp()
{
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SampleApp::~SampleApp()
{
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SampleApp::OnInit()
{
    m_pGraphicsQueue = asdx::GfxDevice().GetGraphicsQueue();

    m_CameraController.Init(asdx::Vector3(0.0f, 0.0f, 2.0f),
        asdx::Vector3(0.0f, 0.0f, 0.0f),
        asdx::Vector3(0.0f, 1.0f, 0.0f),
        0.1f,
        1000.0f);

    m_CameraController.Present();

    auto pDevice = asdx::GfxDevice().GetDevice();

    // モデルをロード.
    {
        std::string path;
        if (!asdx::SearchFilePathA("teapot.mdl", path))
        {
            ELOG("Error : File Not Found.");
            return false;
        }

        asdx::ResModel resource;
        if (!asdx::LoadModel(path.c_str(), resource))
        {
            ELOG("Error : LoadModel() Failed.");
            return false;
        }

        if (!m_Model.Init(resource))
        {
            ELOG("Error : Model::Init() Failed.");
            return false;
        }
    }

    // ルートシグニチャの生成.
    {
        auto flags = 0u;
        flags |= asdx::RSF_DENY_VS;
        flags |= asdx::RSF_DENY_GS;
        flags |= asdx::RSF_DENY_HS;
        flags |= asdx::RSF_DENY_DS;

        asdx::RootSignatureDesc desc;
        desc.AddFromShader(SampleAS, sizeof(SampleAS))
            .AddFromShader(SampleMS, sizeof(SampleMS))
            .AddFromShader(SamplePS, sizeof(SamplePS))
            .SetFlag(flags);

        if (!m_RootSig.Init(pDevice, desc))
        {
            ELOGA("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // パイプラインステートの生成.
    {
        asdx::GEOMETRY_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature             = m_RootSig.GetPtr();
        desc.AS                         = { SampleAS, sizeof(SampleAS) };
        desc.MS                         = { SampleMS, sizeof(SampleMS) };
        desc.PS                         = { SamplePS, sizeof(SamplePS) };
        desc.BlendState                 = asdx::PipelineState::GetBS(asdx::BLEND_STATE_OPAQUE);
        desc.SampleMask                 = UINT_MAX;
        desc.RasterizerState            = asdx::PipelineState::GetRS(asdx::RASTERIZER_STATE_CULL_NONE);
        desc.DepthStencilState          = asdx::PipelineState::GetDSS(asdx::DEPTH_STATE_DEFAULT);
        desc.RTVFormats.RTFormats[0]    = m_SwapChainFormat;
        desc.DSVFormat                  = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count           = 1;
        desc.SampleDesc.Quality         = 0;

        if (!m_PSO.Init(pDevice, &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    if (m_MeshBuffer.Init(sizeof(MeshParam)))
    {
        ELOG("Error : MeshBuffer Init Failed.");
        return false;
    }

    if (m_SceneBuffer.Init(sizeof(SceneParam)))
    {
        ELOG("Error : SceneBuffer Init Failed.");
        return false;
    }

    if (m_LightingBuffer.Init(sizeof(LightingParam)))
    {
        ELOG("Error : LightingBuffer Init Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SampleApp::OnTerm()
{
    m_MeshBuffer    .Term();
    m_SceneBuffer   .Term();
    m_LightingBuffer.Term();

    m_Model  .Term();
    m_RootSig.Term();
    m_PSO    .Term();
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
void SampleApp::OnResize(const asdx::ResizeEventArgs& param)
{
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void SampleApp::OnFrameRender(asdx::FrameEventArgs& param)
{
    if (m_pGraphicsQueue == nullptr)
    { return; }

    auto idx  = GetCurrentBackBufferIndex();
    auto pCmd = m_GfxCmdList.Reset();

    asdx::GfxDevice().SetUploadCommand(pCmd);

    asdx::BarrierTransition(
        pCmd,
        m_ColorTarget[idx].GetResource(),
        0,
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);


    auto pRTV = m_ColorTarget[idx].GetRTV();
    auto pDSV = m_DepthTarget.GetDSV();

    asdx::ClearRTV(pCmd, pRTV, m_ClearColor);
    asdx::ClearDSV(pCmd, pDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

    asdx::SetRenderTarget(pCmd, pRTV, pDSV);
    asdx::SetViewport(pCmd, m_ColorTarget[idx].GetResource());

    // メッシュバッファ更新.
    {
        auto ptr = m_MeshBuffer.MapAs<MeshParam>();
        ptr->World = asdx::Matrix::CreateIdentity();   
        m_MeshBuffer.Unmap();
    }

    // シーンバッファ更新.
    {
        auto proj = asdx::Matrix::CreatePerspectiveFieldOfView(
            asdx::F_PIDIV4,
            m_AspectRatio,
            m_CameraController.GetNearClip(),
            m_CameraController.GetFarClip());

        auto ptr = m_SceneBuffer.MapAs<SceneParam>();
        ptr->View = m_CameraController.GetView();
        ptr->Proj = proj;
        ptr->CameraPos = m_CameraController.GetPosition();

        asdx::CalcFrustumPlanes(ptr->View, ptr->Proj, ptr->Planes);

        m_SceneBuffer.Unmap();
    }

    // ライティングバッファ更新.
    {
        auto ptr = m_LightingBuffer.MapAs<LightingParam>();
        ptr->LightDir = asdx::Vector3(0.0f, 0.0f, 1.0f);
        ptr->LightIntensity = 1.0f;
        m_LightingBuffer.Unmap();
    }

    auto idxPosition        = m_RootSig.Find("Position");
    auto idxTangentSpaces   = m_RootSig.Find("TangentSpaces");
    auto idxTexCoord        = m_RootSig.Find("TexCoords");
    auto idxIndices         = m_RootSig.Find("Indices");
    auto idxPrimitives      = m_RootSig.Find("Primitives");
    auto idxMeshlets        = m_RootSig.Find("Meshlets");
    auto idxScene           = m_RootSig.Find("SceneBuffer");
    auto idxMesh            = m_RootSig.Find("MeshBuffer");
    auto idxCullInfo        = m_RootSig.Find("CullBuffer");
    auto idxLighting        = m_RootSig.Find("LightingBuffer");

    pCmd->SetGraphicsRootSignature(m_RootSig.GetPtr());
    pCmd->SetPipelineState(m_PSO.GetPtr());

    auto pCBV_Mesh      = m_MeshBuffer    .GetResource()->GetGPUVirtualAddress();
    auto pCBV_Scene     = m_SceneBuffer   .GetResource()->GetGPUVirtualAddress();
    auto pCBV_Lighting  = m_LightingBuffer.GetResource()->GetGPUVirtualAddress();
    pCmd->SetGraphicsRootConstantBufferView(idxScene,    pCBV_Scene);
    pCmd->SetGraphicsRootConstantBufferView(idxMesh,     pCBV_Mesh);
    pCmd->SetGraphicsRootConstantBufferView(idxLighting, pCBV_Lighting);

    for(auto i=0u; i<m_Model.GetMeshCount(); ++i)
    {
        auto& mesh = m_Model.GetMesh(i);

        auto pSRV_Positions     = mesh.GetPositions    ().GetDescriptor()->GetHandleGPU();
        auto pSRV_TangentSpaces = mesh.GetTangentSpaces().GetDescriptor()->GetHandleGPU();
        auto pSRV_TexCoords     = mesh.GetTexCoords   (0).GetDescriptor()->GetHandleGPU();
        auto pSRV_Inidices      = mesh.GetInindices    ().GetDescriptor()->GetHandleGPU();
        auto pSRV_Primitives    = mesh.GetPrimitives   ().GetDescriptor()->GetHandleGPU();
        auto pSRV_Meshlet       = mesh.GetMeshlets     ().GetDescriptor()->GetHandleGPU();
        auto pSRV_CullInfos     = mesh.GetCullingInfos ().GetDescriptor()->GetHandleGPU();

        pCmd->SetGraphicsRootDescriptorTable(idxPosition,       pSRV_Positions);
        pCmd->SetGraphicsRootDescriptorTable(idxTangentSpaces,  pSRV_TangentSpaces);
        pCmd->SetGraphicsRootDescriptorTable(idxTexCoord,       pSRV_TexCoords);
        pCmd->SetGraphicsRootDescriptorTable(idxIndices,        pSRV_Inidices);
        pCmd->SetGraphicsRootDescriptorTable(idxPrimitives,     pSRV_Primitives);
        pCmd->SetGraphicsRootDescriptorTable(idxMeshlets,       pSRV_Meshlet);
        pCmd->SetGraphicsRootDescriptorTable(idxCullInfo,       pSRV_CullInfos);

        pCmd->DispatchMesh(mesh.GetMeshletCount(), 1, 1);
    }

    asdx::BarrierTransition(
        pCmd,
        m_ColorTarget[idx].GetResource(),
        0,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

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
//      マウスの処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnMouse(const asdx::MouseEventArgs& param)
{
    m_CameraController.OnMouse(
        param.X,
        param.Y,
        param.WheelDelta,
        param.IsLeftButtonDown,
        param.IsRightButtonDown,
        param.IsMiddleButtonDown,
        param.IsSideButton1Down,
        param.IsSideButton2Down);
}

//-----------------------------------------------------------------------------
//      キー処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnKey(const asdx::KeyEventArgs& param)
{
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnTyping(uint32_t keyCode)
{
}