﻿//-----------------------------------------------------------------------------
// File : SampleApp.h
// Desc : Sample Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxApp.h>
#include <asdxCameraController.h>
#include <asdxModel.h>
#include <asdxRootSignature.h>
#include <asdxPipelineState.h>
#include <asdxConstantBuffer.h>


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
    //! @brief
    //-------------------------------------------------------------------------
    SampleApp();

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    ~SampleApp();

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    asdx::CommandQueue*     m_pGraphicsQueue = nullptr;
    asdx::WaitPoint         m_FrameWaitPoint;
    asdx::CameraController  m_CameraController;
    asdx::Model             m_Model;
    asdx::RootSignature     m_RootSig;
    asdx::PipelineState     m_PSO;
    asdx::ConstantBuffer    m_MeshBuffer;
    asdx::ConstantBuffer    m_SceneBuffer;
    asdx::ConstantBuffer    m_LightingBuffer;

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    bool OnInit() override;

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnTerm() override;

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnFrameRender(asdx::FrameEventArgs& param) override;

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnResize(const asdx::ResizeEventArgs& param) override;

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnMouse(const asdx::MouseEventArgs& param) override;

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnKey(const asdx::KeyEventArgs& param) override;

    //-------------------------------------------------------------------------
    //! @brief
    //-------------------------------------------------------------------------
    void OnTyping(uint32_t keyCode) override;

};