//---------------------------------------------------------------------------
// File : CustomDred.cpp
// Desc : Custom Device Removed Extended Data.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <cstdarg>
#include <cassert>
#include <atomic>
#include <vector>
#include <list>
#include <string>
#include <mutex>
#include <CustomDred.h>


class GraphicsCommandList;

namespace {

//! パンくずタグ.
static const char* g_BreadcrumbTags[] = {
    "SETMARKER",
    "BEGINEVENT",
    "ENDEVENT",
    "DRAWINSTANCED",
    "DRAWINDEXEDINSTANCED",
    "EXECUTEINDIRECT",
    "DISPATCH",
    "COPYBUFFERREGION",
    "COPYTEXTUREREGION",
    "COPYRESOURCE",
    "COPYTILES",
    "RESOLVESUBRESOURCE",
    "CLEARRENDERTARGETVIEW",
    "CLEARUNORDEREDACCESSVIEW",
    "CLEARDEPTHSTENCILVIEW",
    "RESOURCEBARRIER",
    "EXECUTEBUNDLE",
    "PRESENT",
    "RESOLVEQUERYDATA",
    "BEGINSUBMISSION",
    "ENDSUBMISSION",
    "DECODEFRAME",
    "PROCESSFRAMES",
    "ATOMICCOPYBUFFERUINT",
    "ATOMICCOPYBUFFERUINT64",
    "RESOLVESUBRESOURCEREGION",
    "WRITEBUFFERIMMEDIATE",
    "DECODEFRAME1",
    "SETPROTECTEDRESOURCESESSION",
    "DECODEFRAME2",
    "PROCESSFRAMES1",
    "BUILDRAYTRACINGACCELERATIONSTRUCTURE",
    "EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO",
    "COPYRAYTRACINGACCELERATIONSTRUCTURE",
    "DISPATCHRAYS",
    "INITIALIZEMETACOMMAND",
    "EXECUTEMETACOMMAND",
    "ESTIMATEMOTION",
    "RESOLVEMOTIONVECTORHEAP",
    "SETPIPELINESTATE1",
    "INITIALIZEEXTENSIONCOMMAND",
    "EXECUTEEXTENSIONCOMMAND",
    "DISPATCHMESH",
    "ENCODEFRAME2",
    "RESOLVEENCODEROUTPUTMETADATA",
    "BARRIER",
    "BEGIN_COMMAND_LIST",
    "DISPATCHGRAPH",
    "SETPROGRAM",
};


//-----------------------------------------------------------------------------
//! @brief      ログを出力します.
//-----------------------------------------------------------------------------
void ErrorLog(const char* format, ...)
{
    char msg[2048]= {};
    va_list arg = {};

    va_start( arg, format );
    vsprintf_s( msg, format, arg );
    va_end( arg );

    fprintf(stderr, "%s", msg );

    OutputDebugStringA( msg );
}

//-----------------------------------------------------------------------------
//! @brief      ログを出力します.
//-----------------------------------------------------------------------------
void InfoLog(const char* format, ...)
{
    char msg[2048]= {};
    va_list arg = {};

    va_start( arg, format );
    vsprintf_s( msg, format, arg );
    va_end( arg );

    fprintf(stdout, "%s", msg );

    OutputDebugStringA( msg );
}

//-----------------------------------------------------------------------------
//! @brief      コマンドリストタイプを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_COMMAND_LIST_TYPE type)
{
    switch(type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        return "DIRECT";

    case D3D12_COMMAND_LIST_TYPE_BUNDLE:
        return "BUNDLE";

    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return "COMPUTE";

    case D3D12_COMMAND_LIST_TYPE_COPY:
        return "COPY";

    case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
        return "VIDEO_DECODE";

    case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
        return "VIDEO_PROCESS";

    case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
        return "VIDEO_ENCODE";

    case D3D12_COMMAND_LIST_TYPE_NONE:
        return "NONE";

    default:
        return "UNKNOWN";
    }
}

template<typename To, typename From>
To bit_cast(const From& from) 
{ 
    assert(sizeof(To) == sizeof(From));
    To result = {};
    std::memcpy(&result, &from, sizeof(result));
    return result;
}

struct Marker
{
    D3D12_AUTO_BREADCRUMB_OP  Op  : 30;
    uint32_t                  In  : 1;
    uint32_t                  Out : 1;
};
static_assert(sizeof(Marker) == sizeof(uint32_t), "Size not matched.");

} // namespace

