//-------------------------------------------------------------------------------------------------
// File : App.cpp
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdio>
#include <string>
#include <shlwapi.h>
#include <asdxTimer.h>
#include <asdxResTga.h>
#include <TextureHelper.h>
#include <App.h>
#include <DredReport.h>

//-------------------------------------------------------------------------------------------------
// Linker
//-------------------------------------------------------------------------------------------------
#pragma comment( lib, "shlwapi.lib" )


#ifndef ELOG
#define ELOG( x, ... )  fprintf_s( stderr, "[File: %s, Line:%d] " x, __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
const wchar_t AppClassName[] = L"SimpleSample";


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    asdx::Vector3 Position;     //!< 位置座標です.
    asdx::Vector3 Normal;       //!< 法線ベクトルです.
    asdx::Vector2 TexCoord;     //!< テクスチャ座標です.
    asdx::Vector4 Color;        //!< 頂点カラーです.
};

//-------------------------------------------------------------------------------------------------
//      ファイルパスを検索します.
//-------------------------------------------------------------------------------------------------
bool SearchFilePath( const char16* filePath, std::wstring& result )
{
    if ( filePath == nullptr )
    { return false; }

    if ( wcscmp( filePath, L" " ) == 0 || wcscmp( filePath, L"" ) == 0 )
    { return false; }

    char16 exePath[ 520 ] = { 0 };
    GetModuleFileNameW( nullptr, exePath, 520  );
    exePath[ 519 ] = 0; // null終端化.
    PathRemoveFileSpecW( exePath );

    char16 dstPath[ 520 ] = { 0 };

    wcscpy_s( dstPath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"..\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"..\\..\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"\\res\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\..\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\..\\..\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\res\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    return false;
}

} // namespace /* anonymous */


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
App::App()
: m_Width (960)
, m_Height(540)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
App::~App()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      アプリケーションを実行します.
//-------------------------------------------------------------------------------------------------
void App::Run()
{
    if (OnInit())
    { MainLoop(); }

    OnTerm();
}

