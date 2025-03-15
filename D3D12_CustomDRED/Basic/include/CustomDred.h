//-----------------------------------------------------------------------------
// File : CustomDred.h
// Desc : Custom Device Removed Extended Data.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d12.h>

// ラッパーインターフェース.
struct IGraphicsCommandList
{
    virtual ~IGraphicsCommandList() = default;

    virtual void AddRef() = 0;

    virtual void Release() = 0;

    virtual D3D12_COMMAND_LIST_TYPE GetType() = 0;

    virtual HRESULT Close() = 0;
        
    virtual HRESULT Reset( 
        _In_  ID3D12CommandAllocator *pAllocator,
        _In_opt_  ID3D12PipelineState *pInitialState) = 0;
        
    virtual void ClearState( 
        _In_opt_  ID3D12PipelineState *pPipelineState) = 0;
        
    virtual void DrawInstanced( 
        _In_  UINT VertexCountPerInstance,
        _In_  UINT InstanceCount,
        _In_  UINT StartVertexLocation,
        _In_  UINT StartInstanceLocation) = 0;
        
    virtual void DrawIndexedInstanced( 
        _In_  UINT IndexCountPerInstance,
        _In_  UINT InstanceCount,
        _In_  UINT StartIndexLocation,
        _In_  INT BaseVertexLocation,
        _In_  UINT StartInstanceLocation) = 0;
        
    virtual void Dispatch( 
        _In_  UINT ThreadGroupCountX,
        _In_  UINT ThreadGroupCountY,
        _In_  UINT ThreadGroupCountZ) = 0;
        
    virtual void CopyBufferRegion( 
        _In_  ID3D12Resource *pDstBuffer,
        UINT64 DstOffset,
        _In_  ID3D12Resource *pSrcBuffer,
        UINT64 SrcOffset,
        UINT64 NumBytes) = 0;
        
    virtual void CopyTextureRegion( 
        _In_  const D3D12_TEXTURE_COPY_LOCATION *pDst,
        UINT DstX,
        UINT DstY,
        UINT DstZ,
        _In_  const D3D12_TEXTURE_COPY_LOCATION *pSrc,
        _In_opt_  const D3D12_BOX *pSrcBox) = 0;
        
    virtual void CopyResource( 
        _In_  ID3D12Resource *pDstResource,
        _In_  ID3D12Resource *pSrcResource) = 0;
        
    virtual void CopyTiles( 
        _In_  ID3D12Resource *pTiledResource,
        _In_  const D3D12_TILED_RESOURCE_COORDINATE *pTileRegionStartCoordinate,
        _In_  const D3D12_TILE_REGION_SIZE *pTileRegionSize,
        _In_  ID3D12Resource *pBuffer,
        UINT64 BufferStartOffsetInBytes,
        D3D12_TILE_COPY_FLAGS Flags) = 0;
        
    virtual void ResolveSubresource( 
        _In_  ID3D12Resource *pDstResource,
        _In_  UINT DstSubresource,
        _In_  ID3D12Resource *pSrcResource,
        _In_  UINT SrcSubresource,
        _In_  DXGI_FORMAT Format) = 0;
        
    virtual void IASetPrimitiveTopology( 
        _In_  D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology) = 0;
        
