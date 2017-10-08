//-------------------------------------------------------------------------------------------------
// File : asdxCommandQueue.cpp
// Desc : Command Queue.
// Copyright(c) Project Asura. All right reserved
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxCommandQueue.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// CommandQueue class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
CommandQueue::CommandQueue()
: m_SubmitIndex     (0)
, m_MaxSubmitCount  (0)
, m_pSubmitList     (nullptr)
, m_Event           (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
CommandQueue::~CommandQueue()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool CommandQueue::Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t maxSubmitCount)
{
    if (pDevice == nullptr)
    { return false; }

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type       = type;
    desc.Priority   = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.NodeMask   = 0;
    desc.Flags      = D3D12_COMMAND_QUEUE_FLAG_NONE;

    auto hr = pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pQueue.GetAddress()));
    if (FAILED(hr))
    { return false; }

    m_MaxSubmitCount = maxSubmitCount;
    m_pSubmitList = new(std::nothrow) ID3D12CommandList* [m_MaxSubmitCount];
    if (m_pSubmitList == nullptr)
    { return false; }

    m_SubmitIndex = 0;

    hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pFence.GetAddress()));
    if (FAILED(hr))
    { return false; }

    m_Event = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
    if (m_Event == nullptr)
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void CommandQueue::Term()
{
    if (m_pQueue != nullptr && m_pFence != nullptr && m_Event != nullptr)
    { WaitIdle(); }

    m_pQueue.Reset();

    if (m_pSubmitList != nullptr)
    {
        delete[] m_pSubmitList;
        m_pSubmitList = nullptr;
    }

    if (m_Event != nullptr)
    {
        CloseHandle(m_Event);
        m_Event = nullptr;
    }

    m_pFence.Reset();
    m_SubmitIndex    = 0;
    m_MaxSubmitCount = 0;
}

//-------------------------------------------------------------------------------------------------
//      コマンドリストを登録します.
//-------------------------------------------------------------------------------------------------
bool CommandQueue::Submit(ID3D12GraphicsCommandList* pCmdList)
{
    // TODO : 後で Lockfree Queue を使った実装に変更.
    std::lock_guard<std::mutex> locker(m_Mutex);
    if (m_SubmitIndex + 1 >= m_MaxSubmitCount)
    { return false; }

    m_pSubmitList[m_SubmitIndex] = reinterpret_cast<ID3D12CommandList*>(pCmdList);
    m_SubmitIndex++;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      コマンドを実行します.
//-------------------------------------------------------------------------------------------------
void CommandQueue::Execute(ID3D12Fence* pFence, uint64_t value)
{
    m_pQueue->ExecuteCommandLists( m_SubmitIndex, m_pSubmitList );

    if (pFence != nullptr)
    { m_pQueue->Signal( pFence, value ); }

    m_SubmitIndex = 0;
}

//-------------------------------------------------------------------------------------------------
//      コマンドの実行が完了するまで待機します.
//-------------------------------------------------------------------------------------------------
void CommandQueue::WaitIdle()
{
    m_pFence->Signal(0);
    m_pFence->SetEventOnCompletion(1, m_Event);
    m_pQueue->Signal( m_pFence.GetPtr(), 1 );
    WaitForSingleObject( m_Event, INFINITE );
}

//-------------------------------------------------------------------------------------------------
//      コマンドキューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12CommandQueue* CommandQueue::GetQueue() const
{ return m_pQueue.GetPtr(); }

} // namespace asdx
