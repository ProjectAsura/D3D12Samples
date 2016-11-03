//--------------------------------------------------------------------------------------------------
// File : asdxTypedef.h
// Desc : Type definitions.
// Copyright(c) Project Asura. All right reserved.
//--------------------------------------------------------------------------------------------------
#pragma once

#define ASDX_VERSION_MAJOR      1
#define ASDX_VERSION_MINOR      0
#define ASDX_VERSION_PATCH      0
#define ASDX_VERSION_BUILD      0


#ifndef ASDX_VERSION_NUMBER
#define ASDX_VERSION_NUMBER(major, minor, patch, build) ( ((major) << 24) | ((minor) << 16) | ((patch) << 8) | ((build) << 0) )
#endif//ASDX_VERSION_NUMBER

#define ASDX_CURRENT_VERSION_NUMBER     ASDX_VERSION_NUMBER( ASDX_VERSION_MAJOR, ASDX_VERSION_MINOR, ASDX_VERSION_PATCH, ASDX_VERSION_BUILD )


#if defined(WIN32) || defined(_WIN32)
#ifndef ASDX_IS_WIN
#define ASDX_IS_WIN         (1)
#endif//ASDX_IS_WIN
#endif// defined(WIN32) || defined(_WIN32)


#if defined(_XBOX_ONE_)
#ifndef ASDX_IS_XBOX_ONE
#define ASDX_IS_XBOX_ONE    (1)
#endif//ASDX_IS_XBOX_ONE
#endif// defined(_XBOX_ONE_)


#if defined(DEBUG) || defined(_DEBUG)
#define ASDX_IS_DEBUG       (1)
#define ASDX_IS_NDEBUG      (0)
#else
#define ASDX_IS_DEBUG       (0)
#define ASDX_IS_NDEBUG      (1)
#endif


#ifndef ASDX_INLINE
    #ifdef  _MSC_VER
        #if (_MSC_VER >= 1200)
            #define ASDX_INLINE         __forceinline
        #else
            #define ASDX_INLINE         __inline
        #endif//(_MSC_VER >= 1200)
    #else
        #ifdef __cplusplus
            #define ASDX_INLINE         inline
        #else
            #define ASDX_INLINE
        #endif//__cpulusplus
    #endif//_MSC_VER
#endif//ASDX_INLINE


#ifndef ASDX_TEMPLATE
#define ASDX_TEMPLATE(T)                template< typename T >
#endif//ASDX_TEMPLATE


#ifndef ASDX_TEMPLATE2
#define ASDX_TEMPLATE2(T, U)            template< typename T, typename U >
#endif//ASDX_TEMPLATE2


#ifndef ASDX_TEMPLATE3
#define ASDX_TEMPLATE3(T, U, V)         template< typename T, typename U, typename V >
#endif//ASDX_TEMPLATE2


#ifndef ASDX_TEMPLATE_INLINE
#define ASDX_TEMPLATE_INLINE(T)         ASDX_TEMPLATE(T) ASDX_INLINE
#endif//ASDX_TEMPLATE_INLINE


#ifndef ASDX_TEMPLATE2_INLINE
#define ASDX_TEMPLATE2_INLINE(T, U)     ASDX_TEMPLATE2(T, U) ASDX_INLINE
#endif//ASDX_TEMPLATE2_INLINE


#ifndef ASDX_TEMPLATE3_INLINE
#define ASDX_TEMPLATE3_INLINE(T, U, V)  ASDX_TEMPLATE3(T, U, V) ASDX_INLINE
#endif//ASDX_TEMPLATE3_INLINE


#ifndef ASDX_UNUSED_VAR
#define ASDX_UNUSED_VAR(x)              ((void)x)
#endif//ASDX_UNUSED_VAR


#ifndef ASDX_NOTHROW
#define ASDX_NOTHROW                    noexcept
#endif//ASDX_NOTHROW


#ifndef ASDX_API
    #if ASDX_IS_WIN
        #ifndef ASDX_EXPORTS
            #define ASDX_API            __declspec( dllexport )
        #else
            #define ASDX_API            __declspec( dllimport )
        #endif//ASDX_EXPORTS
    #endif//ASDX_IS_WIN
#endif//ASDX_API