    virtual void RSSetViewports( 
        _In_range_(0, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumViewports,
        _In_reads_( NumViewports)  const D3D12_VIEWPORT *pViewports) = 0;
        
    virtual void RSSetScissorRects( 
        _In_range_(0, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumRects,
        _In_reads_( NumRects)  const D3D12_RECT *pRects) = 0;
        
    virtual void OMSetBlendFactor( 
        _In_reads_opt_(4)  const FLOAT BlendFactor[ 4 ]) = 0;
        
    virtual void OMSetStencilRef( 
        _In_  UINT StencilRef) = 0;
        
    virtual void SetPipelineState( 
        _In_  ID3D12PipelineState *pPipelineState) = 0;
        
    virtual void ResourceBarrier( 
        _In_  UINT NumBarriers,
        _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers) = 0;
        
    virtual void ExecuteBundle( 
        _In_  ID3D12GraphicsCommandList *pCommandList) = 0;
        
    virtual void SetDescriptorHeaps( 
        _In_  UINT NumDescriptorHeaps,
        _In_reads_(NumDescriptorHeaps)  ID3D12DescriptorHeap *const *ppDescriptorHeaps) = 0;
        
    virtual void SetComputeRootSignature( 
        _In_opt_  ID3D12RootSignature *pRootSignature) = 0;
        
    virtual void SetGraphicsRootSignature( 
        _In_opt_  ID3D12RootSignature *pRootSignature) = 0;
        
    virtual void SetComputeRootDescriptorTable( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) = 0;
        
    virtual void SetGraphicsRootDescriptorTable( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) = 0;
        
    virtual void SetComputeRoot32BitConstant( 
        _In_  UINT RootParameterIndex,
        _In_  UINT SrcData,
        _In_  UINT DestOffsetIn32BitValues) = 0;
        
    virtual void SetGraphicsRoot32BitConstant( 
        _In_  UINT RootParameterIndex,
        _In_  UINT SrcData,
        _In_  UINT DestOffsetIn32BitValues) = 0;
        
    virtual void SetComputeRoot32BitConstants( 
        _In_  UINT RootParameterIndex,
        _In_  UINT Num32BitValuesToSet,
        _In_reads_(Num32BitValuesToSet*sizeof(UINT))  const void *pSrcData,
        _In_  UINT DestOffsetIn32BitValues) = 0;
        
    virtual void SetGraphicsRoot32BitConstants( 
        _In_  UINT RootParameterIndex,
        _In_  UINT Num32BitValuesToSet,
        _In_reads_(Num32BitValuesToSet*sizeof(UINT))  const void *pSrcData,
        _In_  UINT DestOffsetIn32BitValues) = 0;
        
    virtual void SetComputeRootConstantBufferView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;
        
    virtual void SetGraphicsRootConstantBufferView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;
        
    virtual void SetComputeRootShaderResourceView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;
        
    virtual void SetGraphicsRootShaderResourceView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;
        
    virtual void SetComputeRootUnorderedAccessView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;
        
    virtual void SetGraphicsRootUnorderedAccessView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;
        
    virtual void IASetIndexBuffer( 
        _In_opt_  const D3D12_INDEX_BUFFER_VIEW *pView) = 0;
        
    virtual void IASetVertexBuffers( 
        _In_  UINT StartSlot,
        _In_  UINT NumViews,
        _In_reads_opt_(NumViews)  const D3D12_VERTEX_BUFFER_VIEW *pViews) = 0;
        
    virtual void SOSetTargets( 
        _In_  UINT StartSlot,
        _In_  UINT NumViews,
        _In_reads_opt_(NumViews)  const D3D12_STREAM_OUTPUT_BUFFER_VIEW *pViews) = 0;
        
    virtual void OMSetRenderTargets( 
        _In_  UINT NumRenderTargetDescriptors,
        _In_opt_  const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors,
        _In_  BOOL RTsSingleHandleToDescriptorRange,
        _In_opt_  const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor) = 0;
        
    virtual void ClearDepthStencilView( 
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView,
        _In_  D3D12_CLEAR_FLAGS ClearFlags,
        _In_  FLOAT Depth,
        _In_  UINT8 Stencil,
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT *pRects) = 0;
        
    virtual void ClearRenderTargetView( 
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView,
        _In_  const FLOAT ColorRGBA[ 4 ],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT *pRects) = 0;
        
    virtual void ClearUnorderedAccessViewUint( 
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
        _In_  ID3D12Resource *pResource,
        _In_  const UINT Values[ 4 ],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT *pRects) = 0;
        
    virtual void ClearUnorderedAccessViewFloat( 
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
        _In_  ID3D12Resource *pResource,
        _In_  const FLOAT Values[ 4 ],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT *pRects) = 0;
        
    virtual void DiscardResource( 
        _In_  ID3D12Resource *pResource,
        _In_opt_  const D3D12_DISCARD_REGION *pRegion) = 0;
        
    virtual void BeginQuery( 
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index) = 0;
        
    virtual void EndQuery( 
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index) = 0;
        
    virtual void ResolveQueryData( 
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT StartIndex,
        _In_  UINT NumQueries,
        _In_  ID3D12Resource *pDestinationBuffer,
        _In_  UINT64 AlignedDestinationBufferOffset) = 0;
        
    virtual void SetPredication( 
        _In_opt_  ID3D12Resource *pBuffer,
        _In_  UINT64 AlignedBufferOffset,
        _In_  D3D12_PREDICATION_OP Operation) = 0;
        
    virtual void SetMarker( 
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void *pData,
        UINT Size) = 0;
        
    virtual void BeginEvent( 
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void *pData,
        UINT Size) = 0;
        
    virtual void EndEvent( void) = 0;
        
    virtual void ExecuteIndirect( 
        _In_  ID3D12CommandSignature *pCommandSignature,
        _In_  UINT MaxCommandCount,
        _In_  ID3D12Resource *pArgumentBuffer,
        _In_  UINT64 ArgumentBufferOffset,
        _In_opt_  ID3D12Resource *pCountBuffer,
        _In_  UINT64 CountBufferOffset) = 0;

    virtual ID3D12CommandList* GetCommandList() const = 0;
};

bool CreateGraphicsCommandList(
    ID3D12Device3* pDevice,
    D3D12_COMMAND_LIST_TYPE type,
    ID3D12CommandAllocator* pAllocator,
    IGraphicsCommandList** ppCmdList);

void ReportCustomDRED(HRESULT hr);