#define ELOG(x, ...) ErrorLog("[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__); 
#define ILOG(x, ...) InfoLog( x "\n", ##__VA_ARGS__ );

class GraphicsCommandListHolder
{
public:
    GraphicsCommandListHolder() = default;

    ~GraphicsCommandListHolder()
    { Clear(); }

    void Add(IGraphicsCommandList* pList)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        m_List.push_back(pList);
    
    }

    void Remove(IGraphicsCommandList* pList)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        m_List.remove(pList);
    }

    void Clear()
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        m_List.clear();
    }

    void Print();

private:
    std::mutex                       m_Mutex;
    std::list<IGraphicsCommandList*> m_List;
};

GraphicsCommandListHolder g_Holder;

class CommandBuffer
{
public:
    bool Init(ID3D12Device3* pDevice)
    {
        if (pDevice == nullptr)
        {
            ELOG("Error : Invalid Argument");
            return false;
        }

        m_BufferSize = sizeof(D3D12_AUTO_BREADCRUMB_OP) * UINT16_MAX;

        m_pAddress = VirtualAlloc(nullptr, m_BufferSize, MEM_COMMIT, PAGE_READWRITE);
        if (m_pAddress == nullptr)
        {
            ELOG("Error : Out of Memory");
            return false;
        }

        auto hr = pDevice->OpenExistingHeapFromAddress(m_pAddress, IID_PPV_ARGS(&m_pHeap));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device3::OpenExistingHeapFromAddress() Failed. errcode = 0x%x", hr);
            return false;
        }

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment          = 0;
        desc.Width              = m_BufferSize;
        desc.Height             = 1;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;

        D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST;

        hr = pDevice->CreatePlacedResource(m_pHeap, 0, &desc, state, nullptr, IID_PPV_ARGS(&m_pBuffer));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreatePlacedResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_GpuAddress = m_pBuffer->GetGPUVirtualAddress();

        m_CurrIndex = 0;
        return true;
    }

    void Term()
    {
        if (m_pBuffer != nullptr)
        {
            m_pBuffer->Release();
            m_pBuffer = nullptr;
        }

        if (m_pHeap != nullptr)
        {
            m_pHeap->Release();
            m_pHeap = nullptr;
        }

        if (m_pAddress != nullptr)
        {
            VirtualFree(m_pAddress, 0, MEM_RELEASE);
            m_pAddress = nullptr;
        }
    }

    void Reset()
    {
        m_CurrIndex = 0;
        m_LastIndex = 0;
        memset(m_pAddress, 0, m_BufferSize);
    }

    void Push(ID3D12GraphicsCommandList2* pCmdList, D3D12_AUTO_BREADCRUMB_OP op)
    {
        Marker marker = {};
        marker.Op  = op;
        marker.In  = 1;
        marker.Out = 0;

        D3D12_WRITEBUFFERIMMEDIATE_PARAMETER param = {};
        param.Dest  = m_GpuAddress + sizeof(D3D12_AUTO_BREADCRUMB_OP) * m_CurrIndex;
        param.Value = bit_cast<UINT>(marker);

        auto mode = D3D12_WRITEBUFFERIMMEDIATE_MODE_MARKER_IN;
        pCmdList->WriteBufferImmediate(1, &param, &mode);

        m_CurrentOp = op;
    }

    void Pop(ID3D12GraphicsCommandList2* pCmdList)
    {
        Marker marker = {};
        marker.Op  = m_CurrentOp;
        marker.In  = 0;
        marker.Out = 1;

        D3D12_WRITEBUFFERIMMEDIATE_PARAMETER param = {};
        param.Dest  = m_GpuAddress + sizeof(D3D12_AUTO_BREADCRUMB_OP) * m_CurrIndex;
        param.Value = bit_cast<UINT>(marker);

        auto mode = D3D12_WRITEBUFFERIMMEDIATE_MODE_MARKER_OUT;
        pCmdList->WriteBufferImmediate(1, &param, &mode);

        m_CurrIndex++;
    }

    void Finish()
    {
        m_LastIndex = m_CurrIndex;
    }

    void Print()
    {
        auto marker = reinterpret_cast<Marker*>(m_pAddress);
        if (marker == nullptr)
            return;

        ILOG("BreadcrumbCount : %u", m_LastIndex);

        for(auto i=0; i<m_LastIndex; ++i)
        {
            char state[3] = "  ";
            if (marker[i].Out)
            {
                state[0] = 'O';
                state[1] = 'K';
            }
            else if (marker[i].In)
            {
                state[0] = 'N';
                state[1] = 'G';
            }

            uint32_t opIndex = marker[i].Op;
            ILOG("%04u : [%s] Op = %s", i, state, g_BreadcrumbTags[opIndex]);
        }
    }

