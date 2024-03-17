//-----------------------------------------------------------------------------
// File : App.h
// Desc : Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fw/asdxApp.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxCommandQueue.h>
#include <gfx/asdxTarget.h>
#include <gfx/asdxTexture.h>
#include <gfx/asdxBuffer.h>
#include <gfx/asdxView.h>
#include <gfx/asdxPipelineState.h>
#include <gfx/asdxRayTracing.h>


///////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////
class App : public asdx::Application
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

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    App();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~App();

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    asdx::CommandQueue* m_pGraphicsQueue = nullptr;
    asdx::WaitPoint     m_FrameWaitPoint = {};

    asdx::RefPtr<ID3D12RootSignature>   m_GraphicsRootSig;
    asdx::RefPtr<ID3D12RootSignature>   m_ComputeRootSig;
    asdx::PipelineState                 m_GraphicsPipelineState;
    asdx::PipelineState                 m_ComputePipelineState;
    asdx::Tlas                          m_TLAS;
    asdx::Blas                          m_BLAS;
    asdx::ComputeTarget                 m_Canvas;
    asdx::Texture                       m_Background;
    asdx::ConstantBuffer                m_SceneBuffer;

    asdx::RefPtr<ID3D12Resource>            m_VB;
    asdx::RefPtr<ID3D12Resource>            m_IB;
    asdx::RefPtr<asdx::IShaderResourceView> m_VertexSRV;
    asdx::RefPtr<asdx::IShaderResourceView> m_IndexSRV;
    
    //=========================================================================
    // private methods.
    //=========================================================================
    bool OnInit() override;
    void OnTerm() override;
    void OnFrameRender(asdx::FrameEventArgs& args) override;
};
