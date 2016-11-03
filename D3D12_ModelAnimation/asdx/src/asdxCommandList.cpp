//-------------------------------------------------------------------------------------------------
// File : asdxCommandList.cpp
// Desc : Command List Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxCommandList.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// GraphicsCommandList class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
GraphicsCommandList::GraphicsCommandList()
: m_Allocator()
, m_CmdList  ()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
GraphicsCommandList::~GraphicsCommandList()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool GraphicsCommandList::Init
(
    ID3D12Device*           pDevice,
    D3D12_COMMAND_LIST_TYPE type,
    ID3D12PipelineState*    pipelineState
)
{
    // 引数チェック.
    if ( pDevice == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    // コマンドアロケータを生成.
    auto hr = pDevice->CreateCommandAllocator( type, IID_PPV_ARGS( m_Allocator.GetAddress() ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommandAllocator() Failed." );
        return false;
    }

    // コマンドリストを生成.
    hr = pDevice->CreateCommandList(
        0,
        type,
        m_Allocator.GetPtr(),
        pipelineState,
        IID_PPV_ARGS( m_CmdList.GetAddress() ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommandList() Failed." );
        return false;
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void GraphicsCommandList::Term()
{
    m_CmdList  .Reset();
    m_Allocator.Reset();
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストをクリアします.
//-------------------------------------------------------------------------------------------------
void GraphicsCommandList::Clear( ID3D12PipelineState* pipelineState )
{
    m_Allocator->Reset();
    m_CmdList->Reset( m_Allocator.GetPtr(), pipelineState );
}

//-------------------------------------------------------------------------------------------------
//      遷移によるリソースバリアを設定します.
//-------------------------------------------------------------------------------------------------
void GraphicsCommandList::Transition
(
    ID3D12Resource*       pResource,
    D3D12_RESOURCE_STATES before,
    D3D12_RESOURCE_STATES after
)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = pResource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter  = after;
    m_CmdList->ResourceBarrier( 1, &barrier );
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストを実行します.
//-------------------------------------------------------------------------------------------------
void GraphicsCommandList::Execute( ID3D12CommandQueue* pQueue )
{ pQueue->ExecuteCommandLists( 1, reinterpret_cast<ID3D12CommandList**>(m_CmdList.GetAddress()) ); }

//-------------------------------------------------------------------------------------------------
//      コマンドリストアロケータを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12CommandAllocator* GraphicsCommandList::GetAllocator() const
{ return m_Allocator.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      グラフィックスコマンドリストを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12GraphicsCommandList* GraphicsCommandList::GetList() const
{ return m_CmdList.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      アロー演算子です
//-------------------------------------------------------------------------------------------------
ID3D12GraphicsCommandList* GraphicsCommandList::operator -> () const
{ return m_CmdList.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      コマンドリストにキャストします.
//-------------------------------------------------------------------------------------------------
ID3D12CommandList* GraphicsCommandList::Cast() const
{ return static_cast<ID3D12CommandList*>(m_CmdList.GetPtr()); }


} // namespace asdx
