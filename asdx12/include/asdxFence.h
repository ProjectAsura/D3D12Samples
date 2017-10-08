﻿//-------------------------------------------------------------------------------------------------
// File : Fence.h
// Desc : Fence Module.
// Copyright(c) Pocol. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <d3d12.h>
#include <asdxRefPtr.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Fence class
///////////////////////////////////////////////////////////////////////////////////////////////////
class Fence
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
    Fence();
    ~Fence();
    bool Init(ID3D12Device* pDevice);
    void Term();
    void Wait(ID3D12CommandQueue* pQueue, UINT timeout);
    void Sync(ID3D12CommandQueue* pQueue);

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    RefPtr<ID3D12Fence> m_pFence;           //!< フェンスです.
    HANDLE              m_Event;            //!< イベントです.
    UINT                m_Counter;          //!< 現在のカウンターです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    Fence           (const Fence&) = delete;    // アクセス禁止.
    void operator = (const Fence&) = delete;    // アクセス禁止.
};

} // namespace asdx