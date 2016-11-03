//-------------------------------------------------------------------------------------------------
// File : asdxResDDS.cpp
// Desc : Direct Draw Surface Loader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <new>
#include <cstdio>
#include <asdxMath.h>
#include <asdxLogger.h>
#include "asdxResDDS.h"


namespace /* anonymous */ {

//------------------------------------------------------------------------------------------
// Constant Values
//------------------------------------------------------------------------------------------

// dwFlags Value
static constexpr u32 DDSD_CAPS         = 0x00000001;   // dwCaps/dwCaps2が有効.
static constexpr u32 DDSD_HEIGHT       = 0x00000002;   // dwHeightが有効.
static constexpr u32 DDSD_WIDTH        = 0x00000004;   // dwWidthが有効.
static constexpr u32 DDSD_PITCH        = 0x00000008;   // dwPitchOrLinearSizeがPitchを表す.
static constexpr u32 DDSD_PIXELFORMAT  = 0x00001000;   // dwPfSize/dwPfFlags/dwRGB～等の直接定義が有効.
static constexpr u32 DDSD_MIPMAPCOUNT  = 0x00020000;   // dwMipMapCountが有効.
static constexpr u32 DDSD_LINEARSIZE   = 0x00080000;   // dwPitchOrLinearSizeがLinerSizeを表す.
static constexpr u32 DDSD_DEPTH        = 0x00800000;   // dwDepthが有効.

// dwPfFlags Value
static constexpr u32 DDPF_ALPHAPIXELS      = 0x00000001;   // RGB以外にalphaが含まれている.
static constexpr u32 DDPF_ALPHA            = 0x00000002;   // pixelはAlpha成分のみ.
static constexpr u32 DDPF_FOURCC           = 0x00000004;   // dwFourCCが有効.
static constexpr u32 DDPF_PALETTE_INDEXED4 = 0x00000008;   // Palette 16 colors.
static constexpr u32 DDPF_PALETTE_INDEXED8 = 0x00000020;   // Palette 256 colors.
static constexpr u32 DDPF_RGB              = 0x00000040;   // dwRGBBitCount/dwRBitMask/dwGBitMask/dwBBitMask/dwRGBAlphaBitMaskによってフォーマットが定義されていることを示す.
static constexpr u32 DDPF_LUMINANCE        = 0x00020000;   // 1chのデータがRGB全てに展開される.
static constexpr u32 DDPF_BUMPDUDV         = 0x00080000;   // pixelが符号付きであることを示す.

// dwCaps Value
static constexpr u32 DDSCAPS_ALPHA     = 0x00000002;       // Alphaが含まれている場合.
static constexpr u32 DDSCAPS_COMPLEX   = 0x00000008;       // 複数のデータが含まれている場合Palette/Mipmap/Cube/Volume等.
static constexpr u32 DDSCAPS_TEXTURE   = 0x00001000;       // 常に1.
static constexpr u32 DDSCAPS_MIPMAP    = 0x00400000;       // MipMapが存在する場合.

// dwCaps2 Value
static constexpr u32 DDSCAPS2_CUBEMAP              = 0x00000200;   // CubeMapが存在する場合.
static constexpr u32 DDSCAPS2_CUBEMAP_POSITIVE_X   = 0x00000400;   // CubeMap X+
static constexpr u32 DDSCAPS2_CUBEMAP_NEGATIVE_X   = 0x00000800;   // CubeMap X-
static constexpr u32 DDSCAPS2_CUBEMAP_POSITIVE_Y   = 0x00001000;   // CubeMap Y+
static constexpr u32 DDSCAPS2_CUBEMAP_NEGATIVE_Y   = 0x00002000;   // CubeMap Y-
static constexpr u32 DDSCAPS2_CUBEMAP_POSITIVE_Z   = 0x00004000;   // CubeMap Z+
static constexpr u32 DDSCAPS2_CUBEMAP_NEGATIVE_Z   = 0x00008000;   // CubeMap Z-
static constexpr u32 DDSCAPS2_VOLUME               = 0x00400000;   // VolumeTextureの場合.

// dwFourCC Value
static constexpr u32 FOURCC_DXT1           = '1TXD';           // DXT1
static constexpr u32 FOURCC_DXT2           = '2TXD';           // DXT2
static constexpr u32 FOURCC_DXT3           = '3TXD';           // DXT3
static constexpr u32 FOURCC_DXT4           = '4TXD';           // DXT4
static constexpr u32 FOURCC_DXT5           = '5TXD';           // DXT5
static constexpr u32 FOURCC_ATI1           = '1ITA';           // 3Dc ATI2
static constexpr u32 FOURCC_ATI2           = '2ITA';           // 3Dc ATI2
static constexpr u32 FOURCC_DX10           = '01XD';           // DX10
static constexpr u32 FOURCC_BC4U           = 'U4CB';           // BC4U
static constexpr u32 FOURCC_BC4S           = 'S4CB';           // BC4S
static constexpr u32 FOURCC_BC5U           = 'U5CB';           // BC5U
static constexpr u32 FOURCC_BC5S           = 'S5CB';           // BC5S
static constexpr u32 FOURCC_RGBG           = 'GBGR';           // RGBG
static constexpr u32 FOURCC_GRGB           = 'BGRG';           // GRGB
static constexpr u32 FOURCC_YUY2           = '2YUY';           // YUY2
static constexpr u32 FOURCC_A16B16G16R16   = 0x00000024;
static constexpr u32 FOURCC_Q16W16V16U16   = 0x0000006e;
static constexpr u32 FOURCC_R16F           = 0x0000006f;
static constexpr u32 FOURCC_G16R16F        = 0x00000070;
static constexpr u32 FOURCC_A16B16G16R16F  = 0x00000071;
static constexpr u32 FOURCC_R32F           = 0x00000072;
static constexpr u32 FOURCC_G32R32F        = 0x00000073;
static constexpr u32 FOURCC_A32B32G32R32F  = 0x00000074;
static constexpr u32 FOURCC_CxV8U8         = 0x00000075;
static constexpr u32 FOURCC_Q8W8V8U8       = 0x0000003f;

static constexpr u32 DDS_RESOURCE_MISC_TEXTRECUBE = 0x4L;


///////////////////////////////////////////////////////////////////////////////////////////////////
// DDS_RESOURCE_DIMENSION
///////////////////////////////////////////////////////////////////////////////////////////////////
enum DDS_RESOURCE_DIMENSION
{
    DDS_RESOURCE_DIMENSION_TEXTURE1D = 2,
    DDS_RESOURCE_DIMENSION_TEXTURE2D,
    DDS_RESOURCE_DIMENSION_TEXTURE3D,
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// DDS_FORMAT_TYPE
///////////////////////////////////////////////////////////////////////////////////////////////////
enum DDS_FORMAT_TYPE
{
    DDS_FORMAT_UNKNOWN = 0,