private:
    LPVOID                      m_pAddress   = nullptr;
    ID3D12Resource*             m_pBuffer    = nullptr;
    ID3D12Heap*                 m_pHeap      = nullptr;
    uint16_t                    m_CurrIndex  = 0;
    uint16_t                    m_LastIndex  = 0;
    uint16_t                    m_OpIndex    = 0;
    D3D12_GPU_VIRTUAL_ADDRESS   m_GpuAddress = {};
    D3D12_AUTO_BREADCRUMB_OP    m_CurrentOp;
    size_t                      m_BufferSize = 0;
};

class GraphicsCommandList : public IGraphicsCommandList
{
public:
    static bool Create
    (
        ID3D12Device3*              pDevice,
        D3D12_COMMAND_LIST_TYPE     type,
        ID3D12CommandAllocator*     pAllocator,
        IGraphicsCommandList**      ppCommandList
    )
    {
        auto instance = new GraphicsCommandList();
        if (!instance->Init(pDevice, pAllocator, type))
        {
            delete instance;
            return false;
        }

        (*ppCommandList) = instance;
        return true;
    }

    void AddRef() override
    { m_RefCount++; }

    void Release() override
    {
        m_RefCount--;
        if (m_RefCount == 0)
        { delete this; }
    }

    D3D12_COMMAND_LIST_TYPE GetType() override
    { return m_pCmdList->GetType(); }

    HRESULT Close() override
    {
        auto ret = m_pCmdList->Close();
        m_Cmds[m_CurrentIndex].Finish();
        m_CurrentIndex = (m_CurrentIndex + 1) % uint32_t(m_Cmds.size());
        m_FrameIndex++;
        return ret;
    }
        
    HRESULT Reset( 
        _In_  ID3D12CommandAllocator *pAllocator,
        _In_opt_  ID3D12PipelineState *pInitialState) override
    {
        m_Cmds[m_CurrentIndex].Reset();
        return m_pCmdList->Reset(pAllocator, pInitialState);
    }
        
    void ClearState( 
        _In_opt_  ID3D12PipelineState *pPipelineState) override
    {
        m_pCmdList->ClearState(pPipelineState);
    }
        
