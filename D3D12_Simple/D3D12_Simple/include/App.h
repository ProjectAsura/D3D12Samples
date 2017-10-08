//-------------------------------------------------------------------------------------------------
// File : App.h
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <Windows.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <asdxTypedef.h>
#include <asdxRef.h>


//-------------------------------------------------------------------------------------------------
// Linker
//-------------------------------------------------------------------------------------------------
#pragma comment( lib, "d3d12.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "comctl32.lib" )


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
    HINSTANCE           m_hInst;            //!< インスタンスハンドルです.
    HWND                m_hWnd;             //!< ウィンドウハンドルです.
    UINT                m_BufferCount;      //!< バッファ数です.
    DXGI_FORMAT         m_SwapChainFormat;  //!< スワップチェインのフォーマットです.
    D3D12_VIEWPORT      m_Viewport;         //!< ビューポートです.

    //=============================================================================================
    // protected methods.
    //=============================================================================================
    virtual bool OnInit         ();
    virtual void OnTerm         ();
    virtual void OnFrameMove    ();
    virtual void OnFrameRender  ();
    virtual void OnResize       ( u32 width, u32 height );

    void SetResourceBarrier( 
        ID3D12GraphicsCommandList*  pCmdList,
        ID3D12Resource*             pResource,
        D3D12_RESOURCE_STATES       stateBefore,
        D3D12_RESOURCE_STATES       stateAfter );
    void Present( u32 syncInterval );

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    asdx::RefPtr<ID3D12Device>              m_Device;                   //!< デバイスです.
    asdx::RefPtr<ID3D12CommandAllocator>    m_CmdAllocator;             //!< コマンドアロケータです.
    asdx::RefPtr<ID3D12CommandQueue>        m_CmdQueue;                 //!< コマンドキューです.
    asdx::RefPtr<ID3D12GraphicsCommandList> m_CmdList;                  //!< コマンドリストです.
    asdx::RefPtr<IDXGIAdapter>              m_Adapter;                  //!< アダプターです.
    asdx::RefPtr<IDXGIFactory4>             m_Factory;                  //!< DXGIファクトリーです.
    asdx::RefPtr<IDXGISwapChain>            m_SwapChain;                //!< スワップチェインです.
    asdx::RefPtr<ID3D12DescriptorHeap>      m_DescriptorHeap;           //!< デスクリプターヒープです.
    asdx::RefPtr<ID3D12Resource>            m_ColorTarget;              //!< カラーターゲットのリソースです.
    asdx::RefPtr<ID3D12Fence>               m_Fence;                    //!< フェンスです.
    D3D12_CPU_DESCRIPTOR_HANDLE             m_ColorTargetHandle;        //!< カラーターゲットのハンドルです.
    HANDLE                                  m_EventHandle;              //!< イベントハンドルです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    bool InitApp ();
    void TermApp ();
    bool InitWnd ();
    void TermWnd ();
    bool InitD3D ();
    void TermD3D ();
    void MainLoop();

    static LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);
};