    DDS_FORMAT_R32G32B32A32_FLOAT = 2,

    DDS_FORMAT_R16G16B16A16_FLOAT = 10,
    DDS_FORMAT_R16G16B16A16_UNORM = 11,

    DDS_FORMAT_R32G32_FLOAT = 16,

    DDS_FORMAT_R10G10B10A2_UNORM = 24,

    DDS_FORMAT_R8G8B8A8_UNORM = 28,

    DDS_FORMAT_R16G16_FLOAT = 34,
    DDS_FORMAT_R16G16_UNORM = 35,

    DDS_FORMAT_R32_FLOAT = 41,

    DDS_FORMAT_R8G8_UNORM = 49,

    DDS_FORMAT_R16_FLOAT = 54,
    DDS_FORMAT_R16_UNORM = 56,

    DDS_FORMAT_R8_UNORM = 63,
    DDS_FORMAT_A8_UNORM = 65,

    DDS_FORMAT_R8G8_B8G8_UNORM = 68,
    DDS_FORMAT_G8R8_G8B8_UNORM = 69,

    DDS_FORMAT_BC1_UNORM = 71,
    DDS_FORMAT_BC2_UNORM = 74,
    DDS_FORMAT_BC3_UNORM = 77,
    DDS_FORMAT_BC4_UNORM = 80,
    DDS_FORMAT_BC4_SNORM = 81,
    DDS_FORMAT_BC5_UNORM = 83,
    DDS_FORMAT_BC5_SNORM = 84,
    DDS_FORMAT_BC6H_UF16 = 95,
    DDS_FORMAT_BC6H_SF16 = 96,
    DDS_FORMAT_BC7_UNORM = 98,