#ifndef ASDX_APIENTRY
    #if ASDX_IS_WIN
        #define ASDX_APIENTRY           __stdcall
    #else
        #define ASDX_APIENTRY
    #endif//ASDX_IS_WIN
#endif//ASDX_APIENTRY


#ifndef ASDX_ALIGN
    #if _MSC_VER
        #define ASDX_ALIGN( alignment )    __declspec( align(alignment) )
    #else
        #define ASDX_ALIGN( alignment )    __attribute__( aligned(alignment) )
    #endif
#endif//ASDX_ALIGN


#ifndef __ASDX_WIDE
#define __ASDX_WIDE( _string )      L ## _string
#endif//__ASDX_WIDE


#ifndef ASDX_WIDE
#define ASDX_WIDE( _string )        __ASDX_WIDE( _string )
#endif//ASDX_WIDE


#if defined(_M_IX86) || defined(_M_AMD64)
  #if defined(_M_IX86_FP)
    #define ASDX_IS_SSE2   (1)     // SSE2有効.
    #define ASDX_IS_NEON   (0)     // NEON無効.
  #else
    #define ASDX_IS_SSE2   (0)     // SSE2無効.
    #define ASDX_IS_NEON   (0)     // NEON無効.
  #endif
#elif defined(_M_ARM)
    #define ASDX_IS_SSE2   (0)     // SSE2無効.
    #define ASDX_IS_NEON   (1)     // NEON有効.
#else
    #define ASDX_IS_SSE2   (0)     // SSE2無効.
    #define ASDX_IS_NEON   (0)     // NEON無効.
#endif


#if defined(__AVX__)
    #define ASDX_IS_AVX    (1)     // Advanced Vector Extension有効.
#else
    #define ASDX_IS_AVX    (0)     // Advanced Vector Extension無効.
#endif//defined(_AVX_)

#if defined(__AVX2__)
    #define ASDX_IS_AVX2   (1)
#else
    #define ASDX_IS_AVX2   (0)
#endif


#if defined(ASDX_USE_SIMD)
    #define ASDX_IS_SIMD   (1)     // SIMD演算有効.
#else
    #define ASDX_IS_SIMD   (0)     // SIMD演算無効.
#endif// defined(ASURA_USE_SIMD)


#if defined(_OPENMP)
    #define ASDX_IS_OPENMP (1)     // OpenMP有効.
#else
    #define ASDX_IS_OPENMP (0)     // OpenMP無効.
#endif//defined(_OPENMP)


//--------------------------------------------------------------------------------------------------
// Type Defenition
//--------------------------------------------------------------------------------------------------

//==================================================================================================
//! @ingroup
//! @defgroup DataTypes     Data Definition
//! @{

//--------------------------------------------------------------------------------------------------
//! @typedef    u8
//! @brief      符号無し8bit整数です.
//! @note       書式指定は "%hhu" です.
//--------------------------------------------------------------------------------------------------
using u8 = unsigned char;

//--------------------------------------------------------------------------------------------------
//! @typedef    u16
//! @brief      符号無し16bit整数です.
//! @note       書式指定は "%hu" です.
//--------------------------------------------------------------------------------------------------
using u16 = unsigned short;

//--------------------------------------------------------------------------------------------------
//! @typedef    u32
//! @brief      符号無し32bit整数です.
//! @note       書式指定は "%u" です.
//--------------------------------------------------------------------------------------------------
using u32 = unsigned int;

//--------------------------------------------------------------------------------------------------
//! @typedef    u64
//! @brief      符号無し64bit整数です.
//! @note       書式指定は "%llu" です.
//--------------------------------------------------------------------------------------------------
#if ASDX_IS_WIN
using u64 = unsigned __int64;
#else
using u64 = unsigned long long;
#endif

//--------------------------------------------------------------------------------------------------
//! @typedef    s8
//! @brief      符号付き8bit整数です.
//! @note       書式指定は "%hhd" です.
//--------------------------------------------------------------------------------------------------
using s8 = signed char;

//--------------------------------------------------------------------------------------------------
//! @typedef    s16
//! @brief      符号付き16bit整数です.
//! @note       書式指定は "%hd" です.
//--------------------------------------------------------------------------------------------------
using s16 = signed short;

