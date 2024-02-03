//-----------------------------------------------------------------------------
// File : SampleApp.cpp
// Desc : Sample Application
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SampleApp.h>
#include <fnd/asdxMath.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>


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
    float           Scale;
    float           Padding0[3];
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
    float           Padding0;
    asdx::Vector3   DebugCamearPos;
    float           Padding1;
    asdx::Vector4   DebugPlanes[6];
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
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SampleApp::OnInit()
{
    m_pGraphicsQueue = asdx::GetGraphicsQueue();

    m_CameraController.Init(asdx::Vector3(0.0f, 0.0f, 2.0f),
        asdx::Vector3(0.0f, 0.0f, 0.0f),
        asdx::Vector3(0.0f, 1.0f, 0.0f),
        0.1f,
        1000.0f);
    m_CameraController.SetMoveGain(0.01f);

    m_CameraController.Present();

    auto pDevice = asdx::GetD3D12Device();

    // モデルをロード.
    {
        std::string path;
        if (!asdx::SearchFilePathA("../res/teapot.mdl", path))
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

        //m_RootSig.Dump();
    }

    // パイプラインステートの生成.
    {
        asdx::GEOMETRY_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_RootSig.GetPtr();
        desc.AS                             = { SampleAS, sizeof(SampleAS) };
        desc.MS                             = { SampleMS, sizeof(SampleMS) };
        desc.PS                             = { SamplePS, sizeof(SamplePS) };
        desc.BlendState                     = asdx::GetBS(asdx::BLEND_STATE_OPAQUE);
        desc.SampleMask                     = UINT_MAX;
        desc.RasterizerState                = asdx::GetRS(asdx::RASTERIZER_STATE_CULL_NONE);
        desc.DepthStencilState              = asdx::GetDSS(asdx::DEPTH_STATE_DEFAULT);
        desc.RTVFormats.NumRenderTargets    = 1;
        desc.RTVFormats.RTFormats[0]        = m_SwapChainFormat;
        desc.DSVFormat                      = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        if (!m_PSO.Init(pDevice, &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    if (!m_MeshBuffer.Init(sizeof(MeshParam)))
    {
        ELOG("Error : MeshBuffer Init Failed.");
        return false;
    }

    if (!m_SceneBuffer.Init(sizeof(SceneParam)))
    {
        ELOG("Error : SceneBuffer Init Failed.");
        return false;
    }

    {
        asdx::CommandList commandList;
        if (!commandList.Init(pDevice, D3D12_COMMAND_LIST_TYPE_DIRECT))
        {
            ELOGA("Error : CommandList::Init() Failed.");
            return false;
        }

        auto pCmd = commandList.Reset();

        asdx::GfxDevice().SetUploadCommand(pCmd);

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

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SampleApp::OnTerm()
{
    m_MeshBuffer    .Term();
    m_SceneBuffer   .Term();
    m_DebugSceneBuffer.Term();

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
    m_GfxCmdList.Reset();
    auto pCmd = m_GfxCmdList.GetCommandList();

    auto pRTV = m_ColorTarget[idx].GetRTV();
    auto pDSV = m_DepthTarget.GetDSV();

    // レンダーターゲットに書き込み.
    {
        asdx::ScopedTransition barrier(pCmd, pRTV, asdx::STATE_PRESENT, asdx::STATE_RTV);

        asdx::ClearRTV(pCmd, pRTV, m_ClearColor);
        asdx::ClearDSV(pCmd, pDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

        asdx::SetRenderTarget(pCmd, pRTV, pDSV);
        asdx::SetViewport(pCmd, pRTV);

        // メッシュバッファ更新.
        {
            // メリタ製サイズに戻す.
            auto scale = asdx::Vector3(1.0f, 4.0f / 3.0f, 1.0f);

            auto ptr = m_MeshBuffer.MapAs<MeshParam>();
            ptr->World = asdx::Matrix::CreateScale(scale);
            ptr->Scale = asdx::Max(scale.x, asdx::Max(scale.y, scale.z));
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

            if (!m_DebugPause)
            {
                ptr->DebugCamearPos = ptr->CameraPos;
                for(auto i=0; i<6; ++i)
                { ptr->DebugPlanes[i] = ptr->Planes[i]; }
            }

            m_SceneBuffer.Unmap();
        }

        asdx::Vector4 lightingBuf(0.0f, 1.0f, 0.0f, 1.0f);

        auto idxPosition        = m_RootSig.Find("Positions");
        auto idxTangentSpaces   = m_RootSig.Find("TangentSpaces");
        auto idxTexCoord        = m_RootSig.Find("TexCoords");
        auto idxIndices         = m_RootSig.Find("Indices");
        auto idxPrimitives      = m_RootSig.Find("Primitives");
        auto idxMeshlets        = m_RootSig.Find("Meshlets");
        auto idxScene           = m_RootSig.Find("CbScene");
        auto idxMesh            = m_RootSig.Find("CbMesh");
        auto idxCullInfo        = m_RootSig.Find("CullInfos");
        auto idxLighting        = m_RootSig.Find("CbLighting");
        auto idxMeshletInfo     = m_RootSig.Find("CbMeshletInfo");

        pCmd->SetGraphicsRootSignature(m_RootSig.GetPtr());
        pCmd->SetPipelineState(m_PSO.GetPtr());

        asdx::SetCBV(pCmd, idxScene, m_SceneBuffer.GetView());
        asdx::SetCBV(pCmd, idxMesh,  m_MeshBuffer .GetView());
        asdx::SetConstants(pCmd, idxLighting, 4, &lightingBuf, 0);

        for(auto i=0u; i<m_Model.GetMeshCount(); ++i)
        {
            auto& mesh = m_Model.GetMesh(i);

            asdx::SetTable(pCmd, idxPosition,       mesh.GetPositions    ().GetView());
            asdx::SetTable(pCmd, idxTangentSpaces,  mesh.GetTangentSpaces().GetView());
            asdx::SetTable(pCmd, idxTexCoord,       mesh.GetTexCoords   (0).GetView());
            asdx::SetTable(pCmd, idxIndices,        mesh.GetInindices    ().GetView());
            asdx::SetTable(pCmd, idxPrimitives,     mesh.GetPrimitives   ().GetView());
            asdx::SetTable(pCmd, idxMeshlets,       mesh.GetMeshlets     ().GetView());
            asdx::SetTable(pCmd, idxCullInfo,       mesh.GetCullingInfos ().GetView());

            auto meshletCount = mesh.GetMeshletCount();
            asdx::SetConstant(pCmd, idxMeshletInfo, meshletCount, 0);

            auto dipatchCount = asdx::DivRoundUp(meshletCount, 32);

            pCmd->DispatchMesh(dipatchCount, 1, 1);
        }
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
    m_CameraController.OnKey(param.KeyCode, param.IsKeyDown, param.IsAltDown);

    if (param.IsKeyDown)
    {
        if (param.KeyCode == 'S')
        { m_DebugPause = !m_DebugPause; }
    }
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnTyping(uint32_t keyCode)
{
}