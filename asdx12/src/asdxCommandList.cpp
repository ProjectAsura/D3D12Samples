//-------------------------------------------------------------------------------------------------
// File : asdxCommandList.cpp
// Desc : Command List Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxCommandList.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// CommandList class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
CommandList::CommandList()
: m_pCmdList    (nullptr)
, m_pAllocators ()
, m_Index       (0)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
CommandList::~CommandList()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool CommandList::Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t count)
{
    if (pDevice == nullptr || count == 0)
    { return false; }

    m_pAllocators.resize(count);

    for(auto i=0u; i<count; ++i)
    {
        auto hr = pDevice->CreateCommandAllocator(
            type, IID_PPV_ARGS(&m_pAllocators[i]));
        if (FAILED(hr))
        { return false; }
    }

    {
        auto hr = pDevice->CreateCommandList(
            1,
            type,
            m_pAllocators[0].Get(),
            nullptr,
            IID_PPV_ARGS(&m_pCmdList));
        if (FAILED(hr))
        { return false; }

        m_pCmdList->Close();
    }

    m_Index = 0;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void CommandList::Term()
{
    if (m_pCmdList != nullptr)
    {
        m_pCmdList->Release();
        m_pCmdList = nullptr;
    }

    for(size_t i=0; i<m_pAllocators.size(); ++i)
    {
        if (m_pAllocators[i] != nullptr)
        {
            m_pAllocators[i]->Release();
            m_pAllocators[i] = nullptr;
        }
    }

    m_pAllocators.clear();
    m_pAllocators.shrink_to_fit();
}

//-------------------------------------------------------------------------------------------------
//      リセット処理を行います.
//-------------------------------------------------------------------------------------------------
ID3D12GraphicsCommandList* CommandList::Reset()
{
    auto hr = m_pCmdList->Reset(m_pAllocators[m_Index].Get(), nullptr);
    if (FAILED(hr))
    { return nullptr; }

    m_Index = (m_Index + 1) % uint32_t(m_pAllocators.size());
    return m_pCmdList;
}

} // namespace asdx