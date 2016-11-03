//-------------------------------------------------------------------------------------------------
// File : asdxResTXM.cpp
// Desc : Project Asura Texture Map Data Format (*.txm) Loader
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxLogger.h>
#include <cstdio>
#include "asdxResTXM.h"


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
static constexpr u32 TXM_VERSION = 0x000001;


///////////////////////////////////////////////////////////////////////////////////////////////////
// TXM_FILE_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct TXM_FILE_HEADER
{
    u8      Magic[4];       //!< マジックです. 'T', 'X', 'M'
    u32     Version;        //!< ファイルバージョンです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TXM_SURFACE structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct TXM_SURFACE
{
    u32     Width;          //!< 横幅です.
    u32     Height;         //!< 縦幅です.
    u32     RowPitch;       //!< ローピッチです.
    u32     SlicePitch;     //!< スライスピッチです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TXM_TEXTURE structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct TXM_TEXTURE
{
    u32     Width;          //!< 横幅です.
    u32     Height;         //!< 縦幅です.
    u32     Depth;          //!< 奥行です.
    u32     SurfaceCount;   //!< サーフェイス数です.
    u32     MipMapCount;    //!< ミップマップ数です.
    u32     Format;         //!< フォーマットです.
    u32     Option;         //!< オプションです.
};

} // namespace /* anonymous */


namespace asdx {

//-------------------------------------------------------------------------------------------------
//      TXMファイルから読込します.
//-------------------------------------------------------------------------------------------------
bool LoadResTextureFromTXM( const char16* filename, ResTexture* pResult )
{
    if ( filename == nullptr || pResult == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    FILE* pFile;
    auto err = _wfopen_s( &pFile, filename, L"rb" );
    if ( err != 0 )
    {
        ELOG( "Error : File Open Failed." );
        return false;
    }

    TXM_FILE_HEADER header;
    fread( &header, sizeof(header), 1, pFile );

    if ( header.Magic[0] != 'T' ||
         header.Magic[1] != 'X' ||
         header.Magic[2] != 'M' ||
         header.Magic[3] != '\0' )
    {
        ELOG( "Error : Invalid File." );
        fclose( pFile );
        return false;
    }

    if ( header.Version != TXM_VERSION )
    {
        ELOG( "Error : Invalid File Version." );
        fclose( pFile );
        return false;
    }

    TXM_TEXTURE texture;
    fread( &texture, sizeof(texture), 1, pFile );

    (*pResult).Width        = texture.Width;
    (*pResult).Height       = texture.Height;
    (*pResult).Depth        = texture.Depth;
    (*pResult).SurfaceCount = texture.SurfaceCount;
    (*pResult).MipMapCount  = texture.MipMapCount;
    (*pResult).Format       = texture.Format;
    (*pResult).Option       = texture.Option;
    (*pResult).pSurfaces    = new Surface [texture.SurfaceCount];

    for( u32 i=0; i<texture.SurfaceCount; ++i )
    {
        TXM_SURFACE surface;
        fread( &surface, sizeof(surface), 1, pFile );
        (*pResult).pSurfaces[i].Width      = surface.Width;
        (*pResult).pSurfaces[i].Height     = surface.Height;
        (*pResult).pSurfaces[i].RowPitch   = surface.RowPitch;
        (*pResult).pSurfaces[i].SlicePitch = surface.SlicePitch;
        (*pResult).pSurfaces[i].pPixels    = new u8 [surface.SlicePitch];

        fread( (*pResult).pSurfaces[i].pPixels, 1, surface.SlicePitch, pFile );
    }

    fclose( pFile );
    return true;
}

//-------------------------------------------------------------------------------------------------
//      TXMファイルに保存します.
//-------------------------------------------------------------------------------------------------
bool SaveResTextureToTXM( const char16* filename, const ResTexture* pTexture )
{
    if ( filename == nullptr || pTexture == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    FILE* pFile;
    auto err = _wfopen_s( &pFile, filename, L"wb" );
    if ( err != 0 )
    {
        ELOG( "Error : File Open Failed." );
        return false;
    }

    TXM_FILE_HEADER header;
    header.Magic[0] = 'T';
    header.Magic[1] = 'X';
    header.Magic[2] = 'M';
    header.Magic[3] = '\0';
    header.Version = TXM_VERSION;

    fwrite( &header, sizeof(header), 1, pFile );

    TXM_TEXTURE texture;
    texture.Width        = pTexture->Width;
    texture.Height       = pTexture->Height;
    texture.Depth        = pTexture->Depth;
    texture.SurfaceCount = pTexture->SurfaceCount;
    texture.MipMapCount  = pTexture->MipMapCount;
    texture.Format       = pTexture->Format;
    texture.Option       = pTexture->Option;
    fwrite( &texture, sizeof(texture), 1, pFile );

    for( u32 i=0; i<texture.SurfaceCount; ++i )
    {
        TXM_SURFACE surface;
        surface.Width      = pTexture->pSurfaces[i].Width;
        surface.Height     = pTexture->pSurfaces[i].Height;
        surface.RowPitch   = pTexture->pSurfaces[i].RowPitch;
        surface.SlicePitch = pTexture->pSurfaces[i].SlicePitch;
        fwrite( &surface, sizeof(surface), 1, pFile );
        fwrite( pTexture->pSurfaces[i].pPixels, 1, surface.SlicePitch, pFile );
    }

    fclose( pFile );
    return true;
}

} // namespace asdx