//--------------------------------------------------------------------------------------------------
//! @typedef    s32
//! @brief      符号付き32bit整数です.
//! @note       書式指定は "%d" です.
//--------------------------------------------------------------------------------------------------
using s32 = signed int;

//--------------------------------------------------------------------------------------------------
//! @typedef    s64
//! @brief      符号付き64bit整数です.
//! @note       書式指定は "%lld" です.
//--------------------------------------------------------------------------------------------------
#if ASDX_IS_WIN
using s64 = signed __int64;
#else
using s64 = signed long long;
#endif

//--------------------------------------------------------------------------------------------------
//! @typedef    f16
//! @brief      半精度(16bit)浮動小数です.
//--------------------------------------------------------------------------------------------------
using f16 = unsigned short;

//--------------------------------------------------------------------------------------------------
//! @typedef    f32
//! @brief      単精度(32bit)浮動小数です.
//! @note       書式指定は "%f" です.
//--------------------------------------------------------------------------------------------------
using f32 = float;

//--------------------------------------------------------------------------------------------------
//! @typedef    f64
//! @brief      倍精度(64bit)浮動小数です.
//! @note       書式指定は "%lf" です.
//--------------------------------------------------------------------------------------------------
using f64 = double;

//--------------------------------------------------------------------------------------------------
//! @typedef    char8
//! @brief      8bit文字です.
//--------------------------------------------------------------------------------------------------
using char8 = char;

//--------------------------------------------------------------------------------------------------
//! @typedef    char16
//! @brief      16bit文字です.
//--------------------------------------------------------------------------------------------------
using char16 = wchar_t;

//-------------------------------------------------------------------------------------------------
//! @typedef    sptr
//! @brief      符号付き整数ポインタです.
//-------------------------------------------------------------------------------------------------
#ifdef _WIN64
using sptr = __int64;
#else
using sptr = _w64 int;
#endif

//-------------------------------------------------------------------------------------------------
//! @typedef    uptr
//! @brief      符号なし整数ポインタです.
//-------------------------------------------------------------------------------------------------
#ifdef _WIN64 
using uptr = unsigned __int64;
#else
using uptr = _w64 unsigned int;
#endif

//-------------------------------------------------------------------------------------------------
//! @typedef    nullptr_type
//! @brief      nullptr型です。
//-------------------------------------------------------------------------------------------------
using nullptr_type = decltype(__nullptr);


//--------------------------------------------------------------------------------------------------
//! @def        S8_MIN
//! @brief      符号付き8bit整数型の最小値です.
//--------------------------------------------------------------------------------------------------
#ifndef S8_MIN
#define S8_MIN          (-127i8 - 1)
#endif//S8_MIN

//--------------------------------------------------------------------------------------------------
//! @def        S16_MIN
//! @brief      符号付き16bit整数型の最小値です.
//--------------------------------------------------------------------------------------------------
#ifndef S16_MIN
#define S16_MIN         (-32767i16 - 1)
#endif//S16_MIN

//--------------------------------------------------------------------------------------------------
//! @def        S32_MIN
//! @brief      符号付き32bit整数型の最小値です.
//--------------------------------------------------------------------------------------------------
#ifndef S32_MIN
#define S32_MIN         (-2147483647i32 - 1)
#endif//S32_MIN

//--------------------------------------------------------------------------------------------------
//! @def        S64_MIN
//! @brief      符号付き64bit整数型の最小値です.
//--------------------------------------------------------------------------------------------------
#ifndef S64_MIN
#define S64_MIN         (-9223372036854775807i64 - 1)
#endif//S64_MIN

//--------------------------------------------------------------------------------------------------
//! @def        S8_MAX
//! @brief      符号付8bit整数型の最大値です.
//--------------------------------------------------------------------------------------------------
#ifndef S8_MAX
#define S8_MAX          127i8
#endif//S8_MAX

//--------------------------------------------------------------------------------------------------
//! @def        S16_MAX
//! @brief      符号付き16bit整数型の最大値です.
//--------------------------------------------------------------------------------------------------
#ifndef S16_MAX
#define S16_MAX         32767i16
#endif//S16_MAX

