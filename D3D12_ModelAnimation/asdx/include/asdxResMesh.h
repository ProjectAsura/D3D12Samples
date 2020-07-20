﻿//-------------------------------------------------------------------------------------------------
// File : asdxResMesh.h
// Desc : Resource Mesh Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxMath.h>
#include <vector>
#include <string>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResBone structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResBone
{
    std::wstring        Name;           //!< ボーン名です.
    u32                 ParentId;       //!< 親ボーン番号.
    Matrix              BindPose;       //!< バインドポーズ行列です.
    Matrix              InvBindPose;    //!< 逆バインドポーズ行列です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResSubset structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResSubset
{
    u32     MaterialId;     //!< マテリアル番号
    u32     Offset;         //!< 頂点バッファ先頭からオフセット.
    u32     Count;          //!< 描画インデックス数.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResMesh structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ResMesh
{
    std::vector<Vector3>    Positions;      //!< 位置座標です.
    std::vector<Vector3>    Normals;        //!< 法線ベクトル.
    std::vector<Vector2>    TexCoords;      //!< テクスチャ座標.
    std::vector<uint4>      BoneIndices;    //!< ボーン番号.
    std::vector<Vector4>    BoneWeights;    //!< ボーン重み.
    std::vector<u32>        VertexIndices;  //!< 頂点インデックスです.
    std::vector<ResSubset>  Subsets;        //!< サブセットデータです.
    std::vector<ResBone>    Bones;          //!< ボーン.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MeshFactory class
///////////////////////////////////////////////////////////////////////////////////////////////////
class MeshFactory
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
    //! @brief      メッシュリソースを生成します.
    //!
    //! @param[in]      filename        メッシュファイル名です.
    //! @param[out]     pResult         メッシュリソースの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool Create( const char16* filename, ResMesh* pResult );

    //---------------------------------------------------------------------------------------------
    //! @brief      メッシュリソースを破棄します.
    //!
    //! @param[in]      ptr     破棄するメッシュリソースへのポインタ.
    //---------------------------------------------------------------------------------------------
    static void Dispose( ResMesh*& ptr );
};

} // namespace asdx
