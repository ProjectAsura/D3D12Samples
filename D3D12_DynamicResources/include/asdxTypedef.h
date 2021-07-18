//--------------------------------------------------------------------------------------------------
// File : asdxTypedef.h
// Desc : Type definitions.
// Copyright(c) Project Asura. All right reserved.
//--------------------------------------------------------------------------------------------------

#ifndef _ASDX_TYPEDEF_H_
#define _ASDX_TYPEDEF_H_


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
#define ASDX_NOTHROW                    throw()
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


#ifndef ASDX_DELETE
#define ASDX_DELETE(p)              { if (p) { delete (p); (p) = nullptr; } }
#endif//ASDX_DELETE


#ifndef ASDX_DELETE_ARRAY
#define ASDX_DELETE_ARRAY(p)        { if (p) { delete[] (p); (p) = nullptr; } }
#endif//ASDX_DELETE_ARRAY


#ifndef ASDX_RELEASE
#define ASDX_RELEASE(p)             { if (p) { (p)->Release(); (p) = nullptr; } }
#endif//ASDX_RELEASE


#ifndef __ASDX_WIDE
#define __ASDX_WIDE( _string )      L ## _string
#endif//__ASDX_WIDE


#ifndef ASDX_WIDE
#define ASDX_WIDE( _string )        __ASDX_WIDE( _string )
#endif//ASDX_WIDE


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
typedef unsigned char       u8;

//--------------------------------------------------------------------------------------------------
//! @typedef    u16
//! @brief      符号無し16bit整数です.
//! @note       書式指定は "%hu" です.
//--------------------------------------------------------------------------------------------------
typedef unsigned short      u16;

//--------------------------------------------------------------------------------------------------
//! @typedef    u32
//! @brief      符号無し32bit整数です.
//! @note       書式指定は "%u" です.
//--------------------------------------------------------------------------------------------------
typedef unsigned int        u32;

//--------------------------------------------------------------------------------------------------
//! @typedef    u64
//! @brief      符号無し64bit整数です.
//! @note       書式指定は "%llu" です.
//--------------------------------------------------------------------------------------------------
#if ASDX_IS_WIN
typedef unsigned __int64    u64;
#else
typedef unsigned long long  u64;
#endif

//--------------------------------------------------------------------------------------------------
//! @typedef    s8
//! @brief      符号付き8bit整数です.
//! @note       書式指定は "%hhd" です.
//--------------------------------------------------------------------------------------------------
typedef signed char         s8;

//--------------------------------------------------------------------------------------------------
//! @typedef    s16
//! @brief      符号付き16bit整数です.
//! @note       書式指定は "%hd" です.
//--------------------------------------------------------------------------------------------------
typedef signed short        s16;

//--------------------------------------------------------------------------------------------------
//! @typedef    s32
//! @brief      符号付き32bit整数です.
//! @note       書式指定は "%d" です.
//--------------------------------------------------------------------------------------------------
typedef signed int          s32;

//--------------------------------------------------------------------------------------------------
//! @typedef    s64
//! @brief      符号付き64bit整数です.
//! @note       書式指定は "%lld" です.
//--------------------------------------------------------------------------------------------------
#if ASDX_IS_WIN
typedef signed __int64      s64;
#else
typedef signed long long    s64;
#endif

//--------------------------------------------------------------------------------------------------
//! @typedef    f16
//! @brief      半精度(16bit)浮動小数です.
//--------------------------------------------------------------------------------------------------
typedef unsigned short      f16;

//--------------------------------------------------------------------------------------------------
//! @typedef    f32
//! @brief      単精度(32bit)浮動小数です.
//! @note       書式指定は "%f" です.
//--------------------------------------------------------------------------------------------------
typedef float               f32;

//--------------------------------------------------------------------------------------------------
//! @typedef    f64
//! @brief      倍精度(64bit)浮動小数です.
//! @note       書式指定は "%lf" です.
//--------------------------------------------------------------------------------------------------
typedef double              f64;

//--------------------------------------------------------------------------------------------------
//! @typedef    char8
//! @brief      8bit文字です.
//--------------------------------------------------------------------------------------------------
typedef char                char8;