    void DrawInstanced( 
        _In_  UINT VertexCountPerInstance,
        _In_  UINT InstanceCount,
        _In_  UINT StartVertexLocation,
        _In_  UINT StartInstanceLocation) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_DRAWINSTANCED);
        m_pCmdList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
        Pop();
    }
        
    void DrawIndexedInstanced( 
        _In_  UINT IndexCountPerInstance,
        _In_  UINT InstanceCount,
        _In_  UINT StartIndexLocation,
        _In_  INT BaseVertexLocation,
        _In_  UINT StartInstanceLocation) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_DRAWINDEXEDINSTANCED);
        m_pCmdList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        Pop();
    }
        
    void Dispatch( 
        _In_  UINT ThreadGroupCountX,
        _In_  UINT ThreadGroupCountY,
        _In_  UINT ThreadGroupCountZ) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_DISPATCH);
        m_pCmdList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
        Pop();
    }

    void CopyBufferRegion( 
        _In_  ID3D12Resource *pDstBuffer,
        UINT64 DstOffset,
        _In_  ID3D12Resource *pSrcBuffer,
        UINT64 SrcOffset,
        UINT64 NumBytes) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_COPYBUFFERREGION);
        m_pCmdList->CopyBufferRegion(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);
        Pop();
    }

    void CopyTextureRegion( 
        _In_  const D3D12_TEXTURE_COPY_LOCATION *pDst,
        UINT DstX,
        UINT DstY,
        UINT DstZ,
        _In_  const D3D12_TEXTURE_COPY_LOCATION *pSrc,
        _In_opt_  const D3D12_BOX *pSrcBox) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_COPYTEXTUREREGION);
        m_pCmdList->CopyTextureRegion(pDst, DstX, DstY, DstZ, pSrc, pSrcBox);
        Pop();
    }
        
    void CopyResource( 
        _In_  ID3D12Resource *pDstResource,
        _In_  ID3D12Resource *pSrcResource) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_COPYRESOURCE);
        m_pCmdList->CopyResource(pDstResource, pSrcResource);
        Pop();
    }
        
    void CopyTiles( 
        _In_  ID3D12Resource *pTiledResource,
        _In_  const D3D12_TILED_RESOURCE_COORDINATE *pTileRegionStartCoordinate,
        _In_  const D3D12_TILE_REGION_SIZE *pTileRegionSize,
        _In_  ID3D12Resource *pBuffer,
        UINT64 BufferStartOffsetInBytes,
        D3D12_TILE_COPY_FLAGS Flags) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_COPYTILES);
        m_pCmdList->CopyTiles(pTiledResource, pTileRegionStartCoordinate, pTileRegionSize, pBuffer, BufferStartOffsetInBytes, Flags);
        Pop();
    }
        
    void ResolveSubresource( 
        _In_  ID3D12Resource *pDstResource,
        _In_  UINT DstSubresource,
        _In_  ID3D12Resource *pSrcResource,
        _In_  UINT SrcSubresource,
        _In_  DXGI_FORMAT Format) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCE);
        m_pCmdList->ResolveSubresource(pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
        Pop();
    }
        
    void IASetPrimitiveTopology( 
        _In_  D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology) override
    {
        m_pCmdList->IASetPrimitiveTopology(PrimitiveTopology);
    }

    void RSSetViewports( 
        _In_range_(0, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumViewports,
        _In_reads_( NumViewports)  const D3D12_VIEWPORT *pViewports) override
    {
        m_pCmdList->RSSetViewports(NumViewports, pViewports);
    }
        
    void RSSetScissorRects( 
        _In_range_(0, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumRects,
        _In_reads_( NumRects)  const D3D12_RECT *pRects) override
    {
        m_pCmdList->RSSetScissorRects(NumRects, pRects);
    }
        
    void OMSetBlendFactor( 
        _In_reads_opt_(4)  const FLOAT BlendFactor[ 4 ]) override
    {
        m_pCmdList->OMSetBlendFactor(BlendFactor);
    }
        
    void OMSetStencilRef( 
        _In_  UINT StencilRef) override
    {
        m_pCmdList->OMSetStencilRef(StencilRef);
    }
        
    void SetPipelineState( 
        _In_  ID3D12PipelineState *pPipelineState) override
    {
        m_pCmdList->SetPipelineState(pPipelineState);
    }
        
    void ResourceBarrier( 
        _In_  UINT NumBarriers,
        _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_RESOURCEBARRIER);
        m_pCmdList->ResourceBarrier(NumBarriers, pBarriers);
        Pop();
    }

    void ExecuteBundle( 
        _In_  ID3D12GraphicsCommandList *pCommandList) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_EXECUTEBUNDLE);
        m_pCmdList->ExecuteBundle(pCommandList);
        Pop();
    }
        
    void SetDescriptorHeaps( 
        _In_  UINT NumDescriptorHeaps,
        _In_reads_(NumDescriptorHeaps)  ID3D12DescriptorHeap *const *ppDescriptorHeaps) override
    {
        m_pCmdList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
    }
        
    void SetComputeRootSignature( 
        _In_opt_  ID3D12RootSignature *pRootSignature) override
    {
        m_pCmdList->SetComputeRootSignature(pRootSignature);
    }
        
    void SetGraphicsRootSignature( 
        _In_opt_  ID3D12RootSignature *pRootSignature) override
    {
        m_pCmdList->SetGraphicsRootSignature(pRootSignature);
    }
        
    void SetComputeRootDescriptorTable( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) override
    {
        m_pCmdList->SetComputeRootDescriptorTable(RootParameterIndex, BaseDescriptor);
    }
        
    void SetGraphicsRootDescriptorTable( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) override
    {
        m_pCmdList->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor);
    }
        
    void SetComputeRoot32BitConstant( 
        _In_  UINT RootParameterIndex,
        _In_  UINT SrcData,
        _In_  UINT DestOffsetIn32BitValues) override
    {
        m_pCmdList->SetComputeRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
    }
        
    void SetGraphicsRoot32BitConstant( 
        _In_  UINT RootParameterIndex,
        _In_  UINT SrcData,
        _In_  UINT DestOffsetIn32BitValues) override
    {
        m_pCmdList->SetGraphicsRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
    }
        
    void SetComputeRoot32BitConstants( 
        _In_  UINT RootParameterIndex,
        _In_  UINT Num32BitValuesToSet,
        _In_reads_(Num32BitValuesToSet*sizeof(UINT))  const void *pSrcData,
        _In_  UINT DestOffsetIn32BitValues) override
    {
        m_pCmdList->SetComputeRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
    }
        
    void SetGraphicsRoot32BitConstants( 
        _In_  UINT RootParameterIndex,
        _In_  UINT Num32BitValuesToSet,
        _In_reads_(Num32BitValuesToSet*sizeof(UINT))  const void *pSrcData,
        _In_  UINT DestOffsetIn32BitValues) override
    {
        m_pCmdList->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
    }
        
    void SetComputeRootConstantBufferView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override
    {
        m_pCmdList->SetComputeRootConstantBufferView(RootParameterIndex, BufferLocation);
    }
        
    void SetGraphicsRootConstantBufferView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override
    {
        m_pCmdList->SetGraphicsRootConstantBufferView(RootParameterIndex, BufferLocation);
    }
        
    void SetComputeRootShaderResourceView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override
    {
        m_pCmdList->SetComputeRootShaderResourceView(RootParameterIndex, BufferLocation);
    }
        
    void SetGraphicsRootShaderResourceView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override
    {
        m_pCmdList->SetGraphicsRootShaderResourceView(RootParameterIndex, BufferLocation);
    }
        
    void SetComputeRootUnorderedAccessView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override
    {
        m_pCmdList->SetComputeRootUnorderedAccessView(RootParameterIndex, BufferLocation);
    }
        
    void SetGraphicsRootUnorderedAccessView( 
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override
    {
        m_pCmdList->SetGraphicsRootUnorderedAccessView(RootParameterIndex, BufferLocation);
    }
        
    void IASetIndexBuffer( 
        _In_opt_  const D3D12_INDEX_BUFFER_VIEW *pView) override
    {
        m_pCmdList->IASetIndexBuffer(pView);
    }
        
    void IASetVertexBuffers( 
        _In_  UINT StartSlot,
        _In_  UINT NumViews,
        _In_reads_opt_(NumViews)  const D3D12_VERTEX_BUFFER_VIEW *pViews) override
    {
        m_pCmdList->IASetVertexBuffers(StartSlot, NumViews, pViews);
    }
        
    void SOSetTargets( 
        _In_  UINT StartSlot,
        _In_  UINT NumViews,
        _In_reads_opt_(NumViews)  const D3D12_STREAM_OUTPUT_BUFFER_VIEW *pViews) override
    {
        m_pCmdList->SOSetTargets(StartSlot, NumViews, pViews);
    }
        
    void OMSetRenderTargets( 
        _In_  UINT NumRenderTargetDescriptors,
        _In_opt_  const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors,
        _In_  BOOL RTsSingleHandleToDescriptorRange,
        _In_opt_  const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor) override
    {
        m_pCmdList->OMSetRenderTargets(NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
    }
        
    void ClearDepthStencilView( 
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView,
        _In_  D3D12_CLEAR_FLAGS ClearFlags,
        _In_  FLOAT Depth,
        _In_  UINT8 Stencil,
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT *pRects) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_CLEARDEPTHSTENCILVIEW);
        m_pCmdList->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, pRects);
        Pop();
    }
        
    void ClearRenderTargetView( 
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView,
        _In_  const FLOAT ColorRGBA[ 4 ],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT *pRects) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_CLEARRENDERTARGETVIEW);
        m_pCmdList->ClearRenderTargetView(RenderTargetView, ColorRGBA, NumRects, pRects);
        Pop();
    }
        
    void ClearUnorderedAccessViewUint( 
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
        _In_  ID3D12Resource *pResource,
        _In_  const UINT Values[ 4 ],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT *pRects) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_CLEARUNORDEREDACCESSVIEW);
        m_pCmdList->ClearUnorderedAccessViewUint(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, Values, NumRects, pRects);
        Pop();
    }
        
    void ClearUnorderedAccessViewFloat( 
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
        _In_  ID3D12Resource *pResource,
        _In_  const FLOAT Values[ 4 ],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT *pRects) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_CLEARUNORDEREDACCESSVIEW);
        m_pCmdList->ClearUnorderedAccessViewFloat(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, Values, NumRects, pRects);
        Pop();
    }
        
    void DiscardResource( 
        _In_  ID3D12Resource *pResource,
        _In_opt_  const D3D12_DISCARD_REGION *pRegion) override
    {
        m_pCmdList->DiscardResource(pResource, pRegion);
    }
        
    void BeginQuery( 
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index) override
    {
        m_pCmdList->BeginQuery(pQueryHeap, Type, Index);
    }
        
    void EndQuery( 
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index) override
    {
        m_pCmdList->EndQuery(pQueryHeap, Type, Index);
    }
        
    void ResolveQueryData( 
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT StartIndex,
        _In_  UINT NumQueries,
        _In_  ID3D12Resource *pDestinationBuffer,
        _In_  UINT64 AlignedDestinationBufferOffset) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_RESOLVEQUERYDATA);
        m_pCmdList->ResolveQueryData(pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);
        Pop();
    }
        
    void SetPredication( 
        _In_opt_  ID3D12Resource *pBuffer,
        _In_  UINT64 AlignedBufferOffset,
        _In_  D3D12_PREDICATION_OP Operation) override
    {
        m_pCmdList->SetPredication(pBuffer, AlignedBufferOffset, Operation);
    }
        
    void SetMarker( 
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void *pData,
        UINT Size) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_SETMARKER);
        m_pCmdList->SetMarker(Metadata, pData, Size);
        Pop();
    }
        
    void BeginEvent( 
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void *pData,
        UINT Size) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT);
        m_pCmdList->BeginEvent(Metadata, pData, Size);
        Pop();
    }
        
    void EndEvent() override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_ENDEVENT);
        m_pCmdList->EndEvent();
        Pop();
    }
        
    void ExecuteIndirect( 
        _In_  ID3D12CommandSignature *pCommandSignature,
        _In_  UINT MaxCommandCount,
        _In_  ID3D12Resource *pArgumentBuffer,
        _In_  UINT64 ArgumentBufferOffset,
        _In_opt_  ID3D12Resource *pCountBuffer,
        _In_  UINT64 CountBufferOffset) override
    {
        Push(D3D12_AUTO_BREADCRUMB_OP_EXECUTEINDIRECT);
        m_pCmdList->ExecuteIndirect(pCommandSignature,
            MaxCommandCount,
            pArgumentBuffer,
            ArgumentBufferOffset,
            pCountBuffer,
            CountBufferOffset);
        Pop();
    }

    ID3D12CommandList* GetCommandList() const
    { return m_pCmdList; }

    void Print()
    {
        ILOG("CommandListType : %s", ToString(m_Type));

        for(auto i=0; i<m_Cmds.size(); ++i)
        {
            ILOG("FrameIndex : %lld", m_FrameIndex - 4 + i);
            auto idx = (m_CurrentIndex + 4 + i) % uint32_t(m_Cmds.size());
            m_Cmds[idx].Print();
        }
    }

