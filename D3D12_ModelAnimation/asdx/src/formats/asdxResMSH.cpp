//-------------------------------------------------------------------------------------------------
// File : asdxResMSH.h
// Desc : Project Mesh Data Format (*.msh) Loader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxLogger.h>
#include "asdxResMSH.h"


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
static constexpr u32 MSH_VERSION = 0x000003;


///////////////////////////////////////////////////////////////////////////////////////////////////
// MSH_FILE_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MSH_FILE_HEADER
{
    u8      Magic[4];   //!< マジックです. 'M', 'S', 'H', '\0'
    u32     Version;    //!< ファイルバージョンです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MSH_BONE structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MSH_BONE
{
    char16          Name[32];       //!< ボーン名です.
    u32             ParentId;       //!< 親ボーンのIDです.
    asdx::Vector3   Position;       //!< ボーンの位置座標です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MSH_SUBSET structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MSH_SUBSET
{
    u32     MaterialId;     //!< マテリアルIDです.
    u32     Offset;         //!< 頂点バッファ先頭からのオフセット.
    u32     Count;          //!< 描画インデックス数.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MSH_MESH structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MSH_MESH
{
    u32     PositionCount;      //!< 位置座標数です.
    u32     NormalCount;        //!< 法線ベクトル数です.
    u32     TexCoordCount;      //!< テクスチャ座標数です.
    u32     BoneIndexCount;     //!< ボーン番号数です.
    u32     BoneWeightCount;    //!< ボーン重み数です.
    u32     VertexIndexCount;   //!< 頂点インデックス数です.
    u32     SubsetCount;        //!< サブセット数です.
    u32     BoneCount;          //!< ボーン数です.
};

} // namespace /* anonymous */


namespace asdx {

//-------------------------------------------------------------------------------------------------
//      MSHファイルから読込を行います.
//-------------------------------------------------------------------------------------------------
bool LoadResMeshFromMSH( const char16* filename, ResMesh* pResult )
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

    MSH_FILE_HEADER header;
    fread( &header, sizeof(header), 1, pFile );

    if ( header.Magic[0] != 'M' ||
         header.Magic[1] != 'S' ||
         header.Magic[2] != 'H' ||
         header.Magic[3] != '\0' )
    {
        ELOG( "Error : Invalid File." );
        fclose( pFile );
        return false;
    }

    if ( header.Version != MSH_VERSION )
    {
        ELOG( "Error : Invalid File Version." );
        fclose( pFile );
        return false;
    }

    MSH_MESH mesh;
    fread( &mesh, sizeof(mesh), 1, pFile );

    (*pResult).Positions    .resize( mesh.PositionCount );
    (*pResult).Normals      .resize( mesh.NormalCount );
    (*pResult).TexCoords    .resize( mesh.TexCoordCount );
    (*pResult).BoneIndices  .resize( mesh.BoneIndexCount );
    (*pResult).BoneWeights  .resize( mesh.BoneWeightCount );
    (*pResult).VertexIndices.resize( mesh.VertexIndexCount );
    (*pResult).Subsets      .resize( mesh.SubsetCount );
    (*pResult).Bones        .resize( mesh.BoneCount );

    for( u32 i=0; i<mesh.PositionCount; ++i )
    { fread( &(*pResult).Positions[i], sizeof(Vector3), 1, pFile ); }

    for( u32 i=0; i<mesh.NormalCount; ++i )
    { fread( &(*pResult).Normals[i], sizeof(Vector3), 1, pFile ); }

    for( u32 i=0; i<mesh.TexCoordCount; ++i )
    { fread( &(*pResult).TexCoords[i], sizeof(Vector2), 1, pFile ); }

    for( u32 i=0; i<mesh.BoneIndexCount; ++i )
    { fread( &(*pResult).BoneIndices[i], sizeof(uint4), 1, pFile ); }

    for( u32 i=0; i<mesh.BoneWeightCount; ++i )
    { fread( &(*pResult).BoneWeights[i], sizeof(Vector4), 1, pFile ); }

    for( u32 i=0; i<mesh.VertexIndexCount; ++i )
    { fread( &(*pResult).VertexIndices[i], sizeof(u32), 1, pFile ); }

    for( u32 i=0; i<mesh.SubsetCount; ++i )
    { fread( &(*pResult).Subsets[i], sizeof(ResSubset), 1, pFile ); }

    for( u32 i=0; i<mesh.BoneCount; ++i )
    {
        MSH_BONE bone;
        fread( &bone, sizeof(bone), 1, pFile );

        (*pResult).Bones[i].Name        = bone.Name;
        (*pResult).Bones[i].ParentId    = bone.ParentId;
        (*pResult).Bones[i].BindPose    = asdx::Matrix::CreateTranslation( bone.Position );
        (*pResult).Bones[i].InvBindPose = asdx::Matrix::Invert( (*pResult).Bones[i].BindPose );
    }

    fclose( pFile );
    return true;
}

//-------------------------------------------------------------------------------------------------
//      MSHファイルに保存します.
//-------------------------------------------------------------------------------------------------
bool SaveResMeshToMSH( const char16* filename, const ResMesh* pMesh )
{
    if ( filename == nullptr || pMesh == nullptr )
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

    MSH_FILE_HEADER header;
    header.Magic[0] = 'M';
    header.Magic[1] = 'S';
    header.Magic[2] = 'H';
    header.Magic[3] = '\0';
    header.Version = MSH_VERSION;

    fwrite( &header, sizeof(header), 1, pFile );

    MSH_MESH mesh;
    mesh.PositionCount    = static_cast<u32>( pMesh->Positions    .size() );
    mesh.NormalCount      = static_cast<u32>( pMesh->Normals      .size() );
    mesh.TexCoordCount    = static_cast<u32>( pMesh->TexCoords    .size() );
    mesh.BoneIndexCount   = static_cast<u32>( pMesh->BoneIndices  .size() );
    mesh.BoneWeightCount  = static_cast<u32>( pMesh->BoneWeights  .size() );
    mesh.VertexIndexCount = static_cast<u32>( pMesh->VertexIndices.size() );
    mesh.SubsetCount      = static_cast<u32>( pMesh->Subsets      .size() );
    mesh.BoneCount        = static_cast<u32>( pMesh->Bones        .size() );

    fwrite( &mesh, sizeof(mesh), 1, pFile );

    for( u32 i=0; i<mesh.PositionCount; ++i )
    { fwrite( &pMesh->Positions[i], sizeof(Vector3), 1, pFile ); }

    for( u32 i=0; i<mesh.NormalCount; ++i )
    { fwrite( &pMesh->Normals[i], sizeof(Vector3), 1, pFile ); }

    for( u32 i=0; i<mesh.TexCoordCount; ++i )
    { fwrite( &pMesh->TexCoords[i], sizeof(Vector2), 1, pFile ); }

    for( u32 i=0; i<mesh.BoneIndexCount; ++i )
    { fwrite( &pMesh->BoneIndices[i], sizeof(uint4), 1, pFile ); }

    for( u32 i=0; i<mesh.BoneWeightCount; ++i )
    { fwrite( &pMesh->BoneWeights[i], sizeof(Vector4), 1, pFile ); }

    for( u32 i=0; i<mesh.VertexIndexCount; ++i )
    { fwrite( &pMesh->VertexIndices[i], sizeof(u32), 1, pFile ); }

    for( u32 i=0; i<mesh.SubsetCount; ++i )
    { fwrite( &pMesh->Subsets[i], sizeof(ResSubset), 1, pFile ); }

    for( u32 i=0; i<mesh.BoneCount; ++i )
    {
        MSH_BONE bone = {};
        wcscpy_s( bone.Name, pMesh->Bones[i].Name.c_str());
        bone.ParentId = pMesh->Bones[i].ParentId;
        bone.Position.x = -pMesh->Bones[i].InvBindPose._41;
        bone.Position.y = -pMesh->Bones[i].InvBindPose._42;
        bone.Position.z = -pMesh->Bones[i].InvBindPose._43;
        fwrite( &bone, sizeof(bone), 1, pFile );
    }

    fclose( pFile );
    return true;
}

} // namespace asdx
