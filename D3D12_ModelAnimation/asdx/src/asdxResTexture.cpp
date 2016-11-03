//-------------------------------------------------------------------------------------------------
// File : asdxResTexture.cpp
// Desc : Resource Texture Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxResTexture.h>
#include <asdxLogger.h>
#include <asdxMisc.h>
#include "formats/asdxResTGA.h"
#include "formats/asdxResDDS.h"
#include "formats/asdxResHDR.h"
#include "formats/asdxResWIC.h"
#include "formats/asdxResTXM.h"


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// TextureFactory class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      テクスチャリソースを生成します.
//-------------------------------------------------------------------------------------------------
bool TextureFactory::Create( const char16* filename, ResTexture* pResult )
{
    if ( filename == nullptr || pResult == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    auto ext = GetExt( filename );

    if ( ext == L"tga" )
    { return LoadResTextureFromTGA( filename, pResult ); }
    else if ( ext == L"dds" )
    { return LoadResTextureFromDDS( filename, pResult ); }
    else if ( ext == L"hdr" )
    { return LoadResTextureFromHDR( filename, pResult ); }
    else if ( ext == L"txm" )
    { return LoadResTextureFromTXM( filename, pResult ); }
    else if ( ext == L"bmp"  ||
              ext == L"jpg"  ||
              ext == L"jpeg" ||
              ext == L"tif"  ||
              ext == L"tiff" ||
              ext == L"gif"  ||
              ext == L"png"  ||
              ext == L"hdp")
    { return LoadResTextureFromWIC( filename, pResult ); }

    ELOG( "Error : Invalid File Format. Extension is %s", ext.c_str() );
    return false;
}

//-------------------------------------------------------------------------------------------------
//      テクスチャリソースを破棄します.
//-------------------------------------------------------------------------------------------------
void TextureFactory::Dispose( ResTexture*& ptr )
{
    if ( ptr == nullptr )
    { return; }

    for( u32 i=0; i<ptr->SurfaceCount; ++i )
    {
        ptr->pSurfaces[i].Width      = 0;
        ptr->pSurfaces[i].Height     = 0;
        ptr->pSurfaces[i].RowPitch   = 0;
        ptr->pSurfaces[i].SlicePitch = 0;
        SafeDeleteArray( ptr->pSurfaces[i].pPixels );
    }

    SafeDeleteArray( ptr->pSurfaces );
    SafeDelete( ptr );
}

} // namespace asdx

