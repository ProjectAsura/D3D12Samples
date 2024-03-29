﻿//-------------------------------------------------------------------------------------------------
// File : asdxHash.h
// Desc : Hash Key Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

#ifndef __ASDX_HASH_H__
#define __ASDX_HASH_H__

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTypedef.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// CRC32 class
///////////////////////////////////////////////////////////////////////////////////////////////////
class CRC32
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
    CRC32();

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      size        バッファサイズです.
    //! @param[in]      pBuffer     バッファです.
    //---------------------------------------------------------------------------------------------
    CRC32( const u32 size, const u8* pBuffer );

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      pBuffer     文字列です.
    //---------------------------------------------------------------------------------------------
    CRC32( const char8*  pBuffer );

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      pBuffer     文字列です.
    //---------------------------------------------------------------------------------------------
    CRC32( const char16* pBuffer );

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      value       ハッシュキー.
    //---------------------------------------------------------------------------------------------
    CRC32( const u32 value );

    //---------------------------------------------------------------------------------------------
    //! @brief      コピーコンストラクタです.
    //!
    //! @param[in]      value       コピー元の値.
    //---------------------------------------------------------------------------------------------
    CRC32( const CRC32& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      ハッシュキーを取得します.
    //!
    //! @return     ハッシュキーを返却します.
    //---------------------------------------------------------------------------------------------
    u32 GetHash() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      u32型へのキャストです.
    //!
    //! @return     ハッシュキーを返却します.
    //---------------------------------------------------------------------------------------------
    operator u32();

    //---------------------------------------------------------------------------------------------
    //! @brief      const u32型へのキャストです.
    //!
    //! @return     ハッシュキーを返却します.
    //---------------------------------------------------------------------------------------------
    operator const u32 () const;

    //---------------------------------------------------------------------------------------------
    //! @brief      等価比較演算子です.
    //!
    //! @param[in]      value       比較する値.
    //! @retval true    等価です.
    //! @retval false   非等価です.
    //---------------------------------------------------------------------------------------------
    bool    operator == ( const CRC32& value ) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      非等価比較演算子です.
    //!
    //! @param[in]      value       比較する値.
    //! @retval true    非等価です.
    //! @retval false   等価です.
    //---------------------------------------------------------------------------------------------
    bool    operator != ( const CRC32& value ) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      代入演算子です.
    //!
    //! @param[in]      value       代入する値.
    //! @return     代入結果を返却します.
    //---------------------------------------------------------------------------------------------
    CRC32&  operator =  ( const CRC32& value );

protected:
    //=============================================================================================
    // protected variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // protected methods.
    //=============================================================================================
    /* NOTHING */

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    u32     m_Hash;     //!< ハッシュキーです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// FNV1 class
///////////////////////////////////////////////////////////////////////////////////////////////////
class FNV1
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
    FNV1();

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      size        バッファサイズです.
    //! @param[in]      pBuffer     バッファです.
    //---------------------------------------------------------------------------------------------
    FNV1( const u32 size, const u8* pBuffer );

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      pBuffer     文字列です.
    //---------------------------------------------------------------------------------------------
    FNV1( const char8*  pBuffer );

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      pBuffer     文字列です.
    //---------------------------------------------------------------------------------------------
    FNV1( const char16* pBuffer );

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param[in]      value       ハッシュキー.
    //---------------------------------------------------------------------------------------------
    FNV1( const u32 value );

    //---------------------------------------------------------------------------------------------
    //! @brief      コピーコンストラクタです.
    //!
    //! @param[in]      value       コピー元の値.
    //---------------------------------------------------------------------------------------------
    FNV1( const FNV1& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      ハッシュキーを取得します.
    //!
    //! @return     ハッシュキーを返却します.
    //---------------------------------------------------------------------------------------------
    u32 GetHash() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      u32型へのキャストです.
    //!
    //! @return     ハッシュキーを返却します.
    //---------------------------------------------------------------------------------------------
    operator u32();

    //---------------------------------------------------------------------------------------------
    //! @brief      const u32型へのキャストです.
    //!
    //! @return     ハッシュキーを返却します.
    //---------------------------------------------------------------------------------------------
    operator const u32 () const;

    //---------------------------------------------------------------------------------------------
    //! @brief      等価比較演算子です.
    //!
    //! @param[in]      value       比較する値.
    //! @retval true    等価です.
    //! @retval false   非等価です.
    //---------------------------------------------------------------------------------------------
    bool    operator == ( const FNV1& value ) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      非等価比較演算子です.
    //!
    //! @param[in]      value       比較する値.
    //! @retval true    非等価です.
    //! @retval false   等価です.
    //---------------------------------------------------------------------------------------------
    bool    operator != ( const FNV1& value ) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      代入演算子です.
    //!
    //! @param[in]      value       代入する値.
    //! @return     代入結果を返却します.
    //---------------------------------------------------------------------------------------------
    FNV1&  operator =  ( const FNV1& value );

protected:
    //=============================================================================================
    // protected variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // protected methods.
    //=============================================================================================
    /* NOTHING */

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    u32     m_Hash;     //!< ハッシュキーです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace asdx

#endif//__ASDX_HASH_H__
