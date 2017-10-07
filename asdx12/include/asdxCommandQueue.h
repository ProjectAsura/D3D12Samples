#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <d3d12.h>
#include <cstdint>
#include <mutex>
#include <asdxRefPtr.h>


namespace asdx {

class Fence;

///////////////////////////////////////////////////////////////////////////////////////////////////
// CommandQueue class
///////////////////////////////////////////////////////////////////////////////////////////////////
class CommandQueue
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    CommandQueue();
    ~CommandQueue();

    bool Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t maxSubmitCount);
    void Term();
    bool Submit(ID3D12GraphicsCommandList* pCmdList);
    void Execute(ID3D12Fence* pFence, uint64_t value);
    void WaitIdle();
    ID3D12CommandQueue* GetQueue() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::mutex                  m_Mutex;
    RefPtr<ID3D12CommandQueue>  m_pQueue;
    uint32_t                    m_SubmitIndex;
    uint32_t                    m_MaxSubmitCount;
    ID3D12CommandList**         m_pSubmitList;
    RefPtr<ID3D12Fence>         m_pFence;
    HANDLE                      m_Event;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    void operator = (const CommandQueue&) = delete;
    CommandQueue    (const CommandQueue&) = delete;
};

} // namespace asdx
