//-------------------------------------------------------------------------------------------------
// File : Shader.cpp
// Desc : Shader Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <d3dcompiler.h>
#include <d3d12shader.h>
#include <asdxShader.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Shader class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Shader::Shader()
: m_Blob()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Shader::~Shader()
{ m_Blob.Reset(); }

//-------------------------------------------------------------------------------------------------
//      ソースコードからコンパイルします.
//-------------------------------------------------------------------------------------------------
bool Shader::Compile(const char16* filename, const char8* entryPoint, const char8* shaderModel )
{
    if ( filename == nullptr || entryPoint == nullptr || shaderModel == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    m_Blob.Reset();
    RefPtr<ID3DBlob> errorBlob;

    auto hr = D3DCompileFromFile(
        filename,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        0,
        0,
        m_Blob.GetAddress(),
        errorBlob.GetAddress());

    if ( FAILED(hr) )
    {
        ELOG( "Error : D3DCompileFromFile() Failed. filename = %s, entryPoint = %s, shaderModel = %s",
            filename,
            entryPoint,
            shaderModel );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      コンパイル済みバイナリをロードします.
//-------------------------------------------------------------------------------------------------
bool Shader::Load( const char16* filename )
{
    if ( filename == nullptr )
    { return false; }

    m_Blob.Reset();

    auto hr = D3DReadFileToBlob(filename, m_Blob.GetAddress());
    if ( FAILED(hr) )
    {
        ELOG( "Error : D3DReadFileToBlob() Failed. filename = %s", filename );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      入力要素を取得します.
//-------------------------------------------------------------------------------------------------
bool Shader::GetInputElements( std::vector<D3D12_INPUT_ELEMENT_DESC>& elementDesc )
{
    if ( m_Blob->GetBufferPointer() == nullptr || m_Blob->GetBufferSize() == 0 )
    {
        ELOG( "Error : Invalid Blob." );
        return false;
    }

    RefPtr<ID3D12ShaderReflection> pVSR;

    auto hr = D3DReflect(
        m_Blob->GetBufferPointer(),
        m_Blob->GetBufferSize(),
        IID_PPV_ARGS( pVSR.GetAddress() ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : D3DReflect() Failed." );
        return false;
    }

    D3D12_SHADER_DESC shaderDesc;
    hr = pVSR->GetDesc( &shaderDesc );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12ShaderReflection::GetDesc() Failed." );
        return false;
    }

    elementDesc.resize( shaderDesc.InputParameters );

    for( u32 i=0; i<shaderDesc.InputParameters; ++i )
    {
        D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
        pVSR->GetInputParameterDesc( i, &paramDesc );

        D3D12_INPUT_ELEMENT_DESC inputElementDesc;
        inputElementDesc.SemanticName           = paramDesc.SemanticName;
        inputElementDesc.SemanticIndex          = paramDesc.SemanticIndex;
        inputElementDesc.InputSlot              = 0;
        inputElementDesc.AlignedByteOffset      = D3D12_APPEND_ALIGNED_ELEMENT;
        inputElementDesc.InputSlotClass         = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        inputElementDesc.InstanceDataStepRate   = 0;

        if ( paramDesc.Mask == 1 )
        {
            switch( paramDesc.ComponentType )
            {
            case D3D_REGISTER_COMPONENT_UINT32:
                inputElementDesc.Format = DXGI_FORMAT_R32_UINT;
                break;

            case D3D_REGISTER_COMPONENT_SINT32:
                inputElementDesc.Format = DXGI_FORMAT_R32_SINT;
                break;

            case D3D_REGISTER_COMPONENT_FLOAT32:
                inputElementDesc.Format = DXGI_FORMAT_R32_FLOAT;
                break;

            case D3D_REGISTER_COMPONENT_UNKNOWN:
            default:
                inputElementDesc.Format = DXGI_FORMAT_UNKNOWN;
                break;
            }
        }
        else if ( paramDesc.Mask <= 3 )
        {
            switch( paramDesc.ComponentType )
            {
            case D3D_REGISTER_COMPONENT_UINT32:
                inputElementDesc.Format = DXGI_FORMAT_R32G32_UINT;
                break;

            case D3D_REGISTER_COMPONENT_SINT32:
                inputElementDesc.Format = DXGI_FORMAT_R32G32_SINT;
                break;

            case D3D_REGISTER_COMPONENT_FLOAT32:
                inputElementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
                break;

            case D3D_REGISTER_COMPONENT_UNKNOWN:
            default:
                inputElementDesc.Format = DXGI_FORMAT_UNKNOWN;
                break;
            }
        }
        else if ( paramDesc.Mask <= 7 )
        {
            switch( paramDesc.ComponentType )
            {
            case D3D_REGISTER_COMPONENT_UINT32:
                inputElementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
                break;

            case D3D_REGISTER_COMPONENT_SINT32:
                inputElementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
                break;

            case D3D_REGISTER_COMPONENT_FLOAT32:
                inputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
                break;

            case D3D_REGISTER_COMPONENT_UNKNOWN:
            default:
                inputElementDesc.Format = DXGI_FORMAT_UNKNOWN;
                break;
            }
        }
        else if ( paramDesc.Mask <= 15 )
        {
            switch( paramDesc.ComponentType )
            {
            case D3D_REGISTER_COMPONENT_UINT32:
                inputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
                break;

            case D3D_REGISTER_COMPONENT_SINT32:
                inputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
                break;

            case D3D_REGISTER_COMPONENT_FLOAT32:
                inputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                break;

            case D3D_REGISTER_COMPONENT_UNKNOWN:
            default:
                inputElementDesc.Format = DXGI_FORMAT_UNKNOWN;
                break;
            }
        }

        elementDesc[i] = inputElementDesc;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      バイトコードを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_SHADER_BYTECODE Shader::GetByteCode()
{
    return D3D12_SHADER_BYTECODE{
        m_Blob->GetBufferPointer(),
        m_Blob->GetBufferSize()
    };
}

}// namespace asdx
