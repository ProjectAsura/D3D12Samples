//-----------------------------------------------------------------------------
// File : SampleApp.h
// Desc : Sample Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fw/asdxApp.h>
#include <gfx/asdxRayTracing.h>
#include <gfx/asdxGraphicsSystem.h>
#include <gfx/asdxTexture.h>
#include <gfx/asdxConstantBuffer.h>
#include <gfx/asdxRootSignature.h>
#include <gfx/asdxCommandQueue.h>


///////////////////////////////////////////////////////////////////////////////
// SampleApp class
///////////////////////////////////////////////////////////////////////////////
class SampleApp : public asdx::Application
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
    SampleApp();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~SampleApp();

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    asdx::CommandQueue* m_pGraphicsQueue = nullptr;
    asdx::WaitPoint     m_FrameWaitPoint = {};

    asdx::RootSignature                     m_GlobalRootSig;
    asdx::RootSignature                     m_LocalRootSig;
    asdx::RayTracingPipelineState           m_RayTracingPSO;
    asdx::RefPtr<ID3D12Resource>            m_VB;
    asdx::RefPtr<ID3D12Resource>            m_IB;
    asdx::Tlas                              m_TLAS;
    asdx::Blas                              m_BLAS;
    asdx::ComputeTarget                     m_Canvas;
    asdx::Texture                           m_BackGround;
    asdx::ShaderTable                       m_RayGenTable;
    asdx::ShaderTable                       m_MissTable;
    asdx::ShaderTable                       m_HitGroupTable;
    asdx::ConstantBuffer                    m_SceneCB;
    asdx::RefPtr<asdx::IShaderResourceView> m_VertexSRV;
    asdx::RefPtr<asdx::IShaderResourceView> m_IndexSRV;
    bool                                    m_ConstructAS = false;
    asdx::Texture                           m_BaseColor[2];

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool OnInit() override;

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void OnTerm() override;

    //-------------------------------------------------------------------------
    //! @brief      フレーム描画時の処理です.
    //-------------------------------------------------------------------------
    void OnFrameRender(asdx::FrameEventArgs& args) override;

    //-------------------------------------------------------------------------
    //! @brief      リサイズ時の処理です.
    //-------------------------------------------------------------------------
    void OnResize(const asdx::ResizeEventArgs& args) override;

    //-------------------------------------------------------------------------
    //! @brief      キー処理です.
    //-------------------------------------------------------------------------
    void OnKey(const asdx::KeyEventArgs& args) override;

    //-------------------------------------------------------------------------
    //! @brief      マウス処理です.
    //-------------------------------------------------------------------------
    void OnMouse(const asdx::MouseEventArgs& args) override;

    //-------------------------------------------------------------------------
    //! @brief      タイピング処理です.
    //-------------------------------------------------------------------------
    void OnTyping(uint32_t keyCode) override;
};