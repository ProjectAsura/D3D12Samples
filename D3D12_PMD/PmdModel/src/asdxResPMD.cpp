//-------------------------------------------------------------------------------------------------
// File : asdxResPMD.cpp
// Desc : Polygon Model Data Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxResPMD.h>
#include <asdxLogger.h>
#include <cstdio>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResPmd class 
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ResPmd::ResPmd()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ResPmd::~ResPmd()
{ Dispose(); }

//-------------------------------------------------------------------------------------------------
//      ファイルから読み込みを行います.
//-------------------------------------------------------------------------------------------------
bool ResPmd::Load( const char16* filename )
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

    // ヘッダ読み込み.
    PMD_HEADER header;
    fread( &header, sizeof(header), 1, pFile );

    // ファイルマジックをチェック.
    if ( header.Magic[0] != 'P' || header.Magic[1] != 'm' || header.Magic[2] != 'd' )
    {
        ELOG( "Error : Invalid File. filename = %s", filename );
        return false;
    }

    Name    = header.ModelName;
    Version = header.Version;
    Comment = header.Comment;

    // 頂点リストを読み込み.
    {
        u32 vertexCount = 0;
        fread( &vertexCount, sizeof(vertexCount), 1, pFile );

        Vertices.resize( vertexCount );

        for( u32 i=0; i<vertexCount; ++i )
        { fread( &Vertices[i], sizeof(PMD_VERTEX), 1, pFile ); }
    }

    // 頂点インデックスを読み込み.
    {
        u32 vertexCount = 0;
        fread( &vertexCount, sizeof(vertexCount), 1, pFile );

        Indices.resize(vertexCount);

        for( u32 i=0; i<vertexCount; ++i )
        { fread( &Indices[i], sizeof(u16), 1, pFile ); }
    }

    // マテリアルを読み込み.
    {
        u32 materialCount = 0;
        fread( &materialCount, sizeof(materialCount), 1, pFile );

        Materials.resize( materialCount );

        for( u32 i=0; i<materialCount; ++i )
        { fread( &Materials[i], sizeof(PMD_MATERIAL), 1, pFile ); }
    }

    // ボーンデータを読み込み.
    {
        u16 boneCount = 0;
        fread( &boneCount, sizeof(boneCount), 1, pFile );

        Bones.resize( boneCount );

        for( u16 i=0; i<boneCount; ++i )
        { fread( &Bones[i], sizeof(PMD_BONE), 1, pFile ); }
    }

    // IKデータを読み込み.
    {
        u16 count = 0;
        fread( &count, sizeof(count), 1, pFile );

        IKs.resize( count );

        for( u32 i=0; i<count; ++i )
        {
            PMD_IK data;
            fread( &data, sizeof(data), 1, pFile );

            IKs[i].BoneIndex       = data.BoneIndex;
            IKs[i].TargetBoneIndex = data.TargetBoneIndex;
            IKs[i].RecursiveCount  = data.RecursiveCount;
            IKs[i].ControlWeight   = data.ControlWeight;

            IKs[i].ChildBoneIndices.resize( data.ChainCount );

            for( u32 j=0; j<data.ChainCount; ++j )
            { fread( &IKs[i].ChildBoneIndices[j], sizeof(u16), 1, pFile ); }
        }
    }

    // モーフデータを読み込み.
    {
        u16 morphCount = 0;
        fread( &morphCount, sizeof(morphCount), 1, pFile );

        Morphes.resize( morphCount );

        for( u16 i=0; i<morphCount; ++i )
        {
            PMD_MORPH morph;
            fread( &morph, sizeof(morph), 1, pFile );

            Morphes[i].Name = morph.Name;
            Morphes[i].Type = morph.Type;

            Morphes[i].Vertices.resize( morph.VertexCount );

            for( u32 j=0; j<morph.VertexCount; ++j )
            { fread( &Morphes[i].Vertices[j], sizeof(PMD_MORPH_VERTEX), 1, pFile ); }
        }
    }

    // 表情枠用表示リスト.
    {
        u8 count = 0;
        fread( &count, sizeof(count), 1, pFile );

        MorphLabelIndices.resize( count );

        for( u8 i=0; i<count; ++i )
        { fread( &MorphLabelIndices[i], sizeof(u16), 1, pFile ); }
    }

    // ボーン枠用枠名リスト.
    {
        u8 count = 0;
        fread( &count, sizeof(count), 1, pFile );

        BoneLabels.resize( count );

        for( u32 i=0; i<count; ++i )
        {
            char name[50];
            fread( name, sizeof(char), 50, pFile );

            BoneLabels[i] = name;
        }
    }

    // ボーン枠用表示リスト.
    {
        u32 count = 0;
        fread( &count, sizeof(count), 1, pFile );

        BoneLabelIndices.resize( count );

        for( u32 i=0; i<count; ++i )
        { fread( &BoneLabelIndices[i], sizeof(PMD_BONE_LABEL_INDEX), 1, pFile ); }
    }

    // 拡張データ ローカライズデータ.
    if ( feof( pFile ) == 0 )
    {
        PMD_LOCALIZE_HEADER headerEn;
        fread( &headerEn, sizeof(headerEn), 1, pFile );

        if ( headerEn.LocalizeFlag == 0x1 )
        {
            //std::string modelName = headerEn.ModelName;
            //std::string comment = headerEn.Comment;
            //ILOGA( "ModelName[EN] : %s", modelName.c_str() );
            //ILOGA( "Comment[EN] : %s", comment.c_str() );

            for( size_t i=0; i<Bones.size(); ++i )
            {
                char boneName[20];
                fread( boneName, sizeof(char), 20, pFile );
                //ILOGA( "BoneName[EN] : %s", boneName );
            }

            for( size_t i=0; i<Morphes.size() - 1; ++i )
            {
                char morphName[20];
                fread( morphName, sizeof(char), 20, pFile );
                //ILOGA( "MorphName[EN] : %s", morphName );
            }

            for( size_t i=0; i<BoneLabels.size(); ++i )
            {
                char label[50];
                fread( label, sizeof(char), 50, pFile );
                //ILOGA( "BoneLabel[EN] : %s", label );
            }
        }
    }

    // 拡張データ トゥーンテクスチャリスト.
    if ( feof( pFile ) == 0 )
    {
        fread( &ToonTextureList, sizeof(ToonTextureList), 1, pFile );
    }

    // 拡張データ 剛体データ.
    if ( feof( pFile ) == 0 )
    {
        u32 rigidBodyCount = 0;
        fread( &rigidBodyCount, sizeof(rigidBodyCount), 1, pFile );

        RigidBodies.resize( rigidBodyCount );

        for( u32 i=0; i<rigidBodyCount; ++i )
        { fread( &RigidBodies[i], sizeof(PMD_RIGIDBODY), 1, pFile ); }
    }

    // 拡張データ ジョイントリスト.
    if ( feof( pFile ) == 0 )
    {
        u32 jointCount = 0;
        fread( &jointCount, sizeof(jointCount), 1, pFile );

        Joints.resize( jointCount );

        for( u32 i=0; i<jointCount; ++i )
        { fread( &Joints[i], sizeof(PMD_PHYSICS_JOINT), 1, pFile ); }
    }

    // ファイルを閉じる.
    fclose( pFile );

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      破棄処理を行います.
//-------------------------------------------------------------------------------------------------
void ResPmd::Dispose()
{
    for( size_t i=0; i<IKs.size(); ++i )
    { IKs[i].ChildBoneIndices.clear(); }

    for( size_t i=0; i<Morphes.size(); ++i )
    { Morphes[i].Vertices.clear(); }

    Name     .clear();
    Comment  .clear();
    Vertices .clear();
    Indices  .clear();
    Materials.clear();
    Bones    .clear();
    IKs      .clear();
    Morphes  .clear();

    MorphLabelIndices.clear();
    BoneLabels       .clear();
    BoneLabelIndices .clear();
    RigidBodies      .clear();
    Joints           .clear();
}

} // namespace asdx

