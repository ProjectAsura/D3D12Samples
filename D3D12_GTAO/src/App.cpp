//-------------------------------------------------------------------------------------------------
// File : App.cpp
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

#define NOMINMAX

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <App.h>
#include <cstdio>
#include <array>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMath.h>
#include <fnd/asdxMisc.h>
#include <res/asdxResModel.h>
#include <gfx/asdxCommandQueue.h>
#include <gfx/asdxSampler.h>
#include <fw/asdxCameraController.h>
#include <edit/asdxGuiMgr.h>
#include "../external/imgui/imgui.h"

#ifndef ASDX_WND_CLASSNAME
#define ASDX_WND_CLASSNAME      TEXT("asdxWindowClass")
#endif//ASDX_WND_CLASSNAME


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
// Global Variables.
//-------------------------------------------------------------------------------------------------
App*    g_pApp = nullptr;

#include "../res/shaders/Compiled/SimpleVS.inc"
#include "../res/shaders/Compiled/SimplePS.inc"
#include "../res/shaders/Compiled/FullScreenVS.inc"
#include "../res/shaders/Compiled/GTAO_PS.inc"
#include "../res/shaders/Compiled/CrossBilateralFilterPS.inc"
#include "../res/shaders/Compiled/CopyPS.inc"


///////////////////////////////////////////////////////////////////////////////
// MeshVertex structure
///////////////////////////////////////////////////////////////////////////////
struct MeshVertex
{
    asdx::Vector3   Position;
    asdx::Vector3   Normal;
    asdx::Vector3   Tangent;
    asdx::Vector2   TexCoord;
};

///////////////////////////////////////////////////////////////////////////////
// SceneParam structure
///////////////////////////////////////////////////////////////////////////////
struct SceneParam
{
    asdx::Matrix    View;
    asdx::Matrix    Proj;
    asdx::Matrix    InvView;
    asdx::Matrix    InvProj;
    float           NearClip;
    float           FarClip;
    float           FieldOfView;
    float           AspectRatio;
};

///////////////////////////////////////////////////////////////////////////////
// SsaoParam structure
///////////////////////////////////////////////////////////////////////////////
struct SsaoParam
{
    asdx::Vector2   InvSize;
    float           RadiusScreenSpace;
    float           RadiusViewSpace;
};

static const D3D12_INPUT_ELEMENT_DESC g_InputElements[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};

template<typename T>
void SafeRelease(T*& ptr)
{
    if (ptr != nullptr)
    {
        ptr->Release();
        ptr = nullptr;
    }
}

enum PARAM_INDEX
{
    PARAM_INDEX_B0,
    PARAM_INDEX_B1,
    PARAM_INDEX_B2,
    PARAM_INDEX_B3,
    PARAM_INDEX_T0,
    PARAM_INDEX_T1,
};

} // namespace /* anonymous */


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      �R���X�g���N�^�ł�.
//-------------------------------------------------------------------------------------------------
App::App()
: asdx::Application(L"GTAO", 1920, 1080, nullptr, nullptr, nullptr)
{
    m_SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_ClearDepth = 0.0f;
}

