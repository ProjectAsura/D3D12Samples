//-------------------------------------------------------------------------------------------------
// File : App.cpp
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <App.h>
#include <asdxLogger.h>
#include <asdxSound.h>
#include <asdxRenderState.h>
#include <asdxMisc.h>
#include <asdxShader.h>
#include <asdxMotionPlayer.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
App::App()
: DesktopApp( L"Sample", 960, 540, LoadIconW(nullptr, L"res/Icon/asura.ico"), nullptr, nullptr)
{
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
App::~App()
{}

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool App::OnInit()
{
    HRESULT hr = S_OK;

    // デバイスの初期化.
    {
        asdx::DEVICE_DESC desc{};
        desc.CountDesc.Buffer  = 512;
        desc.CountDesc.DSV     = 32;
        desc.CountDesc.RTV     = 32;
        desc.CountDesc.Sampler = 32;

        if ( !m_Device.Init( &desc ) )
        {
            ELOG( "Error : Device::Init() Failed." );
            return false;
        }
    }

    // デバイスコンテキストの初期化.
    {
        if ( !m_DeviceContext.Init( m_Device.GetDevice() ) )
        {
            ELOG( "Error : DeviceContext::Init() Failed." );
            return false;
        }
    }

    // スワップチェインの生成.
    {
        DXGI_SWAP_CHAIN_DESC desc{};
        desc.BufferDesc.Width   = m_Width;
        desc.BufferDesc.Height  = m_Height;
        desc.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        desc.BufferCount        = BufferCount;
        desc.OutputWindow       = m_hWnd;
        desc.Windowed           = TRUE;
        desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags              = 0;

        if ( !m_Device.CreateSwapChain( m_DeviceContext.GetQueue(), &desc, m_SwapChain.GetAddress() ) )
        {
            ELOG( "Error : Device::CreateSwapChain() Failed.");
            return false;
        }

        // フレームバッファ番号を設定.
        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
    }

    // レンダーターゲットビューを生成.
    {
        for( u32 i=0; i<BufferCount; ++i )
        {
            if ( !m_ColorTarget[i].Init( m_Device, m_SwapChain.GetPtr(), i ) )
            {
                ELOG( "Error : ColorTarget::Init() Failed." );
                return false;
            }
        }
    }

    // 深度ステンシルビューを生成.
    {
        if ( !m_DepthTarget.Init( m_Device, m_Width, m_Height, DXGI_FORMAT_D32_FLOAT ) )
        {
            ELOG( "Error : DepthTarget::Init() Failed." );
            return false;
        }
    }

    // ビューポートとシザー矩形の設定.
    {
        m_Viewport.TopLeftX = 0;
        m_Viewport.TopLeftY = 0;
        m_Viewport.Width    = static_cast<f32>( m_Width );
        m_Viewport.Height   = static_cast<f32>( m_Height );
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;

        m_ScissorRect.left   = 0;
        m_ScissorRect.right  = m_Width;
        m_ScissorRect.top    = 0;
        m_ScissorRect.bottom = m_Height;
    }

    // ルートシグニチャを生成.
    {
        D3D12_DESCRIPTOR_RANGE range[3];
        range[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        range[0].NumDescriptors                    = 1;
        range[0].BaseShaderRegister                = 0;
        range[0].RegisterSpace                     = 0;
        range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        range[1].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[1].NumDescriptors                    = 1;
        range[1].BaseShaderRegister                = 0;
        range[1].RegisterSpace                     = 0;
        range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        range[2].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        range[2].NumDescriptors                    = 1;
        range[2].BaseShaderRegister                = 1;
        range[2].RegisterSpace                     = 0;
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
        hr = m_Device->CreateRootSignature(
            0,
            pSignature->GetBufferPointer(),
            pSignature->GetBufferSize(),
            IID_PPV_ARGS(m_RootSignature.GetAddress()) );
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
        if ( !asdx::SearchFilePath( L"BasicVS.cso", path ) )
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
        if ( !asdx::SearchFilePath( L"BasicPS.cso", path ) )
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
            { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "BONE_INDEX",  0, DXGI_FORMAT_R32G32_UINT,        0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "BONE_WEIGHT", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        // パイプラインステートの設定.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = asdx::RenderState::DefaultPipelineStateDesc;
        desc.InputLayout                        = { inputElements, _countof(inputElements) };
        desc.pRootSignature                     = m_RootSignature.GetPtr();
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
        hr = m_Device->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS(m_PSO.GetAddress()) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateGraphicsPipelineState() Failed." );
            return false;
        }
    }


    // アスペクト比算出.
    auto aspectRatio = static_cast<FLOAT>( m_Width ) / static_cast<FLOAT>( m_Height );

    m_TransformParam.World = asdx::Matrix::CreateIdentity();
    m_TransformParam.View  = asdx::Matrix::CreateLookAt( 
        asdx::Vector3(0.0f, 15.0f, -35.0f),
        asdx::Vector3(0.0f, 10.0f, 0.0f),
        asdx::Vector3(0.0f, 1.0f, 0.0f));
    m_TransformParam.Proj  = asdx::Matrix::CreatePerspectiveFieldOfView( asdx::F_PIDIV4, aspectRatio, 1.0f, 1000.0f );

    if ( !InitModel() )
    {
        ELOG( "Error : InitModel() Failed." );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      モデルの初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool App::InitModel()
{
    // モデルデータ読み込み.
    {
        std::wstring meshPath;
        if ( !asdx::SearchFilePath( L"res/model/pronama-chan/プロ生ちゃん.msh", meshPath ) )
        {
            ELOG( "Error : File Not Found." );
            return false;
        }

        std::wstring matPath;
        if ( !asdx::SearchFilePath( L"res/model/pronama-chan/プロ生ちゃん.mat", matPath ) )
        {
            ELOG( "Error : File Not Found." );
            return false;
        }

        if ( !m_Model.Init( m_Device, m_DeviceContext, meshPath.c_str(), matPath.c_str() ) )
        {
            ELOG( "Error : Model::Init() Failed." );
            return false;
        }
    }

    // モーション ファイル読み込み.
    {
        std::wstring path;
        if ( !asdx::SearchFilePath( L"res/motion/daisuke.mtn", path ) )
        {
            ELOG( "Error : File Not Found." );
            return false;
        }

        if ( !asdx::MotionFactory::Create( path.c_str(), &m_Motion ) )
        {
            ELOG( "Error : Motion::Load() Failed." );
            return false;
        }

        m_MotionPlayer.Bind(
            m_Model.GetBoneCount(),
            m_Model.GetBones() );
        m_MotionPlayer.SetMotion( &m_Motion );
        m_MotionPlayer.SetLoop( false );
        m_MotionPlayer.Update( 0.0f );
    }

    // 定数バッファ生成.

    {
        u32 size = asdx::RoundUp(
            static_cast<u32>( sizeof(m_TransformParam) + sizeof(asdx::Matrix) * 256 ),
            256 );

        if ( !m_TransformCB.Init( m_Device.GetDevice(), size ) )
        {
            ELOG( "Error : ConstantBuffer::Init() Failed." );
            return false;
        }

        m_TransformCB.Update( &m_TransformParam, sizeof(m_TransformParam) );
        m_TransformCB.Update( m_MotionPlayer.GetSkinTransforms(), sizeof(asdx::Matrix) * m_MotionPlayer.GetTransformCount(), 0, sizeof(m_TransformParam) );

        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
        desc.SizeInBytes    = size;
        desc.BufferLocation = m_TransformCB->GetGPUVirtualAddress();

        m_TransformHandle = m_Device.CreateCBV( &desc );

    }

    if ( !m_Bundle.Init( m_Device.GetDevice(), D3D12_COMMAND_LIST_TYPE_BUNDLE, m_PSO.GetPtr() ) )
    {
        ELOG( "Error : Bundle Create Failed." );
        return false;
    }

    {
        m_Device.MakeSetDescHeapCmd( m_Bundle.GetList() );
        m_Bundle->SetGraphicsRootSignature( m_RootSignature.GetPtr() );
        m_Model.DrawCmd( m_Bundle.GetList() );
        m_Bundle->Close();
    }

    // サウンドデータ読み込み.
    {
        std::wstring path;
        if ( !asdx::SearchFilePath( L"res/music/daisuke.wav", path ) )
        {
            ELOG( "Error : File Not Found." );
            return false;
        }

        // ファイルオープン.
        if ( !asdx::SndMgr::GetInstance().Open( 0, path.c_str() ) )
        {
            ELOG( "Error : SndMgr::Open() Failed." );
            return false;
        }

        m_IsPlay = false;
    }


    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void App::OnTerm()
{
    TermModel();

    m_DeviceContext.Term();
    m_Device.Term();
}

//-------------------------------------------------------------------------------------------------
//      モデルの終了処理です.
//-------------------------------------------------------------------------------------------------
void App::TermModel()
{
    // ファイルクローズ.
    asdx::SndMgr::GetInstance().Close( 0 );

    m_Model.Term();
    m_MotionPlayer.Unbind();
}

//-------------------------------------------------------------------------------------------------
//      フレーム遷移処理です.
//-------------------------------------------------------------------------------------------------
void App::OnFrameMove(const asdx::FrameEventArgs& args)
{
    m_StopWatch.End();
    if ( m_StopWatch.GetElapsedSec() > 0.8 && m_IsPlay )
    {
        m_MotionPlayer.Update( f32(args.ElapsedSec) * 30.0f );
        m_TransformCB.Update( m_MotionPlayer.GetSkinTransforms(), sizeof(asdx::Matrix) * m_MotionPlayer.GetTransformCount(), 0, sizeof(m_TransformParam) );
    }
}

//-------------------------------------------------------------------------------------------------
//      フレーム描画処理です.
//-------------------------------------------------------------------------------------------------
void App::OnFrameRender(const asdx::FrameEventArgs& args)
{
    m_DeviceContext.Clear( m_PSO.GetPtr() );

    m_Device.MakeSetDescHeapCmd( m_DeviceContext.GetGraphicsCommandList() );
    m_DeviceContext->SetGraphicsRootSignature( m_RootSignature.GetPtr() );

    m_DeviceContext.Transition(
        m_ColorTarget[m_FrameIndex].GetResource(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    m_DeviceContext->RSSetViewports( 1, &m_Viewport );
    m_DeviceContext->RSSetScissorRects( 1, &m_ScissorRect );

    const FLOAT clearColor[] = { 0.39f, 0.58f, 0.92f, 0.0f };
    auto handleRTV = m_ColorTarget[m_FrameIndex].GetRTV().GetHandleCpu();
    auto handleDSV = m_DepthTarget.GetDSV().GetHandleCpu();
    m_DeviceContext->OMSetRenderTargets( 1, &handleRTV, FALSE, &handleDSV );
    m_DeviceContext->ClearRenderTargetView( handleRTV, clearColor, 0, nullptr );
    m_DeviceContext->ClearDepthStencilView( handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );

    auto handleCBV = m_TransformHandle.GetHandleGpu();
    m_DeviceContext->SetGraphicsRootDescriptorTable( 0, handleCBV );

    m_Model.DrawCmd( m_DeviceContext.GetGraphicsCommandList() );

    m_DeviceContext.Transition(
        m_ColorTarget[m_FrameIndex].GetResource(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    m_DeviceContext->Close();
    m_DeviceContext.Execute();

    m_SwapChain->Present( 1, 0 );
    m_DeviceContext.Wait( INFINITE );

    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

//-------------------------------------------------------------------------------------------------
//      リサイズ処理です.
//-------------------------------------------------------------------------------------------------
void App::OnResize(const asdx::ResizeEventArgs& args)
{
}

void App::OnKey(const asdx::KeyEventArgs& args)
{
    if (args.IsKeyDown)
    {
        if (args.KeyCode == 'S' )
        {
            if (!m_IsPlay)
            {
                // 再生.
                asdx::SndMgr::GetInstance().Play( 0, 1 );
                m_IsPlay = true;
                m_StopWatch.Start();
            }
        }
    }
}