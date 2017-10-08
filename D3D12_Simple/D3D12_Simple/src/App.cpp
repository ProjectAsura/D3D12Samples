//-------------------------------------------------------------------------------------------------
// File : App.cpp
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <App.h>
#include <cstdio>
#include <array>


#ifndef DLOG
#if defined(DEBUG) || defined(_DEBUG)
#define DLOG(x, ...)       printf_s("[File: %s, Line:%d] "x"\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DLOG(x, ...)        ((void)0)
#endif
#endif//DLOG

#ifndef ELOG
#define ELOG(x, ...)        fprintf_s(stderr, "[File:%s, Line:%d]"x"\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG

#ifndef ASDX_WND_CLASSNAME
#define ASDX_WND_CLASSNAME      TEXT("asdxWindowClass")
#endif//ASDX_WND_CLASSNAME


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
// Global Variables.
//-------------------------------------------------------------------------------------------------
App*    g_pApp = nullptr;


} // namespace /* anonymous */


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
App::App()
: m_hInst           ( nullptr )
, m_hWnd            ( nullptr )
, m_BufferCount     ( 2 )
, m_SwapChainFormat ( DXGI_FORMAT_R8G8B8A8_UNORM )  // SRGBだとエラーが発生したので暫定的に...
, m_EventHandle     ( nullptr )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
App::~App()
{ TermApp(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool App::InitApp()
{
    // COMライブラリの初期化.
    HRESULT hr = CoInitialize( nullptr );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : Com Library Initialize Failed." );
        return false;
    }

    // ウィンドウの初期化.
    if ( !InitWnd() )
    {
        ELOG( "Error : InitWnd() Failed." );
        return false;
    }

    // D3D12の初期化.
    if ( !InitD3D() )
    {
        ELOG( "Error : InitD3D() Failed." );
        return false;
    }

    // アプリケーション固有の初期化.
    if ( !OnInit() )
    {
        ELOG( "Error : OnInit() Failed." );
        return false;
    }

    // ポインタ設定.
    g_pApp = this;

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウの初期化処理です.
//-------------------------------------------------------------------------------------------------
bool App::InitWnd()
{
    // インスタンスハンドルを取得.
    HINSTANCE hInst = GetModuleHandle( nullptr );
    if ( !hInst )
    {
        ELOG( "Error : GetModuleHandle() Failed." );
        return false;
    }

    // 拡張ウィンドウクラスの設定.
    WNDCLASSEXW wc;
    wc.cbSize           = sizeof(WNDCLASSEXW);
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MsgProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = LoadIcon( hInst, IDI_APPLICATION );
    wc.hCursor          = LoadCursor( nullptr, IDC_ARROW );
    wc.hbrBackground    = (HBRUSH)( COLOR_WINDOW + 1 );
    wc.lpszMenuName     = nullptr;
    wc.lpszClassName    = ASDX_WND_CLASSNAME;
    wc.hIconSm          = LoadIcon( hInst, IDI_APPLICATION );

    // 拡張ウィンドウクラスを登録.
    if ( !RegisterClassExW( &wc ) )
    {
        ELOG( "Error : RegisterClassEx() Failed." );
        return false;
    }

    // インスタンスハンドルを設定.
    m_hInst = hInst;

    RECT rc = { 0, 0, 960, 540 };

    // ウィンドウの矩形を調整.
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
    AdjustWindowRect( &rc, style, FALSE );

    // ウィンドウを生成.
    m_hWnd = CreateWindowW(
        ASDX_WND_CLASSNAME,
        TEXT("SimpleSample"),
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (rc.right - rc.left),
        (rc.bottom - rc.top),
        nullptr,
        nullptr,
        m_hInst,
        nullptr );

    // エラーチェック.
    if ( !m_hWnd )
    {
        ELOG( "Error : CreateWindowW() Failed." );
        return false;
    }

    // ウィンドウを表示
    ShowWindow( m_hWnd, SW_SHOWNORMAL );
    UpdateWindow( m_hWnd );

    // フォーカス設定.
    SetFocus( m_hWnd );

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      D3D12の初期化処理です.
//-------------------------------------------------------------------------------------------------
bool App::InitD3D()
{
    HRESULT hr = S_OK;

    // ウィンドウ幅を取得.
    RECT rc;
    GetClientRect( m_hWnd, &rc );
    u32 w = rc.right - rc.left;
    u32 h = rc.bottom - rc.top;

    UINT flags = 0;

#if defined(DEBUG) || defined(_DEBUG)
    ID3D12Debug* pDebug;
    D3D12GetDebugInterface(IID_ID3D12Debug, (void**)&pDebug);
    if (pDebug)
    {
        pDebug->EnableDebugLayer();
        pDebug->Release();
    }
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    hr = CreateDXGIFactory2(flags, IID_IDXGIFactory4, (void**)m_Factory.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : CreateDXGIFactory() Failed.");
        return false;
    }

    hr = m_Factory->EnumAdapters(0, m_Adapter.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : IDXGIFactory::EnumAdapters() Failed.");
        return false;
    }

    // デバイス生成.
    hr = D3D12CreateDevice(
        m_Adapter.GetPtr(),
        D3D_FEATURE_LEVEL_11_0,
        IID_ID3D12Device,
        (void**)m_Device.GetAddress() );

    // 生成チェック.
    if ( FAILED( hr ) )
    {
        // Warpアダプターで再トライ.
        m_Adapter.Reset();
        m_Device.Reset();

        hr = m_Factory->EnumWarpAdapter(IID_PPV_ARGS(m_Adapter.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : IDXGIFactory::EnumWarpAdapter() Failed.");
            return false;
        }

        // デバイス生成.
        hr = D3D12CreateDevice(
            m_Adapter.GetPtr(),
            D3D_FEATURE_LEVEL_11_0,
            IID_ID3D12Device,
            (void**)m_Device.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error: D3D12CreateDevice() Failed.");
            return false;
        }
    }

    // コマンドアロケータを生成.
    hr = m_Device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, 
        IID_ID3D12CommandAllocator,
        (void**)m_CmdAllocator.GetAddress() );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommandAllocator() Failed." );
        return false;
    }

    // コマンドキューを生成.
    {
       D3D12_COMMAND_QUEUE_DESC desc;
       ZeroMemory( &desc, sizeof(desc) );
       desc.Type        = D3D12_COMMAND_LIST_TYPE_DIRECT;
       desc.Priority    = 0;
       desc.Flags       = D3D12_COMMAND_QUEUE_FLAG_NONE;

       hr = m_Device->CreateCommandQueue( &desc, IID_ID3D12CommandQueue, (void**)m_CmdQueue.GetAddress() );
       if ( FAILED( hr ) )
       {
           ELOG( "Error : ID3D12Device::CreateCommandQueue() Failed." );
           return false;
       }
    }

    // スワップチェインを生成.
    {
        DXGI_SWAP_CHAIN_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.BufferCount                        = m_BufferCount;
        desc.BufferDesc.Format                  = m_SwapChainFormat;
        desc.BufferDesc.Width                   = w;
        desc.BufferDesc.Height                  = h;
        desc.BufferDesc.RefreshRate.Numerator   = 60;
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        desc.OutputWindow                       = m_hWnd;
        desc.SampleDesc.Count                   = 1;
        desc.SampleDesc.Quality                 = 0;
        desc.Windowed                           = TRUE;
        desc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        desc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // アダプター単位の処理にマッチするのは m_Device ではなく m_CmdQueue　なので，m_CmdQueue　を第一引数として渡す.
        hr = m_Factory->CreateSwapChain( m_CmdQueue.GetPtr(), &desc, m_SwapChain.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : IDXGIFactory::CreateSwapChain() Failed." );
            return false;
        }
    }

    // デスクリプタヒープの生成.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );

        desc.NumDescriptors = 1;
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        hr = m_Device->CreateDescriptorHeap( &desc, IID_ID3D12DescriptorHeap, (void**)m_DescriptorHeap.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateDescriptorHeap() Failed." );
            return false;
        }
    }

    // コマンドリストの生成.
    {
        hr = m_Device->CreateCommandList(
            1,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_CmdAllocator.GetPtr(),
            nullptr,
            IID_ID3D12GraphicsCommandList,
            (void**)m_CmdList.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommandList() Failed." );
            return false;
        }
    }

    // バックバッファからレンダーターゲットを生成.
    {
        hr = m_SwapChain->GetBuffer( 0, IID_ID3D12Resource, (void**)m_ColorTarget.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : IDXGISwapChain::GetBuffer() Failed." );
            return false;
        }

        m_ColorTargetHandle = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        m_Device->CreateRenderTargetView( m_ColorTarget.GetPtr(), nullptr, m_ColorTargetHandle );
    }

    // フェンスの生成.
    {
        m_EventHandle = CreateEvent( 0, FALSE, FALSE, 0 );

        hr = m_Device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_ID3D12Fence, (void**)m_Fence.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateFence() Failed." );
            return false;
        }
    }

    // ビューポートの設定.
    {
        m_Viewport.TopLeftX = 0;
        m_Viewport.TopLeftY = 0;
        m_Viewport.Width    = FLOAT(w);
        m_Viewport.Height   = FLOAT(h);
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void App::TermApp()
{
    // アプリケーション固有の終了処理.
    OnTerm();

    // D3D12の終了処理.
    TermD3D();

    // ウィンドウの終了処理.
    TermWnd();

    // COMライブラリの終了処理.
    CoUninitialize();

    // ポインタクリア.
    g_pApp = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウの終了処理.
//-------------------------------------------------------------------------------------------------
void App::TermWnd()
{
    if ( m_hInst != nullptr )
    { UnregisterClassW( ASDX_WND_CLASSNAME, m_hInst ); }

    m_hInst = nullptr;
    m_hWnd  = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      D3D12の終了処理.
//-------------------------------------------------------------------------------------------------
void App::TermD3D()
{
    CloseHandle( m_EventHandle );

    m_EventHandle = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      メインループです.
//-------------------------------------------------------------------------------------------------
void App::MainLoop()
{
    MSG msg = { 0 };

    while( WM_QUIT != msg.message )
    {
        auto gotMsg = PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE );

        if ( gotMsg )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            OnFrameMove();
            OnFrameRender();
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      アプリケーションを実行します.
//-------------------------------------------------------------------------------------------------
void App::Run()
{
    // 初期化に成功したらメインループに入る.
    if ( InitApp() )
    { MainLoop(); }

    // 終了処理.
    TermApp();
}

//-------------------------------------------------------------------------------------------------
//      アプリケーション固有の初期化処理です.
//-------------------------------------------------------------------------------------------------
bool App::OnInit()
{
    /* DO_NOTHING */

    return true;
}

//-------------------------------------------------------------------------------------------------
//      アプリケーション固有の終了処理です.
//-------------------------------------------------------------------------------------------------
void App::OnTerm()
{
    /* DO_NOTHING */
}

//-------------------------------------------------------------------------------------------------
//      フレーム遷移処理です.
//-------------------------------------------------------------------------------------------------
void App::OnFrameMove()
{
    /* DO_NOTHING */
}

//-------------------------------------------------------------------------------------------------
//      フレーム描画処理です.
//-------------------------------------------------------------------------------------------------
void App::OnFrameRender()
{
    // ビューポートを設定.
    m_CmdList->RSSetViewports( 1, &m_Viewport );
    SetResourceBarrier( m_CmdList.GetPtr(), m_ColorTarget.GetPtr(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // カラーバッファをクリア.
    float clearColor[] = { 0.39f, 0.58f, 0.92f, 1.0f };
    m_CmdList->ClearRenderTargetView( m_ColorTargetHandle, clearColor, 0, nullptr );
    SetResourceBarrier( m_CmdList.GetPtr(), m_ColorTarget.GetPtr(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    // 画面に表示.
    Present( 0 );
}

//-------------------------------------------------------------------------------------------------
//      リサイズ時の処理です.
//-------------------------------------------------------------------------------------------------
void App::OnResize( u32 width, u32 height )
{
    m_Viewport.Width  = FLOAT( width );
    m_Viewport.Height = FLOAT( height );

    // レンダーターゲットを破棄.
    m_ColorTarget.Reset();
    m_ColorTargetHandle.ptr = 0;

    // バックバッファをリサイズ.
    HRESULT hr = m_SwapChain->ResizeBuffers( m_BufferCount, 0, 0, m_SwapChainFormat, 0 );
    if ( FAILED( hr ) )
    { ELOG( "Error : IDXGISwapChain::ResizeBuffer() Failed." ); }

    // バックバッファを取得.
    hr = m_SwapChain->GetBuffer( 0, IID_ID3D12Resource, (void**)m_ColorTarget.GetAddress() );
    if ( FAILED( hr ) )
    { ELOG( "Error : IDXGISwapChain::GetBuffer() Failed." ); }

    // レンダーターゲットを生成.
    m_ColorTargetHandle = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_Device->CreateRenderTargetView( m_ColorTarget.GetPtr(), nullptr, m_ColorTargetHandle );
}

//-------------------------------------------------------------------------------------------------
//      リソースバリアの設定.
//-------------------------------------------------------------------------------------------------
void App::SetResourceBarrier
(
    ID3D12GraphicsCommandList* pCmdList,
    ID3D12Resource* pResource,
    D3D12_RESOURCE_STATES stateBefore,
    D3D12_RESOURCE_STATES stateAfter
)
{
    D3D12_RESOURCE_BARRIER desc = {};
    desc.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    desc.Transition.pResource   = pResource;
    desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    desc.Transition.StateBefore = stateBefore;
    desc.Transition.StateAfter  = stateAfter;

    pCmdList->ResourceBarrier( 1, &desc );
}

//-------------------------------------------------------------------------------------------------
//      コマンドを実行して画面に表示します.
//-------------------------------------------------------------------------------------------------
void App::Present( u32 syncInterval )
{
    ID3D12CommandList* cmdList = m_CmdList.GetPtr();

    // コマンドリストへの記録を終了し，コマンド実行.
    m_CmdList->Close();
    m_CmdQueue->ExecuteCommandLists( 1, &cmdList );

    // コマンドの実行の終了を待機する
    m_Fence->Signal( 0 );
    m_Fence->SetEventOnCompletion( 1, m_EventHandle );
    m_CmdQueue->Signal( m_Fence.GetPtr(), 1 );
    WaitForSingleObject( m_EventHandle, INFINITE );

    // 画面に表示する.
    m_SwapChain->Present( syncInterval, 0 );

    // コマンドリストとコマンドアロケータをリセットする.
    m_CmdAllocator->Reset();
    m_CmdList->Reset( m_CmdAllocator.GetPtr(), nullptr );
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウプロシージャです.
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK App::MsgProc( HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp )
{
    switch( uMsg )
    {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint( hWnd, &ps );
                EndPaint( hWnd, &ps );
            }
            break;

        case WM_DESTROY:
            {
                PostQuitMessage( 0 );
            }
            break;

        case WM_SIZE:
            {
                if ( g_pApp != nullptr )
                {
                    u32 w = LOWORD( lp );
                    u32 h = HIWORD( lp );
                    g_pApp->OnResize( w, h );
                }
            }
    }

    return DefWindowProc( hWnd, uMsg, wp, lp );
}