//--------------------------------------------------------------------------------------------------
//! @def        S32_MAX
//! @brief      符号付き32bit整数型の最大値です.
//--------------------------------------------------------------------------------------------------
#ifndef S32_MAX
#define S32_MAX         2147483647i32
#endif//S32_MAX

//--------------------------------------------------------------------------------------------------
//! @def        S64_MAX
//! @brief      符号付き64bit整数型の最大値です.
//--------------------------------------------------------------------------------------------------
#ifndef S64_MAX
#define S64_MAX         9223372036854775807i64
#endif//S64_MAX

//--------------------------------------------------------------------------------------------------
//! @def        U8_MAX
//! @brief      符号無し8bit整数型の最大値です.
//--------------------------------------------------------------------------------------------------
#ifndef U8_MAX
#define U8_MAX          0xffui8
#endif//U8_MAX

//--------------------------------------------------------------------------------------------------
//! @def        U16_MAX
//! @brief      符号無し16bit整数型の最大値です.
//--------------------------------------------------------------------------------------------------
#ifndef U16_MAX
#define U16_MAX         0xffffui16
#endif//U16_MAX

//--------------------------------------------------------------------------------------------------
//! @def        U32_MAX
//! @brief      符号無し32bit整数型の最大値です.
//--------------------------------------------------------------------------------------------------
#ifndef U32_MAX
#define U32_MAX         0xffffffffui32
#endif//U32_MAX

//--------------------------------------------------------------------------------------------------
//! @def        U64_MAX
//! @brief      符号無し64bit整数型の最大値です.
//--------------------------------------------------------------------------------------------------
#ifndef U64_MAX
#define U64_MAX         0xffffffffffffffffui64
#endif//U64_MAX

//--------------------------------------------------------------------------------------------------
//! @def        S64_C
//! @brief      値xをs64型の符号付き整数定数式へ展開します.
//--------------------------------------------------------------------------------------------------
#ifndef S64_C
#define S64_C(x)        ( (x) + ( S64_MAX - S64_MAX ) )
#endif//S64_C

//--------------------------------------------------------------------------------------------------
//! @def        U64_C
//! @brief      値xをu64型の符号無し整数定数式へ展開します.
//--------------------------------------------------------------------------------------------------
#ifndef U64_C
#define U64_C(x)        ( (x) + ( U64_MAX - U64_MAX ) )
#endif//U64_C

//--------------------------------------------------------------------------------------------------
//! @def        F16_MIN
//! @brief      半精度(16bit)浮動小数型の最小値です.
//--------------------------------------------------------------------------------------------------
#ifndef F16_MIN
#define F16_MIN             5.96046448e-08F
#endif//F16_MIN

//--------------------------------------------------------------------------------------------------
//! @def        F32_MIN
//! @brief      単精度(32bit)浮動小数型の最小値です.
//--------------------------------------------------------------------------------------------------
#ifndef F32_MIN
#define F32_MIN             1.175494351e-38F
#endif//F32_MIN

//--------------------------------------------------------------------------------------------------
//! @def        F64_MIN
//! @brief      倍精度(64bit)浮動小数型の最小値です.
//--------------------------------------------------------------------------------------------------
#ifndef F64_MIN
#define F64_MIN             2.2250738585072014e-308
#endif//F64_MIN

//--------------------------------------------------------------------------------------------------
//! @def        F16_MAX
//! @brief      半精度(16bit)浮動小数型の最大値です.
//--------------------------------------------------------------------------------------------------
#ifndef F16_MAX
#define F16_MAX             65504.0f
#endif//F16_MAX

//--------------------------------------------------------------------------------------------------
//! @def        F32_MAX
//! @brief      単精度(32bit)浮動小数型の最大値です.
//--------------------------------------------------------------------------------------------------
#ifndef F32_MAX
#define F32_MAX             3.402823466e+38F
#endif//F32_MAX

//--------------------------------------------------------------------------------------------------
//! @def        F64_MAX
//! @brief      倍精度(64bit)浮動小数型の最大値です.
//--------------------------------------------------------------------------------------------------
#ifndef F64_MAX
#define F64_MAX             1.7976931348623158e+308
#endif//F64_MAX

