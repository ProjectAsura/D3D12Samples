//-------------------------------------------------------------------------------------------------
// File : asdxResMAT.cpp
// Desc : Project Asura MaterialSet Data Format (*.mts) Loader
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxLogger.h>
#include "asdxResMAT.h"


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
static constexpr u32 MAT_VERSION = 0x000001;

///////////////////////////////////////////////////////////////////////////////////////////////////
// MAT_FILE_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MAT_FILE_HEADER
{
    u8      Magic[4];       //!< マジックです. 'M', 'A', 'T', '\0'
    u32     Version;        //!< ファイルバージョンです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MAT_FILE_PATH structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MAT_FILE_PATH
{
    char16  Path[256];      //!< パス名です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MAT_MATERIAL structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MAT_MATERIAL
{
    u32     PathCount;      //!< テクスチャ数です.
    u32     PhongCount;     //!< Phong BRDF 数です.
    u32     DisneyCount;    //!< Disney BRDF 数です.
};

} // namespace /* anonymous */


namespace asdx {

//-------------------------------------------------------------------------------------------------
//      MATファイルからマテリアルリソースを読み込みます.
//-------------------------------------------------------------------------------------------------
bool LoadResMaterialFromMAT( const char16* filename, ResMaterial* pResult )
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
        ELOG( "Error : File Open Failed. filename = %s", filename );
        return false;
    }

    MAT_FILE_HEADER header;
    fread( &header, sizeof(header), 1, pFile );

    if ( header.Magic[0] != 'M' ||
         header.Magic[1] != 'A' ||
         header.Magic[2] != 'T' ||
         header.Magic[3] != '\0' )
    {
        ELOG( "Error : Invalid File." );
        fclose( pFile );
        return false;
    }

    if ( header.Version != MAT_VERSION )
    {
        ELOG( "Error : Invalid File Version." );
        fclose( pFile );
        return false;
    }

    MAT_MATERIAL material;
    fread( &material, sizeof(material), 1, pFile );

    (*pResult).Paths .resize( material.PathCount );
    (*pResult).Phong .resize( material.PhongCount );
    (*pResult).Disney.resize( material.DisneyCount );

    for( u32 i=0; i<material.PathCount; ++i )
    {
        MAT_FILE_PATH texture;
        fread( &texture, sizeof(texture), 1, pFile );
        (*pResult).Paths[i] = texture.Path;
    }

    for( u32 i=0; i<material.PhongCount; ++i )
    { fread( &(*pResult).Phong[i], sizeof(ResPhong), 1, pFile ); }

    for( u32 i=0; i<material.DisneyCount; ++i )
    { fread( &(*pResult).Disney[i], sizeof(ResDisney), 1, pFile ); }

    fclose( pFile );
    return true;
}

//-------------------------------------------------------------------------------------------------
//      MATファイルに保存します.
//-------------------------------------------------------------------------------------------------
bool SaveResMaterialToMAT( const char16* filename, const ResMaterial* pMaterial )
{
    if ( filename == nullptr || pMaterial == nullptr )
    {
        ELOG( "Error : Invalid Argument.");
        return false;
    }

    FILE* pFile;
    auto err = _wfopen_s( &pFile, filename, L"wb" );
    if ( err != 0 )
    {
        ELOG( "Error : File Open Failed." );
        return false;
    }

    MAT_FILE_HEADER header;
    header.Magic[0] = 'M';
    header.Magic[1] = 'A';
    header.Magic[2] = 'T';
    header.Magic[3] = '\0';
    header.Version = MAT_VERSION;

    fwrite( &header, sizeof(header), 1, pFile );

    MAT_MATERIAL material = {};
    material.PathCount   = static_cast<u32>( pMaterial->Paths.size() );
    material.PhongCount  = static_cast<u32>( pMaterial->Phong.size() );
    material.DisneyCount = static_cast<u32>( pMaterial->Disney.size() );
    fwrite( &material, sizeof(material), 1, pFile );

    for( u32 i=0; i<material.PathCount; ++i )
    {
        MAT_FILE_PATH texture = {};
        wcscpy_s( texture.Path, pMaterial->Paths[i].c_str() );
        fwrite( &texture, sizeof(texture), 1, pFile );
    }

    for( u32 i=0; i<material.PhongCount; ++i )
    { fwrite( &pMaterial->Phong[i], sizeof(ResPhong), 1, pFile ); }

    for( u32 i=0; i<material.DisneyCount; ++i )
    { fwrite( &pMaterial->Disney[i], sizeof(ResDisney), 1, pFile ); }

    fclose( pFile );
    return true;
}

} // namespace asdx