//-------------------------------------------------------------------------------------------------
//      初期化時の処理です.
//-------------------------------------------------------------------------------------------------
bool App::OnInit()
{
    if (!InitWnd())
    {
        ELOG( "Error : InitWnd() Failed." );
        return false;
    }

    if (!InitD3D())
    {
        ELOG( "Error : InitD3D() Failed." );
        return false;
    }

    if (!InitApp())
    {
        ELOG( "Error : InitApp() Failed." );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了時の処理です.
//-------------------------------------------------------------------------------------------------
void App::OnTerm()
{
    TermApp();
    TermD3D();
    TermWnd();
}

//-------------------------------------------------------------------------------------------------
//      メインループです.
//-------------------------------------------------------------------------------------------------
void App::MainLoop()
{
    MSG msg = { 0 };

    asdx::Timer timer;
    timer.Start();

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
            OnRender(static_cast<FLOAT>(timer.GetElapsedTime()));
        }
    }
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
    wc.lpfnWndProc      = WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = LoadIcon( hInst, IDI_APPLICATION );
    wc.hCursor          = LoadCursor( nullptr, IDC_ARROW );
    wc.hbrBackground    = (HBRUSH)( COLOR_WINDOW + 1 );
    wc.lpszMenuName     = nullptr;
    wc.lpszClassName    = AppClassName;
    wc.hIconSm          = LoadIcon( hInst, IDI_APPLICATION );

    // 拡張ウィンドウクラスを登録.
    if ( !RegisterClassExW( &wc ) )
    {
        ELOG( "Error : RegisterClassEx() Failed." );
        return false;
    }

    // インスタンスハンドルを設定.
    m_hInst = hInst;

    RECT rc = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };

    // ウィンドウの矩形を調整.
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRect( &rc, style, FALSE );

    // ウィンドウを生成.
    m_hWnd = CreateWindowW(
        AppClassName,
        TEXT("SimpleSample"),
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (rc.right - rc.left),
        (rc.bottom - rc.top),
        nullptr,
        nullptr,
        m_hInst,
        this );

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
//      ウィンドウの終了処理.
//-------------------------------------------------------------------------------------------------
void App::TermWnd()
{
    if ( m_hInst != nullptr )
    { UnregisterClassW( AppClassName, m_hInst ); }

    m_hInst = nullptr;
    m_hWnd  = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      D3D12の初期化処理です.
//-------------------------------------------------------------------------------------------------
bool App::InitD3D()
{
    HRESULT hr = S_OK;
#if defined(DEBUG) || defined(_DEBUG)
    {
        asdx::RefPtr<ID3D12Debug> pDebug;
        hr = D3D12GetDebugInterface( IID_PPV_ARGS(pDebug.GetAddress()) );
        if ( SUCCEEDED( hr ) )
        {
            pDebug->EnableDebugLayer();
        }
    }
#endif

    {
        asdx::RefPtr<ID3D12DeviceRemovedExtendedDataSettings1> dredSettings1;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(dredSettings1.GetAddress()));
        if (SUCCEEDED(hr))
        {
            dredSettings1->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            dredSettings1->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            dredSettings1->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            dredSettings1->SetWatsonDumpEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
        }
        else
        {
            asdx::RefPtr<ID3D12DeviceRemovedExtendedDataSettings> dredSettings;
            hr = D3D12GetDebugInterface(IID_PPV_ARGS(dredSettings.GetAddress()));
            if (SUCCEEDED(hr))
            {
                dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                dredSettings->SetWatsonDumpEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            }
        }
    }

    asdx::RefPtr<IDXGIFactory4> pFactory;
    hr = CreateDXGIFactory1( IID_PPV_ARGS( pFactory.GetAddress() ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : CreateDXGIFactory() Failed." );
        return false;
    }

    // デバイスを生成.
    hr = D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( m_pDevice.GetAddress() ) );
    if ( FAILED( hr ))
    {
        ELOG( "Error : D3D12CreateDevice() Failed." );
        return false;
    }

    // コマンドキューを生成.
    {
        D3D12_COMMAND_QUEUE_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;

        hr = m_pDevice->CreateCommandQueue( &desc, IID_PPV_ARGS( m_pCmdQueue.GetAddress() ) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommandQueue() Failed." );
            return false;
        }
    }

    // スワップチェインを生成.
    {
        // スワップチェインの設定.
        DXGI_SWAP_CHAIN_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.BufferCount        = BufferCount;
        desc.BufferDesc.Width   = m_Width;
        desc.BufferDesc.Height  = m_Height;
        desc.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.OutputWindow       = m_hWnd;
        desc.SampleDesc.Count   = 1;
        desc.Windowed           = TRUE;

        // スワップチェインを生成.
        asdx::RefPtr<IDXGISwapChain> pSwapChain;
        hr = pFactory->CreateSwapChain( m_pCmdQueue.GetPtr(), &desc, pSwapChain.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : IDXGIFactory::CreateSwapChain() Failed." );
            return false;
        }

        // IDXGISwapChain3にする.
        hr = pSwapChain->QueryInterface( IID_PPV_ARGS( m_pSwapChain.GetAddress() ) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : IDXGISwapChain::QueryInterface() Failed." );
            return false;
        }

        // フレームバッファ番号を設定.
        m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
    }

    // レンダーターゲットビュー・深度ステンシルビュー用ディスクリプターヒープを生成.
    {
        // レンダーターゲットビュー用ディスクリプタヒープの設定.
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.NumDescriptors = BufferCount;
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        // レンダーターゲットビュー用ディスクリプタヒープを生成.
        hr = m_pDevice->CreateDescriptorHeap( &desc, IID_PPV_ARGS( m_pRtvHeap.GetAddress() ) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateDescriptorHeap() Failed." );
            return false;
        }

        // レンダーターゲットビューのディスクリプタサイズを取得.
        m_RtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // 深度ステンシルビュー用ディスクリプタヒープの設定.
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        // 深度ステンシルビュー用ディスクリプタヒープを生成.
        hr = m_pDevice->CreateDescriptorHeap( &desc, IID_PPV_ARGS( m_pDsvHeap.GetAddress() ) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateDescriptorHeap() Failed." );
            return false;
        }

        // 深度ステンシルビューのディスクリプタサイズを取得.
        m_DsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    // レンダーターゲットビューを生成.
    {
        // CPUハンドルの先頭を取得.
        auto handle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();

        // フレームバッファ数分ループさせる.
        for( UINT i=0; i<BufferCount; ++i )
        {
            // バックバッファ取得.
            hr = m_pSwapChain->GetBuffer( i, IID_PPV_ARGS( m_pRenderTarget[i].GetAddress() ) );
            if ( FAILED( hr ) )
            {
                ELOG( "Error : IDXGISwapChain3::GetBuffer() Failed." );
                return false;
            }

            // レンダーターゲットビューを生成.
            m_pDevice->CreateRenderTargetView( m_pRenderTarget[i].GetPtr(), nullptr, handle );

            // ハンドルのポインタを進める.
            handle.ptr += m_RtvDescriptorSize;
        }
    }

    // 深度ステンシルビューを生成.
    {
        // ヒーププロパティの設定.
        D3D12_HEAP_PROPERTIES prop;
        prop.Type                   = D3D12_HEAP_TYPE_DEFAULT;
        prop.CPUPageProperty        = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference   = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask       = 1;
        prop.VisibleNodeMask        = 1;

        // リソースの設定.
        D3D12_RESOURCE_DESC desc;
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Alignment          = 0;
        desc.Width              = m_Width;
        desc.Height             = m_Height;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 0;
        desc.Format             = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;

        // クリア値の設定.
        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format               = DXGI_FORMAT_D32_FLOAT;
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        // リソースを生成.
        hr = m_pDevice->CreateCommittedResource( 
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(m_pDepthStencil.GetAddress()) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed." );
            return false;
        }

        // 深度ステンシルビューの設定.
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format        = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Flags         = D3D12_DSV_FLAG_NONE;

        // 深度ステンシルビューを生成.
        m_pDevice->CreateDepthStencilView(
            m_pDepthStencil.GetPtr(),
            &dsvDesc,
            m_pDsvHeap->GetCPUDescriptorHandleForHeapStart() );
    }

    // コマンドアロケータを生成.
    hr = m_pDevice->CreateCommandAllocator( 
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(m_pCmdAllocator.GetAddress()));
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommandAllocator() Failed." );
        return false;
    }

    // コマンドリストを生成.
    hr = m_pDevice->CreateCommandList(
        0, 
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_pCmdAllocator.GetPtr(),
        nullptr,
        IID_PPV_ARGS(m_pCmdList.GetAddress()) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommandList() Failed." );
        return false;
    }

    // フェンスを生成.
    hr = m_pDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pFence.GetAddress()) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateFence() Failed." );
        return false;
    }

    // フェンス用イベントを生成.
    m_FenceValue = 1;
    m_FenceEvent = CreateEventEx( nullptr, FALSE, FALSE, EVENT_ALL_ACCESS );
    if ( m_FenceEvent == nullptr )
    {
        ELOG( "Error : CreateEventEx() Failed." );
        return false;
    }

    // ビューポートを設定.
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.Width    = static_cast<FLOAT>( m_Width );
    m_Viewport.Height   = static_cast<FLOAT>( m_Height );
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    // シザー矩形を設定.
    m_ScissorRect.left   = 0;
    m_ScissorRect.right  = m_Width;
    m_ScissorRect.top    = 0;
    m_ScissorRect.bottom = m_Height;

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      D3D12の終了処理です.
//-------------------------------------------------------------------------------------------------
void App::TermD3D()
{
    // コマンドの終了を待機.
    WaitForGpu();

    // イベントハンドルを閉じる.
    CloseHandle(m_FenceEvent);
    m_FenceEvent = nullptr;

    for( UINT i=0; i<BufferCount; ++i )
    { m_pRenderTarget[i].Reset(); }

    m_pDepthStencil.Reset();

    m_pSwapChain    .Reset();
    m_pFence        .Reset();
    m_pCmdList      .Reset();
    m_pCmdAllocator .Reset();
    m_pCmdQueue     .Reset();
    m_pDevice       .Reset();
}

