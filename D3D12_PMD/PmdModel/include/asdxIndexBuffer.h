﻿//-------------------------------------------------------------------------------------------------
// File : asdxIndexBuffer.h
// Desc : Index Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxRef.h>
#include <d3d12.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// IndexBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////
class IndexBuffer : private NonCopyable
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

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    IndexBuffer();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~IndexBuffer();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice         デバイスです.
    //! @param[in]      size            インデックスバッファサイズです.
    //! @param[in]      format          インデックスバッファフォーマットです.
    //! @param[in]      pIndices        インデックスデータです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool Init( ID3D12Device* pDevice, u64 size, DXGI_FORMAT format, const void* pIndices );

    //---------------------------------------------------------------------------------------------
    //! @brieff     終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void Term();

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースを取得します.
    //!
    //! @return     リソースを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D12Resource* GetResource() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      インデックスバッファビューを取得します.
    //!
    //! @return     インデックスバッファビューを返却します.
    //---------------------------------------------------------------------------------------------
    D3D12_INDEX_BUFFER_VIEW GetView() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    RefPtr<ID3D12Resource>      m_Resource;     //!< リソースです.
    D3D12_INDEX_BUFFER_VIEW     m_View;         //!< インデックスバッファビューです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace asdx
