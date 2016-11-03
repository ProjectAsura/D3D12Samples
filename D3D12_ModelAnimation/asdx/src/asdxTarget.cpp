//-------------------------------------------------------------------------------------------------
// File : asdxTarget.cpp
// Desc : Render Target Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTarget.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ColorTarget class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ColorTarget::ColorTarget()
: m_Resource()
, m_RTV     ()
, m_SRV     ()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ColorTarget::~ColorTarget()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool ColorTarget::Init( Device& device, u32 width, u32 height, DXGI_FORMAT format )
{
    HRESULT hr = S_OK;

    {
        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1    
        };

        D3D12_RESOURCE_DESC desc = {
            D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            0,
            width,
            height,
            1,
            0,
            format,
            { 1, 0 },
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        };

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format = format;
        clearValue.Color[0] = 1.0f;
        clearValue.Color[1] = 1.0f;
        clearValue.Color[2] = 1.0f;
        clearValue.Color[3] = 1.0f;

        hr = device->CreateCommittedResource( 
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource()" );
            return false;
        }
    }

    {
        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        desc.Format             = format;
        desc.Texture2D.MipSlice = 0;

        m_RTV = device.CreateRTV( m_Resource.GetPtr(), &desc );
    }

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format = format;
        desc.Texture2D.MipLevels = 1;
        desc.Texture2D.MostDetailedMip = 0;

        m_SRV = device.CreateSRV( m_Resource.GetPtr(), &desc );
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool ColorTarget::Init( Device& device, IDXGISwapChain* pSwapChain, u32 backBufferIndex )
{
    HRESULT hr = S_OK;

    hr = pSwapChain->GetBuffer( backBufferIndex, IID_PPV_ARGS(m_Resource.GetAddress()));
    if ( FAILED( hr ) )
    {
        ELOG( "Error : IDXGISwapChain::GetBuffer() Failed." );
        return false;
    }

    auto desc = m_Resource->GetDesc();
    if ( desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D )
    {
        ELOG( "Error : Invalid Resource Dimension. ");
        return false;
    }

    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format              = desc.Format;

        if ( desc.DepthOrArraySize > 1 )
        {
            if ( desc.MipLevels <= 1 )
            {
                rtvDesc.Texture2DMSArray.ArraySize       = desc.DepthOrArraySize;
                rtvDesc.Texture2DMSArray.FirstArraySlice = 0;
            }
            else
            {
                rtvDesc.Texture2DArray.ArraySize        = desc.DepthOrArraySize;
                rtvDesc.Texture2DArray.FirstArraySlice  = 0;
                rtvDesc.Texture2DArray.MipSlice         = 0;
                rtvDesc.Texture2DArray.PlaneSlice       = 0;
            }
        }
        else
        {
            if ( desc.MipLevels <= 1 )
            {
                rtvDesc.Texture2D.MipSlice = 0;
            }
        }

        m_RTV = device.CreateRTV( m_Resource.GetPtr(), nullptr );
    }

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                    = desc.Format;
        srvDesc.Texture2D.MipLevels       = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;

        m_SRV = device.CreateSRV( m_Resource.GetPtr(), &srvDesc );
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void ColorTarget::Term()
{
    m_Resource.Reset();
    m_RTV = DescHandle();
    m_SRV = DescHandle();
}

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* ColorTarget::GetResource() const
{ return m_Resource.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューを取得します.
//-------------------------------------------------------------------------------------------------
const DescHandle& ColorTarget::GetRTV() const
{ return m_RTV; }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
const DescHandle& ColorTarget::GetSRV() const
{ return m_SRV; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// DepthTarget class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DepthTarget::DepthTarget()
: m_Resource()
, m_DSV     ()
, m_SRV     ()
{ /* DO_NOTHING */  }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DepthTarget::~DepthTarget()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DepthTarget::Init( Device& device, u32 width, u32 height, DXGI_FORMAT format )
{
    HRESULT hr = S_OK;

    {
        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1    
        };

        D3D12_RESOURCE_DESC desc = {
            D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            0,
            width,
            height,
            1,
            0,
            format,
            { 1, 0 },
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        };

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format               = format;
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        hr = device->CreateCommittedResource( 
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource()" );
            return false;
        }
    }

    {
        D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
        desc.Format        = format;
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        desc.Flags         = D3D12_DSV_FLAG_NONE;

        m_DSV = device.CreateDSV( m_Resource.GetPtr(), &desc );
    }

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format                    = format;
        desc.Texture2D.MipLevels       = 1;
        desc.Texture2D.MostDetailedMip = 0;

        m_SRV = device.CreateSRV( m_Resource.GetPtr(), &desc );
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DepthTarget::Term()
{
    m_Resource.Reset();
    m_DSV = DescHandle();
    m_SRV = DescHandle();
}

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* DepthTarget::GetResource() const
{ return m_Resource.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューです.
//-------------------------------------------------------------------------------------------------
const DescHandle& DepthTarget::GetDSV() const
{ return m_DSV; }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューです.
//-------------------------------------------------------------------------------------------------
const DescHandle& DepthTarget::GetSRV() const
{ return m_SRV; }

} // namespace asdx
