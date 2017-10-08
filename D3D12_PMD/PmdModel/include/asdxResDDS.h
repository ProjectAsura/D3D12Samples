//-------------------------------------------------------------------------------------------------
// File : asdxResDDS.h
// Desc : Direct Draw Surface Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTypedef.h>
#include <asdxILoadable.h>
#include <asdxIDisposable.h>


namespace asdx {

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

    DDS_FORMAT_B8G8R8A8_UNORM = 87,
    DDS_FORMAT_B8G8R8X8_UNORM = 88,
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

///////////////////////////////////////////////////////////////////////////////////////////////
// Surface structure
///////////////////////////////////////////////////////////////////////////////////////////////
struct Surface
{
    u32     Width;          //!< 横幅です.
    u32     Height;         //!< 高さです.
    u32     Pitch;          //!< ピッチです.
    u32     SlicePitch;     //!< スライスピッチです.
    u8*     pPixels;        //!< ピクセルデータです.

    void Dispose();
    Surface& operator = ( const Surface& value );
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResDDS class
///////////////////////////////////////////////////////////////////////////////////////////////////
class ResDDS : public ILoadable, public IDisposable
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
    ResDDS();

    //---------------------------------------------------------------------------------------------
    //! @brief      コピーコンストラクタです.
    //!
    //! @param[in]      value       コピー元の値です
    //---------------------------------------------------------------------------------------------
    ResDDS( const ResDDS& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual ~ResDDS();

    //---------------------------------------------------------------------------------------------
    //! @brief      ファイルから読み込みを行います.
    //!
    //! @param[in]      filename        ファイル名です.
    //! @retval true    読み込みに成功.
    //! @retval false   読み込みに失敗.
    //---------------------------------------------------------------------------------------------
    bool Load( const char16* filename ) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      メモリを解放します.
    //---------------------------------------------------------------------------------------------
    void Dispose() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      画像の横幅を取得します.
    //!
    //! @return     画像の横幅を返却します.
    //---------------------------------------------------------------------------------------------
    const u32 GetWidth() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      画像の縦幅を取得します.
    //!
    //! @return     画像の縦幅を返却します.
    //---------------------------------------------------------------------------------------------
    const u32 GetHeight() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      画像の奥行きを取得します.
    //!
    //! @return     画像の奥行きを返却します.
    //---------------------------------------------------------------------------------------------
    const u32 GetDepth() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      サーフェイス数を取得します.
    //!
    //! @return     サーフェイス数を返却します.
    //---------------------------------------------------------------------------------------------
    const u32 GetSurfaceCount() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      ミップマップ数を取得します.
    //!
    //! @return     ミップマップ数を返却します.
    //---------------------------------------------------------------------------------------------
    const u32 GetMipMapCount() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      フォーマットを取得します.
    //!
    //! @return     フォーマットを返却します.
    //---------------------------------------------------------------------------------------------
    const u32 GetFormat() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      次元数を取得します.
    //!
    //! @return     次元数を返却します.
    //---------------------------------------------------------------------------------------------
    const DDS_RESOURCE_DIMENSION GetDimension() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      キューブマップかどうかチェックします.
    //!
    //! @return     キューブマップであればtrueを返却します.
    //---------------------------------------------------------------------------------------------
    const bool IsCubeMap() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      サーフェイスを取得します.
    //!
    //! @return     サーフェイスを返却します.
    //---------------------------------------------------------------------------------------------
    const Surface* GetSurfaces() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      代入演算子です.
    //!
    //! @param[in]      value       代入する値です.
    //! @return     代入結果を返却します.
    //---------------------------------------------------------------------------------------------
    ResDDS& operator = ( const ResDDS& value );

    //---------------------------------------------------------------------------------------------
    //! @brief      等価比較演算子です.
    //!
    //! @param[in]      value       比較する値です.
    //! @retval true    等価です.
    //! @retval false   非等価です.
    //---------------------------------------------------------------------------------------------
    bool operator == ( const ResDDS& value ) const;

    //---------------------------------------------------------------------------------------------
    //! @brief      非等価比較演算子です.
    //!
    //! @param[in]      value       比較する値です.
    //! @retval true    非等価です.
    //! @retval false   等価です.
    //---------------------------------------------------------------------------------------------
    bool operator != ( const ResDDS& value ) const;

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
    u32                     m_Width;                //!< 画像の横幅です.
    u32                     m_Height;               //!< 画像の縦幅です.
    u32                     m_Depth;                //!< 画像の奥行きです.
    u32                     m_SurfaceCount;         //!< サーフェイス数です.
    u32                     m_MipMapCount;          //!< ミップマップ数です.
    u32                     m_Format;               //!< フォーマットです.
    DDS_RESOURCE_DIMENSION  m_Dimension;            //!< 次元数です.
    bool                    m_IsCubeMap;            //!< キューブマップかどうか?
    Surface*                m_pSurfaces;            //!< サーフェイスです.
    u32                     m_HashKey;              //!< ハッシュキーです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};


} // namespace asdx



