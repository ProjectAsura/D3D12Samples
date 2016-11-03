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
#include <asdxRenderState.h>
#include <asdxLogger.h>
#include <asdxMisc.h>
#include <asdxResource.h>
#include <asdxShader.h>
#include <App.h>


//-------------------------------------------------------------------------------------------------
// Linker
//-------------------------------------------------------------------------------------------------
#pragma comment( lib, "shlwapi.lib" )


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
        { pDebug->EnableDebugLayer(); }
    }
  #endif

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

        if ( !m_Heap[DESC_HEAP_RTV].Init( m_pDevice.GetPtr(), &desc ) )
        {
            ELOG( "Error : DescHeap::Init() Failed." );
            return false;
        }

        // 深度ステンシルビュー用ディスクリプタヒープの設定.
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        if ( !m_Heap[DESC_HEAP_DSV].Init( m_pDevice.GetPtr(), &desc ) )
        {
            ELOG(" Error : DescHeap::Init() Failed." );
            return false;
        }
    }

    // レンダーターゲットビューを生成.
    {
        // フレームバッファ数分ループさせる.
        for( UINT i=0; i<BufferCount; ++i )
        {
            // ハンドルのポインタを進める.
            auto handle = m_Heap[DESC_HEAP_RTV].GetHandleCPU( i );

            // バックバッファ取得.
            hr = m_pSwapChain->GetBuffer( i, IID_PPV_ARGS( m_pRenderTarget[i].GetAddress() ) );
            if ( FAILED( hr ) )
            {
                ELOG( "Error : IDXGISwapChain3::GetBuffer() Failed." );
                return false;
            }

            // レンダーターゲットビューを生成.
            m_pDevice->CreateRenderTargetView( m_pRenderTarget[i].GetPtr(), nullptr, handle );
        }
    }

    // 深度ステンシルビューを生成.
    {
        // ヒーププロパティの設定.
        D3D12_HEAP_PROPERTIES prop = asdx::Resource::HeapProps;
        D3D12_RESOURCE_DESC   desc = asdx::Resource::DescTexture2D;
        desc.Width  = m_Width;
        desc.Height = m_Height;
        desc.Format = DXGI_FORMAT_D32_FLOAT;
        desc.Flags  = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

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
            m_Heap[DESC_HEAP_DSV].GetHandleCPU(0) );
    }

    // コマンドリストの初期化.
    if ( !m_Immediate.Init( m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr ) )
    {
        ELOG( "Error : GraphicsCmdList::Init() " );
        return false;
    }

    // フェンスの初期化.
    if ( !m_Fence.Init( m_pDevice.GetPtr() ) )
    {
        ELOG( "Error : Fence::Init() Failed." );
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

    // フェンスの終了処理.
    m_Fence.Term();

    // レンダーターゲットを解放.
    for( UINT i=0; i<BufferCount; ++i )
    { m_pRenderTarget[i].Reset(); }
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
        D3D12_STATIC_SAMPLER_DESC sampler = asdx::RenderState::DefaultStaticSamplerComparisonDesc;
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
        if ( !asdx::SearchFilePath( L"SimpleVS.cso", path ) )
        {
            ELOG( "Error : File Not Found." );
            return false;
        }

        asdx::Shader vs;
        if ( !vs.Load( path.c_str() ) )
        {
            ELOG( "Error : Shader::Load() Failed." );
            return false;
        }

        // ピクセルシェーダのファイルパスを検索.
        if ( !asdx::SearchFilePath( L"SimplePS.cso", path ) )
        {
            ELOG( "Error : File Not Found." );
            return false;
        }

        asdx::Shader ps;
        if ( !ps.Load( path.c_str() ) )
        {
            ELOG( "Error : Shader::Load() Failed." );
            return false;
        }

        // 入力レイアウトの設定.
        D3D12_INPUT_ELEMENT_DESC inputElements[] = {
            { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "VTX_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        // パイプラインステートの設定.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = asdx::RenderState::DefaultPipelineStateDesc;
        desc.InputLayout                        = { inputElements, _countof(inputElements) };
        desc.pRootSignature                     = m_pRootSignature.GetPtr();
        desc.VS                                 = vs.GetByteCode();
        desc.PS                                 = ps.GetByteCode();
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

        // リソースの設定.
        D3D12_HEAP_PROPERTIES props = asdx::Resource::HeapProps;
        props.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC desc = asdx::Resource::DescBuffer;
        desc.Width = sizeof(vertices);
        if ( !m_VertexBuffer.Init( 
            m_pDevice.GetPtr(),
            &desc,
            &props,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ ) )
        {
            ELOG( "Error : Buffer::Init() Failed." );
            return false;
        }

        // マップする.
        UINT8* pData;
        hr = m_VertexBuffer->Map( 0, nullptr, reinterpret_cast<void**>( &pData ) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Resource::Map() Failed." );
            return false;
        }

        // 頂点データをコピー.
        memcpy( pData, vertices, sizeof(vertices) );

        // アンマップする.
        m_VertexBuffer->Unmap( 0, nullptr );

        // 頂点バッファビューの設定.
        m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
        m_VertexBufferView.StrideInBytes  = sizeof(Vertex);
        m_VertexBufferView.SizeInBytes    = sizeof(vertices);
    }

    // CBV・SRV・UAV用ディスクリプターヒープを生成.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 2;    // 定数バッファとシェーダリソースビューを作るので２つ。
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        if ( !m_Heap[DESC_HEAP_BUFFER].Init( m_pDevice.GetPtr(), &desc ) )
        {
            ELOG( "Error : DescHeap::Init() Failed." );
            return false;
        }
    }

    // 定数バッファを生成.
    {
        D3D12_HEAP_PROPERTIES props = asdx::Resource::HeapProps;
        props.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC   desc  = asdx::Resource::DescBuffer;
        desc.Width = sizeof(ResConstantBuffer);

        if ( !m_ConstantBuffer.Init( 
            m_pDevice.GetPtr(),
            &desc,
            &props,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ ))
        {
            ELOG( "Error : Resource::Init() Failed." );
            return false;
        }

        // 定数バッファビューの設定.
        D3D12_CONSTANT_BUFFER_VIEW_DESC bufferDesc = {};
        bufferDesc.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress();
        bufferDesc.SizeInBytes    = sizeof(ResConstantBuffer);

        // 定数バッファビューを生成.
        m_pDevice->CreateConstantBufferView( &bufferDesc, m_Heap[DESC_HEAP_BUFFER].GetHandleCPU(0) );

        // マップする. アプリケーション終了まで Unmap しない.
        hr = m_ConstantBuffer->Map( 0, nullptr, reinterpret_cast<void**>(&m_pCbvDataBegin) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Resource::Map() Failed." );
            return false;
        }

        // アスペクト比算出.
        auto aspectRatio = static_cast<FLOAT>( m_Width ) / static_cast<FLOAT>( m_Height );

        // 定数バッファデータの設定.
        m_ConstantBufferData.World = asdx::Matrix::CreateIdentity();
        m_ConstantBufferData.View  = asdx::Matrix::CreateLookAt( asdx::Vector3(0.0f, 0.0f, 5.0f), asdx::Vector3(0.0f, 0.0f, 0.0f), asdx::Vector3( 0.0f, 1.0f, 0.0f ) );
        m_ConstantBufferData.Proj  = asdx::Matrix::CreatePerspectiveFieldOfView( asdx::F_PIDIV4, aspectRatio, 1.0f, 1000.0f );

        // コピる.
        memcpy( m_pCbvDataBegin, &m_ConstantBufferData, sizeof(m_ConstantBufferData) );
    }

    // テクスチャ・シェーダリソースビューの生成.
    asdx::RefPtr<ID3D12Resource> uploadOnlyTexture;
    {
        // TGAファイルを検索.
        std::wstring path;
        if ( !asdx::SearchFilePath( L"res/sample32bitRLE.tga", path ) )
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
        D3D12_HEAP_PROPERTIES prop = asdx::Resource::HeapProps;
        D3D12_RESOURCE_DESC   desc = asdx::Resource::DescTexture2D;
        desc.Width  = tga.GetWidth();
        desc.Height = tga.GetHeight();
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        // リソース生成.
        hr = m_pDevice->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(m_pTexture.GetAddress()) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed." );
            return false;
        }

        prop.Type = D3D12_HEAP_TYPE_UPLOAD;

        // アップロード用リソースの設定.
        desc = asdx::Resource::DescBuffer;
        desc.Width = asdx::GetRequiredIntermediateSize( m_pTexture.GetPtr(), 0, 1 );

        // リソースを生成.
        hr = m_pDevice->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(uploadOnlyTexture.GetAddress()) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed." );
            return false;
        }

        D3D12_SUBRESOURCE_DATA res = {};
        res.pData      = tga.GetPixels();
        res.RowPitch   = tga.GetWidth() * tga.GetBitPerPixel() / 8;
        res.SlicePitch = res.RowPitch * tga.GetHeight();

        asdx::UpdateSubresources( 
            m_Immediate.GetGfxCmdList(),
            m_pTexture.GetPtr(),
            uploadOnlyTexture.GetPtr(),
            0,
            0,
            1,
            &res );

        // リソースバリアを設定.
        m_Immediate.Transition(
            m_pTexture.GetPtr(), 
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

        // シェーダリソースビューの設定.
        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
        viewDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
        viewDesc.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        viewDesc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
        viewDesc.Texture2D.MipLevels       = 1;
        viewDesc.Texture2D.MostDetailedMip = 0;

        // シェーダリソースビューを生成.
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_Heap[DESC_HEAP_BUFFER].GetHandleCPU(1);
        m_pDevice->CreateShaderResourceView( m_pTexture.GetPtr(), &viewDesc, handle );
    }

    // コマンドリストを閉じておく.
    m_Immediate->Close();

    // GPUにテクスチャ転送.
    m_Immediate.Execute( m_pCmdQueue.GetPtr() );

    // 実行完了を待機.
    WaitForGpu();

    // バンドルの初期化.
    if ( !m_Bundle.Init( m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_BUNDLE, m_pPipelineState.GetPtr() ) )
    {
        ELOG( "Error : GraphicsCmdList()::Init() Failed." );
        return false;
    }

    // バンドルにコマンドを積む.
    {
        // ルートシグニチャを設定.
        m_Bundle->SetGraphicsRootSignature( m_pRootSignature.GetPtr() );

        // プリミティブトポロジーの設定.
        m_Bundle->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

        // 頂点バッファビューを設定.
        m_Bundle->IASetVertexBuffers( 0, 1, &m_VertexBufferView );

        // 描画コマンドを生成.
        m_Bundle->DrawInstanced( 6, 1, 0, 0 );

        // バンドルへの記録を終了.
        m_Bundle->Close();
    }

    // 正常終了.
    return true;
}

