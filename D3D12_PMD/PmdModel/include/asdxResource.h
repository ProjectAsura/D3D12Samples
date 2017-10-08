//-------------------------------------------------------------------------------------------------
// File : asdxResource.h
// Desc : Resource Module.
// Copyright(c) Project Asura.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTypedef.h>
#include <d3d12.h>


namespace asdx {

//-------------------------------------------------------------------------------------------------
//! @brief      サブリソースをコピーします.
//-------------------------------------------------------------------------------------------------
void CopySubresource(
    const D3D12_MEMCPY_DEST*        pDst,
    const D3D12_SUBRESOURCE_DATA*   pSrc,
    size_t                          rowSize,
    u32                             rowCounts,
    u32                             sliceCount );

//-------------------------------------------------------------------------------------------------
//! @brief      データのアップロードに必要なサイズを取得します.
//-------------------------------------------------------------------------------------------------
u64 GetRequiredIntermediateSize(
    ID3D12Resource* pDstRes,
    u32             firstSubResource,
    u32             subResourceCount );

//-------------------------------------------------------------------------------------------------
//! @brief      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
u64 UpdateSubresources( 
    ID3D12GraphicsCommandList*  pCmdList,
    ID3D12Resource*             pDstResource,
    ID3D12Resource*             pIntermediate,
    u64                         intermediateOffset,
    u32                         firstSubresource,
    u32                         subResourceCount,
    D3D12_SUBRESOURCE_DATA*     pSrcData);

} // namespace asdx
