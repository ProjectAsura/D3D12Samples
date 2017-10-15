//-------------------------------------------------------------------------------------------------
// File : asdxTarget.cpp
// Desc : Render Target.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTarget.h>


namespace {

//-------------------------------------------------------------------------------------------------
//      sRGBフォーマットに変更します.
//-------------------------------------------------------------------------------------------------
DXGI_FORMAT ToSRGB(DXGI_FORMAT format)
{
    DXGI_FORMAT result = format;
    switch(format)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        result = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        break;

    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
        result = DXGI_FORMAT_BC1_UNORM_SRGB;
        break;

    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
        result = DXGI_FORMAT_BC2_UNORM_SRGB;
        break;

    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
        result = DXGI_FORMAT_BC3_UNORM_SRGB;
        break;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        result = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        break;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        result = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
        break;

    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        result = DXGI_FORMAT_BC7_UNORM_SRGB;
        break;
    }

    return result;
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ColorTarget class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ColorTarget::ColorTarget()
: m_pResource()
, m_pRTV     ()
, m_pSRV     ()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ColorTarget::~ColorTarget()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool ColorTarget::Init( ID3D12Device* pDevice, const TargetDesc* pDesc, bool enableSRGB )
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
            pDesc->Width,
            pDesc->Height,
            1,
            0,
            pDesc->Format,
            { 1, 0 },
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        };

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format = pDesc->Format;
        clearValue.Color[0] = 1.0f;
        clearValue.Color[1] = 1.0f;
        clearValue.Color[2] = 1.0f;
        clearValue.Color[3] = 1.0f;

        hr = pDevice->CreateCommittedResource( 
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if ( FAILED( hr ) )
        { return false; }
    }

    auto format = pDesc->Format;
    if (enableSRGB)
    { format = ToSRGB(format); }

    if (pDesc->pHeapRT != nullptr)
    {
        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        desc.Format             = format;
        desc.Texture2D.MipSlice = 0;

        m_pRTV = pDesc->pHeapRT->CreateDescriptor();
        if (!m_pRTV)
        { return false; }

        pDevice->CreateRenderTargetView(m_pResource.GetPtr(), &desc, m_pRTV->GetHandleCPU());
    }

    if (pDesc->pHeapSRV != nullptr)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format = format;
        desc.Texture2D.MipLevels = 1;
        desc.Texture2D.MostDetailedMip = 0;

        m_pSRV = pDesc->pHeapSRV->CreateDescriptor();
        if (!m_pSRV)
        { return false;}

        pDevice->CreateShaderResourceView(m_pResource.GetPtr(), &desc, m_pSRV->GetHandleCPU());
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool ColorTarget::Init
(
    ID3D12Device*   pDevice,
    DescriptorHeap* pHeapRT,
    IDXGISwapChain* pSwapChain,
    uint32_t        index,
    bool            enableSRGB
)
{
    if (pDevice == nullptr || pHeapRT == nullptr || pSwapChain == nullptr)
    { return false; }

    HRESULT hr = S_OK;

    hr = pSwapChain->GetBuffer( index, IID_PPV_ARGS(m_pResource.GetAddress()));
    if ( FAILED( hr ) )
    {
        return false;
    }

    auto desc = m_pResource->GetDesc();
    if ( desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D )
    {
        return false;
    }

    DXGI_FORMAT format = desc.Format;
    if (enableSRGB)
    { format = ToSRGB(format); }

    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format              = format;

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

        m_pRTV = pHeapRT->CreateDescriptor();
        if (!m_pRTV)
        { return false; }

        pDevice->CreateRenderTargetView(m_pResource.GetPtr(), &rtvDesc, m_pRTV->GetHandleCPU());
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void ColorTarget::Term()
{
    m_pResource.Reset();
    m_pRTV.Reset();
    m_pSRV.Reset();
}

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* ColorTarget::GetResource() const
{ return m_pResource.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* ColorTarget::GetRTV() const
{ return m_pRTV.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
const Descriptor* ColorTarget::GetSRV() const
{ return m_pSRV.GetPtr(); }


///////////////////////////////////////////////////////////////////////////////////////////////////
// DepthTarget class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DepthTarget::DepthTarget()
: m_pResource()
, m_pDSV     ()
, m_pSRV     ()
{ /* DO_NOTHING */  }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DepthTarget::~DepthTarget()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DepthTarget::Init( ID3D12Device* pDevice, const TargetDesc* pDesc )
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
            pDesc->Width,
            pDesc->Height,
            1,
            0,
            pDesc->Format,
            { 1, 0 },
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        };

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format               = pDesc->Format;
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        hr = pDevice->CreateCommittedResource( 
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if ( FAILED( hr ) )
        {
            return false;
        }
    }

    if (pDesc->pHeapRT != nullptr)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
        desc.Format        = pDesc->Format;
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        desc.Flags         = D3D12_DSV_FLAG_NONE;

        m_pDSV = pDesc->pHeapRT->CreateDescriptor();
        if (!m_pDSV)
        { return false; }

        pDevice->CreateDepthStencilView(m_pResource.GetPtr(), &desc, m_pDSV->GetHandleCPU());
    }

    if (pDesc->pHeapSRV != nullptr)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format                    = pDesc->Format;
        desc.Texture2D.MipLevels       = 1;
        desc.Texture2D.MostDetailedMip = 0;

        if (!m_pSRV)
        { return false; }

        pDevice->CreateShaderResourceView(m_pResource.GetPtr(), &desc, m_pSRV->GetHandleCPU());
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DepthTarget::Term()
{
    m_pResource.Reset();
    m_pDSV.Reset();
    m_pSRV.Reset();
}

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* DepthTarget::GetResource() const
{ return m_pResource.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューです.
//-------------------------------------------------------------------------------------------------
const Descriptor* DepthTarget::GetDSV() const
{ return m_pDSV.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューです.
//-------------------------------------------------------------------------------------------------
const Descriptor* DepthTarget::GetSRV() const
{ return m_pSRV.GetPtr(); }

} // namespace asdx