//--------------------------------------------------------------------------------------------------
//      アプリケーション固有の終了処理です.
//--------------------------------------------------------------------------------------------------
void App::TermApp()
{
    for( auto i=0; i<NUM_DESC_HEAP; ++i )
    { m_Heap[i].Term(); }

    m_VertexBuffer.Term();
    m_VertexBufferView.BufferLocation = 0;
    m_VertexBufferView.SizeInBytes    = 0;
    m_VertexBufferView.StrideInBytes  = 0;

    m_ConstantBuffer.Term();
    m_pTexture.Reset();

    m_pPipelineState.Reset();
    m_pRootSignature.Reset();

    m_Bundle.Term();
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
    m_Immediate.Clear( m_pPipelineState.GetPtr() );

    ID3D12DescriptorHeap* pHeap = m_Heap[DESC_HEAP_BUFFER].GetPtr();

    // ディスクリプタヒープを設定.
    m_Immediate->SetDescriptorHeaps( 1, &pHeap );

    // ルートシグニチャを設定.
    m_Immediate->SetGraphicsRootSignature( m_pRootSignature.GetPtr() );

    // ディスクリプタヒープテーブルを設定.
    auto handleCBV = m_Heap[DESC_HEAP_BUFFER].GetHandleGPU(0);
    auto handleSRV = m_Heap[DESC_HEAP_BUFFER].GetHandleGPU(1);
    m_Immediate->SetGraphicsRootDescriptorTable( 0, handleCBV );
    m_Immediate->SetGraphicsRootDescriptorTable( 1, handleSRV );

    // リソースバリアの設定.
    m_Immediate.Transition(
        m_pRenderTarget[m_FrameIndex].GetPtr(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET );

    // ビューポートの設定.
    m_Immediate->RSSetViewports( 1, &m_Viewport );

    // シザー矩形の設定.
    m_Immediate->RSSetScissorRects( 1, &m_ScissorRect );

    // レンダーターゲットのハンドルを取得.
    auto handleRTV = m_Heap[DESC_HEAP_RTV].GetHandleCPU(m_FrameIndex);
    auto handleDSV = m_Heap[DESC_HEAP_DSV].GetHandleCPU(0);

    // レンダーターゲットの設定.
    m_Immediate->OMSetRenderTargets( 1, &handleRTV, FALSE, &handleDSV );

    // レンダーターゲットビューをクリア.
    const FLOAT clearColor[] = { 0.39f, 0.58f, 0.92f, 1.0f };
    m_Immediate->ClearRenderTargetView( handleRTV, clearColor, 0, nullptr );

    // 深度ステンシルビューをクリア.
    m_Immediate->ClearDepthStencilView( handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );

    // バンドルを実行.
    m_Immediate->ExecuteBundle( m_Bundle.GetGfxCmdList() );

    // リソースバリアの設定.
    m_Immediate.Transition(
        m_pRenderTarget[m_FrameIndex].GetPtr(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT );

    // コマンドの記録を終了.
    m_Immediate->Close();

    // コマンド実行.
    m_Immediate.Execute( m_pCmdQueue.GetPtr() );

    // 表示する.
    m_pSwapChain->Present( 1, 0 );

    // コマンドの完了を待機.
    WaitForGpu();
}

//-------------------------------------------------------------------------------------------------
//      コマンドの完了を待機します.
//-------------------------------------------------------------------------------------------------
void App::WaitForGpu()
{
    // 完了を待機.
    m_Fence.Wait( m_pCmdQueue.GetPtr(), INFINITE );

    // フレームバッファ番号を更新.
    m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウプロシージャです.
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK App::WndProc( HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp )
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

        default:
            break;
    }

    return DefWindowProc( hWnd, uMsg, wp, lp );
}