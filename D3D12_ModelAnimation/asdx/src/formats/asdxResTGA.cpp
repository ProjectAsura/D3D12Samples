//-------------------------------------------------------------------------------------------------
// File : asdxResTGA.cpp
// Desc : Targa Texture Loader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <new>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <dxgiformat.h>
#include <asdxLogger.h>
#include "asdxResTGA.h"


namespace /* anonymous */ {

////////////////////////////////////////////////////////////////////////////////////////////////////
// TGA_FORMA_TYPE enum
////////////////////////////////////////////////////////////////////////////////////////////////////
enum TGA_FORMAT_TYPE
{
    TGA_FORMAT_NONE             = 0,        //!< イメージなし.
    TGA_FORMAT_INDEXCOLOR       = 1,        //!< インデックスカラー(256色).
    TGA_FORMAT_FULLCOLOR        = 2,        //!< フルカラー
    TGA_FORMAT_GRAYSCALE        = 3,        //!< 白黒.
    TGA_FORMAT_RLE_INDEXCOLOR   = 9,        //!< RLE圧縮インデックスカラー.
    TGA_FORMAT_RLE_FULLCOLOR    = 10,       //!< RLE圧縮フルカラー.
    TGA_FORMAT_RLE_GRAYSCALE    = 11,       //!< RLE圧縮白黒.
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// TGA_HEADER structure
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack( push, 1 )
struct TGA_HEADER
{
    u8  IdFieldLength;      // IDフィードのサイズ(範囲は0～255).
    u8  HasColorMap;        // カラーマップ有無(0=なし, 1=あり)
    u8  Format;             // 画像形式.
    u16 ColorMapEntry;      // カラーマップエントリー.
    u16 ColorMapLength;     // カラーマップのエントリーの総数.
    u8  ColorMapEntrySize;  // カラーマップの1エントリー当たりのビット数.
    u16 OffsetX;            // 画像のX座標.
    u16 OffsetY;            // 画像のY座標.
    u16 Width;              // 画像の横幅.
    u16 Height;             // 画像の縦幅.
    u8  BitPerPixel;        // ビットの深さ.
    u8  ImageDescriptor;    // (0~3bit) : 属性, 4bit : 格納方向(0=左から右,1=右から左), 5bit : 格納方向(0=下から上, 1=上から下), 6~7bit : インタリーブ(使用不可).
};
#pragma pack( pop )


////////////////////////////////////////////////////////////////////////////////////////////////////
// TGA_FOOTER structure
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack( push, 1 )
struct TGA_FOOTER
{
    u32  OffsetExt;      // 拡張データへのオフセット(byte数) [オフセットはファイルの先頭から].
    u32  OffsetDev;      // ディベロッパーエリアへのオフセット(byte数)[オフセットはファイルの先頭から].
    char Tag[18];        // 'TRUEVISION-XFILE.\0'
};
#pragma pack( pop )


///////////////////////////////////////////////////////////////////////////////////////////////////
// TGA_EXTENSION structure
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack( push, 1 )
struct TGA_EXTENSION
{
    u16     Size;                       //!< サイズ.
    char    AuthorName[ 41 ];           //!< 著作者名.
    char    AuthorComment[ 324 ];       //!< 著作者コメント.
    u16     StampMonth;                 //!< タイムスタンプ　月(1-12).
    u16     StampDay;                   //!< タイムスタンプ　日(1-31).
    u16     StampYear;                  //!< タイムスタンプ　年(4桁, 例1989).
    u16     StampHour;                  //!< タイムスタンプ　時(0-23).
    u16     StampMinute;                //!< タイムスタンプ　分(0-59).
    u16     StampSecond;                //!< タイムスタンプ　秒(0-59).
    char    JobName[ 41 ];              //!< ジョブ名 (最後のバイトはゼロが必須).
    u16     JobHour;                    //!< ジョブ時間  時(0-65535)
    u16     JobMinute;                  //!< ジョブ時間　分(0-59)
    u16     JobSecond;                  //!< ジョブ時間　秒(0-59)
    char    SoftwareId[ 41 ];           //!< ソフトウェアID (最後のバイトはゼロが必須).
    u16     VersionNumber;              //!< ソフトウェアバージョン    VersionNumber * 100になる.
    u8      VersionLetter;              //!< ソフトウェアバージョン
    u32     KeyColor;                   //!< キーカラー.
    u16     PixelNumerator;             //!< ピクセル比分子　ピクセル横幅.
    u16     PixelDenominator;           //!< ピクセル比分母　ピクセル縦幅.
    u16     GammaNumerator;             //!< ガンマ値分子.
    u16     GammaDenominator;           //!< ガンマ値分母
    u32     ColorCorrectionOffset;      //!< 色補正テーブルへのオフセット.
    u32     StampOffset;                //!< ポステージスタンプ画像へのオフセット.
    u32     ScanLineOffset;             //!< スキャンラインオフセット.
    u8      AttributeType;              //!< アルファチャンネルデータのタイプ
};
#pragma pack( pop )


//-------------------------------------------------------------------------------------------------
//! @brief      8Bitインデックスカラー形式を解析します.
//!
//! @param[in]      pColorMap       カラーマップです.
//-------------------------------------------------------------------------------------------------
void Parse8Bits( FILE* pFile, u32 size, u8* pColorMap, u8* pPixels )
{
    u8 color = 0;
    for( u32 i=0; i<size; ++i )
    {
        color = (u8)fgetc( pFile );
        pPixels[ i * 4 + 2 ] = pColorMap[ color * 3 + 0 ];
        pPixels[ i * 4 + 1 ] = pColorMap[ color * 3 + 1 ];
        pPixels[ i * 4 + 0 ] = pColorMap[ color * 3 + 2 ];
        pPixels[ i * 4 + 3 ] = 255;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      16Bitフルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse16Bits( FILE* pFile, u32 size, u8* pPixels )
{
    for( u32 i=0; i<size; ++i )
    {
        u16 color = static_cast<u16>(fgetc( pFile ) + ( fgetc( pFile ) << 8 ));
        pPixels[ i * 4 + 0 ] = (u8)(( ( color & 0x7C00 ) >> 10 ) << 3);
        pPixels[ i * 4 + 1 ] = (u8)(( ( color & 0x03E0 ) >>  5 ) << 3);
        pPixels[ i * 4 + 2 ] = (u8)(( ( color & 0x001F ) >>  0 ) << 3);
        pPixels[ i * 4 + 3 ] = 255;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      24Bitフルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse24Bits( FILE* pFile, u32 size, u8* pPixels )
{
    for( u32 i=0; i<size; ++i )
    {
        pPixels[ i * 4 + 2 ] = (u8)fgetc( pFile );
        pPixels[ i * 4 + 1 ] = (u8)fgetc( pFile );
        pPixels[ i * 4 + 0 ] = (u8)fgetc( pFile );
        pPixels[ i * 4 + 3 ] = 255;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      32Bitフルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse32Bits( FILE* pFile, u32 size, u8* pPixels )
{
    for( u32 i=0; i<size; ++i )
    {
        pPixels[ i * 4 + 2 ] = (u8)fgetc( pFile );
        pPixels[ i * 4 + 1 ] = (u8)fgetc( pFile );
        pPixels[ i * 4 + 0 ] = (u8)fgetc( pFile );
        pPixels[ i * 4 + 3 ] = (u8)fgetc( pFile );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief     8Bitグレースケール形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse8BitsGrayScale( FILE* pFile, u32 size, u8* pPixels )
{
    for( u32 i=0; i<size; ++i )
    {
        pPixels[ i ] = (u8)fgetc( pFile );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      16Bitグレースケール形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse16BitsGrayScale( FILE* pFile, u32 size, u8* pPixels )
{
    for( u32 i=0; i<size; ++i )
    {
        u8 gray  = (u8)fgetc( pFile );
        u8 alpha = (u8)fgetc( pFile );
        pPixels[ i * 4 + 0 ] = gray;
        pPixels[ i * 4 + 1 ] = gray;
        pPixels[ i * 4 + 2 ] = gray;
        pPixels[ i * 4 + 3 ] = alpha; 
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      8BitRLE圧縮インデックスカラー形式を解析します.
//!
//! @param[in]  pColorMap       カラーマップです.
//-------------------------------------------------------------------------------------------------
void Parse8BitsRLE( FILE* pFile, u8* pColorMap, u32 size, u8* pPixels )
{
    u32 count  = 0;
    u8  color  = 0;
    u8  header = 0;
    u8* ptr    = pPixels;

    while( ptr < pPixels + size )   // size = width * height * 3.
    {
        header = (u8)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            color = (u8)fgetc( pFile );

            for( u32 i=0; i<count; ++i, ptr+=4 )
            {
                ptr[ 0 ] = pColorMap[ color * 3 + 2 ];
                ptr[ 1 ] = pColorMap[ color * 3 + 1 ];
                ptr[ 2 ] = pColorMap[ color * 3 + 0 ];
                ptr[ 3 ] = 255;
            }
        }
        else
        {
            for( u32 i=0; i<count; ++i, ptr+=4 )
            {
                color = (u8)fgetc( pFile );

                ptr[ 0 ] = pColorMap[ color * 3 + 2 ];
                ptr[ 1 ] = pColorMap[ color * 3 + 1 ];
                ptr[ 2 ] = pColorMap[ color * 3 + 0 ];
                ptr[ 3 ] = 255;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      16BitRLE圧縮フルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse16BitsRLE( FILE* pFile, u32 size, u8* pPixels )
{
    u32 count  = 0;
    u16 color  = 0;
    u8  header = 0;
    u8* ptr    = pPixels;

    while( ptr < pPixels + size )   // size = width * height * 3.
    {
        header = (u8)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            color = static_cast<u16>(fgetc( pFile ) + ( fgetc( pFile ) << 8 )); 

            for( u32 i=0; i<count; ++i, ptr+=4 )
            {
                ptr[ 0 ] = (u8)(( ( color & 0x7C00 ) >> 10 ) << 3);
                ptr[ 1 ] = (u8)(( ( color & 0x03E0 ) >>  5 ) << 3);
                ptr[ 2 ] = (u8)(( ( color & 0x001F ) >>  0 ) << 3);
                ptr[ 3 ] = 255;
            }
        }
        else
        {
            for( u32 i=0; i<count; ++i, ptr+=4 )
            {
                color = static_cast<u16>(fgetc( pFile ) + ( fgetc( pFile ) << 8 ));

                ptr[ 0 ] = (u8)(( ( color & 0x7C00 ) >> 10 ) << 3);
                ptr[ 1 ] = (u8)(( ( color & 0x03E0 ) >>  5 ) << 3);
                ptr[ 2 ] = (u8)(( ( color & 0x001F ) >>  0 ) << 3);
                ptr[ 3 ] = 255;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      24BitRLE圧縮フルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse24BitsRLE( FILE* pFile, u32 size, u8* pPixels )
{
    u32 count    = 0;
    u8  color[3] = { 0, 0, 0 };
    u8  header   = 0;
    u8* ptr      = pPixels;

    while( ptr < pPixels + size )   // size = width * height * 3.
    {
        header = (u8)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            fread( color, sizeof(u8), 3, pFile );

            for( u32 i=0; i<count; ++i, ptr+=4 )
            {
                ptr[ 0 ] = color[ 2 ];
                ptr[ 1 ] = color[ 1 ];
                ptr[ 2 ] = color[ 0 ];
                ptr[ 3 ] = 255;
            }
        }
        else
        {
            for( u32 i=0; i<count; ++i, ptr+=4 )
            {
                ptr[ 2 ] = (u8)fgetc( pFile );
                ptr[ 1 ] = (u8)fgetc( pFile );
                ptr[ 0 ] = (u8)fgetc( pFile );
                ptr[ 3 ] = 255;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      32BitRLE圧縮フルカラー形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse32BitsRLE( FILE* pFile, u32 size, u8* pPixels )
{
    u32 count    = 0;
    u8  color[4] = { 0, 0, 0, 0 };
    u8  header   = 0;
    u8* ptr      = pPixels;

    while( ptr < pPixels + size )   // size = width * height * 4.
    {
        header = (u8)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            fread( color, sizeof(u8), 4, pFile );

            for( u32 i=0; i<count; ++i, ptr+=4 )
            {
                ptr[ 0 ] = color[ 2 ];
                ptr[ 1 ] = color[ 1 ];
                ptr[ 2 ] = color[ 0 ];
                ptr[ 3 ] = color[ 3 ];
            }
        }
        else
        {
            for( u32 i=0; i<count; ++i, ptr+=4 )
            {
                ptr[ 2 ] = (u8)fgetc( pFile );
                ptr[ 1 ] = (u8)fgetc( pFile );
                ptr[ 0 ] = (u8)fgetc( pFile );
                ptr[ 3 ] = (u8)fgetc( pFile );
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      8BitRLE圧縮グレースケール形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse8BitsGrayScaleRLE( FILE* pFile, u32 size, u8* pPixles )
{
    u32 count  = 0;
    u8  color  = 0;
    u8  header = 0;
    u8* ptr    = pPixles;

    while( ptr < pPixles + size ) // size = width * height
    {
        header = (u8)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            color = (u8)fgetc( pFile );

            for( u32 i=0; i<count; ++i, ptr++ )
            { (*ptr) = color; }
        }
        else
        {
            for( u32 i=0; i<count; ++i, ptr++ )
            { (*ptr) = (u8)fgetc( pFile ); }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      16BitRLE圧縮グレースケール形式を解析します.
//-------------------------------------------------------------------------------------------------
void Parse16BitsGrayScaleRLE( FILE* pFile, u32 size, u8* pPixles )
{
    u32 count  = 0;
    u8  color  = 0;
    u8  alpha  = 0;
    u8  header = 0;
    u8* ptr    = pPixles;

    while( ptr < pPixles + size ) // size = width * height * 2
    {
        header = (u8)fgetc( pFile );
        count = 1 + ( header & 0x7F );

        if ( header & 0x80 )
        {
            color = (u8)fgetc( pFile );
            alpha = (u8)fgetc( pFile );

            for( u32 i=0; i<count; ++i, ptr+=4 )
            {
                ptr[ 0 ] = color;
                ptr[ 1 ] = color;
                ptr[ 2 ] = color;
                ptr[ 3 ] = alpha;
            }
        }
        else
        {
            for( u32 i=0; i<count; ++i, ptr+=4 )
            {
                color = (u8)fgetc( pFile );
                alpha = (u8)fgetc( pFile );
                ptr[ 0 ] = color;
                ptr[ 1 ] = color;
                ptr[ 2 ] = color;
                ptr[ 3 ] = alpha;
            }
        }
    }
}

} // namespace /* anonymous */

namespace asdx {

//-------------------------------------------------------------------------------------------------
//      TGAファイルからリソーステクスチャを生成します.
//-------------------------------------------------------------------------------------------------
bool LoadResTextureFromTGA( const char16* filename, ResTexture* pResult )
{
        // 引数チェック.
    if ( filename == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    FILE* pFile;

    // ファイルを開く.
    auto err = _wfopen_s( &pFile, filename, L"rb" );
    if ( err != 0 )
    {
        ELOG( "Error : File Open Failed." );
        return false;
    }

    // フッターを読み込み.
    TGA_FOOTER footer;
    long offset = sizeof(footer);
    fseek( pFile, -offset, SEEK_END );
    fread( &footer, sizeof(footer), 1, pFile );

    // ファイルマジックをチェック.
    if ( strcmp( footer.Tag, "TRUEVISION-XFILE." ) != 0 &&
         strcmp( footer.Tag, "TRUEVISION-TARGA." ) != 0 )
    {
        ELOG( "Error : Invalid File Format." );
        fclose( pFile );
        return false;
    }

    // 拡張データがある場合は読み込み.
    if ( footer.OffsetExt != 0 )
    {
        TGA_EXTENSION extension;

        fseek( pFile, footer.OffsetExt, SEEK_SET );
        fread( &extension, sizeof(extension), 1, pFile );
    }

    // ディベロッパーエリアがある場合.
    if ( footer.OffsetDev != 0 )
    {
        /* NOT IMPLEMENT */
    }

    // ファイル先頭に戻す.
    fseek( pFile, 0, SEEK_SET );

    // ヘッダデータを読み込む.
    TGA_HEADER header;
    fread( &header, sizeof(header), 1, pFile );

    // フォーマット判定.
    u32 bytePerPixel;
    switch( header.Format )
    {
    // 該当なし.
    case TGA_FORMAT_NONE:
        {
            ELOG( "Error : Invalid Format." );
            fclose( pFile );
            return false;
        }
        break;

    // グレースケール
    case TGA_FORMAT_GRAYSCALE:
    case TGA_FORMAT_RLE_GRAYSCALE:
        { 
            if ( header.BitPerPixel == 8 )
            { bytePerPixel = 1; }
            else
            {
            #if 0
                //bytePerPixel = 2;
            #endif
                bytePerPixel = 4;   // R8L8フォーマットが使えないためR8G8B8A8に変更.
            }
        }
        break;

    // カラー.
    case TGA_FORMAT_INDEXCOLOR:
    case TGA_FORMAT_FULLCOLOR:
    case TGA_FORMAT_RLE_INDEXCOLOR:
    case TGA_FORMAT_RLE_FULLCOLOR:
        {
        #if 0
            //if ( header.BitPerPixel <= 24 )
            //{ bytePerPixel = 3; }
            //else
            //{ bytePerPixel = 4; }
        #endif
            bytePerPixel = 4;   // R8G8B8が使えないためR8G8B8A8に変更.
        }
        break;

    // 上記以外.
    default:
        {
            ELOG( "Error : Unsupported Format." );
            fclose( pFile );
            return false;
        }
        break;
    }

    // IDフィールドサイズ分だけオフセットを移動させる.
    fseek( pFile, header.IdFieldLength, SEEK_CUR );

    Surface* pSurface = new (std::nothrow) Surface();
    if ( pSurface == nullptr )
    {
        fclose( pFile );
        return false;
    }

    // ピクセルサイズを決定.
    auto rowPitch   = header.Width * bytePerPixel;
    auto slicePitch = rowPitch * header.Height;

    pSurface->Width      = header.Width;
    pSurface->Height     = header.Height;
    pSurface->RowPitch   = rowPitch;
    pSurface->SlicePitch = slicePitch;
    pSurface->pPixels    = new (std::nothrow) u8 [ slicePitch ];
    if ( pSurface->pPixels == nullptr )
    {
        ELOG( "Error : Out Of Memory." );
        fclose( pFile );
        SafeDelete( pSurface );
        return false;
    }

    // カラーマップを持つかチェック.
    u8* pColorMap = nullptr;
    if ( header.HasColorMap )
    {
        // カラーマップサイズを算出.
        u32 colorMapSize = header.ColorMapEntry * ( header.ColorMapEntrySize >> 3 );

        // メモリを確保.
        pColorMap = new (std::nothrow) u8 [ colorMapSize ];
        if ( pColorMap == nullptr )
        {
            ELOG( "Error : Out Of Memory." );
            SafeDeleteArray( pSurface->pPixels );
            SafeDelete( pSurface );
            fclose( pFile );
            return false;
        }

        // がばっと読み込む.
        fread( pColorMap, sizeof(u8), colorMapSize, pFile );
    }

    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

    // フォーマットに合わせてピクセルデータを解析する.
    switch( header.Format )
    {
    // パレット.
    case TGA_FORMAT_INDEXCOLOR:
        { 
            Parse8Bits( pFile, pSurface->Width * pSurface->Height, pColorMap, pSurface->pPixels );
            format = DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        break;

    // フルカラー.
    case TGA_FORMAT_FULLCOLOR:
        {
            switch( header.BitPerPixel )
            {
            case 16:
                { 
                    Parse16Bits( pFile, pSurface->Width * pSurface->Height, pSurface->pPixels );
                    format = DXGI_FORMAT_R8G8B8A8_UNORM;
                }
                break;

            case 24:
                {
                    Parse24Bits( pFile, pSurface->Width * pSurface->Height, pSurface->pPixels );
                    format = DXGI_FORMAT_R8G8B8A8_UNORM;
                }
                break;

            case 32:
                {
                    Parse32Bits( pFile, pSurface->Width * pSurface->Height, pSurface->pPixels );
                    format = DXGI_FORMAT_R8G8B8A8_UNORM;
                }
                break;
            }
        }
        break;

    // グレースケール.
    case TGA_FORMAT_GRAYSCALE:
        {
            if ( header.BitPerPixel == 8 )
            { 
                Parse8BitsGrayScale( pFile, pSurface->Width * pSurface->Height, pSurface->pPixels );
                format = DXGI_FORMAT_R8_UNORM;
            }
            else
            { 
                Parse16BitsGrayScale( pFile, pSurface->Width * pSurface->Height, pSurface->pPixels );
                format = DXGI_FORMAT_R8G8B8A8_UNORM;
            }
        }
        break;

    // パレットRLE圧縮.
    case TGA_FORMAT_RLE_INDEXCOLOR:
        { 
            Parse8BitsRLE( pFile, pColorMap, pSurface->Width * pSurface->Height * 3, pSurface->pPixels );
            format = DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        break;

    // フルカラーRLE圧縮.
    case TGA_FORMAT_RLE_FULLCOLOR:
        {
            switch( header.BitPerPixel )
            {
            case 16:
                {
                    Parse16BitsRLE( pFile, pSurface->Width * pSurface->Height * 3, pSurface->pPixels );
                    format = DXGI_FORMAT_R8G8B8A8_UNORM;
                }
                break;

            case 24:
                {
                    Parse24BitsRLE( pFile, pSurface->Width * pSurface->Height * 3, pSurface->pPixels );
                    format = DXGI_FORMAT_R8G8B8A8_UNORM;
                }
                break;

            case 32:
                { 
                    Parse32BitsRLE( pFile, pSurface->Width * pSurface->Height * 4, pSurface->pPixels ); 
                    format = DXGI_FORMAT_R8G8B8A8_UNORM;
                }
                break;
            }
        }
        break;

    // グレースケールRLE圧縮.
    case TGA_FORMAT_RLE_GRAYSCALE:
        {
            if ( header.BitPerPixel == 8 )
            { 
                Parse8BitsGrayScaleRLE( pFile, pSurface->Width * pSurface->Height, pSurface->pPixels );
                format = DXGI_FORMAT_R8_UNORM;
            }
            else
            {
                Parse16BitsGrayScaleRLE( pFile, pSurface->Width * pSurface->Height * 2, pSurface->pPixels ); 
                format = DXGI_FORMAT_R8G8B8A8_UNORM;
            }
        }
        break;
    }

    // 不要なメモリを解放.
    SafeDeleteArray( pColorMap );

    // ファイルを閉じる.
    fclose( pFile );

    assert( format != DXGI_FORMAT_UNKNOWN );

    // リソーステクスチャを設定.
    (*pResult).Width        = pSurface->Width;
    (*pResult).Height       = pSurface->Height;
    (*pResult).Depth        = 0;
    (*pResult).Format       = u32( format );
    (*pResult).MipMapCount  = 1;
    (*pResult).SurfaceCount = 1;
    (*pResult).Option       = RESTEXTURE_OPTION_NONE;
    (*pResult).pSurfaces    = pSurface;

    // 正常終了.
    return true;
}


} // namespace asdx

