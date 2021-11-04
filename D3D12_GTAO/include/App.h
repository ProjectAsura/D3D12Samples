//-------------------------------------------------------------------------------------------------
// File : App.h
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <fw/asdxApp.h>
#include <fw/asdxCameraController.h>
#include <gfx/asdxTexture.h>
#include <gfx/asdxVertexBuffer.h>
#include <gfx/asdxIndexBuffer.h>
#include <gfx/asdxCommandQueue.h>
#include <gfx/asdxRootSignature.h>
#include <gfx/asdxPipelineState.h>
#include <gfx/asdxConstantBuffer.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////
class App : public asdx::Application
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    App();
    ~App();

protected:
    //=============================================================================================
    // protected variables.

    //=============================================================================================
    // protected methods.
    //=============================================================================================
    bool OnInit         () override;
    void OnTerm         () override;
    void OnFrameMove    (asdx::FrameEventArgs& param) override;
    void OnFrameRender  (asdx::FrameEventArgs& param) override;
    void OnResize       (const asdx::ResizeEventArgs& param) override;
    void OnMouse        (const asdx::MouseEventArgs& param) override;
    void OnKey          (const asdx::KeyEventArgs& param) override;
    void OnTyping       (uint32_t code) override;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    struct Mesh
    {
        asdx::VertexBuffer      VB;
        asdx::IndexBuffer       IB;
        asdx::ConstantBuffer    CB;
        UINT                    IndexCount;
    };

    std::vector<Mesh>       m_Meshes;
    asdx::RootSignature     m_RootSig;
    asdx::PipelineState     m_SimplePSO;
    asdx::PipelineState     m_SsaoPSO;
    asdx::PipelineState     m_BlurPSO;
    asdx::PipelineState     m_CopyPSO;
    asdx::ColorTarget       m_NormalTarget;
    asdx::ColorTarget       m_AoTarget;
    asdx::ColorTarget       m_BlurTarget0;
    asdx::ColorTarget       m_BlurTarget1;
    asdx::WaitPoint         m_WaitPoint;
    asdx::ConstantBuffer    m_SceneParam;
    asdx::ConstantBuffer    m_SsaoParam;
    asdx::VertexBuffer      m_FullScreenVB;
    asdx::CameraController  m_CameraController;

    float m_Radius          = 20.0f;
    float m_Intensity       = 2.0f;
    float m_Bias            = 0.0f;
    float m_BlurSharpenss   = 1.0f;
    float m_Sigma           = 2.0f;

    //=============================================================================================
    // private methods.
    //=============================================================================================

};