    DDS_FORMAT_B5G6R5_UNORM   = 85,
    DDS_FORMAT_B5G5R5A1_UNORM = 86,
    DDS_FORMAT_B8G8R8A8_UNORM = 87,
    DDS_FORMAT_B8G8R8X8_UNORM = 88,

    DDS_FORMAT_YUY2 = 107,

    //DDS_FORMAT_A8P8 = 114,
    DDS_FORMAT_B4G4R4A4_UNORM = 115,
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// DDS_PIXEL_FORMAT structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DDS_PIXEL_FORMAT
{
    u32     Size;
    u32     Flags;
    u32     FourCC;
    u32     Bpp;
    u32     MaskR;
    u32     MaskG;
    u32     MaskB;
    u32     MaskA;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DDS_COLOR_KEY structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DDS_COLOR_KEY
{
    u32     Low;
    u32     Hight;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DDS_SURFACE_DESC structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DDS_SURFACE_DESC
{
    u32                 Size;
    u32                 Flags;
    u32                 Height;
    u32                 Width;
    u32                 Pitch;
    u32                 Depth;
    u32                 MipMapLevels;
    u32                 AlphaBitDepth;
    u32                 Reserved;
    u32                 Surface;

    DDS_COLOR_KEY       DstOverlay;
    DDS_COLOR_KEY       DstBit;
    DDS_COLOR_KEY       SrcOverlay;
    DDS_COLOR_KEY       SrcBit;

    DDS_PIXEL_FORMAT    PixelFormat;
    u32                 Caps;
    u32                 Caps2;
    u32                 ReservedCaps[ 2 ];

    u32                 TextureStage;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DDS_DXT10_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DDS_DXT10_HEADER
{
    u32     DXGIFormat;
    u32     ResourceDimension;
    u32     MiscFlag;
    u32     ArraySize;
    u32     MiscFlags2;
};



//-------------------------------------------------------------------------------------------------
//      マスクをチェックします.
//-------------------------------------------------------------------------------------------------
bool CheckMask
(
    const DDS_PIXEL_FORMAT& pixelFormat,
    unsigned int maskR,
    unsigned int maskG,
    unsigned int maskB,
    unsigned int maskA
)
{
    if ( ( pixelFormat.MaskR == maskR )
      && ( pixelFormat.MaskG == maskG )
      && ( pixelFormat.MaskB == maskB )
      && ( pixelFormat.MaskA == maskA ) )
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      1ピクセル当たりのビット数を取得します.
//-------------------------------------------------------------------------------------------------
u32 GetBitPerPixel( const u32 format )
{
    u32 result = 0;
    switch( format )
    {
    case DDS_FORMAT_R32G32B32A32_FLOAT:
        { result = 128; }
        break;

    case DDS_FORMAT_R16G16B16A16_UNORM:
    case DDS_FORMAT_R16G16B16A16_FLOAT:
    case DDS_FORMAT_R32G32_FLOAT:
        { result = 64; }
        break;

    case DDS_FORMAT_R10G10B10A2_UNORM:
    case DDS_FORMAT_R8G8B8A8_UNORM:
    case DDS_FORMAT_R16G16_FLOAT:
    case DDS_FORMAT_R16G16_UNORM:
    case DDS_FORMAT_R32_FLOAT:
    case DDS_FORMAT_B8G8R8A8_UNORM:
    case DDS_FORMAT_B8G8R8X8_UNORM:
    case DDS_FORMAT_R8G8_B8G8_UNORM:
    case DDS_FORMAT_G8R8_G8B8_UNORM:
    case DDS_FORMAT_YUY2:
        { result = 32; }
        break;

    case DDS_FORMAT_R8G8_UNORM:
    case DDS_FORMAT_R16_FLOAT:
    case DDS_FORMAT_R16_UNORM:
    case DDS_FORMAT_B5G6R5_UNORM:
    case DDS_FORMAT_B5G5R5A1_UNORM:
    case DDS_FORMAT_B4G4R4A4_UNORM:
#if 0
    //case DDS_FORMAT_A8P8:
#endif
        { result = 16; }
        break;

    case DDS_FORMAT_R8_UNORM:
    case DDS_FORMAT_A8_UNORM:
#if 0
    //case DDS_FORMAT_P8:
#endif
        { result = 8; }
        break;

    case DDS_FORMAT_BC1_UNORM:
    case DDS_FORMAT_BC4_SNORM:
    case DDS_FORMAT_BC4_UNORM:
        { result = 4; }
        break;

    case DDS_FORMAT_BC2_UNORM:
    case DDS_FORMAT_BC3_UNORM:
    case DDS_FORMAT_BC5_SNORM:
    case DDS_FORMAT_BC5_UNORM:
    case DDS_FORMAT_BC6H_SF16:
    case DDS_FORMAT_BC6H_UF16:
    case DDS_FORMAT_BC7_UNORM:
        { result = 8; }
        break;

    default:
        { ELOG( "Error : Unsupported Format(%u).", format ); }
        break;
    }

    return result;
}

//-------------------------------------------------------------------------------------------------
//      サーフェイス情報を取得します.
//-------------------------------------------------------------------------------------------------
void GetSurfaceInfo
(
    const s32 width,
    const s32 height,
    const u32 format,
    u32* pNumBytes, 
    u32* pRowBytes,
    u32* pNumRows 
)
{
    auto numBytes = 0;
    auto rowBytes = 0;
    auto numRows  = 0;

    auto bc     = false;
    auto packed = false;
    u32 bpe = 0;

    switch (format)
    {
    case DDS_FORMAT_BC1_UNORM:
    case DDS_FORMAT_BC4_UNORM:
    case DDS_FORMAT_BC4_SNORM:
        {
            bc = true;
            bpe = 8;
        }
        break;

    case DDS_FORMAT_BC2_UNORM:
    case DDS_FORMAT_BC3_UNORM:
    case DDS_FORMAT_BC5_UNORM:
    case DDS_FORMAT_BC5_SNORM:
    case DDS_FORMAT_BC6H_SF16:
    case DDS_FORMAT_BC6H_UF16:
    case DDS_FORMAT_BC7_UNORM:
        {
            bc = true;
            bpe = 16;
        }
        break;

    case DDS_FORMAT_R8G8_B8G8_UNORM:
    case DDS_FORMAT_G8R8_G8B8_UNORM:
    case DDS_FORMAT_YUY2:
        {
            packed = true;
            bpe = 4;
        }
        break;
    }

    if ( bc )
    {
        u32 numBlockWide = 0;
        if ( width > 0 )
        { numBlockWide = asdx::Max<u32>( 1, (width + 3) / 4 ); }

        u32 numBlockHeigh = 0;
        if ( height > 0 )
        { numBlockHeigh = asdx::Max<u32>( 1, (height + 3) / 4 ); }

        rowBytes = numBlockWide * bpe;
        numRows  = numBlockHeigh;
        numBytes = rowBytes * numRows;
    }
    else if ( packed )
    {
        rowBytes = (( width + 1 ) >> 1 ) * bpe;
        numRows  = height;
        numBytes = rowBytes * numRows;
    }
    else
    {
        auto bpp = GetBitPerPixel( format );
        rowBytes = ( width * bpp + 7 ) / 8;
        numRows  = height;
        numBytes = rowBytes * numRows;
    }

    if ( pRowBytes )
    { (*pRowBytes) = rowBytes; }

    if ( pNumRows )
    { (*pNumRows) = numRows; }

    if ( pNumBytes )
    { (*pNumBytes) = numBytes; }
}


} // namespace /* anonymous */

namespace asdx {

//-------------------------------------------------------------------------------------------------
//      DDSからリソーステクスチャを読込します.
//-------------------------------------------------------------------------------------------------
bool LoadResTextureFromDDS( const char16* filename, ResTexture* pResult )
{
    if ( filename == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    FILE* pFile;
    auto err = _wfopen_s( &pFile, filename, L"rb" );
    if ( err != 0 )
    {
        ELOG( "Error : File Open Failed. filename = %s", filename );
        return false;
    }

    char sig[4];
    fread( sig, sizeof(char), 4, pFile );

    if ( (sig[0] != 'D')
      || (sig[1] != 'D')
      || (sig[2] != 'S')
      || (sig[3] != ' '))
    {
        ELOG( "Error : Invalid File." );
        fclose( pFile );
        return false;
    }

    auto width        = 0;
    auto height       = 0;
    auto depth        = 0;
    auto mipMapCount  = 1;
    auto surfaceCount = 1;
    auto isCubeMap    = false;
    auto isVolume     = false;
    auto format       = (u32)DDS_FORMAT_UNKNOWN;

    DDS_SURFACE_DESC desc;
    fread( &desc, sizeof(desc), 1, pFile );

    if ( desc.Flags & DDSD_HEIGHT )
    { height = desc.Height; }

    if ( desc.Flags & DDSD_WIDTH )
    { width = desc.Width; }

    if ( desc.Flags & DDSD_DEPTH )
    { depth = desc.Depth; }

    if ( desc.Flags & DDSD_MIPMAPCOUNT )
    { mipMapCount = desc.MipMapLevels; }

    if ( desc.Caps & DDSCAPS_COMPLEX )
    {
        if ( desc.Caps2 & DDSCAPS2_CUBEMAP )
        {
            surfaceCount = 0;
            if ( desc.Caps2 & DDSCAPS2_CUBEMAP_NEGATIVE_X ) { surfaceCount++; }
            if ( desc.Caps2 & DDSCAPS2_CUBEMAP_NEGATIVE_Y ) { surfaceCount++; }
            if ( desc.Caps2 & DDSCAPS2_CUBEMAP_NEGATIVE_Z ) { surfaceCount++; }
            if ( desc.Caps2 & DDSCAPS2_CUBEMAP_POSITIVE_X ) { surfaceCount++; }
            if ( desc.Caps2 & DDSCAPS2_CUBEMAP_POSITIVE_Y ) { surfaceCount++; }
            if ( desc.Caps2 & DDSCAPS2_CUBEMAP_POSITIVE_Z ) { surfaceCount++; }
            assert( surfaceCount == 6 );

            if ( surfaceCount == 6 )
            { isCubeMap = true; }
        }
        else if ( desc.Caps2 & DDSCAPS2_VOLUME )
        { isVolume = true; }
    }

    // ピクセルフォーマット有効.
    if ( desc.Flags & DDSD_PIXELFORMAT )
    {
        if ( desc.PixelFormat.Flags & DDPF_FOURCC )
        {
            switch( desc.PixelFormat.FourCC )
            {
            case FOURCC_DXT1:
                { format = DDS_FORMAT_BC1_UNORM; }
                break;

            case FOURCC_DXT2:
            case FOURCC_DXT3:
                { format = DDS_FORMAT_BC2_UNORM; }
                break;

            case FOURCC_DXT4:
            case FOURCC_DXT5:
                { format = DDS_FORMAT_BC3_UNORM; }
                break;

            case FOURCC_ATI1:
            case FOURCC_BC4U:
                { format = DDS_FORMAT_BC4_UNORM; }
                break;

            case FOURCC_BC4S:
                { format = DDS_FORMAT_BC4_SNORM; }
                break;

            case FOURCC_ATI2:
            case FOURCC_BC5U:
                { format = DDS_FORMAT_BC5_UNORM; }
                break;

            case FOURCC_BC5S:
                { format = DDS_FORMAT_BC5_SNORM; }
                break;

            case FOURCC_DX10:
                {
                    DDS_DXT10_HEADER ext;
                    fread( &ext, sizeof(ext), 1, pFile );

                    format = ext.DXGIFormat;
                    surfaceCount = ext.ArraySize;

                    switch( ext.ResourceDimension )
                    {
                    case DDS_RESOURCE_DIMENSION_TEXTURE1D:
                        {
                            if ( height != 1 )
                            {
                                ELOG( "Error : Texture1D Height is must be 1." );
                                fclose( pFile );
                                return false;
                            }
                        }
                        break;

                    case DDS_RESOURCE_DIMENSION_TEXTURE2D:
                        {
                            if ( ext.MiscFlag & DDS_RESOURCE_MISC_TEXTRECUBE )
                            { surfaceCount = ext.ArraySize * 6; }
                        }
                        break;

                    case DDS_RESOURCE_DIMENSION_TEXTURE3D:
                        {
                            if ( !isVolume )
                            {
                                ELOG( "Error : Invalid Texture3D. Volume Flag is none." );
                                fclose( pFile );
                                return false;
                            }

                            if ( surfaceCount > 1 )
                            {
                                ELOG( "Error : Texture3D is not support array." );
                                fclose( pFile );
                                return false;
                            }
                        }
                        break;
                    }
                }
                break;

            case FOURCC_RGBG:
                { format = DDS_FORMAT_R8G8_B8G8_UNORM; }
                break;

            case FOURCC_GRGB:
                { format = DDS_FORMAT_G8R8_G8B8_UNORM; }
                break;

            case FOURCC_YUY2:
                { format = DDS_FORMAT_YUY2; }
                break;

            case FOURCC_A16B16G16R16:
                { format = DDS_FORMAT_R16G16B16A16_UNORM; }
                break;

            case FOURCC_Q16W16V16U16:
                { format = DDS_FORMAT_R16G16B16A16_UNORM; }
                break;

            case FOURCC_R16F:
                { format = DDS_FORMAT_R16_FLOAT; }
                break;

            case FOURCC_G16R16F:
                { format = DDS_FORMAT_R16G16_FLOAT; }
                break;

            case FOURCC_A16B16G16R16F:
                { format = DDS_FORMAT_R16G16B16A16_FLOAT; }
                break;

            case FOURCC_R32F:
                { format = DDS_FORMAT_R32_FLOAT; }
                break;

            case FOURCC_G32R32F:
                { format = DDS_FORMAT_R32G32_FLOAT; }
                break;

            case FOURCC_A32B32G32R32F:
                { format = DDS_FORMAT_R32G32B32A32_FLOAT; }
                break;
            }
        }
        else if ( desc.PixelFormat.Flags & DDPF_RGB )
        {
            switch( desc.PixelFormat.Bpp )
            {
            case 32:
                {
                    if ( CheckMask( desc.PixelFormat, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 ) )
                    { format = DDS_FORMAT_R8G8B8A8_UNORM; }

                    if ( CheckMask( desc.PixelFormat, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) )
                    { format = DDS_FORMAT_B8G8R8A8_UNORM; }

                    if ( CheckMask( desc.PixelFormat, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 ) )
                    { format = DDS_FORMAT_B8G8R8X8_UNORM; }
 
                    if ( CheckMask( desc.PixelFormat, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000 ) )
                    { format = DDS_FORMAT_R10G10B10A2_UNORM; }

                    if ( CheckMask( desc.PixelFormat, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 ) )
                    { format = DDS_FORMAT_R16G16_UNORM; }

                    if ( CheckMask( desc.PixelFormat, 0xffffffff, 0x00000000,0x00000000,0x00000000 ) )
                    { format = DDS_FORMAT_R32_FLOAT; }
                }
                break;

            case 24:
                { /* RGBフォーマット非サポート */ }
                break;

            case 16:
                {
                    if ( CheckMask( desc.PixelFormat, 0x7c00, 0x03e0, 0x001f, 0x8000 ) )
                    { format = DDS_FORMAT_B5G5R5A1_UNORM; }

                    if ( CheckMask( desc.PixelFormat, 0xf800, 0x07e0, 0x001f, 0x0000 ) )
                    { format = DDS_FORMAT_B5G6R5_UNORM; }

                    if ( CheckMask( desc.PixelFormat, 0x0f00, 0x00f0, 0x000f, 0xf000 ) )
                    { format = DDS_FORMAT_B4G4R4A4_UNORM; }
                }
                break;

            default:
                break;
            }
        }
        else if ( desc.PixelFormat.Flags & DDPF_LUMINANCE )
        {
            switch( desc.PixelFormat.Bpp )
            {
            case 8:
                {
                    if ( CheckMask( desc.PixelFormat, 0x000000ff, 0x00000000, 0x00000000, 0x00000000 ) )
                    { format = DDS_FORMAT_R8_UNORM; }
                }
                break;

            case 16:
                {
                    if ( CheckMask( desc.PixelFormat, 0x0000ffff, 0x00000000, 0x00000000, 0x00000000 ) )
                    { format = DDS_FORMAT_R16_UNORM; }

                    if ( CheckMask( desc.PixelFormat, 0x000000ff, 0x00000000, 0x00000000, 0x0000ff00 ) )
                    { format = DDS_FORMAT_R8G8_UNORM; }
                }
                break;
            }
        }
        else if ( desc.PixelFormat.Flags & DDPF_ALPHA )
        {
            if ( 8 == desc.PixelFormat.Bpp )
            { format = DDS_FORMAT_A8_UNORM; }
        }
    }

    if ( format == DDS_FORMAT_UNKNOWN )
    {
        ELOG( "Error : Unsupported Format." );
        return false;
    }

    auto cur = ftell( pFile );
    fseek( pFile, 0, SEEK_END );
    auto end = ftell( pFile );
    fseek( pFile, cur, SEEK_SET );
    auto pixelSize = end - cur;

    auto pPixels = new (std::nothrow) u8 [ pixelSize ];
    assert( pPixels != nullptr );
    if ( pPixels == nullptr )
    {
        ELOG( "Error : Out of Memory." );
        fclose( pFile );
        return false;
    }

    fread( pPixels, sizeof(u8), pixelSize, pFile );
    fclose( pFile );

    auto pSurfaces = new (std::nothrow) Surface[ mipMapCount * surfaceCount ];
    assert( pSurfaces != nullptr );
    if ( pSurfaces == nullptr )
    {
        ELOG( "Error : Out of Memory." );
        SafeDeleteArray( pPixels );
        return false;
    }

    u32 w = width;
    u32 h = height;
    u32 d = depth;
    u32 offset = 0;

    for( auto j=0; j<surfaceCount; ++j )
    {
        for( auto i=0; i<mipMapCount; ++i )
        {
            auto idx = ( mipMapCount * j ) + i;
            u32 rowBytes = 0;
            u32 numRows  = 0;
            u32 numBytes = 0;

            GetSurfaceInfo( w, h, format, &numBytes, &rowBytes, &numRows );

            pSurfaces[ idx ].Width      = w;
            pSurfaces[ idx ].Height     = h;
            pSurfaces[ idx ].RowPitch   = rowBytes;
            pSurfaces[ idx ].SlicePitch = numBytes;
            pSurfaces[ idx ].pPixels    = new (std::nothrow) u8 [ numBytes ];
            assert( pSurfaces[ idx ].pPixels != nullptr );

            if ( pSurfaces[ idx ].pPixels == nullptr )
            {
                ELOG( "Error : Out of Memory." );
                for( auto k=0; k<idx; ++k )
                { SafeDelete( pSurfaces[k].pPixels ); }

                SafeDeleteArray( pPixels );
                SafeDeleteArray( pSurfaces );
                return false;
            }

            memcpy( pSurfaces[ idx ].pPixels, pPixels + offset, numBytes );

            if ( depth != 0 )
            { offset += numBytes * d; }
            else
            { offset += numBytes; }

            w = w >> 1;
            h = h >> 1;
            d = d >> 1;

            if ( w == 0 ) { w = 1; }
            if ( h == 0 ) { h = 1; }
            if ( d == 0 ) { d = 1; }
        }
    }

    SafeDeleteArray( pPixels );

    u32 option = ( isCubeMap ) ? RESTEXTURE_OPTION_CUBEMAP : RESTEXTURE_OPTION_NONE;
    if ( isVolume )
    { option |= RESTEXTURE_OPTION_VOLUME; }

    (*pResult).Width         = width;
    (*pResult).Height        = height;
    (*pResult).Depth         = depth;
    (*pResult).Format        = format;
    (*pResult).SurfaceCount  = surfaceCount;
    (*pResult).MipMapCount   = mipMapCount;
    (*pResult).Option        = option;
    (*pResult).pSurfaces     = pSurfaces;

    return true;
}

} // namespace asdx
