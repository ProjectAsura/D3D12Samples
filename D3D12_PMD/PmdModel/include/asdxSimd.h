﻿//-------------------------------------------------------------------------------------------------
// File : asdxSimd.h
// Desc : SIMD Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTypedef.h>

#if ASDX_IS_SIMD

#if ASDX_IS_SSE
    #include <xmmintrin.h>
    #include <emmintrin.h>
    #include <tmmintrin.h>
    #include <nmmintrin.h>

    typedef __m64       b64;
    typedef __m128      b128;

#elif ASDX_IS_NEON
    #include <armintr.h>
    #include <arm_neon.h>

    typedef __n64       b64;
    typedef __n128      b128;

#endif


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Simd class
///////////////////////////////////////////////////////////////////////////////////////////////////
class Simd
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public varibles.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      生成処理を行います.
    //!
    //! @param[in]      x       X成分.
    //! @param[in]      y       Y成分.
    //! @param[in]      z       Z成分.
    //! @param[in]      w       W成分.
    //! @return     ベクトルを返します.
    //---------------------------------------------------------------------------------------------
    static b128 Create( f32 x, f32 y, f32 z, f32 w );

    //---------------------------------------------------------------------------------------------
    //! @brief      X成分を設定します.
    //!
    //! @param[in,out]      value       設定するベクトル.
    //! @param[in]          x           設定するX成分.
    //---------------------------------------------------------------------------------------------
    static void SetX( b128& value, f32 x );

    //---------------------------------------------------------------------------------------------
    //! @brief      Y成分を設定します.
    //!
    //! @param[in,out]      value       設定するベクトル.
    //! @param[in]          y           設定するY成分.
    //---------------------------------------------------------------------------------------------
    static void SetY( b128& value, f32 y );

    //---------------------------------------------------------------------------------------------
    //! @brief      Z成分を設定します.
    //!
    //! @param[in,out]      value       設定するベクトル.
    //! @param[in]          z           設定するZ成分.
    //---------------------------------------------------------------------------------------------
    static void SetZ( b128& value, f32 z );

    //---------------------------------------------------------------------------------------------
    //! @brief      W成分を設定します.
    //!
    //! @param[in,out]      value       設定するベクトル.
    //! @param[in]          w           設定するW成分.
    //---------------------------------------------------------------------------------------------
    static void SetW( b128& value, f32 w );

    //---------------------------------------------------------------------------------------------
    //! @brief      X成分を取得します.
    //!
    //! @return     X成分を返却します.
    //---------------------------------------------------------------------------------------------
    static f32 GetX( const b128& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      Y成分を取得します.
    //!
    //! @return     Y成分を返却します.
    //---------------------------------------------------------------------------------------------
    static f32 GetY( const b128& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      Z成分を取得します.
    //!
    //! @return     Z成分を返却します.
    //---------------------------------------------------------------------------------------------
    static f32 GetZ( const b128& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      W成分を取得します.
    //!
    //! @return     W成分を返却します.
    //---------------------------------------------------------------------------------------------
    static f32 GetW( const b128& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      加算計算を行います.
    //!
    //! @param[in]      a       左オペランド
    //! @param[in]      b       右オペランド
    //! @return     a + b の計算結果を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Add( const b128& a, const b128& b );

    //---------------------------------------------------------------------------------------------
    //! @brief      減算計算を行います.
    //!
    //! @param[in]      a       左オペランド.
    //! @param[in]      b       右オペランド.
    //! @return     a - b の計算結果を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Sub( const b128& a, const b128& b );

    //---------------------------------------------------------------------------------------------
    //! @brief      乗算計算を行います.
    //!
    //! @param[in]      a       左オペランド.
    //! @param[in]      b       右オペランド.
    //! @return     a * b の計算結果を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Mul( const b128& a, const b128& b );

    //---------------------------------------------------------------------------------------------
    //! @brief      除算計算を行います.
    //!
    //! @param[in]      a       左オペランド.
    //! @param[in]      b       右オペランド.
    //! @return     a / b  の計算結果を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Div( const b128& a, const b128& b );

    //---------------------------------------------------------------------------------------------
    //! @brief      積和計算を行います.
    //!
    //! @param[in]      a       オペランド1.
    //! @param[in]      b       オペランド2.
    //! @param[in]      c       オペランド3.
    //! @return     a * b + c の計算結果を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Mad( const b128& a, const b128& b, const b128& c );

    //---------------------------------------------------------------------------------------------
    //! @brief      平方根を求めます.
    //!
    //! @param[in]      value       平方根を求める値.
    //! @return     各成分の平方根を求めた結果を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Sqrt( const b128& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      逆数を求めます.
    //!
    //! @param[in]      value       逆数を求める値.
    //! @return     各成分の逆数を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Rcp( const b128& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      平方根の逆数を求めます.
    //!
    //! @param[in]      value       平方根の逆数を求める値.
    //! @return     各成分の平方根の逆数( 1 / sqrt(v) ) を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Rsqrt( const b128& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      負符号をつけた値を求めます.
    //!
    //! @param[in]      value       負符号をつける値.
    //! @return     各成分に負符号をつけた値を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Negate( const b128& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      絶対値を求めます.
    //!
    //! @param[in]      value       絶対値を求める値.
    //! @return     各成分の絶対値を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Abs( const b128& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      最小値を求めます.
    //!
    //! @param[in]      a       値1.
    //! @param[in]      b       値2.
    //! @return     各成分の最小値を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Min( const b128& a, const b128& b );

    //---------------------------------------------------------------------------------------------
    //! @brief      最大値を求めます.
    //!
    //! @param[in]      a       値1.
    //! @param[in]      b       値2.
    //! @return     各成分の最大値を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Max( const b128& a, const b128& b );

    //---------------------------------------------------------------------------------------------
    //! @brief      内積を求めます.
    //!
    //! @param[in]      a       値1.
    //! @param[in]      b       値2.
    //! @return     3次元ベクトルの内積を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Dp3( const b128& a, const b128& b );

    //---------------------------------------------------------------------------------------------
    //! @brief      内積を求めます.
    //!
    //! @param[in]      a       左オペランド.
    //! @param[in]      b       右オペランド.
    //! @return     4次元ベクトルの内積を返却します.
    //---------------------------------------------------------------------------------------------
    static b128 Dp4( const b128& a, const b128& b );
};

} // namespace asdx

//-------------------------------------------------------------------------------------------------
// Inline Files.
//-------------------------------------------------------------------------------------------------
#if ASDX_IS_SSE
    #include <asdxSse.inl>
#elif ASDX_IS_NEON
    #include <asdxNeon.inl>
#endif

#endif//ASDX_IS_SIMD
