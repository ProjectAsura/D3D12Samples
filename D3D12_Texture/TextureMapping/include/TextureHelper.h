//-------------------------------------------------------------------------------------------------
// File : TextureHelper.h
// Desc : Texture Helper Functions.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <d3d12.h>


//-------------------------------------------------------------------------------------------------
//      サブリソースをコピーします.
//-------------------------------------------------------------------------------------------------
void CopySubresource(
    const D3D12_MEMCPY_DEST* pDst,
    const D3D12_SUBRESOURCE_DATA* pSrc,
    SIZE_T rowSizeInBytes,
    UINT rowCounts,
    UINT sliceCount);

//-------------------------------------------------------------------------------------------------
//      データのアップロードに必要なサイズを取得します.
//-------------------------------------------------------------------------------------------------
UINT64 GetRequiredIntermediateSize(
    ID3D12Resource* pDstResource,
    UINT firstSubResource,
    UINT subResourceCount);

//-------------------------------------------------------------------------------------------------
//      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
UINT64 UpdateSubresources(
    ID3D12GraphicsCommandList* pList,
    ID3D12Resource* pDstResource,
    ID3D12Resource* pIntermediateResource,
    UINT firstSubResource,
    UINT64 requiredSize,
    const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
    const UINT* pRowCounts,
    const UINT64* pRowSizeInBytes,
    const D3D12_SUBRESOURCE_DATA* pSrcData);

//-------------------------------------------------------------------------------------------------
//      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
UINT64 UpdateSubresources(
    ID3D12GraphicsCommandList* pList,
    ID3D12Resource* pDstResource,
    ID3D12Resource* pIntermediate,
    UINT64 intermediateOffset,
    UINT firstSubresource,
    UINT subResourceCOunt,
    D3D12_SUBRESOURCE_DATA* pSrcData);

//-------------------------------------------------------------------------------------------------
//      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
bool UpdateSubresources(
    ID3D12GraphicsCommandList* pList,
    ID3D12CommandQueue* pQueue,
    ID3D12Fence* pFence,
    HANDLE handle,
    ID3D12Resource* pResource,
    UINT firstSubResource,
    UINT subResourceCount,
    D3D12_SUBRESOURCE_DATA* pSrcData);
