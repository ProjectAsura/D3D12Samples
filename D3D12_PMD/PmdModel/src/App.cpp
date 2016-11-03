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
#include <asdxResDDS.h>
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
        D3D12_HEAP_PROPERTIES prop = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1
        };

        D3D12_RESOURCE_DESC desc = {
            D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            0,
            m_Width,
            m_Height,
            1,
            0,
            DXGI_FORMAT_D32_FLOAT,
            { 1, 0 },
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
        };

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

    {
        std::wstring path;
        if ( !asdx::SearchFilePathW( L"res/pronama/プロ生ちゃん.pmd", path ) )
        {
            ELOG( "Error : SearchFilePath() Failed. " );
            return false;
        }

        if ( !m_ModelData.Load( path.c_str() ) )
        {
            ELOG( "Error : ResPmd::Load() Failed." );
            return false;
        }
    }

    std::vector<s32> textureIdList;
    std::vector<std::wstring> textureNameList;
    {
        std::string path;
        textureIdList.resize( m_ModelData.Materials.size() );

        {
            std::wstring wpath;
            if ( asdx::SearchFilePathW( L"res/dummy.dds", wpath ) )
            {
                textureNameList.push_back( wpath.c_str() );
            }
        }

        for( size_t i=0; i<m_ModelData.Materials.size(); ++i )
        {
            std::string temp = m_ModelData.Materials[i].TextureName;
            if ( temp.empty() )
            { continue; }

            // スフィアマップ名は取り除く.
            size_t idx = temp.find('*');
            if ( idx != std::string::npos )
            { temp = temp.substr(0, idx); }

            std::string input = "res/pronama/" + temp;

            // 存在確認.
            if ( !asdx::SearchFilePathA( input.c_str(), path ) )
            {
                textureIdList[i] = 0; // 見つからなかったらダミーテクスチャを使用.
                continue;
            }

            // ワイド文字に変換.
            auto filePath = asdx::ToStringW( path );

            // リストを検索.
            auto isFind = false;
            for( size_t j=0; j<textureNameList.size(); ++j )
            {
                if ( textureNameList[j] == filePath )
                {
                    isFind = true;
                    textureIdList[i] = u32(j); 
                    break;
                }
            }

            // 検索に引っかからなかったら追加.
            if ( !isFind )
            { 
                textureNameList.push_back( filePath );
                textureIdList[i] = u32( textureNameList.size() - 1 );
            }
        }
    }

    // ルートシグニチャを生成.
    {
        // ディスクリプタレンジの設定.
        D3D12_DESCRIPTOR_RANGE range[3];
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

        range[2].RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        range[2].NumDescriptors     = 1;
        range[2].BaseShaderRegister = 1;
        range[2].RegisterSpace      = 0;
        range[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


        // ルートパラメータの設定.
        D3D12_ROOT_PARAMETER param[3];
        param[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_VERTEX;
        param[0].DescriptorTable.NumDescriptorRanges = 1;
        param[0].DescriptorTable.pDescriptorRanges   = &range[0];

        param[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
        param[1].DescriptorTable.NumDescriptorRanges = 1;
        param[1].DescriptorTable.pDescriptorRanges   = &range[1];

        param[2].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[2].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
        param[2].DescriptorTable.NumDescriptorRanges = 1;
        param[2].DescriptorTable.pDescriptorRanges   = &range[2];

        // 静的サンプラーの設定.
        D3D12_STATIC_SAMPLER_DESC sampler = asdx::RenderState::DefaultStaticSamplerDesc;
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
        if ( !asdx::SearchFilePathW( L"SimpleVS.cso", path ) )
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
        if ( !asdx::SearchFilePathW( L"SimplePS.cso", path ) )
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
        desc.BlendState                         = asdx::RenderState::CreateBlendDesc( asdx::BlendState::NonPremultiplied );

        // パイプラインステートを生成.
        hr = m_pDevice->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS(m_pPipelineState.GetAddress()) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateGraphicsPipelineState() Failed." );
            return false;
        }
    }

    // CBV・SRV・UAV用ディスクリプターヒープを生成.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1
            + static_cast<u32>(m_ModelData.Materials.size())
            + static_cast<u32>(textureNameList.size());
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        if ( !m_Heap[DESC_HEAP_BUFFER].Init( m_pDevice.GetPtr(), &desc ) )
        {
            ELOG( "Error : DescHeap::Init() Failed." );
            return false;
        }
    }

    // 頂点バッファの生成.
    {
        if ( !m_ModelVB.Init( 
            m_pDevice.GetPtr(), 
            sizeof(asdx::PMD_VERTEX) * m_ModelData.Vertices.size(),
            sizeof(asdx::PMD_VERTEX),
            &m_ModelData.Vertices[0] ) )
        {
            ELOG( "Error : VertexBuffer::Init() Failed." );
            return false;
        }
    }

    // インデックスバッファの生成.
    {
        if ( !m_ModelIB.Init(
            m_pDevice.GetPtr(),
            sizeof(u16) * m_ModelData.Indices.size(),
            DXGI_FORMAT_R16_UINT,
            &m_ModelData.Indices[0] ) )
        {
            ELOG( "Error : IndexBuffer::Init() Faild." );
            return false;
        }
    }

    // マテリアルバッファの生成.
    {
        if ( !m_ModelMB.Init( 
            m_pDevice.GetPtr(),
            sizeof(ResMaterialBuffer) * m_ModelData.Materials.size() ) )
        {
            ELOG( "Error : ConstantBuffer::Init() Failed." );
            return false;
        }

        u32 size = static_cast<u32>(sizeof(ResMaterialBuffer));

        // 定数バッファビューの設定.
        D3D12_CONSTANT_BUFFER_VIEW_DESC bufferDesc = {};
        bufferDesc.BufferLocation = m_ModelMB->GetGPUVirtualAddress();
        bufferDesc.SizeInBytes    = size;

        u32 offset = 0;
        for( size_t i=0; i<m_ModelData.Materials.size(); ++i )
        {
            m_ModelMB.Update( &m_ModelData.Materials[i], size, offset );

            m_pDevice->CreateConstantBufferView( &bufferDesc, m_Heap[DESC_HEAP_BUFFER].GetHandleCPU(1 + u32(i)) );
            bufferDesc.BufferLocation += size;
            offset += size;
        }
    }

    // 定数バッファを生成.
    {
        if ( !m_ModelTB.Init( m_pDevice.GetPtr(), sizeof(ResConstantBuffer ) ) )
        {
            ELOG( "Error : ConstantBuffer::Init() Failed." );
            return false;
        }

        // 定数バッファビューの設定.
        D3D12_CONSTANT_BUFFER_VIEW_DESC bufferDesc = {};
        bufferDesc.BufferLocation = m_ModelTB->GetGPUVirtualAddress();
        bufferDesc.SizeInBytes    = sizeof(ResConstantBuffer);

        // 定数バッファビューを生成.
        m_pDevice->CreateConstantBufferView( &bufferDesc, m_Heap[DESC_HEAP_BUFFER].GetHandleCPU(0) );

        // アスペクト比算出.
        auto aspectRatio = static_cast<FLOAT>( m_Width ) / static_cast<FLOAT>( m_Height );

        // 定数バッファデータの設定.
        m_ModelParam.World = asdx::Matrix::CreateIdentity();
        m_ModelParam.View  = asdx::Matrix::CreateLookAt( asdx::Vector3(0.0f, 15.0f, -35.0f), asdx::Vector3(0.0f, 10.0f, 0.0f), asdx::Vector3( 0.0f, 1.0f, 0.0f ) );
        m_ModelParam.Proj  = asdx::Matrix::CreatePerspectiveFieldOfView( asdx::F_PIDIV4, aspectRatio, 1.0f, 1000.0f );

        // コピる.
        m_ModelTB.Update( &m_ModelParam, sizeof(m_ModelParam), 0 );
    }

    // テクスチャ・シェーダリソースビューの生成.
    {
        m_ModelTexture.resize( textureNameList.size() );

        auto idx = 0;
        auto offset = 1 + static_cast<u32>(m_ModelData.Materials.size());

        for( auto textureName : textureNameList )
        {
            auto ext = asdx::GetExtW( textureName.c_str() );

            // DDSを読み込みリソースを生成.
            if ( ext == L"dds" )
            {
                asdx::ResDDS dds;
                if ( dds.Load( textureName.c_str() ) )
                {
                    auto desc = asdx::Texture::Desc();
                    desc.ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                    desc.ResourceDesc.Width     = dds.GetWidth();
                    desc.ResourceDesc.Height    = dds.GetHeight();
                    desc.ResourceDesc.MipLevels = 1;
                    desc.ResourceDesc.Format    = static_cast<DXGI_FORMAT>(dds.GetFormat());
                    desc.HandleCPU              = m_Heap[DESC_HEAP_BUFFER].GetHandleCPU( offset + idx );

                    if ( !m_ModelTexture[idx].Init( m_pDevice.GetPtr(), desc ) )
                    {
                        continue;
                    }

                    // サブリソースを設定.
                    D3D12_SUBRESOURCE_DATA res = {};
                    res.pData      = dds.GetSurfaces()->pPixels;
                    res.RowPitch   = dds.GetSurfaces()->Pitch;
                    res.SlicePitch = dds.GetSurfaces()->SlicePitch;

                    asdx::RefPtr<ID3D12Resource> intermediate;

                    m_Immediate.Clear( m_pPipelineState.GetPtr() );

                    // サブリソースを更新.
                    if ( !m_ModelTexture[idx].Upload(
                        m_Immediate.GetGfxCmdList(),
                        &res,
                        intermediate.GetAddress() ) )
                    {
                        continue;
                    }

                    // コマンドリストを閉じておく.
                    m_Immediate->Close();

                    // GPUにテクスチャ転送.
                    m_Immediate.Execute( m_pCmdQueue.GetPtr() );

                    // 実行完了を待機.
                    WaitForGpu();

                }
            }

            idx++;
        }
    }

    // バンドルの初期化.
    if ( !m_Bundle.Init( m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_BUNDLE, m_pPipelineState.GetPtr() ) )
    {
        ELOG( "Error : GraphicsCmdList()::Init() Failed." );
        return false;
    }

    // バンドルにコマンドを積む.
    {
        // ディスクリプタヒープを設定.
        ID3D12DescriptorHeap* pHeap = m_Heap[DESC_HEAP_BUFFER].GetPtr();
        m_Bundle->SetDescriptorHeaps( 1, &pHeap );

        // ルートシグニチャを設定.
        m_Bundle->SetGraphicsRootSignature( m_pRootSignature.GetPtr() );

        // プリミティブトポロジーの設定.
        m_Bundle->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

        auto vbv = m_ModelVB.GetView();
        auto ibv = m_ModelIB.GetView();

        // 頂点バッファビューを設定.
        m_Bundle->IASetVertexBuffers( 0, 1, &vbv );
        m_Bundle->IASetIndexBuffer( &ibv );

        // インデックスオフセット.
        u32 offset = 0;
        u32 materialCount = u32( m_ModelData.Materials.size() );

        for( size_t i=0; i<materialCount; ++i )
        {
            // テクスチャとマテリアルを設定.
            auto handleSRV = m_Heap[DESC_HEAP_BUFFER].GetHandleGPU( 1 + materialCount + textureIdList[i] );
            auto handleMat = m_Heap[DESC_HEAP_BUFFER].GetHandleGPU( 1 + u32(i) );
            m_Bundle->SetGraphicsRootDescriptorTable( 1, handleSRV );
            m_Bundle->SetGraphicsRootDescriptorTable( 2, handleMat );

            u32 count = m_ModelData.Materials[i].VertexCount;

            // 描画コマンドを生成.
            m_Bundle->DrawIndexedInstanced( count, 1, offset, 0, 0 );

            offset += count;
        }

        // バンドルへの記録を終了.
        m_Bundle->Close();
    }

    // モデル情報を表示する.
    MessageBoxA( nullptr, (m_ModelData.Name + "\n" + m_ModelData.Comment).c_str(), "モデル情報", MB_OK | MB_ICONINFORMATION );

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

    m_ModelVB.Term();
    m_ModelIB.Term();
    m_ModelTB.Term();
    m_ModelMB.Term();

    m_pPipelineState.Reset();
    m_pRootSignature.Reset();

    for( size_t i=0; i<m_ModelTexture.size(); ++i )
    { m_ModelTexture[i].Term(); }

    m_Bundle.Term();
}

