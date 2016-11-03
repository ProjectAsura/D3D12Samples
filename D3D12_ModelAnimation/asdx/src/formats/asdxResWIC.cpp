//-------------------------------------------------------------------------------------------------
// File : asdxResWIC.cpp
// Desc : WIC Texture Loader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cassert>
#include <new>
#include <wincodec.h>
#include <asdxRef.h>
#include <asdxLogger.h>
#include "asdxResWIC.h"


namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////////////////////////////////
// WICTranslate structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct WICTranslate
{
    GUID        Wic;
    DXGI_FORMAT Format;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// WICConvert structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct WICConvert
{
    GUID    Source;
    GUID    Target;
};

static bool g_WIC2 = false;
static const WICTranslate g_WICFormats[] = {
    {GUID_WICPixelFormat128bppRGBAFloat,    DXGI_FORMAT_R32G32B32A32_FLOAT},
    {GUID_WICPixelFormat64bppRGBAHalf,      DXGI_FORMAT_R16G16B16A16_FLOAT},
    {GUID_WICPixelFormat64bppRGBA,          DXGI_FORMAT_R16G16B16A16_UNORM},
    {GUID_WICPixelFormat32bppRGBA1010102,   DXGI_FORMAT_R10G10B10A2_UNORM},
    {GUID_WICPixelFormat32bppRGBA,          DXGI_FORMAT_R8G8B8A8_UNORM},
    {GUID_WICPixelFormat32bppBGRA,          DXGI_FORMAT_B8G8R8A8_UNORM},
    {GUID_WICPixelFormat32bppBGR,           DXGI_FORMAT_B8G8R8X8_UNORM},
    {GUID_WICPixelFormat16bppBGRA5551,      DXGI_FORMAT_B5G5R5A1_UNORM},
    {GUID_WICPixelFormat16bppBGR565,        DXGI_FORMAT_B5G6R5_UNORM},
    {GUID_WICPixelFormat32bppGrayFloat,     DXGI_FORMAT_R32_FLOAT},
    {GUID_WICPixelFormat16bppGrayHalf,      DXGI_FORMAT_R16_FLOAT},
    {GUID_WICPixelFormat16bppGray,          DXGI_FORMAT_R16_UNORM},
    {GUID_WICPixelFormat8bppGray,           DXGI_FORMAT_R8_UNORM},
    {GUID_WICPixelFormat8bppAlpha,          DXGI_FORMAT_A8_UNORM}, 
    {GUID_WICPixelFormatBlackWhite,         DXGI_FORMAT_R1_UNORM},
};

static const WICConvert g_WICConvert[] = {
    { GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

    { GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM
    { GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

    { GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf }, // DXGI_FORMAT_R16_FLOAT
    { GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT

    { GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM

    { GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

    { GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

    { GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

    { GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT

    { GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat32bppRGBE,             GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT

    { GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
    { GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
#endif
};

static bool GetWIC(IWICImagingFactory** ppResult)
{
    HRESULT hr = S_OK;
    hr = CoCreateInstance(
        CLSID_WICImagingFactory2,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(ppResult));
    if ( SUCCEEDED(hr) )
    { g_WIC2 = true; }
    else
    {
        hr = CoCreateInstance(
            CLSID_WICImagingFactory1,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(ppResult));
        if ( FAILED(hr) )
        {
            (*ppResult) = nullptr;
            return false;
        }
    }

    return true;
}

static DXGI_FORMAT WICtoDXGI( const GUID& guid )
{
    for( size_t i=0; i<_countof(g_WICFormats); ++i)
    {
        if ( memcmp( &g_WICFormats[i].Wic, &guid, sizeof(GUID)) == 0 )
        { return g_WICFormats[i].Format; }
    }

    if ( g_WIC2 )
    {
        if ( memcmp( &GUID_WICPixelFormat96bppRGBFloat, &guid, sizeof(GUID)) == 0 )
        { return DXGI_FORMAT_R32G32B32_FLOAT; }
    }

    return DXGI_FORMAT_UNKNOWN;
}

static size_t WICBitsPerPixel( IWICImagingFactory* pFactory, const GUID& guid )
{
    if ( pFactory == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return 0;
    }

    asdx::RefPtr<IWICComponentInfo> info;
    if ( FAILED( pFactory->CreateComponentInfo( guid, info.GetAddress() )) ) 
    {
        ELOG( "Error : IWICComponentInfo::CreateComponentInfo() Failed." );
        return 0;
    }

    WICComponentType type;
    if ( FAILED( info->GetComponentType( &type ) ) )
    {
        ELOG( "Error : IWICComponentInfo::GetComponentType() Failed." );
        return 0;
    }

    if ( type != WICPixelFormat )
    {
        ELOG( "Error : Invalid Component Type." );
        return 0;
    }

    asdx::RefPtr<IWICPixelFormatInfo> formatInfo;
    auto hr = info->QueryInterface(__uuidof(IWICPixelFormatInfo), reinterpret_cast<void**>(formatInfo.GetAddress()));
    if ( FAILED(hr) )
    {
        ELOG( "Error : IWICCompoentInfo::QueryInterface() Failed." );
        return 0;
    }

    UINT bpp;
    if ( FAILED( formatInfo->GetBitsPerPixel( &bpp )))
    {
        ELOG( "Error : IWICPixelFormatInfo::GetBitsPerPixel() Failed." );
        return 0;
    }

    return static_cast<size_t>(bpp);
}

static DXGI_FORMAT MakeSRGB( DXGI_FORMAT format )
{
    switch( format )
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    case DXGI_FORMAT_BC1_UNORM:
        return DXGI_FORMAT_BC1_UNORM_SRGB;

    case DXGI_FORMAT_BC2_UNORM:
        return DXGI_FORMAT_BC2_UNORM_SRGB;

    case DXGI_FORMAT_BC3_UNORM:
        return DXGI_FORMAT_BC3_UNORM_SRGB;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
        return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

    case DXGI_FORMAT_BC7_UNORM:
        return DXGI_FORMAT_BC7_UNORM_SRGB;

    default:
        return format;
    }
}

} // namespace /* anonymous */


namespace asdx {

//-------------------------------------------------------------------------------------------------
//      WICからリソーステクスチャを読込します.
//      (*bmp, *.jpg, *.png, *.tif, *.gif, *.hdp)
//-------------------------------------------------------------------------------------------------
bool LoadResTextureFromWIC( const char16* filename, asdx::ResTexture* pResult )
{
    if ( filename == nullptr || pResult == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    RefPtr<IWICImagingFactory> factory;
    if ( !GetWIC( factory.GetAddress() ) )
    {
        ELOG( "Error : GetWIC() Failed." );
        return false;
    }

    RefPtr<IWICBitmapDecoder> decoder;
    auto hr = factory->CreateDecoderFromFilename(
        filename,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand, decoder.GetAddress() );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : IWICImagingFactory::CreateDecoderFromFilename() Failed." );
        return false;
    }

    RefPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame( 0, frame.GetAddress() );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : IWICBitmapDecoder::GetFrame() Failed." );
        return false;
    }

    u32 width  = 0;
    u32 height = 0;

    hr = frame->GetSize( &width, &height );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : IWICBitmapFrameDecoder::GetSize() Failed." );
        return false;
    }

    assert( width > 0 && height > 0 );

    u32 origWidth  = width;
    u32 origHeight = height;

    static const u32 MaxTextureSize = 16384;    // D3D_FEATURE_LEVEL_11_0

    if ( width > MaxTextureSize || height > MaxTextureSize )
    {
        f32 ratio = f32( height ) / f32( width );
        if ( width > height )
        {
            width  = MaxTextureSize;
            height = u32( f32( MaxTextureSize ) * ratio );
        }
        else
        {
            width  = u32( f32( MaxTextureSize ) / ratio );
            height = MaxTextureSize;
        }
    }

    WICPixelFormatGUID pixelFormat;
    hr = frame->GetPixelFormat( &pixelFormat );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : IWICBitmapFrameDecode::GetPixelFomat() Failed." );
        return false;
    }

    WICPixelFormatGUID convertGuid;
    memcpy( &convertGuid, &pixelFormat, sizeof(WICPixelFormatGUID) );

    size_t bpp = 0;
    auto format = WICtoDXGI( pixelFormat );
    if ( format == DXGI_FORMAT_UNKNOWN )
    {
        if ( memcmp( &GUID_WICPixelFormat96bppRGBFixedPoint, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0 )
        {
            if ( g_WIC2 )
            {
                memcpy( &convertGuid, &GUID_WICPixelFormat96bppRGBFloat, sizeof(WICPixelFormatGUID) );
                format = DXGI_FORMAT_R32G32B32_FLOAT;
                bpp = WICBitsPerPixel( factory.GetPtr(), GUID_WICPixelFormat96bppRGBFloat );
            }
            else
            {
                memcpy( &convertGuid, &GUID_WICPixelFormat128bppRGBAFloat, sizeof(WICPixelFormatGUID) );
                format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                bpp = WICBitsPerPixel( factory.GetPtr(), convertGuid );
            }
        }
        else
        {
            for( size_t i=0; i<_countof(g_WICFormats); ++i )
            {
                if ( memcmp( &g_WICConvert[i].Source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0 )
                {
                    memcpy( &convertGuid, &g_WICConvert[i].Target, sizeof(WICPixelFormatGUID) );
                    format = WICtoDXGI( g_WICConvert[i].Target );
                    assert( format != DXGI_FORMAT_UNKNOWN );
                    bpp = WICBitsPerPixel( factory.GetPtr(), convertGuid );
                    break;
                }
            }
        }

        if ( format == DXGI_FORMAT_UNKNOWN )
        {
            ELOG( "Error : Convert Failed." );
            return false;
        }
    }
    else
    {
        bpp = WICBitsPerPixel( factory.GetPtr(), pixelFormat );
    }

    if ( !bpp )
    {
        ELOG( "Error : Invalid Bits Per Pixel." );
        return false;
    }

    format = MakeSRGB( format );

    // １行当たりのバイト数.
    size_t rowPitch = ( width * bpp + 7 ) / 8;

    // ピクセルデータのサイズ.
    size_t slicePitch = rowPitch * height;

    // ピクセルデータのメモリを確保.
    u8* pPixels = new (std::nothrow) u8 [ slicePitch ];
    if ( !pPixels )
    { return false; }

    // 元サイズと同じ場合.
    if ( 0 == memcmp( &convertGuid, &pixelFormat, sizeof(GUID) )
      && width  == origWidth
      && height == origHeight )
    {
        hr = frame->CopyPixels( 0, u32( rowPitch ), u32( slicePitch ), pPixels );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : IWICBitmapFrameDecode::CopyPixel() Failed." );
            SafeDeleteArray( pPixels );
            return false;
        }
    }
    else if ( width != origWidth
          || height != origHeight )
    {
        // リサイズ処理.
        RefPtr<IWICBitmapScaler> scaler;
        hr = factory->CreateBitmapScaler( scaler.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : IWICImagingFactory::CreateBitmapScaler() Failed." );
            SafeDeleteArray( pPixels );
            return false;
        }

        hr = scaler->Initialize( frame.GetPtr(), width, height, WICBitmapInterpolationModeFant );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        WICPixelFormatGUID pfScalar;
        hr = scaler->GetPixelFormat( &pfScalar );
        if ( FAILED( hr ) )
        {
            SafeDeleteArray( pPixels );
            return false;
        }

        if ( 0 == memcmp( &convertGuid, &pfScalar, sizeof(GUID) ) )
        {
            hr = scaler->CopyPixels( 0, u32( rowPitch ), u32( slicePitch ), pPixels );
            if ( FAILED( hr ) )
            {
                ELOG( "Error : IWICBitmapScaler::CopyPixels() Failed." );
                SafeDeleteArray( pPixels );
                return false;
            }
        }
        else
        {
            RefPtr<IWICFormatConverter> conv;
            hr =  factory->CreateFormatConverter( conv.GetAddress() );
            if ( FAILED( hr ) )
            {
                ELOG( "Errro : IWICImagingFactory::CreateFormatConvert() Failed." );
                SafeDeleteArray( pPixels );
                return false;
            }

            hr = conv->Initialize( scaler.GetPtr(), convertGuid, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom );
            if ( FAILED( hr ) )
            {
                ELOG( "Error : IWICFormatConverter::Initialize() Failed." );
                SafeDeleteArray( pPixels );
                return false;
            }

            hr = conv->CopyPixels( 0, u32( rowPitch ), u32( slicePitch ), pPixels );
            if ( FAILED( hr ) )
            {
                ELOG( "Error : IWICFormatConverter::CopyPixels() Failed." );
                SafeDeleteArray( pPixels );
                return false;
            }
        }
    }
    else
    {
        // フォーマットのみが違う場合.
        RefPtr<IWICFormatConverter> conv;
        hr = factory->CreateFormatConverter( conv.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : IWICImagingFactory::CreateFormatConverter() Failed." );
            SafeDeleteArray( pPixels );
            return false;
        }

        hr = conv->Initialize( frame.GetPtr(), convertGuid, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : IWICFormatConverter::Initialize() Failed." );
            SafeDeleteArray( pPixels );
            return false;
        }

        hr = conv->CopyPixels( 0, u32( rowPitch ), u32( slicePitch ), pPixels );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : IWICFormatConverter::CopyPixels() Failed." );
            SafeDeleteArray( pPixels );
            return false;
        }
    }

    // サブリソースのメモリを確保.
    Surface* pRes = new (std::nothrow) Surface();
    if ( pRes == nullptr )
    {
        SafeDeleteArray( pPixels );
        return false;
    }

    // サブリソースを設定.
    pRes->Width      = width;
    pRes->Height     = height;
    pRes->RowPitch   = u32( rowPitch );
    pRes->SlicePitch = u32( slicePitch );
    pRes->pPixels    = pPixels;

    // リソーステクスチャを設定.
    (*pResult).Width        = width;
    (*pResult).Height       = height;
    (*pResult).Depth        = 0;
    (*pResult).Format       = u32( format );
    (*pResult).MipMapCount  = 1;
    (*pResult).SurfaceCount = 1;
    (*pResult).Option       = RESTEXTURE_OPTION_NONE;
    (*pResult).pSurfaces    = pRes;

    // 正常終了.
    return true;
}

} // namespace asdx