private:
    std::atomic<uint32_t>       m_RefCount = {};
    ID3D12GraphicsCommandList2* m_pCmdList = nullptr;
    std::vector<CommandBuffer>  m_Cmds;
    uint32_t                    m_CurrentIndex = 0;
    int64_t                     m_FrameIndex  = 0;
    D3D12_COMMAND_LIST_TYPE     m_Type;

    GraphicsCommandList()
        : m_RefCount(1)
    {
    }

    ~GraphicsCommandList()
    {
        Term();
    }

    bool Init(ID3D12Device3* pDevice, ID3D12CommandAllocator* pAllocator, D3D12_COMMAND_LIST_TYPE type)
    {
        m_Cmds.resize(4);
        for(auto i=0; i<m_Cmds.size(); ++i)
        {
            if (!m_Cmds[i].Init(pDevice))
            { return false; }
        }

        auto hr = pDevice->CreateCommandList(0, type, pAllocator, nullptr, IID_PPV_ARGS(&m_pCmdList));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommandList() Failed. errcode = 0x%x", hr);
            return false;
        }

        g_Holder.Add(this);

        return true;
    }

    void Term()
    {
        g_Holder.Remove(this);

        if (m_pCmdList != nullptr)
        {
            m_pCmdList->Release();
            m_pCmdList = nullptr;
        }

        for(size_t i=0; i<m_Cmds.size(); ++i)
        {
            m_Cmds[i].Term();
        }
        m_Cmds.clear();
    }

    void Push(D3D12_AUTO_BREADCRUMB_OP op)
    {
        m_Cmds[m_CurrentIndex].Push(m_pCmdList, op);
    }

    void Pop()
    {
        m_Cmds[m_CurrentIndex].Pop(m_pCmdList);
    }
};

void GraphicsCommandListHolder::Print()
{
    std::lock_guard<std::mutex> locker(m_Mutex);
    for(auto& list : m_List)
    {
        ILOG("------");
        auto impl = reinterpret_cast<GraphicsCommandList*>(list);
        impl->Print();
    }
}

bool CreateGraphicsCommandList(ID3D12Device3* pDevice, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pAllocator, IGraphicsCommandList** ppCommandList)
{
    return GraphicsCommandList::Create(pDevice, type, pAllocator, ppCommandList);
}

void ReportCustomDRED(HRESULT hr)
{
    ILOG("ErrorCode : 0x%x - %s", hr, std::system_category().message(hr).c_str());
    g_Holder.Print();
}