//-------------------------------------------------------------------------------------------------
// File : TextureHelper.cpp
// Desc : Texture Helper Functions.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <TextureHelper.h>
#include <asdxRef.h>
#include <cassert>


//-------------------------------------------------------------------------------------------------
//      サブリソースをコピーします.
//-------------------------------------------------------------------------------------------------
void CopySubresource
(
    const D3D12_MEMCPY_DEST*        pDst,
    const D3D12_SUBRESOURCE_DATA*   pSrc,
    SIZE_T                          rowSizeInBytes,
    UINT                            rowCounts,
    UINT                            sliceCount
)
{
    for ( UINT z=0; z <sliceCount; ++z )
    {
        auto       pDstSlice = reinterpret_cast<BYTE*>(pDst->pData) + pDst->SlicePitch * z;
        const auto pSrcSlice = reinterpret_cast<const BYTE*>(pSrc->pData) + pSrc->SlicePitch * z;
        for ( UINT y=0; y<rowCounts; ++y )
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
UINT64 GetRequiredIntermediateSize
(
    ID3D12Resource* pDstResource,
    UINT             firstSubresource,
    UINT             subResourceCount
)
{
    auto desc = pDstResource->GetDesc();
    UINT64 result = 0;
    
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
inline UINT64 UpdateSubresources
(
    //ID3D12GraphicsCommandList* pCmdList,
    IGraphicsCommandList* pCmdList,
    ID3D12Resource* pDstResource,
    ID3D12Resource* pIntermediate,
    UINT firstSubresource,
    UINT subResourceCount,
    UINT64 requiredSize,
    const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
    const UINT* pRowCounts,
    const UINT64* pRowSizesInBytes,
    const D3D12_SUBRESOURCE_DATA* pSrcData
)
{
    auto imdDesc = pIntermediate->GetDesc();
    auto dstDesc = pDstResource->GetDesc();
    if ( imdDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER || 
         imdDesc.Width < requiredSize + pLayouts[0].Offset || 
         requiredSize > SIZE_T(-1) || 
         (dstDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && (firstSubresource != 0 || subResourceCount != 1)) )
    { return 0; }
    
    BYTE* pData;
    auto hr = pIntermediate->Map(0, nullptr, reinterpret_cast<void**>(&pData));
    if ( FAILED( hr ) )
    { return 0; }
    
    for ( UINT i=0; i <subResourceCount; ++i )
    {
        if ( pRowSizesInBytes[i] > SIZE_T(-1) )
        { return 0; }

        D3D12_MEMCPY_DEST dstData = { 
            pData + pLayouts[i].Offset, 
            pLayouts[i].Footprint.RowPitch,
            pLayouts[i].Footprint.RowPitch * pRowCounts[i]
        };
        CopySubresource(
            &dstData,
            &pSrcData[i],
            SIZE_T(pRowSizesInBytes[i]),
            pRowCounts[i],
            pLayouts[i].Footprint.Depth );
    }
    pIntermediate->Unmap(0, nullptr);
    
    if ( dstDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER )
    {
        D3D12_BOX srcBox;
        srcBox.left   = UINT( pLayouts[0].Offset );
        srcBox.right  = UINT( pLayouts[0].Offset + pLayouts[0].Footprint.Width );
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
        for ( UINT i=0; i<subResourceCount; ++i )
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
UINT64 UpdateSubresources
( 
    //ID3D12GraphicsCommandList*  pCmdList,
    IGraphicsCommandList*       pCmdList,
    ID3D12Resource*             pDstResource,
    ID3D12Resource*             pIntermediate,
    UINT64                         intermediateOffset,
    UINT                         firstSubresource,
    UINT                         subResourceCount,
    D3D12_SUBRESOURCE_DATA*     pSrcData
)
{
    UINT64 requiredSize = 0;
    auto bufferSize = static_cast<UINT64>(
        sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) +
        sizeof(UINT) +
        sizeof(UINT64) ) * subResourceCount;
    if ( bufferSize > SIZE_MAX )
    { return 0; }

    auto buffer = HeapAlloc(GetProcessHeap(), 0, static_cast<SIZE_T>(bufferSize));
    if (buffer == nullptr)
    { return 0; }

    auto pLayouts         = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(buffer);
    auto pRowSizesInBytes = reinterpret_cast<UINT64*>(pLayouts + subResourceCount);
    auto pRowCounts       = reinterpret_cast<UINT*>(pRowSizesInBytes + subResourceCount);
    
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

bool UpdateSubresources
(
    //ID3D12GraphicsCommandList* pList,
    IGraphicsCommandList*       pList,
    ID3D12CommandQueue* pQueue,
    ID3D12Fence* pFence,
    HANDLE handle,
    ID3D12Resource* pResource,
    UINT firstSubResource,
    UINT subResourceCount,
    D3D12_SUBRESOURCE_DATA* pSrcData
)
{
    if ( pList     == nullptr ||
         pQueue    == nullptr ||
         pFence    == nullptr ||
         pResource == nullptr ||
         pSrcData  == nullptr )
    {
        return false;
    }

    asdx::RefPtr<ID3D12Device> device;
    pResource->GetDevice( IID_PPV_ARGS( device.GetAddress() )); 

    asdx::RefPtr<ID3D12Resource> intermediate;
    {
        // アップロード用リソースを生成.
        D3D12_RESOURCE_DESC uploadDesc = {
            D3D12_RESOURCE_DIMENSION_BUFFER,
            0,
            GetRequiredIntermediateSize( pResource, firstSubResource, subResourceCount ),
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
            IID_PPV_ARGS(intermediate.GetAddress()));
        if ( FAILED( hr ) )
        {
            return false;
        }
    }

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = pResource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;
    pList->ResourceBarrier( 1, &barrier );

    UpdateSubresources( 
        pList,
        pResource,
        intermediate.GetPtr(),
        0,
        firstSubResource,
        subResourceCount,
        pSrcData );

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_GENERIC_READ;
    pList->ResourceBarrier( 1, &barrier );

    pList->Close();

    //auto list = reinterpret_cast<ID3D12CommandList*>(pList);
    auto list = pList->GetCommandList();
    pQueue->ExecuteCommandLists(1, &list);

    auto fence = 1;
    auto hr = pQueue->Signal( pFence, fence );
    if ( FAILED(hr) )
    { return false; }

    if ( pFence->GetCompletedValue() < fence )
    {
        hr = pFence->SetEventOnCompletion( fence, handle );
        if ( FAILED(hr) )
        { return false; }

        WaitForSingleObject( handle, INFINITE );
    }

    return true;
}