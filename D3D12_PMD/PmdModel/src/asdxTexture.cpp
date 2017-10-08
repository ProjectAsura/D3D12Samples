//-------------------------------------------------------------------------------------------------
// File : asdxTexture.cpp
// Desc : Texture Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTexture.h>
#include <asdxLogger.h>
#include <cassert>


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      サブリソースをコピーします.
//-------------------------------------------------------------------------------------------------
void CopySubresource
(
    const D3D12_MEMCPY_DEST*        pDst,
    const D3D12_SUBRESOURCE_DATA*   pSrc,
    size_t                          rowSizeInBytes,
    u32                             rowCounts,
    u32                             sliceCount
)
{
    for ( u32 z=0; z <sliceCount; ++z )
    {
        auto       pDstSlice = reinterpret_cast<u8*>(pDst->pData) + pDst->SlicePitch * z;
        const auto pSrcSlice = reinterpret_cast<const u8*>(pSrc->pData) + pSrc->SlicePitch * z;
        for ( u32 y=0; y<rowCounts; ++y )
        {
            memcpy(pDstSlice + pDst->RowPitch * y,
                   pSrcSlice + pSrc->RowPitch * y,
                   rowSizeInBytes );
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      データのアップロードに必要なサイズを取得します.
//-------------------------------------------------------------------------------------------------
u64 GetRequiredIntermediateSize
(
    ID3D12Resource* pDstResource,
    u32             firstSubresource,
    u32             subResourceCount
)
{
    auto desc = pDstResource->GetDesc();
    u64 result = 0;
    
    ID3D12Device* pDevice;
    pDstResource->GetDevice( IID_PPV_ARGS( &pDevice ) );
    assert( pDevice != nullptr );
    pDevice->GetCopyableFootprints(
        &desc, 
        firstSubresource,
        subResourceCount,
        0,
        nullptr,
        nullptr,
        nullptr,
        &result );
    pDevice->Release();
    
    return result;
}

//-------------------------------------------------------------------------------------------------
//      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
inline u64 UpdateSubresources
(
    ID3D12GraphicsCommandList* pCmdList,
    ID3D12Resource* pDstResource,
    ID3D12Resource* pIntermediate,
    u32 firstSubresource,
    u32 subResourceCount,
    u64 requiredSize,
    const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
    const u32* pRowCounts,
    const u64* pRowSizesInBytes,
    const D3D12_SUBRESOURCE_DATA* pSrcData
)
{
    auto imdDesc = pIntermediate->GetDesc();
    auto dstDesc = pDstResource->GetDesc();
    if ( imdDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER || 
         imdDesc.Width < requiredSize + pLayouts[0].Offset || 
         requiredSize > size_t(-1) || 
         (dstDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && (firstSubresource != 0 || subResourceCount != 1)) )
    { return 0; }
    
    u8* pData;
    auto hr = pIntermediate->Map(0, nullptr, reinterpret_cast<void**>(&pData));
    if ( FAILED( hr ) )
    { return 0; }
    
    for ( u32 i=0; i <subResourceCount; ++i )
    {
        if ( pRowSizesInBytes[i] > size_t(-1) )
        { return 0; }

        D3D12_MEMCPY_DEST dstData = { 
            pData + pLayouts[i].Offset, 
            pLayouts[i].Footprint.RowPitch,
            pLayouts[i].Footprint.RowPitch * pRowCounts[i]
        };
        CopySubresource(
            &dstData,
            &pSrcData[i],
            size_t(pRowSizesInBytes[i]),
            pRowCounts[i],
            pLayouts[i].Footprint.Depth );
    }
    pIntermediate->Unmap(0, nullptr);
    
    if ( dstDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER )
    {
        D3D12_BOX srcBox;
        srcBox.left   = u32( pLayouts[0].Offset );
        srcBox.right  = u32( pLayouts[0].Offset + pLayouts[0].Footprint.Width );
        srcBox.top    = 0;
        srcBox.front  = 0;
        srcBox.bottom = 1;
        srcBox.back   = 1;
        pCmdList->CopyBufferRegion(
            pDstResource,
            0,
            pIntermediate,
            pLayouts[0].Offset,
            pLayouts[0].Footprint.Width );
    }
    else
    {
        for ( u32 i=0; i<subResourceCount; ++i )
        {
            D3D12_TEXTURE_COPY_LOCATION dst;
            D3D12_TEXTURE_COPY_LOCATION src;

            dst.pResource        = pDstResource;
            dst.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dst.SubresourceIndex = i + firstSubresource;

            src.pResource       = pIntermediate;
            src.Type            = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            src.PlacedFootprint = pLayouts[i];

            pCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
        }
    }

    return requiredSize;
}

//-------------------------------------------------------------------------------------------------
//      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
u64 UpdateSubresources
( 
    ID3D12GraphicsCommandList*  pCmdList,
    ID3D12Resource*             pDstResource,
    ID3D12Resource*             pIntermediate,
    u64                         intermediateOffset,
    u32                         firstSubresource,
    u32                         subResourceCount,
    D3D12_SUBRESOURCE_DATA*     pSrcData
)
{
    u64 requiredSize = 0;
    auto bufferSize = static_cast<u64>(
        sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) +
        sizeof(u32) +
        sizeof(u64) ) * subResourceCount;
    if ( bufferSize > SIZE_MAX )
    { return 0; }

    auto buffer = HeapAlloc(GetProcessHeap(), 0, static_cast<size_t>(bufferSize));
    if (buffer == nullptr)
    { return 0; }

    auto pLayouts         = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(buffer);
    auto pRowSizesInBytes = reinterpret_cast<u64*>(pLayouts + subResourceCount);
    auto pRowCounts       = reinterpret_cast<u32*>(pRowSizesInBytes + subResourceCount);
    
    auto desc = pDstResource->GetDesc();
    ID3D12Device* pDevice;
    pDstResource->GetDevice( IID_PPV_ARGS( &pDevice ) );
    pDevice->GetCopyableFootprints(
        &desc,
        firstSubresource,
        subResourceCount,
        intermediateOffset,
        pLayouts,
        pRowCounts,
        pRowSizesInBytes,
        &requiredSize);
    pDevice->Release();
    
    auto result = UpdateSubresources(
        pCmdList,
        pDstResource,
        pIntermediate,
        firstSubresource,
        subResourceCount,
        requiredSize,
        pLayouts,
        pRowCounts,
        pRowSizesInBytes,
        pSrcData);

    HeapFree(GetProcessHeap(), 0, buffer);

    return result;
}

} // namespace /* anonymous */


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Texture::Desc class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Texture::Desc::Desc()
: IsCubeMap( false )
{
    ResourceDesc = {
        D3D12_RESOURCE_DIMENSION_UNKNOWN,
        0,
        0,
        1,
        1,
        0,
        DXGI_FORMAT_UNKNOWN,
        { 1, 0 },
        D3D12_TEXTURE_LAYOUT_UNKNOWN,
        D3D12_RESOURCE_FLAG_NONE
    };
    HandleCPU.ptr = 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Texture class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Texture::Texture()
: m_Resource()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Texture::~Texture()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool Texture::Init( ID3D12Device* pDevice, Desc& desc )
{
    if ( pDevice == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    if ( desc.ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE1D &&
         desc.ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D &&
         desc.ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D)
    {
        ELOG( "Error : Invalid Resource Dimension." );
        return false;
    }

    HRESULT hr = S_OK;

    D3D12_HEAP_PROPERTIES props = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1,
        1
    };

    m_State = D3D12_RESOURCE_STATE_COMMON;

    hr = pDevice->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc.ResourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS( m_Resource.GetAddress() ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed." );
        return false;
    }

    auto mipLevel = ( desc.ResourceDesc.MipLevels == 0 ) ? -1 : desc.ResourceDesc.MipLevels;

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.Format = desc.ResourceDesc.Format;

    switch( desc.ResourceDesc.Dimension )
    {
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        {
            if ( desc.ResourceDesc.DepthOrArraySize > 1 )
            {
                viewDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                viewDesc.Texture1DArray.ArraySize       = desc.ResourceDesc.DepthOrArraySize;
                viewDesc.Texture1DArray.FirstArraySlice = 0;
                viewDesc.Texture1DArray.MipLevels       = mipLevel;
            }
            else
            {
                viewDesc.ViewDimension       = D3D12_SRV_DIMENSION_TEXTURE1D;
                viewDesc.Texture1D.MipLevels = mipLevel;
            }
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        {
            if ( desc.IsCubeMap )
            {
                if ( desc.ResourceDesc.DepthOrArraySize > 6 )
                {
                    viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                    viewDesc.TextureCubeArray.MipLevels = mipLevel;
                    viewDesc.TextureCubeArray.NumCubes  = ( desc.ResourceDesc.DepthOrArraySize / 6 );
                }
                else
                {
                    viewDesc.ViewDimension         = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    viewDesc.TextureCube.MipLevels = mipLevel;
                }
            }
            else if ( desc.ResourceDesc.SampleDesc.Count > 1 )
            {
                if ( desc.ResourceDesc.DepthOrArraySize > 1 )
                {
                    viewDesc.ViewDimension                    = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                    viewDesc.Texture2DMSArray.ArraySize       = desc.ResourceDesc.DepthOrArraySize;
                    viewDesc.Texture2DMSArray.FirstArraySlice = 0;
                }
                else
                {
                    viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                }
            }
            else
            {
                if ( desc.ResourceDesc.DepthOrArraySize > 1 )
                {
                    viewDesc.ViewDimension            = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.ArraySize = desc.ResourceDesc.DepthOrArraySize;
                    viewDesc.Texture2DArray.MipLevels = mipLevel;
                }
                else
                {
                    viewDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
                    viewDesc.Texture2D.MipLevels       = mipLevel;
                    viewDesc.Texture2D.MostDetailedMip = 0;
                }
            }
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        {
            viewDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE3D;
            viewDesc.Texture3D.MipLevels       = mipLevel;
            viewDesc.Texture3D.MostDetailedMip = 0;
        }
        break;
    }

    pDevice->CreateShaderResourceView( m_Resource.GetPtr(), &viewDesc, desc.HandleCPU );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Texture::Term()
{
    m_Resource.Reset();
}

//-------------------------------------------------------------------------------------------------
//      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
bool Texture::Upload
(
    ID3D12GraphicsCommandList*  pCmdList,
    D3D12_SUBRESOURCE_DATA*     pData,
    ID3D12Resource**            ppIntermediate
)
{
    if ( pCmdList == nullptr || pData == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    RefPtr<ID3D12Device> device;
    m_Resource->GetDevice( IID_PPV_ARGS(device.GetAddress()));

    {
        // アップロード用リソースを生成.

        D3D12_RESOURCE_DESC uploadDesc = {
            D3D12_RESOURCE_DIMENSION_BUFFER,
            0,
            GetRequiredIntermediateSize( m_Resource.GetPtr(), 0, 1 ),
            1,
            1,
            1,
            DXGI_FORMAT_UNKNOWN,
            { 1, 0 },
            D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            D3D12_RESOURCE_FLAG_NONE
        };

        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_UPLOAD,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1
        };

        auto hr = device->CreateCommittedResource(
            &props,
            D3D12_HEAP_FLAG_NONE,
            &uploadDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(ppIntermediate));
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed." );
            return false;
        }
    }

    D3D12_RESOURCE_STATES oldState = m_State;
    m_State = D3D12_RESOURCE_STATE_COPY_DEST;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = m_Resource.GetPtr();
    barrier.Transition.StateBefore = oldState;
    barrier.Transition.StateAfter  = m_State;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    pCmdList->ResourceBarrier( 1, &barrier );

    UpdateSubresources(
        pCmdList,
        m_Resource.GetPtr(),
        (*ppIntermediate),
        0,
        0,
        1,
        pData );

    oldState = m_State;
    m_State = D3D12_RESOURCE_STATE_GENERIC_READ;

    barrier.Transition.StateBefore = oldState;
    barrier.Transition.StateAfter  = m_State;
    pCmdList->ResourceBarrier( 1, &barrier );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      アロー演算子です.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* Texture::operator -> () const
{ return m_Resource.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* Texture::GetResource() const
{ return m_Resource.GetPtr(); }

} // namespace asdx