//--------------------------------------------------------------------------------------------------
//! @def        F16_EPSILON
//! @brief      半精度(16bit)浮動小数型で表現可能な1より大きい最小値と1との差.
//--------------------------------------------------------------------------------------------------
#ifndef F16_EPSILON
#define F16_EPSILON         0.00097656f
#endif//F16_EPSILON

//--------------------------------------------------------------------------------------------------
//! @def        F32_EPSILON
//! @brief      単精度(32bit)浮動小数型で表現可能な1より大きい最小値と1との差.
//--------------------------------------------------------------------------------------------------
#ifndef F32_EPSILON
#define F32_EPSILON         1.192092896e-07F
#endif//F32_EPSILON

//--------------------------------------------------------------------------------------------------
//! @def        F64_EPSILON
//! @brief      倍精度(64bit)浮動小数型で表現可能な1より大きい最小値と1との差.
//--------------------------------------------------------------------------------------------------
#ifndef F64_EPSILON
#define F64_EPSILON         2.2204460492503131e-016
#endif//F64_EPSILON

//! @}
//==================================================================================================


//--------------------------------------------------------------------------------------------------
//! @brief      メモリを破棄します.
//!
//! @param[in]      ptr     破棄するデータへのポインタ.
//--------------------------------------------------------------------------------------------------
ASDX_TEMPLATE(T)
void SafeDelete(T*& ptr)
{
    // 不完全型かどうかを判定. 不完全型の場合は負要素となる配列を宣言し，コンパイラーにエラーを出す.
    typedef u8 type_checker[ sizeof(T) ? 1 : - 1 ];
    (void) sizeof(type_checker);

    if (ptr != nullptr)
    {
        delete ptr;
        ptr = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      配列メモリを破棄します.
//!
//! @param[in]      ptr     破棄するデータへのポインタ.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE(T)
void SafeDeleteArray(T*& ptr)
{
    // 不完全型かどうかを判定. 不完全型の場合は負要素となる配列を宣言し，コンパイラーにエラーを出す.
    typedef u8 type_checker[ sizeof(T) ? 1 : - 1 ];
    (void) sizeof(type_checker);

    if (ptr != nullptr)
    {
        delete[] ptr;
        ptr = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      解放処理を行います.
//!
//! @param[in]      ptr     解放するデータへのポインタ.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE(T)
void SafeRelease(T*& ptr)
{
    if (ptr != nullptr)
    {
        ptr->Release();
        ptr = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NonCopyable structure
////////////////////////////////////////////////////////////////////////////////////////////////////
struct NonCopyable
{
    NonCopyable             ()                     = default;
    NonCopyable             ( const NonCopyable& ) = delete;
    NonCopyable& operator = ( const NonCopyable& ) = delete;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// FixedArray structure
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, int S>
struct FixedArray
{
    static_assert(S > 0, "FixedArray size is more than 1.");

    T data[S];   //!< 要素です.

    //---------------------------------------------------------------------------------------------
    //! @brief      配列サイズを取得します.
    //!
    //! @return 配列サイズを返却します.
    //---------------------------------------------------------------------------------------------
    int size() const { return S; }

    //---------------------------------------------------------------------------------------------
    //! @brief      インデクサです.
    //!
    //! @param[in]      index       取得する配列要素番号です.
    //! @return     指定された配列要素を返却します.
    //---------------------------------------------------------------------------------------------
    T& operator[] (int index) { return data[index]; }

    //---------------------------------------------------------------------------------------------
    //! @brief      インデクサです(const版).
    //!
    //! @param[in]      index       取得する配列要素番号です.
    //! @return     指定された配列要素を返却します.
    //---------------------------------------------------------------------------------------------
    const T& operator[] (int index) const { return data[index]; }

    //---------------------------------------------------------------------------------------------
    //! @brief      最初のイテレータを返します.
    //!
    //! @return     最初のイテレータを返却します.
    //---------------------------------------------------------------------------------------------
    T* begin() { return &data[0]; }

    //---------------------------------------------------------------------------------------------
    //! @brief      最後のイテレータを返します.
    //!
    //! @return     最後のイテレータを返却します.
    //---------------------------------------------------------------------------------------------
    T* end() { return &data[S - 1]; }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// FixedArray<T, 2> structure
///////////////////////////////////////////////////////////////////////////////////////////////////
ASDX_TEMPLATE(T)
struct FixedArray<T, 2>
{
    union
    {
        T data[2];
        struct { T x, y; };
    };

    explicit FixedArray<T, 2>(T val = 0)
    : x(val)
    , y(val)
    { /* DO_NOTHING */ }

    explicit FixedArray<T, 2>(T nx, T ny)
    : x(nx)
    , y(ny)
    { /* DO_NOTHING */ }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// FixedArray<T, 3> structure
///////////////////////////////////////////////////////////////////////////////////////////////////
ASDX_TEMPLATE(T)
struct FixedArray<T, 3>
{
    union
    {
        T data[3];
        struct { T x, y, z; };
        struct { T r, g, b; };
    };

    explicit FixedArray<T, 3>(T val = 0)
    : x(val)
    , y(val)
    , z(val)
    { /* DO_NOTHING */ }

    explicit FixedArray<T, 3>(T nx, T ny, T nz)
    : x(nx)
    , y(ny)
    , z(nz)
    { /* DO_NOTHING */ }

    explicit FixedArray<T, 3>(FixedArray<T, 2>& vec, T nz)
    : x(vec.x)
    , y(vec.y)
    , z(nz)
    { /* DO_NOTHING */ }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// FixedArray<T, 4> structure
///////////////////////////////////////////////////////////////////////////////////////////////////
ASDX_TEMPLATE(T)
struct FixedArray<T, 4>
{
    union
    {
        T data[4];
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
    };

    explicit FixedArray<T, 4>(T val = 0)
    : x(val)
    , y(val)
    , z(val)
    , w(val)
    { /* DO_NOTHING */ }

    explicit FixedArray<T, 4>(T nx, T ny, T nz, T nw)
    : x(nx)
    , y(ny)
    , z(nz)
    , w(nw)
    { /* DO_NOTHING */ }

    explicit FixedArray<T, 4>(FixedArray<T, 2>& vec, T nz, T nw)
    : x(vec.x)
    , y(vec.y)
    , z(nz)
    , w(nw)
    { /* DO_NOTHING */ }

    explicit FixedArray<T, 4>(FixedArray<T, 3>& vec, T nw)
    : x(vec.x)
    , y(vec.y)
    , z(vec.z)
    , w(nw)
    { /* DO_NOTHING */ }
};


namespace asdx { 

using bool2 = FixedArray<bool, 2>;
using bool3 = FixedArray<bool, 3>;
using bool4 = FixedArray<bool, 4>;

using sbyte2 = FixedArray<s8, 2>;
using sbyte3 = FixedArray<s8, 3>;
using sbyte4 = FixedArray<s8, 4>;

using byte2 = FixedArray<u8, 2>;
using byte3 = FixedArray<u8, 3>;
using byte4 = FixedArray<u8, 4>;

using ushort2 = FixedArray<u16, 2>;
using ushort3 = FixedArray<u16, 3>;
using ushrot4 = FixedArray<u16, 4>;

using short2 = FixedArray<s16, 2>;
using short3 = FixedArray<s16, 3>;
using short4 = FixedArray<s16, 4>;

using int2 = FixedArray<s32, 2>;
using int3 = FixedArray<s32, 3>;
using int4 = FixedArray<s32, 4>;

using uint2 = FixedArray<u32, 2>;
using uint3 = FixedArray<u32, 3>;
using uint4 = FixedArray<u32, 4>;

using long2 = FixedArray<s64, 2>;
using long3 = FixedArray<s64, 3>;
using long4 = FixedArray<s64, 4>;

using ulong2 = FixedArray<u64, 2>;
using ulong3 = FixedArray<u64, 3>;
using ulong4 = FixedArray<u64, 4>;

using half2 = FixedArray<f16, 2>;
using half3 = FixedArray<f16, 3>;
using half4 = FixedArray<f16, 4>;

using float2 = FixedArray<f32, 2>;
using float3 = FixedArray<f32, 3>;
using float4 = FixedArray<f32, 4>;

using double2 = FixedArray<f64, 2>;
using double3 = FixedArray<f64, 3>;
using double4 = FixedArray<f64, 4>;

using path8  = FixedArray<char8,  256>;
using path16 = FixedArray<char16, 256>;

} // namespace asdx
