//-------------------------------------------------------------------------------------------------
// File : asdxResource.cpp
// Desc : Resource Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxResource.h>
#include <asdxLogger.h>
#include <cassert>


namespace asdx {

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
const D3D12_RESOURCE_DESC Resource::DescBuffer = {
    D3D12_RESOURCE_DIMENSION_BUFFER,
    0,
    0,
    1,
    1,
    1,
    DXGI_FORMAT_UNKNOWN,
    { 1, 0 },
    D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
    D3D12_RESOURCE_FLAG_NONE
};

const D3D12_RESOURCE_DESC Resource::DescTexture1D = {
    D3D12_RESOURCE_DIMENSION_TEXTURE1D,
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

const D3D12_RESOURCE_DESC Resource::DescTexture2D = {
    D3D12_RESOURCE_DIMENSION_TEXTURE2D,
    0,
    0,
    0,
    1,
    0,
    DXGI_FORMAT_UNKNOWN,
    { 1, 0 },
    D3D12_TEXTURE_LAYOUT_UNKNOWN,
    D3D12_RESOURCE_FLAG_NONE
};

const D3D12_RESOURCE_DESC Resource::DescTexture3D = {
    D3D12_RESOURCE_DIMENSION_TEXTURE3D,
    0,
    0,
    0,
    0,
    0,
    DXGI_FORMAT_UNKNOWN,
    { 1, 0 },
    D3D12_TEXTURE_LAYOUT_UNKNOWN,
    D3D12_RESOURCE_FLAG_NONE
};

const D3D12_HEAP_PROPERTIES Resource::HeapProps = {
    D3D12_HEAP_TYPE_DEFAULT,
    D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    D3D12_MEMORY_POOL_UNKNOWN,
    1,
    1
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////
// Resource class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Resource::Resource()
: m_Resource()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Resource::~Resource()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Resource::Init
(
    ID3D12Device*                   pDevice,
    const D3D12_RESOURCE_DESC*      pDesc,
    const D3D12_HEAP_PROPERTIES*    pProps,
    const D3D12_HEAP_FLAGS          heapFlags,
    const D3D12_RESOURCE_STATES     states
)
{
    assert( pDevice != nullptr );
    assert( pDesc   != nullptr );
    assert( pProps  != nullptr );

    auto hr = pDevice->CreateCommittedResource(
        pProps,
        heapFlags,
        pDesc,
        states,
        nullptr,
        IID_PPV_ARGS(m_Resource.GetAddress()));
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed." );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Resource::Term()
{ m_Resource.Reset(); }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* Resource::GetPtr() const
{ return m_Resource.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      アロー演算子です.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* Resource::operator -> () const
{ return m_Resource.GetPtr(); }



} // namespace asdx
