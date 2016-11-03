//-------------------------------------------------------------------------------------------------
// File : asdxDeviceContext.cpp
// Desc : Device Context Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxDeviceContext.h>
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
// DeviceContext class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DeviceContext::DeviceContext()
: m_Queue    ()
, m_Immediate()
, m_Fence    ()
, m_IsInit   ( false )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DeviceContext::~DeviceContext()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DeviceContext::Init( ID3D12Device* pDevice )
{
    HRESULT hr = S_OK;

    // コマンドキューの生成.
    {
        D3D12_COMMAND_QUEUE_DESC desc = {
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            0,
            D3D12_COMMAND_QUEUE_FLAG_NONE,
            0
        };

        hr = pDevice->CreateCommandQueue( &desc, IID_PPV_ARGS( m_Queue.GetAddress() ) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreatCommandQueue() Failed." );
            return false;
        }
    }

    // グラフィックスコマンドリストの初期化.
    if ( !m_Immediate.Init( pDevice, D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr ) )
    {
        ELOG( "Error : GraphicsCmdList::Init() Failed." );
        return false;
    }

    // フェンスの初期化
    if ( !m_Fence.Init( pDevice ) )
    {
        ELOG( "Error : Fence::Init() Failed." );
        return false;
    }

    // 初期化済みフラグを立てる.
    m_IsInit = true;

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DeviceContext::Term()
{
    // 初期化されていなかったら処理しない.
    if ( !m_IsInit )
    { return; }

    // コマンドの完了を待機.
    m_Immediate->Close();
    Execute();
    Wait( INFINITE );

    m_Fence.Term();
    m_Immediate.Term();
    m_Queue.Reset();

    // 初期化済みフラグを下す.
    m_IsInit = false;
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストを実行します.
//-------------------------------------------------------------------------------------------------
void DeviceContext::Execute()
{ m_Immediate.Execute( m_Queue.GetPtr() ); }

//-------------------------------------------------------------------------------------------------
//      コマンドリストの完了を待機します.
//-------------------------------------------------------------------------------------------------
void DeviceContext::Wait( u32 mesc )
{ m_Fence.Wait( m_Queue.GetPtr(), mesc ); }

//-------------------------------------------------------------------------------------------------
//      コマンドリストをクリアします.
//-------------------------------------------------------------------------------------------------
void DeviceContext::Clear( ID3D12PipelineState* pPSO )
{ m_Immediate.Clear( pPSO ); }

//-------------------------------------------------------------------------------------------------
//      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
bool DeviceContext::UpdateSubRes
(
    ID3D12Resource* pResource,
    const u32 firstSubResource,
    const u32 subResourceCount,
    D3D12_SUBRESOURCE_DATA* pSrcData
)
{
    if ( pResource == nullptr || pSrcData == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    RefPtr<ID3D12Device> device;
    pResource->GetDevice( IID_PPV_ARGS( device.GetAddress() )); 

    RefPtr<ID3D12Resource> intermediate;
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
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed." );
            return false;
        }
    }

    Transition(
        pResource,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COPY_DEST );

    UpdateSubresources( 
        m_Immediate.GetList(),
        pResource,
        intermediate.GetPtr(),
        0,
        firstSubResource,
        subResourceCount,
        pSrcData );

    Transition( 
        pResource,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_GENERIC_READ );

    m_Immediate->Close();

    Execute();
    Wait( INFINITE );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      遷移によるリソースバリアを設定します.
//-------------------------------------------------------------------------------------------------
void DeviceContext::Transition
(
    ID3D12Resource* pResource,
    D3D12_RESOURCE_STATES before,
    D3D12_RESOURCE_STATES after
)
{ m_Immediate.Transition(pResource, before, after); }

//-------------------------------------------------------------------------------------------------
//      コマンドキューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12CommandQueue* DeviceContext::GetQueue() const
{ return m_Queue.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      グラフィックスコマンドリストを生成します.
//-------------------------------------------------------------------------------------------------
ID3D12GraphicsCommandList* DeviceContext::GetGraphicsCommandList() const
{ return m_Immediate.GetList(); }

//-------------------------------------------------------------------------------------------------
//      コマンドアロケータを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12CommandAllocator* DeviceContext::GetCommandAllocator() const
{ return m_Immediate.GetAllocator(); }

//-------------------------------------------------------------------------------------------------
//      アロー演算子です.
//-------------------------------------------------------------------------------------------------
ID3D12GraphicsCommandList* DeviceContext::operator -> () const
{ return m_Immediate.GetList(); }

} // namespace asdx