//--------------------------------------------------------------------------------------------------
//! @typedef    char16
//! @brief      16bit文字です.
//--------------------------------------------------------------------------------------------------
typedef wchar_t             char16;


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
// GenericVector structure
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, int size>
struct GenericVector
{
    T data[size];
};

ASDX_TEMPLATE(T)
struct GenericVector<T, 2>
{
    union
    {
        T data[2];
        struct { T x, y; };
    };

    explicit GenericVector<T, 2>( T val = 0 )
    : x( val )
    , y( val )
    { /* DO_NOTHING */ }

    explicit GenericVector<T, 2>( T nx, T ny )
    : x( nx )
    , y( ny )
    { /* DO_NOTHING */ }
};

ASDX_TEMPLATE(T)
struct GenericVector<T, 3>
{
    union
    {
        T data[3];
        struct { T x, y, z; };
        struct { T r, g, b; };
    };

    explicit GenericVector<T, 3>( T val = 0 )
    : x( val )
    , y( val )
    , z( val )
    { /* DO_NOTHING */ }

    explicit GenericVector<T, 3>( T nx, T ny, T nz )
    : x( nx )
    , y( ny )
    , z( nz )
    { /* DO_NOTHING */ }

    explicit GenericVector<T,3>( GenericVector<T, 2>& vec, T nz )
    : x( vec.x )
    , y( vec.y )
    , z( nz )
    { /* DO_NOTHING */ }
};

ASDX_TEMPLATE(T)
struct GenericVector<T, 4>
{
    union
    {
        T data[4];
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
    };

    explicit GenericVector<T, 4>( T val = 0 )
    : x( val )
    , y( val )
    , z( val )
    , w( val )
    { /* DO_NOTHING */ }

    explicit GenericVector<T, 4>( T nx, T ny, T nz, T nw )
    : x( nx )
    , y( ny )
    , z( nz )
    , w( nw )
    { /* DO_NOTHING */ }

    explicit GenericVector<T, 4>( GenericVector<T, 2>& vec, T nz, T nw )
    : x( vec.x )
    , y( vec.y )
    , z( nz )
    , w( nw )
    { /* DO_NOTHING */ }

    explicit GenericVector<T, 4>( GenericVector<T, 3>& vec, T nw )
    : x( vec.x )
    , y( vec.y )
    , z( vec.z )
    , w( nw )
    { /* DO_NOTHING */ }
};


namespace asdx { 

typedef GenericVector<bool, 2>      bool2;
typedef GenericVector<bool, 3>      bool3;
typedef GenericVector<bool, 4>      bool4;

typedef GenericVector<s8, 2>        sbyte2;
typedef GenericVector<s8, 3>        sbyte3;
typedef GenericVector<s8, 4>        sbyte4;

typedef GenericVector<u8, 2>        byte2;
typedef GenericVector<u8, 3>        byte3;
typedef GenericVector<u8, 4>        byte4;

typedef GenericVector<u16, 2>       ushort2;
typedef GenericVector<u16, 3>       ushort3;
typedef GenericVector<u16, 4>       ushort4;

typedef GenericVector<s16, 2>       short2;
typedef GenericVector<s16, 3>       short3;
typedef GenericVector<s16, 4>       short4;

typedef GenericVector<s32, 2>       int2;
typedef GenericVector<s32, 3>       int3;
typedef GenericVector<s32, 4>       int4;

typedef GenericVector<u32, 2>       uint2;
typedef GenericVector<u32, 3>       uint3;
typedef GenericVector<u32, 4>       uint4;

typedef GenericVector<s64, 2>       long2;
typedef GenericVector<s64, 3>       long3;
typedef GenericVector<s64, 4>       long4;

typedef GenericVector<u64, 2>       ulong2;
typedef GenericVector<u64, 3>       ulong3;
typedef GenericVector<u64, 4>       ulong4;

typedef GenericVector<f32, 2>       float2;
typedef GenericVector<f32, 3>       float3;
typedef GenericVector<f32, 4>       float4;

typedef GenericVector<f64, 2>       double2;
typedef GenericVector<f64, 3>       double3;
typedef GenericVector<f64, 4>       double4;

} // namespace asdx


#endif//_ASDX_TYPEDEF_H_