//-------------------------------------------------------------------------------------------------
//      �f�X�g���N�^�ł�.
//-------------------------------------------------------------------------------------------------
App::~App()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      �A�v���P�[�V�����ŗL�̏����������ł�.
//-------------------------------------------------------------------------------------------------
bool App::OnInit()
{
    auto pDevice = asdx::GetD3D12Device();

    // �@���o�b�t�@.
    {
        asdx::TargetDesc desc;
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = m_Width;
        desc.Height             = m_Height;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_R32G32_FLOAT;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.InitState          = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        if (!m_NormalTarget.Init(&desc, false))
        {
            ELOG("Error : ColorTarget::Init() Failed");
            return false;
        }
    }

    // AO�o�b�t�@.
    {
        asdx::TargetDesc desc;
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = m_Width;
        desc.Height             = m_Height;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_R8_UNORM;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.InitState          = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        if (!m_AoTarget.Init(&desc, false))
        {
            ELOG("Error : ColorTarget::Init() Failed.");
            return false;
        }
    }

    // �s���|���o�b�t�@.
    {
        asdx::TargetDesc desc;
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = m_Width;
        desc.Height             = m_Height;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_R8_UNORM;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.InitState          = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        if (!m_BlurTarget0.Init(&desc, false))
        {
            ELOG("Error : ColorTarget::Init() Failed.");
            return false;
        }

        if (!m_BlurTarget1.Init(&desc, false))
        {
            ELOG("Error : ColorTarget::Init() Failed.");
            return false;
        }
    }

    // ���[�g�V�O�j�`��.
    {
        D3D12_DESCRIPTOR_RANGE range[2] = {};
        asdx::RangeSRV(range[0], 0);
        asdx::RangeSRV(range[1], 1);

        D3D12_ROOT_PARAMETER params[6] = {};
        asdx::ParamCBV      (params[PARAM_INDEX_B0], D3D12_SHADER_VISIBILITY_VERTEX, 0);
        asdx::ParamCBV      (params[PARAM_INDEX_B1], D3D12_SHADER_VISIBILITY_ALL,    1);
        asdx::ParamCBV      (params[PARAM_INDEX_B2], D3D12_SHADER_VISIBILITY_PIXEL,  2);
        asdx::ParamConstants(params[PARAM_INDEX_B3], D3D12_SHADER_VISIBILITY_PIXEL,  3, 3);
        asdx::ParamTable    (params[PARAM_INDEX_T0], D3D12_SHADER_VISIBILITY_PIXEL,  1, &range[0]);
        asdx::ParamTable    (params[PARAM_INDEX_T1], D3D12_SHADER_VISIBILITY_PIXEL,  1, &range[1]);

        D3D12_STATIC_SAMPLER_DESC samplers[] = {
            asdx::STATIC_SAMPLER_DESC(asdx::SAMPLER_POINT_CLAMP, D3D12_SHADER_VISIBILITY_PIXEL, 0),
            asdx::STATIC_SAMPLER_DESC(asdx::SAMPLER_LINEAR_CLAMP, D3D12_SHADER_VISIBILITY_PIXEL, 1)
        };

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(params);
        desc.pParameters        = params;
        desc.NumStaticSamplers  = _countof(samplers);
        desc.pStaticSamplers    = samplers;
        desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        if (!m_RootSig.Init(pDevice, &desc))
        {
            ELOG("Error : RootSignature::Init() Failed");
            return false;
        }
    }

    {
        struct Vertex {
            asdx::Vector2 Position;
            asdx::Vector2 TexCoord;
        };

        const Vertex vertices[] = {
        { asdx::Vector2(-1.0f,  1.0f), asdx::Vector2(0.0f, 0.0f) },
        { asdx::Vector2( 3.0f,  1.0f), asdx::Vector2(2.0f, 0.0f) },
        { asdx::Vector2(-1.0f, -3.0f), asdx::Vector2(0.0f, 2.0f) },
        };

        if (!m_FullScreenVB.Init(sizeof(vertices), sizeof(Vertex)))
        {
            return false;
        }

        auto ptr = m_FullScreenVB.Map<Vertex>();
        memcpy(ptr, vertices, sizeof(vertices));
        m_FullScreenVB.Unmap();
    }

    D3D12_INPUT_ELEMENT_DESC elements[] = {
       {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
       {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };


    // ���b�V���`��pPSO.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature         = m_RootSig.GetPtr();
        desc.VS                     = { SimpleVS, sizeof(SimpleVS) };
        desc.PS                     = { SimplePS, sizeof(SimplePS) };
        desc.DepthStencilState      = asdx::DEPTH_STENCIL_DESC(asdx::DEPTH_STATE_DEFAULT, D3D12_COMPARISON_FUNC_GREATER);
        desc.RasterizerState        = asdx::RASTERIZER_DESC(asdx::RASTERIZER_STATE_CULL_NONE);
        desc.BlendState             = asdx::BLEND_DESC(asdx::BLEND_STATE_OPAQUE);
        desc.SampleMask             = D3D12_DEFAULT_SAMPLE_MASK;
        desc.InputLayout            = { g_InputElements, _countof(g_InputElements) };
        desc.PrimitiveTopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets       = 1;
        desc.RTVFormats[0]          = DXGI_FORMAT_R32G32_FLOAT;
        desc.DSVFormat              = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count       = 1;
        desc.SampleDesc.Quality     = 0;

        if (!m_SimplePSO.Init(pDevice, &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    // GTAO�pPSO.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature         = m_RootSig.GetPtr();
        desc.VS                     = { FullScreenVS, sizeof(FullScreenVS) };
        desc.PS                     = { GTAO_PS, sizeof(GTAO_PS) };
        desc.DepthStencilState      = asdx::DEPTH_STENCIL_DESC(asdx::DEPTH_STATE_NONE);
        desc.RasterizerState        = asdx::RASTERIZER_DESC(asdx::RASTERIZER_STATE_CULL_BACK);
        desc.BlendState             = asdx::BLEND_DESC(asdx::BLEND_STATE_OPAQUE);
        desc.SampleMask             = D3D12_DEFAULT_SAMPLE_MASK;
        desc.InputLayout            = { elements, _countof(elements) };
        desc.PrimitiveTopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets       = 1;
        desc.RTVFormats[0]          = DXGI_FORMAT_R8_UNORM;
        desc.DSVFormat              = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count       = 1;
        desc.SampleDesc.Quality     = 0;

        if (!m_SsaoPSO.Init(pDevice, &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    // CrossBilateralFilter�pPSO.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature         = m_RootSig.GetPtr();
        desc.VS                     = { FullScreenVS, sizeof(FullScreenVS) };
        desc.PS                     = { CrossBilateralFilterPS, sizeof(CrossBilateralFilterPS) };
        desc.DepthStencilState      = asdx::DEPTH_STENCIL_DESC(asdx::DEPTH_STATE_NONE);
        desc.RasterizerState        = asdx::RASTERIZER_DESC(asdx::RASTERIZER_STATE_CULL_BACK);
        desc.BlendState             = asdx::BLEND_DESC(asdx::BLEND_STATE_OPAQUE);
        desc.SampleMask             = D3D12_DEFAULT_SAMPLE_MASK;
        desc.InputLayout            = { elements, _countof(elements) };
        desc.PrimitiveTopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets       = 1;
        desc.RTVFormats[0]          = DXGI_FORMAT_R8_UNORM;
        desc.DSVFormat              = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count       = 1;
        desc.SampleDesc.Quality     = 0;

        if (!m_BlurPSO.Init(pDevice, &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    // �R�s�[�pPSO.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature         = m_RootSig.GetPtr();
        desc.VS                     = { FullScreenVS, sizeof(FullScreenVS) };
        desc.PS                     = { CopyPS, sizeof(CopyPS) };
        desc.DepthStencilState      = asdx::DEPTH_STENCIL_DESC(asdx::DEPTH_STATE_NONE);
        desc.RasterizerState        = asdx::RASTERIZER_DESC(asdx::RASTERIZER_STATE_CULL_BACK);
        desc.BlendState             = asdx::BLEND_DESC(asdx::BLEND_STATE_OPAQUE);
        desc.SampleMask             = D3D12_DEFAULT_SAMPLE_MASK;
        desc.InputLayout            = { elements, _countof(elements) };
        desc.PrimitiveTopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets       = 1;
        desc.RTVFormats[0]          = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count       = 1;
        desc.SampleDesc.Quality     = 0;

        if (!m_CopyPSO.Init(pDevice, &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    {
        auto size = asdx::RoundUp(sizeof(SceneParam), 256);
        if (!m_SceneParam.Init(size))
        {
            ELOG("Error : ConstantBuffer::Init() Failed.");
            return false;
        }
    }

    {
        auto size = asdx::RoundUp(sizeof(SsaoParam), 256);
        if (!m_SsaoParam.Init(size))
        {
            ELOG("Error : ConstantBuffer::Init() Failed.");
            return false;
        }
    }

    asdx::ResModel model;
    // ���f���ǂݍ���.
    {
        asdx::MeshLoader loader;
        if (!loader.Load("../res/models/sponza.obj", model))
        {
            ELOG("Error : Model Load Failed.");
            return false;
        }
    }

    // ���f���̏�����.
    {
        auto meshCount = model.Meshes.size();
        m_Meshes.resize(meshCount);

        auto matrix = asdx::Matrix::CreateIdentity();

        for(size_t i=0; i<meshCount; ++i)
        {
            auto& mesh = model.Meshes[i];

            // ���_�o�b�t�@��������.
            {
                std::vector<MeshVertex> vertices;
                vertices.resize(mesh.Positions.size());

                auto hasTexCoord = (mesh.TexCoords[0].empty() == false);
                auto hasTangent  = (mesh.Tangents.empty() == false);

                auto vertexCount = mesh.Positions.size();
                for(size_t idx=0; idx<vertexCount; ++idx)
                {
                    vertices[idx].Position    = mesh.Positions[idx];
                    vertices[idx].Normal      = mesh.Normals[idx];
                    vertices[idx].Tangent     = (hasTangent) ? mesh.Tangents[idx] : asdx::Vector3(1.0f, 0.0f, 0.0f);
                    vertices[idx].TexCoord    = (hasTexCoord) ? mesh.TexCoords[0][idx] : asdx::Vector2(0.0f, 0.0f);
                }

                if (!m_Meshes[i].VB.Init(sizeof(MeshVertex) * vertices.size(), sizeof(MeshVertex)))
                {
                    return false;
                }

                uint8_t* ptr = m_Meshes[i].VB.Map<uint8_t>();
                if (ptr == nullptr)
                {
                    ELOG("Error : VertexBuffer::Map() Failed.");
                    return false;
                }

                memcpy(ptr, vertices.data(), sizeof(MeshVertex) * vertices.size());

                m_Meshes[i].VB.Unmap();
            }

            // �C���f�b�N�X�o�b�t�@��������.
            {
                if (!m_Meshes[i].IB.Init(sizeof(uint32_t) * mesh.Indices.size(), false))
                {
                    ELOG("Error : IndexBuffer::Init() Failed.");
                    return false;
                }

                uint8_t* ptr = m_Meshes[i].IB.Map<uint8_t>();
                if (ptr == nullptr)
                {
                    ELOG("Error : IndexBuffer::Map() Failed.");
                    return false;
                }

                memcpy(ptr, mesh.Indices.data(), sizeof(uint32_t) * mesh.Indices.size());

                m_Meshes[i].IB.Unmap();
            }

            // �萔�o�b�t�@��������.
            {
                auto size = asdx::RoundUp(sizeof(asdx::Matrix), 256);
                if (!m_Meshes[i].CB.Init(size))
                {
                    ELOG("Error : ConstantBuffer::Init() Failed.");
                    return false;
                }

                for(auto j=0; j<2; ++j)
                {
                    uint8_t* ptr = m_Meshes[i].CB.MapAs<uint8_t>(j);
                    memcpy(ptr, &matrix, sizeof(matrix));
                    m_Meshes[i].CB.Unmap(j);
                }
            }

            m_Meshes[i].IndexCount = UINT(mesh.Indices.size());
        }
    }
    // ���f���̃����������.
    model.Dispose();

    auto pos = asdx::Vector3(1000.0f, 100.0f, 0.0f);
    auto at  = asdx::Vector3(0.0f, 100.0f, 0.0f);
    auto up  = asdx::Vector3(0.0f, 1.0f, 0.0f);
    m_CameraController.Init(pos, at, up, 0.1f, 10000.0f);
    m_CameraController.SetMoveGain(0.1f);

    m_GfxCmdList.Reset();

    if (!asdx::GuiMgr::Instance().Init(
        m_GfxCmdList,
        m_hWnd,
        m_Width,
        m_Height,
        m_SwapChainFormat,
        "../res/fonts/07�₳�����S�V�b�N.ttf"))
    {
        ELOG("Error : GuiMgr::Init() Failed.");
        return false;
    }

    m_GfxCmdList.Close();

    ID3D12CommandList* lists[] = {
        m_GfxCmdList.GetCommandList()
    };

    auto queue = asdx::GetGraphicsQueue();
    queue->Execute(1, lists);
    m_WaitPoint = queue->Signal();
    queue->Sync(m_WaitPoint);

    return true;
}

//-------------------------------------------------------------------------------------------------
//      �A�v���P�[�V�����ŗL�̏I�������ł�.
//-------------------------------------------------------------------------------------------------
void App::OnTerm()
{
    asdx::GuiMgr::Instance().Term();

    for(size_t i=0; i<m_Meshes.size(); ++i)
    { 
        m_Meshes[i].VB.Term();
        m_Meshes[i].IB.Term();
        m_Meshes[i].IndexCount = 0;
    }

    m_Meshes.clear();

    m_SimplePSO.Term();
    m_SsaoPSO.Term();
    m_BlurPSO.Term();
    m_CopyPSO.Term();
    m_RootSig.Term();

    m_NormalTarget  .Term();
    m_AoTarget      .Term();
    m_BlurTarget0   .Term();
    m_BlurTarget1   .Term();
    m_SceneParam    .Term();
    m_SsaoParam     .Term();
    m_FullScreenVB  .Term();
}

//-------------------------------------------------------------------------------------------------
//      �t���[���J�ڏ����ł�.
//-------------------------------------------------------------------------------------------------
void App::OnFrameMove(asdx::FrameEventArgs& param)
{
    m_SceneParam.SwapBuffer();
    m_SsaoParam.SwapBuffer();

    auto fov        = asdx::F_PIDIV4;
    auto aspect     = float(m_Width) / float(m_Height);
    auto nearClip   = m_CameraController.GetNearClip();
    auto farClip    = m_CameraController.GetFarClip();

    // �V�[���p�����[�^�X�V.
    {
        auto ptr = m_SceneParam.MapAs<SceneParam>();

        ptr->View           = m_CameraController.GetView();
        ptr->Proj           = asdx::Matrix::CreatePerspectiveFieldOfViewReverseZ(fov, aspect, nearClip);
        ptr->InvView        = asdx::Matrix::Invert(ptr->View);
        ptr->InvProj        = asdx::Matrix::Invert(ptr->Proj);
        ptr->NearClip       = nearClip;
        ptr->FarClip        = farClip;
        ptr->AspectRatio    = aspect;
        ptr->FieldOfView    = fov;

        m_SceneParam.Unmap();
    }

    // SSAO�p�����[�^�X�V.
    {
        float radius = m_Radius;
        float tanHalfFovy = tan(fov * 0.5f);

        auto ptr = m_SsaoParam.MapAs<SsaoParam>();

        ptr->InvSize.x          = 1.0f / float(m_Width);
        ptr->InvSize.y          = 1.0f / float(m_Height);
        ptr->RadiusScreenSpace  = radius * 0.5f / tanHalfFovy * m_Height;
        ptr->RadiusViewSpace    = radius;;

        m_SsaoParam.Unmap();
    }
}

//-------------------------------------------------------------------------------------------------
//      �t���[���`�揈���ł�.
//-------------------------------------------------------------------------------------------------
void App::OnFrameRender(asdx::FrameEventArgs& param)
{
    // �R�}���h�L�^�J�n.
    m_GfxCmdList.Reset();

    auto idx  = GetCurrentBackBufferIndex();
    auto pCmd = m_GfxCmdList.GetCommandList();

    // �r���[�|�[�g��ݒ�.
    pCmd->RSSetViewports(1, &m_Viewport);
    pCmd->RSSetScissorRects(1, &m_ScissorRect);

    // ���[�g�V�O�j�`���ݒ�.
    pCmd->SetGraphicsRootSignature(m_RootSig.GetPtr());

    pCmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    // G-Buffer ����.
    {
        asdx::ScopedBarrier barrier0(
            pCmd, m_NormalTarget.GetResource(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        // �o�b�t�@���N���A.
        m_GfxCmdList.ClearDSV(m_DepthTarget.GetDSV(), m_ClearDepth);
        m_GfxCmdList.ClearRTV(m_NormalTarget.GetRTV(), clearColor);

        // �����_�[�^�[�Q�b�g�ݒ�.
        m_GfxCmdList.SetTarget(m_NormalTarget.GetRTV(), m_DepthTarget.GetDSV());

        // �p�C�v���C���X�e�[�g�ݒ�.
        m_GfxCmdList.SetPipelineState(m_SimplePSO.GetPtr());

        // �f�B�X�N���v�^�ݒ�.
        m_GfxCmdList.SetCBV(PARAM_INDEX_B1, m_SceneParam.GetView()); 

        // ���b�V����`��.
        auto count = m_Meshes.size();
        for(size_t i=0; i<count; ++i)
        {
            m_GfxCmdList.SetCBV(PARAM_INDEX_B0, m_Meshes[i].CB.GetView());

            auto vbv = m_Meshes[i].VB.GetView();
            auto ibv = m_Meshes[i].IB.GetView();
            auto indexCount = m_Meshes[i].IndexCount;
            m_GfxCmdList.SetVertexBuffers(0, 1, &vbv);
            m_GfxCmdList.SetIndexBuffer(&ibv);

            m_GfxCmdList.DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
        }
    }

    // AlchemyAO��`��.
    {
        asdx::ScopedBarrier barrier0(
            pCmd, m_DepthTarget.GetResource(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        asdx::ScopedBarrier barrier1(
            pCmd, m_AoTarget.GetResource(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        // �o�b�t�@���N���A.
        m_GfxCmdList.ClearRTV(m_AoTarget.GetRTV(), clearColor);

        // �����_�[�^�[�Q�b�g�ݒ�.
        m_GfxCmdList.SetTarget(m_AoTarget.GetRTV(), nullptr);

        // �p�C�v���C���X�e�[�g�ݒ�.
        m_GfxCmdList.SetPipelineState(m_SsaoPSO.GetPtr());

        auto vbv = m_FullScreenVB.GetView();
        m_GfxCmdList.SetVertexBuffers(0, 1, &vbv);
        m_GfxCmdList.SetIndexBuffer(nullptr);

        // �f�B�X�N���v�^�ݒ�.
        m_GfxCmdList.SetCBV(PARAM_INDEX_B1, m_SceneParam.GetView());
        m_GfxCmdList.SetCBV(PARAM_INDEX_B2, m_SsaoParam.GetView());
        m_GfxCmdList.SetTable(PARAM_INDEX_T0, m_DepthTarget.GetSRV());
        m_GfxCmdList.SetTable(PARAM_INDEX_T1, m_NormalTarget.GetSRV());
        
        // �`��L�b�N.
        m_GfxCmdList.DrawInstanced(3, 1, 0, 0);
    }

    auto blurSharpness = m_BlurSharpenss;

    //�@�����u���[.
    {
        asdx::ScopedBarrier barrier(
            pCmd, m_BlurTarget0.GetResource(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        // �����_�[�^�[�Q�b�g�ݒ�.
        m_GfxCmdList.SetTarget(m_BlurTarget0.GetRTV(), nullptr);

        // �p�C�v���C���X�e�[�g�ݒ�.
        m_GfxCmdList.SetPipelineState(m_BlurPSO.GetPtr());

        auto vbv = m_FullScreenVB.GetView();
        m_GfxCmdList.SetVertexBuffers(0, 1, &vbv);
        m_GfxCmdList.SetIndexBuffer(nullptr);

        // �f�B�X�N���v�^�ݒ�.
        float param[3] = {
            1.0f / float(m_Width),
            0.0f,
            blurSharpness,
        };
        m_GfxCmdList.SetConstants(PARAM_INDEX_B3, 3, param, 0);
        m_GfxCmdList.SetTable(PARAM_INDEX_T0, m_DepthTarget.GetSRV());
        m_GfxCmdList.SetTable(PARAM_INDEX_T1, m_AoTarget.GetSRV());

        // �`��L�b�N.
        m_GfxCmdList.DrawInstanced(3, 1, 0, 0);
    }

    //�@�����u���[.
    {
        asdx::ScopedBarrier barrier(
            pCmd, m_BlurTarget1.GetResource(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        // �����_�[�^�[�Q�b�g�ݒ�.
        m_GfxCmdList.SetTarget(m_BlurTarget1.GetRTV(), nullptr);

        // �p�C�v���C���X�e�[�g�ݒ�.
        m_GfxCmdList.SetPipelineState(m_BlurPSO.GetPtr());

        auto vbv = m_FullScreenVB.GetView();
        m_GfxCmdList.SetVertexBuffers(0, 1, &vbv);
        m_GfxCmdList.SetIndexBuffer(nullptr);

        // �f�B�X�N���v�^�ݒ�.
        float param[3] = {
            0.0f,
            1.0f / float(m_Height),
            blurSharpness
        };
        m_GfxCmdList.SetConstants(PARAM_INDEX_B3, 3, param, 0);
        m_GfxCmdList.SetTable(PARAM_INDEX_T0, m_DepthTarget.GetSRV());
        m_GfxCmdList.SetTable(PARAM_INDEX_T1, m_BlurTarget0.GetSRV());

        // �`��L�b�N.
        m_GfxCmdList.DrawInstanced(3, 1, 0, 0);
    }

    // �J���[�o�b�t�@.
    {
        asdx::ScopedBarrier barrier(
            pCmd, m_ColorTarget[idx].GetResource(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        // �o�b�t�@���N���A.
        m_GfxCmdList.ClearRTV(m_ColorTarget[idx].GetRTV(), clearColor);

        m_GfxCmdList.SetTarget(m_ColorTarget[idx].GetRTV(), nullptr);

        // �p�C�v���C���X�e�[�g�ݒ�.
        m_GfxCmdList.SetPipelineState(m_CopyPSO.GetPtr());
        m_GfxCmdList.SetTable(PARAM_INDEX_T0, m_BlurTarget1.GetSRV());

        auto vbv = m_FullScreenVB.GetView();
        m_GfxCmdList.SetVertexBuffers(0, 1, &vbv);
        m_GfxCmdList.SetIndexBuffer(nullptr);

        // �`��L�b�N.
        m_GfxCmdList.DrawInstanced(3, 1, 0, 0);

        asdx::GuiMgr::Instance().Update(m_Width, m_Height);
        ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Once);
        if (ImGui::Begin(u8"SSAO �p�����[�^"))
        {
            ImGui::DragFloat(u8"���a", &m_Radius, 0.1f, 0.0f, 1000.0f, "%.2f");
            ImGui::DragFloat(u8"�N���x", &m_BlurSharpenss, 1.0f, 0.1f, 10000.0f, "%.2f");
            ImGui::End();
        }
        asdx::GuiMgr::Instance().Draw(pCmd);
    }

    // �R�}���h�L�^�I��.
    m_GfxCmdList.Close();

    auto pQueue = asdx::GetGraphicsQueue();

    // �O�t���[���̃R�}���h���I�����Ă��Ȃ���Αҋ@.
    pQueue->Sync(m_WaitPoint);

    ID3D12CommandList* pLists[] = {
        pCmd
    };

    // �R�}���h���s.
    pQueue->Execute(_countof(pLists), pLists);

    // �ҋ@�_���L�^.
    m_WaitPoint = pQueue->Signal();
    
    // ��ʂɕ\��.
    Present( 0 );
}

//-------------------------------------------------------------------------------------------------
//      ���T�C�Y���̏����ł�.
//-------------------------------------------------------------------------------------------------
void App::OnResize(const asdx::ResizeEventArgs& param)
{
    m_NormalTarget.Resize(param.Width, param.Height);
    m_AoTarget    .Resize(param.Width, param.Height);
    m_BlurTarget0 .Resize(param.Width, param.Height);
    m_BlurTarget1 .Resize(param.Width, param.Height);
}

//-----------------------------------------------------------------------------
//      �}�E�X�̏����ł�.
//-----------------------------------------------------------------------------
void App::OnMouse(const asdx::MouseEventArgs& param)
{
    bool isAltDown = GetAsyncKeyState(VK_MENU) & 0x1;
    if (isAltDown) {
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
    else
    {
        asdx::GuiMgr::Instance().OnMouse(
            param.X,
            param.Y,
            param.WheelDelta,
            param.IsLeftButtonDown,
            param.IsMiddleButtonDown,
            param.IsRightButtonDown);
    }
}

//-----------------------------------------------------------------------------
//      �L�[�̏����ł�.
//-----------------------------------------------------------------------------
void App::OnKey(const asdx::KeyEventArgs& param)
{
    m_CameraController.OnKey(param.KeyCode, param.IsKeyDown, param.IsAltDown);
    asdx::GuiMgr::Instance().OnKey(param.IsKeyDown, param.IsAltDown, param.KeyCode);
}

//-----------------------------------------------------------------------------
//      �^�C�s���O�����ł�.
//-----------------------------------------------------------------------------
void App::OnTyping(uint32_t code)
{
    asdx::GuiMgr::Instance().OnTyping(code);
}