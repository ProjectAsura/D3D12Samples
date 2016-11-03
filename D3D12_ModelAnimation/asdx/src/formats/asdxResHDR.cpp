//-------------------------------------------------------------------------------------------------
// File : asdxResHDR.cpp
// Desc : Radiance HDR Format Loader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <new>
#include <cstdio>
#include <cstring>
#include <asdxLogger.h>
#include "asdxResHDR.h"


namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SCANLINE_TYPE
///////////////////////////////////////////////////////////////////////////////////////////////////
enum SCANLINE_TYPE
{
    SCANLINE_NY_PX = 0,     //!< -Y +X
    SCANLINE_NY_NX,         //!< -Y -X
    SCANLINE_PY_PX,         //!< +Y +X
    SCANLINE_PY_NX,         //!< +Y -X
    SCANLINE_NX_PY,         //!< -X +Y
    SCANLINE_NX_NY,         //!< -X -Y
    SCANLINE_PX_PY,         //!< +X +Y
    SCANLINE_PX_NY,         //!< +X -Y
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// RGBE structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct RGBE
{
    union
    {
        struct { u8 r, g, b, e; };
        u8 v[4];
    };
};

//-------------------------------------------------------------------------------------------------
//      CRLFを取り除きます.
//-------------------------------------------------------------------------------------------------
void RemoveEndline( char* pBuf )
{
    for( auto i = static_cast<int>(strlen(pBuf)) -1; i >= 0; i-- )
    {
        if ( pBuf[ i ] != '\r' && pBuf[ i ] != '\n')
            break;

        pBuf[ i ] = '\0';
    }
}

//-------------------------------------------------------------------------------------------------
//      旧形式のカラーを読み取ります.
//-------------------------------------------------------------------------------------------------
bool ReadOldColors( FILE* pFile, RGBE* pLine, s32 count )
{
    auto shift = 0;
    while( 0 < count )
    {
        pLine[0].r = static_cast<u8>(getc( pFile ));
        pLine[0].g = static_cast<u8>(getc( pFile ));
        pLine[0].b = static_cast<u8>(getc( pFile ));
        pLine[0].e = static_cast<u8>(getc( pFile ));

        if ( feof( pFile ) || ferror( pFile ) )
            return false;

        if ( pLine[0].r == 1
          && pLine[0].g == 1
          && pLine[0].b == 1 )
        {
            for( auto i=pLine[0].e << shift; i > 0; i-- )
            {
                pLine[0].r = pLine[-1].r;
                pLine[0].g = pLine[-1].g;
                pLine[0].b = pLine[-1].b;
                pLine[0].e = pLine[-1].e;
                pLine++;
                count--;
            }
            shift += 8;
        }
        else
        {
            pLine++;
            count--;
            shift = 0;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      カラーを読み取ります.
//-------------------------------------------------------------------------------------------------
bool ReadColor( FILE* pFile, RGBE* pLine, s32 count )
{
    if ( count < 8 || 0x7fff < count )
    { return ReadOldColors( pFile, pLine, count ); }

    auto j = 0;
    auto i = static_cast<u8>(getc( pFile ));
    if ( i == EOF )
        return false;

    if ( i != 2 )
    {
        ungetc( i, pFile );
        return ReadOldColors( pFile, pLine, count );
    }

    pLine[0].g = static_cast<u8>(getc( pFile ));
    pLine[0].b = static_cast<u8>(getc( pFile ));

    if ( ( i = static_cast<u8>(getc( pFile )) ) == EOF )
        return false;

    if ( pLine[0].g != 2 || pLine[0].b & 128 )
    {
        pLine[0].r = 2;
        pLine[0].e = i;
        return ReadOldColors( pFile, pLine + 1, count -1 );
    }

    if ( ( pLine[0].b << 8 | i ) != count )
        return false;

    for( i=0; i<4; ++i )
    {
        for( j=0; j<count; )
        {
            auto code = getc( pFile );
            if ( code == EOF )
                return false;

            if ( 128 < code )
            {
                code &= 127;
                auto val = static_cast<u8>(getc( pFile ));
                while( code-- )
                { 
                    pLine[j++].v[i] = val;
                }
            }
            else
            {
                while( code-- )
                { 
                    pLine[j++].v[i] = static_cast<u8>(getc( pFile ));
                }
            }
        }
    }

    return ( feof( pFile ) ? false : true );
}

} // namespace /* anonymous */

namespace asdx {

//-------------------------------------------------------------------------------------------------
//      HDRからリソーステクスチャを読込します.
//-------------------------------------------------------------------------------------------------
bool LoadResTextureFromHDR( const char16* filename, ResTexture* pResult )
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

    const u32 BUFFER_SIZE = 256;
    char buf[ BUFFER_SIZE ];
    fgets( buf, BUFFER_SIZE, pFile );
    RemoveEndline( buf );

    // マジックをチェック.
    if ( strcmp( buf, "#?RADIANCE") != 0 )
    {
        ELOG( "Error : Invalid File." );
        fclose( pFile );
        return false;
    }

    u32 scanlineType = 0;
    u32 width    = 0;
    u32 height   = 0;
    f32 exposure = 1.0f;
    f32 gamma    = 1.0f;

    while( feof(pFile) != 0 )
    {
        if ( feof( pFile ) )
        {
             fclose( pFile );
             ELOG( "Error : End Of File.");
             return false;
        }

        if ( fgets( buf, BUFFER_SIZE, pFile ) == nullptr )
        {
            fclose( pFile );
            return false;
        }

        // CRLFを削除.
        RemoveEndline( buf );

        if ( buf[0] == '#' )
        { 
            continue;
        }
        else if ( buf[0] == 'F' )
        {
            char format[ BUFFER_SIZE ];
            sscanf_s( buf, "FORMAT=%s", format, static_cast<int>(_countof(format)) );

            if ( strcmp( format, "32-bit_rle_rgbe" ) != 0
              && strcmp( format, "32-bit_rle_xyze" ) != 0 )
            {
                fclose( pFile );
                ELOG( "Error : Invalid Format." );
                return false;
            }
        }
        else if ( buf[0] == 'E' )
        {
            sscanf_s( buf, "EXPOSURE=%f", &exposure );
        }
        else if ( buf[0] == 'C' )
        {
            /* COLORCORR は非サポート */
        }
        else if ( buf[0] == 'S' )
        {
            ILOG( "Info : Software = %s", buf );
        }
        else if ( buf[0] == 'P' && buf[1] == 'I' )
        {
            /* PIXASPECT は非サポート */
        }
        else if ( buf[0] == 'V' )
        {
            /* VIEW は非サポート */
        }
        else if ( buf[0] == 'P' && buf[1] == 'R' )
        {
            /* PRIMARIES は非サポート */
        }
        else if ( buf[0] == 'G' )
        {
            sscanf_s( buf, "GAMMA=%f", &gamma );
        }
        else if ( buf[0] == '-' && buf[1] == 'Y' )
        {
            // Y m X n 形式なので，n行のデータがm列分ある　普通のテクスチャ形式.
            char sig;
            sscanf_s( buf, "-Y %d %cX %d", &height, &sig, 1, &width );
            if ( sig == '+')
            {
                // テクスチャのデータがYは下に進んで，Xは右に進む.
                scanlineType = SCANLINE_NY_PX;
            }
            else if ( sig == '-')
            {
                // テクスチャのデータがYは下に進んで，Xは左に進む.
                scanlineType = SCANLINE_NY_NX;
            }
            break;
        }
        else if ( buf[0] == '+' && buf[1] == 'Y' )
        {
            // Y m X n 形式なので，n行のデータがm列分ある　普通のテクスチャ形式.
            char sig;
            sscanf_s( buf, "+Y %d %cX %d", &height, &sig, 1, &width );
            if ( sig == '+' )
            {
                // テクスチャのデータがYは上に進んで，Xは右に進む.
                scanlineType = SCANLINE_PY_PX;
            }
            else if ( sig == '-' )
            {
                // テクスチャのデータがYは上に進んで，Xは左に進む.
                scanlineType = SCANLINE_PY_NX;
            }
            break;
        }
        else if ( buf[0] == '-' && buf[1] == 'X' )
        {
            // X n Y m 形式なので，m列のデータがn行分ある. 90度回転したようなデータ.
            char sig;
            sscanf_s( buf, "-X %d %cY %d", &width, &sig, 1, &height );
            if ( sig == '+' )
            {
                // テクスチャのデータがXは左に進んで，Yは上に進む.
                scanlineType = SCANLINE_NX_PY;
            }
            else if ( sig == '-' )
            {
                // テクスチャのデータがXは左に進んで，Yは下に進む.
                scanlineType = SCANLINE_NX_NY;
            }
            break;
        }
        else if ( buf[0] == '+' && buf[1] == 'X' )
        {
            // X n Y m 形式なので，m列のデータがn行分ある. 90度回転したようなデータ.

            char sig;
            sscanf_s( buf, "+X %d %cY %d", &width, &sig, 1, &height );
            if ( sig == '+' )
            {
                // テクスチャのデータがXは右に進んで，Yは上に進む.
                scanlineType = SCANLINE_PX_PY;
            }
            else if ( sig == '-' )
            {
                // テクスチャのデータがXは右に進んで，Yは下に進む.
                scanlineType = SCANLINE_PX_NY;
            }
            break;
        }
    }

    if ( scanlineType != SCANLINE_NY_PX &&
         scanlineType != SCANLINE_PY_PX )
    {
        ELOG( "Error : Unsupported Scanline Format" );
        fclose( pFile );
        return false;
    }

    // メモリを確保.
    auto pixels = new (std::nothrow) RGBE [ width * height ];
    if ( pixels == nullptr )
    {
        ELOG( "Error : Out of Memory.");
        fclose( pFile );
        SafeDeleteArray( pixels );
        return false;
    }

    if ( scanlineType == SCANLINE_NY_PX )
    {
        // Direct3Dのテクスチャ座標系に合わせるので，Y方向は逆から読み込み.
        for( int y = (s32)height - 1; y >= 0; y-- )
        {
            if ( !ReadColor( pFile, &pixels[y * width], width ) )
            {
                fclose( pFile );
                SafeDeleteArray( pixels );
                return false;
            }
        }
    }
    else if ( scanlineType == SCANLINE_PY_PX )
    {
        // Direct3Dのテクスチャ座標系に合わせるので，Y方向は逆から読み込み.
        for( int y = 0; y < (s32)height; y++ )
        {
            if ( !ReadColor( pFile, &pixels[y * width], width ) )
            {
                fclose( pFile );
                SafeDeleteArray( pixels );
                return false;
            }
        }
    }

    fclose( pFile );

    auto surface = new(std::nothrow) Surface();
    if ( surface == nullptr )
    {
        SafeDeleteArray(pixels);
        return false;
    }

    surface->Width      = width;
    surface->Height     = height;
    surface->RowPitch   = width * sizeof(f32) * 3;
    surface->SlicePitch = surface->RowPitch * height;
    surface->pPixels    = reinterpret_cast<u8*>(pixels);

    (*pResult).Width        = width;
    (*pResult).Height       = height;
    (*pResult).Depth        = 0;
    (*pResult).MipMapCount  = 1;
    (*pResult).SurfaceCount = 1;
    (*pResult).Format       = 2;  // DXGI_FORMAT_R32G32B32A32_FLOAT = 2
    (*pResult).Option       = RESTEXTURE_OPTION_NONE;
    (*pResult).pSurfaces    = surface;
 
    return true;
}


} // namespace asdx
