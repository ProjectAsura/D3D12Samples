//-------------------------------------------------------------------------------------------------
// File : App.h
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxDesktopApp.h>
#include <asdxDevice.h>
#include <asdxDeviceContext.h>
#include <asdxMath.h>
#include <asdxMisc.h>
#include <asdxTarget.h>
#include <asdxStopWatch.h>
#include <asdxMotionPlayer.h>
#include "Model.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////
class App : public asdx::DesktopApp
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
    virtual ~App();

private:
    struct TransformParam
    {
        asdx::Matrix    World;
        asdx::Matrix    View;
        asdx::Matrix    Proj;
    };

    //=============================================================================================
    // private variables.
    //=============================================================================================
    static const u32 BufferCount = 2;
    asdx::Device                        m_Device;
    asdx::DeviceContext                 m_DeviceContext;
    asdx::RefPtr<IDXGISwapChain3>       m_SwapChain;
    u32                                 m_FrameIndex;
    asdx::ColorTarget                   m_ColorTarget[2];
    asdx::DepthTarget                   m_DepthTarget;
    D3D12_VIEWPORT                      m_Viewport;
    D3D12_RECT                          m_ScissorRect;
    Model                               m_Model;
    asdx::ResMotion                     m_Motion;
    asdx::MotionPlayer                  m_MotionPlayer;
    asdx::RefPtr<ID3D12RootSignature>   m_RootSignature;
    asdx::RefPtr<ID3D12PipelineState>   m_PSO;
    asdx::GraphicsCommandList           m_Bundle;
    asdx::ConstantBuffer                m_TransformCB;
    TransformParam                      m_TransformParam;
    asdx::DescHandle                    m_TransformHandle;
    bool                                m_IsPlay;
    asdx::StopWatch                     m_StopWatch;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    bool OnInit() override;
    void OnTerm() override;
    void OnFrameMove(const asdx::FrameEventArgs& args) override;
    void OnFrameRender(const asdx::FrameEventArgs& args) override;
    void OnResize(const asdx::ResizeEventArgs& args) override;
    void OnKey(const asdx::KeyEventArgs& args) override;

    bool InitModel();
    void TermModel();
};
