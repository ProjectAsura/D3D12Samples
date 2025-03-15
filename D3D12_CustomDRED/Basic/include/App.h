//-------------------------------------------------------------------------------------------------
// File : App.h
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <asdxRef.h>
#include <asdxMath.h>
#include <asdxResTGA.h>
#include <CustomDred.h>

//-------------------------------------------------------------------------------------------------
// Linker
//-------------------------------------------------------------------------------------------------
#pragma comment( lib, "d3d12.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3dcompiler.lib" )


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////
class App
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
    void Run();

protected:
    //=============================================================================================
    // protected variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // protected methods.
    //=============================================================================================
    virtual bool OnInit();
    virtual void OnTerm();
    virtual void OnRender(FLOAT elapsedSec);

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    static const UINT BufferCount = 2;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ResConstantBuffer structure
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ASDX_ALIGN(256)             // 定数バッファは 256 byte アライメント必須.
    struct ResConstantBuffer
    {
        asdx::Matrix   World;
        asdx::Matrix   View;
        asdx::Matrix   Proj;
    };

    HINSTANCE                                m_hInst;
    HWND                                     m_hWnd;
    UINT                                     m_Width;
    UINT                                     m_Height;
    FLOAT                                    m_AspectRatio;
    D3D12_VIEWPORT                           m_Viewport;
    D3D12_RECT                               m_ScissorRect;
    asdx::RefPtr<IDXGISwapChain3>            m_pSwapChain;
    asdx::RefPtr<ID3D12Device3>              m_pDevice;
    asdx::RefPtr<ID3D12Resource>             m_pRenderTarget[BufferCount];
    asdx::RefPtr<ID3D12Resource>             m_pDepthStencil;
    asdx::RefPtr<ID3D12CommandAllocator>     m_pCmdAllocator;
    asdx::RefPtr<ID3D12CommandQueue>         m_pCmdQueue;
    asdx::RefPtr<ID3D12RootSignature>        m_pRootSignature;
    asdx::RefPtr<ID3D12DescriptorHeap>       m_pRtvHeap;
    asdx::RefPtr<ID3D12DescriptorHeap>       m_pDsvHeap;
    asdx::RefPtr<ID3D12DescriptorHeap>       m_pCbvSrvHeap;
    asdx::RefPtr<ID3D12PipelineState>        m_pPipelineState;
    //asdx::RefPtr<ID3D12GraphicsCommandList>  m_pCmdList;
    asdx::RefPtr<IGraphicsCommandList>       m_pCmdList;
    asdx::RefPtr<ID3D12Resource>             m_pVertexBuffer;
    asdx::RefPtr<ID3D12Resource>             m_pConstantBuffer;
    asdx::RefPtr<ID3D12Resource>             m_pTexture;
    asdx::RefPtr<ID3D12Fence>                m_pFence;
    D3D12_VERTEX_BUFFER_VIEW                 m_VertexBufferView;
    UINT                                     m_FrameIndex;
    HANDLE                                   m_FenceEvent;
    UINT64                                   m_FenceValue;
    UINT                                     m_RtvDescriptorSize;
    UINT                                     m_DsvDescriptorSize;
    UINT                                     m_CbvSrvDescriptorSize;
    ResConstantBuffer                        m_ConstantBufferData;
    UINT8*                                   m_pCbvDataBegin;
    FLOAT                                    m_RotateAngle;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    bool InitWnd();
    void TermWnd();
    bool InitD3D();
    void TermD3D();
    bool InitApp();
    void TermApp();
    void MainLoop();
    void WaitForGpu();
 
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};