//-------------------------------------------------------------------------------------------------
//      描画時の処理です.
//-------------------------------------------------------------------------------------------------
void App::OnRender(FLOAT elapsedSec)
{
    // 回転角を増やす.
    m_RotateAngle += ( elapsedSec / 1.5f );

    // ワールド行列を更新.
    m_ModelParam.World = asdx::Matrix::CreateRotationY( m_RotateAngle );

    // 定数バッファを更新.
    m_ModelTB.Update( &m_ModelParam, sizeof(m_ModelParam), 0 );

    // コマンドアロケータとコマンドリストをリセット.
    m_Immediate.Clear( m_pPipelineState.GetPtr() );

    ID3D12DescriptorHeap* pHeap = m_Heap[DESC_HEAP_BUFFER].GetPtr();

    // ディスクリプタヒープを設定.
    m_Immediate->SetDescriptorHeaps( 1, &pHeap );

    // ルートシグニチャを設定.
    m_Immediate->SetGraphicsRootSignature( m_pRootSignature.GetPtr() );

    // ディスクリプタヒープテーブルを設定.
    auto handleCBV = m_Heap[DESC_HEAP_BUFFER].GetHandleGPU(0);
    m_Immediate->SetGraphicsRootDescriptorTable( 0, handleCBV );

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
    const FLOAT clearColor[] = { 0.39f, 0.58f, 0.92f, 0.0f };
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