//-------------------------------------------------------------------------------------------------
//      アプリケーション固有の初期化処理です.
//-------------------------------------------------------------------------------------------------
bool App::InitApp()
{
    HRESULT hr = S_OK;

    // ルートシグニチャを生成.
    {
        // ディスクリプタレンジの設定.
        D3D12_DESCRIPTOR_RANGE range[2];
        range[0].RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        range[0].NumDescriptors     = 1;
        range[0].BaseShaderRegister = 0;
        range[0].RegisterSpace      = 0;
        range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        range[1].RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[1].NumDescriptors     = 1;
        range[1].BaseShaderRegister = 0;
        range[1].RegisterSpace      = 0;
        range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        // ルートパラメータの設定.
        D3D12_ROOT_PARAMETER param[2];
        param[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_VERTEX;
        param[0].DescriptorTable.NumDescriptorRanges = 1;
        param[0].DescriptorTable.pDescriptorRanges   = &range[0];

        param[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
        param[1].DescriptorTable.NumDescriptorRanges = 1;
        param[1].DescriptorTable.pDescriptorRanges   = &range[1];

        // 静的サンプラーの設定.
        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.MipLODBias       = 0;
        sampler.MaxAnisotropy    = 0;
        sampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD           = 0.0f;
        sampler.MaxLOD           = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister   = 0;
        sampler.RegisterSpace    = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // ルートシグニチャの設定.
        D3D12_ROOT_SIGNATURE_DESC desc;
        desc.NumParameters     = _countof(param);
        desc.pParameters       = param;
        desc.NumStaticSamplers = 1;
        desc.pStaticSamplers   = &sampler;
        desc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        asdx::RefPtr<ID3DBlob> pSignature;
        asdx::RefPtr<ID3DBlob> pError;

        // シリアライズする.
        hr = D3D12SerializeRootSignature(
            &desc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            pSignature.GetAddress(),
            pError.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : D3D12SerializeRootSignataure() Failed." );
            return false;
        }

        // ルートシグニチャを生成.
        hr = m_pDevice->CreateRootSignature(
            0,
            pSignature->GetBufferPointer(),
            pSignature->GetBufferSize(),
            IID_PPV_ARGS(m_pRootSignature.GetAddress()) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateRootSignature() Failed." );
            return false;
        }
    }

    // パイプラインステートの生成.
    {
        asdx::RefPtr<ID3DBlob> pVSBlob;
        asdx::RefPtr<ID3DBlob> pPSBlob;

        // 頂点シェーダのファイルパスを検索.
        std::wstring path;
        if ( !SearchFilePath( L"SimpleVS.cso", path ) )
        {
            ELOG( "Error : File Not Found." );
            return false;
        }

        // コンパイル済み頂点シェーダを読み込む.
        hr = D3DReadFileToBlob( path.c_str(), pVSBlob.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : D3DReadFileToBlob() Failed." );
            return false;
        }

        // ピクセルシェーダのファイルパスを検索.
        if ( !SearchFilePath( L"SimplePS.cso", path ) )
        {
            ELOG( "Error : File Not Found." );
            return false;
        }

        // コンパイル済みピクセルシェーダを読み込む.
        hr = D3DReadFileToBlob( path.c_str(), pPSBlob.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : D3DReadFileToBlob() Failed." );
            return false;
        }

        // 入力レイアウトの設定.
        D3D12_INPUT_ELEMENT_DESC inputElements[] = {
            { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "VTX_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        // ラスタライザーステートの設定.
        D3D12_RASTERIZER_DESC descRS;
        descRS.FillMode                 = D3D12_FILL_MODE_SOLID;
        descRS.CullMode                 = D3D12_CULL_MODE_NONE;
        descRS.FrontCounterClockwise    = FALSE;
        descRS.DepthBias                = D3D12_DEFAULT_DEPTH_BIAS;
        descRS.DepthBiasClamp           = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        descRS.SlopeScaledDepthBias     = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        descRS.DepthClipEnable          = TRUE;
        descRS.MultisampleEnable        = FALSE;
        descRS.AntialiasedLineEnable    = FALSE;
        descRS.ForcedSampleCount        = 0;
        descRS.ConservativeRaster       = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        // レンダーターゲットのブレンド設定.
        D3D12_RENDER_TARGET_BLEND_DESC descRTBS = {
            FALSE, FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL
        };

        // ブレンドステートの設定.
        D3D12_BLEND_DESC descBS;
        descBS.AlphaToCoverageEnable  = FALSE;
        descBS.IndependentBlendEnable = FALSE;
        for( UINT i=0; i<D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i )
        { descBS.RenderTarget[i] = descRTBS; }

        // パイプラインステートの設定.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.InputLayout                        = { inputElements, _countof(inputElements) };
        desc.pRootSignature                     = m_pRootSignature.GetPtr();
        desc.VS                                 = { reinterpret_cast<UINT8*>( pVSBlob->GetBufferPointer() ), pVSBlob->GetBufferSize() };
        desc.PS                                 = { reinterpret_cast<UINT8*>( pPSBlob->GetBufferPointer() ), pPSBlob->GetBufferSize() };
        desc.RasterizerState                    = descRS;
        desc.BlendState                         = descBS;
        desc.DepthStencilState.DepthEnable      = TRUE;
        desc.DepthStencilState.DepthFunc        = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        desc.DepthStencilState.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
        desc.DepthStencilState.StencilEnable    = FALSE;
        desc.SampleMask                         = UINT_MAX;
        desc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets                   = 1;
        desc.RTVFormats[0]                      = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.DSVFormat                          = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count                   = 1;

        // パイプラインステートを生成.
        hr = m_pDevice->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS(m_pPipelineState.GetAddress()) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateGraphicsPipelineState() Failed." );
            return false;
        }
    }

    // 頂点バッファの生成.
    {
        // 頂点データ.
        Vertex vertices[] = {
            // 右下
            { {  1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
            { {  1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
            { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },

            // 左上
            { {  1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
            { { -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
            { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },

        };

        // ヒーププロパティの設定.
        D3D12_HEAP_PROPERTIES prop;
        prop.Type                 = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask     = 1;
        prop.VisibleNodeMask      = 1;

        // リソースの設定.
        D3D12_RESOURCE_DESC desc;
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment          = 0;
        desc.Width              = sizeof(vertices);
        desc.Height             = 1;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

        // リソースを生成.
        hr = m_pDevice->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_pVertexBuffer.GetAddress()) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed." );
            return false;
        }

        // マップする.
        UINT8* pData;
        hr = m_pVertexBuffer->Map( 0, nullptr, reinterpret_cast<void**>( &pData ) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Resource::Map() Failed." );
            return false;
        }

        // 頂点データをコピー.
        memcpy( pData, vertices, sizeof(vertices) );

        // アンマップする.
        m_pVertexBuffer->Unmap( 0, nullptr );

        // 頂点バッファビューの設定.
        m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
        m_VertexBufferView.StrideInBytes  = sizeof(Vertex);
        m_VertexBufferView.SizeInBytes    = sizeof(vertices);
    }

    // CBV・SRV・UAV用ディスクリプターヒープを生成.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 2;    // 定数バッファとシェーダリソースビューを作るので２つ。
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        hr = m_pDevice->CreateDescriptorHeap( &desc, IID_PPV_ARGS(m_pCbvSrvHeap.GetAddress()) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateDescriptorHeap() Failed." );
            return false;
        }
    }

    // 定数バッファを生成.
    {
        // ヒーププロパティの設定.
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type                 = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask     = 1;
        prop.VisibleNodeMask      = 1;

        // リソースの設定.
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment          = 0;
        desc.Width              = sizeof(ResConstantBuffer);
        desc.Height             = 1;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

        // リソースを生成.
        hr = m_pDevice->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_pConstantBuffer.GetAddress()) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed." );
            return false;
        }

        // 定数バッファビューの設定.
        D3D12_CONSTANT_BUFFER_VIEW_DESC bufferDesc = {};
        bufferDesc.BufferLocation = m_pConstantBuffer->GetGPUVirtualAddress();
        bufferDesc.SizeInBytes    = sizeof(ResConstantBuffer);

        // 定数バッファビューを生成.
        m_pDevice->CreateConstantBufferView( &bufferDesc, m_pCbvSrvHeap->GetCPUDescriptorHandleForHeapStart() );

        m_CbvSrvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // マップする. アプリケーション終了まで Unmap しない.
        // "Keeping things mapped for the lifetime of the resource is okay." とのこと。
        hr = m_pConstantBuffer->Map( 0, nullptr, reinterpret_cast<void**>(&m_pCbvDataBegin) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Resource::Map() Failed." );
            return false;
        }

        // アスペクト比算出.
        auto aspectRatio = static_cast<FLOAT>( m_Width ) / static_cast<FLOAT>( m_Height );

        // 定数バッファデータの設定.
        m_ConstantBufferData.World = asdx::Matrix::Identity();
        m_ConstantBufferData.View  = asdx::Matrix::CreateLookAt( asdx::Vector3(0.0f, 0.0f, 5.0f), asdx::Vector3(0.0f, 0.0f, 0.0f), asdx::Vector3( 0.0f, 1.0f, 0.0f ) );
        m_ConstantBufferData.Proj  = asdx::Matrix::CreatePerspectiveFieldOfView( asdx::F_PIDIV4, aspectRatio, 1.0f, 1000.0f );

        // コピる.
        memcpy( m_pCbvDataBegin, &m_ConstantBufferData, sizeof(m_ConstantBufferData) );
    }

    // TGAファイルを検索.
    std::wstring path;
    if ( !SearchFilePath( L"res/sample32bitRLE.tga", path ) )
    {
        ELOG( "Error : File Not Found." );
        return false;
    }

    // TGAファイルをロード.
    asdx::ResTGA tga;
    if ( !tga.Load( path.c_str() ) )
    {
        ELOG( "Error : Targa File Load Failed." );
        return false;
    }

    // ヒーププロパティの設定.
    D3D12_HEAP_PROPERTIES prop;
    prop.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask     = 1;
    prop.VisibleNodeMask      = 1;

    // リソースの設定.
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width              = tga.GetWidth();
    desc.Height             = tga.GetHeight();
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    // リソースを生成.
    hr = m_pDevice->CreateCommittedResource(
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_pTexture.GetAddress()) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed." );
        return false;
    }

    // サブリソースデータ設定.
    D3D12_SUBRESOURCE_DATA subRes;
    subRes.pData      = tga.GetPixels();
    subRes.RowPitch   = tga.GetWidth() * tga.GetBitPerPixel() / 8;
    subRes.SlicePitch = subRes.RowPitch * tga.GetHeight();

    // サブリソースの更新.
    auto ret = UpdateSubresources(
        m_pCmdList.GetPtr(),
        m_pCmdQueue.GetPtr(),
        m_pFence.GetPtr(),
        m_FenceEvent,
        m_pTexture.GetPtr(),
        0,
        1,
        &subRes);
    if ( !ret )
    {
        ELOG( "Error : UdpateSubresources() Failed." );
        return false;
    }

    // シェーダリソースビューの設定.
    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    viewDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
    viewDesc.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
    viewDesc.Texture2D.MipLevels       = 1;
    viewDesc.Texture2D.MostDetailedMip = 0;

    // シェーダリソースビューを生成.
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pCbvSrvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += m_CbvSrvDescriptorSize;
    m_pDevice->CreateShaderResourceView( m_pTexture.GetPtr(), &viewDesc, handle );

    // 正常終了.
    return true;
}

//--------------------------------------------------------------------------------------------------
//      アプリケーション固有の終了処理です.
//--------------------------------------------------------------------------------------------------
void App::TermApp()
{
    m_pConstantBuffer->Unmap( 0, nullptr );

    m_pConstantBuffer.Reset();
    m_pCbvSrvHeap.Reset();

    m_pVertexBuffer.Reset();
    m_VertexBufferView.BufferLocation = 0;
    m_VertexBufferView.SizeInBytes    = 0;
    m_VertexBufferView.StrideInBytes  = 0;

    m_pPipelineState.Reset();
    m_pRootSignature.Reset();
}

//-------------------------------------------------------------------------------------------------
//      描画時の処理です.
//-------------------------------------------------------------------------------------------------
void App::OnRender(FLOAT elapsedSec)
{
    // 回転角を増やす.
    m_RotateAngle += ( elapsedSec / 0.5f );

    // ワールド行列を更新.
    m_ConstantBufferData.World = asdx::Matrix::CreateRotationY( m_RotateAngle );

    // 定数バッファを更新.
    memcpy( m_pCbvDataBegin, &m_ConstantBufferData, sizeof(m_ConstantBufferData) );


    // コマンドアロケータとコマンドリストをリセット.
    m_pCmdAllocator->Reset();
    m_pCmdList->Reset( m_pCmdAllocator.GetPtr(), m_pPipelineState.GetPtr() );

    // ディスクリプタヒープを設定.
    m_pCmdList->SetDescriptorHeaps( 1, m_pCbvSrvHeap.GetAddress() );

    // ルートシグニチャを設定.
    m_pCmdList->SetGraphicsRootSignature( m_pRootSignature.GetPtr() );

    // ディスクリプタヒープテーブルを設定.
    auto handleCBV = m_pCbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
    auto handleSRV = m_pCbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
    handleSRV.ptr += m_CbvSrvDescriptorSize;
    m_pCmdList->SetGraphicsRootDescriptorTable( 0, handleCBV );
    m_pCmdList->SetGraphicsRootDescriptorTable( 1, handleSRV );

    // ビューポートの設定.
    m_pCmdList->RSSetViewports( 1, &m_Viewport );

    // シザー矩形の設定.
    m_pCmdList->RSSetScissorRects( 1, &m_ScissorRect );

    // リソースバリアの設定.
    // Present ---> RenderTarget
    D3D12_RESOURCE_BARRIER barrier;
    ZeroMemory( &barrier, sizeof(barrier) );
    barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                   = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource    = m_pRenderTarget[m_FrameIndex].GetPtr();
    barrier.Transition.StateBefore  = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter   = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    m_pCmdList->ResourceBarrier( 1, &barrier );

    // レンダーターゲットのハンドルを取得.
    auto handleRTV = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    auto handleDSV = m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();
    handleRTV.ptr += ( m_FrameIndex * m_RtvDescriptorSize );

    // レンダーターゲットの設定.
    m_pCmdList->OMSetRenderTargets( 1, &handleRTV, FALSE, &handleDSV );

    // レンダーターゲットビューをクリア.
    const FLOAT clearColor[] = { 0.39f, 0.58f, 0.92f, 1.0f };
    m_pCmdList->ClearRenderTargetView( handleRTV, clearColor, 0, nullptr );

    // 深度ステンシルビューをクリア.
    m_pCmdList->ClearDepthStencilView( handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );

    // プリミティブトポロジーの設定.
    m_pCmdList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // 頂点バッファビューを設定.
    m_pCmdList->IASetVertexBuffers( 0, 1, &m_VertexBufferView );

    // 描画コマンドを生成.
    m_pCmdList->DrawInstanced( 6, 1, 0, 0 );

    // リソースバリアの設定.
    // RenderTarget ---> Present
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    m_pCmdList->ResourceBarrier( 1, &barrier );

    // コマンドの記録を終了.
    m_pCmdList->Close();

    // コマンド実行.
    ID3D12CommandList* ppCmdLists[] = { m_pCmdList.GetPtr() };
    m_pCmdQueue->ExecuteCommandLists( _countof(ppCmdLists), ppCmdLists );

    // 表示する.
    auto hr = m_pSwapChain->Present( 1, 0 );

    // GPUクラッシュチェック.
    if (hr == DXGI_ERROR_DEVICE_REMOVED
     || hr == DXGI_ERROR_DEVICE_HUNG
     || hr == DXGI_ERROR_DEVICE_REMOVED)
    {
        ELOG("Error : IDXGISwapChain::Preset() Failed. errcode = 0x%x", hr);
        ReportDRED(hr, m_pDevice.GetPtr());
        abort();
    }

    // コマンドの完了を待機.
    WaitForGpu();
}

//-------------------------------------------------------------------------------------------------
//      コマンドの完了を待機します.
//-------------------------------------------------------------------------------------------------
void App::WaitForGpu()
{
    // シグナル状態にして，フェンス値を増加.
    const UINT64 fence = m_FenceValue;
    auto hr = m_pCmdQueue->Signal( m_pFence.GetPtr(), fence );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12CommandQueue::Signal() Failed." );
        return;
    }
    m_FenceValue++;

    // 完了を待機.
    if ( m_pFence->GetCompletedValue() < fence )
    {
        hr = m_pFence->SetEventOnCompletion( fence, m_FenceEvent );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Fence::SetEventOnCompletaion() Failed." );
            return;
        }

        WaitForSingleObject( m_FenceEvent, INFINITE );
    }

    // フレームバッファ番号を更新.
    m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウプロシージャです.
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK App::WndProc( HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp )
{
    auto instance = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch( uMsg )
    {
        case WM_CREATE:
            {
                auto st = reinterpret_cast<LPCREATESTRUCT>(lp);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(st->lpCreateParams));
            }
            break;

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

        case WM_KEYDOWN:
            {
                if (instance == nullptr)
                    break;

                auto keyCode = uint32_t(wp);
                switch(keyCode)
                {
                case 'T':
                    {
                        // テクスチャを解放して，GPUクラッシュさせる.
                        instance->m_pTexture.Reset();
                    }
                    break;

                default:
                    break;
                }
            }
            break;

        default:
            break;
    }

    return DefWindowProc( hWnd, uMsg, wp, lp );
}