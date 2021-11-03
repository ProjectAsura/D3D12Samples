//-----------------------------------------------------------------------------
// File : MeshLoader.h
// Desc : Mesh Loader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <vector>
#include <string>
#include <asdxMath.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>


///////////////////////////////////////////////////////////////////////////////
// ResMesh structure
///////////////////////////////////////////////////////////////////////////////
struct ResMesh
{
    std::string                         Name;               // メッシュ名.
    uint32_t                            MaterialId;         // マテリアルID.
    std::vector<asdx::Vector3>          Positions;          // 頂点位置.
    std::vector<asdx::Vector3>          Normals;            // 法線ベクトル.
    std::vector<asdx::Vector3>          Tangents;           // 接線ベクトル.
    std::vector<asdx::Vector4>          Colors;             // 頂点カラー.
    std::vector<asdx::Vector2>          TexCoords[4];       // テクスチャ座標.
    std::vector<uint32_t>               Indices;            // 頂点インデックス.

    //-------------------------------------------------------------------------
    //! @brief      破棄処理を行います.
    //-------------------------------------------------------------------------
    void Dispose()
    {
        Name.clear();
        MaterialId = UINT32_MAX;
        Positions.clear();
        Positions.shrink_to_fit();

        Normals.clear();
        Normals.shrink_to_fit();

        Tangents.clear();
        Tangents.shrink_to_fit();

        Colors.clear();
        Colors.shrink_to_fit();

        for(auto i=0; i<4; ++i)
        {
            TexCoords[i].clear();
            TexCoords[i].shrink_to_fit();
        }

        Indices.clear();
        Indices.shrink_to_fit();
    }
};

///////////////////////////////////////////////////////////////////////////////
// ResModel structure
///////////////////////////////////////////////////////////////////////////////
struct ResModel
{
    std::string                 Name;           // モデル名.
    bool                        Visible;        // 可視フラグ.
    std::vector<ResMesh>        Meshes;         // メッシュ.
    std::vector<std::string>    MaterialNames;  // マテリアル名.

    //-------------------------------------------------------------------------
    //! @brief      破棄処理を行います.
    //-------------------------------------------------------------------------
    void Dispose()
    {
        for(size_t i=0; i<Meshes.size(); ++i)
        { Meshes[i].Dispose(); }

        Meshes.clear();
        Meshes.shrink_to_fit();

        Visible = false;
        Name.clear();
    }
};


///////////////////////////////////////////////////////////////////////////////
// MeshLoader class
///////////////////////////////////////////////////////////////////////////////
class MeshLoader
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    MeshLoader() = default;
    ~MeshLoader() = default;

    //-------------------------------------------------------------------------
    //! @brief      モデルをロードします.
    //-------------------------------------------------------------------------
    bool Load(const char* path, ResModel& model)
    {
        if (path == nullptr)
        { return false; }

        Assimp::Importer importer;
        unsigned int flag = 0;
        flag |= aiProcess_Triangulate;
        flag |= aiProcess_PreTransformVertices;
        flag |= aiProcess_CalcTangentSpace;
        flag |= aiProcess_GenSmoothNormals;
        flag |= aiProcess_GenUVCoords;
        flag |= aiProcess_RemoveRedundantMaterials;
        flag |= aiProcess_OptimizeMeshes;

        // ファイルを読み込み.
        m_pScene = importer.ReadFile(path, flag);

        // チェック.
        if (m_pScene == nullptr)
        { return false; }

        // メッシュのメモリを確保.
        auto meshCount = m_pScene->mNumMeshes;
        model.Meshes.clear();
        model.Meshes.resize(meshCount);

        // メッシュデータを変換.
        for(size_t i=0; i<meshCount; ++i)
        {
            const auto pMesh = m_pScene->mMeshes[i];
            ParseMesh(model.Meshes[i], pMesh);
        }

        // マテリアル名取得.
        auto materialCount = m_pScene->mNumMaterials;
        model.MaterialNames.resize(materialCount);
        for(size_t i=0; i<materialCount; ++i)
        {
            aiString name;
            aiGetMaterialString(m_pScene->mMaterials[i], AI_MATKEY_NAME, &name);
            model.MaterialNames[i] = name.C_Str();
        }

        // 不要になったのでクリア.
        importer.FreeScene();
        m_pScene = nullptr;

        // 正常終了.
        return true;
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    const aiScene*  m_pScene = nullptr;

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      メッシュを解析します.
    //-------------------------------------------------------------------------
    void ParseMesh(ResMesh& dstMesh, const aiMesh* srcMesh)
    {
        // マテリアル番号を設定.
        dstMesh.MaterialId = srcMesh->mMaterialIndex;

        dstMesh.Name = srcMesh->mName.C_Str();

        aiVector3D zero3D(0.0f, 0.0f, 0.0f);

        // 頂点データのメモリを確保.
        auto vertexCount = srcMesh->mNumVertices;
        dstMesh.Positions.resize(vertexCount);

        if (srcMesh->HasNormals())
        { dstMesh.Normals.resize(vertexCount); }

        if (srcMesh->HasTangentsAndBitangents()) 
        { dstMesh.Tangents.resize(vertexCount); }

        if (srcMesh->HasVertexColors(0))
        { dstMesh.Colors.resize(vertexCount); }

        auto uvCount = srcMesh->GetNumUVChannels();
        uvCount = (uvCount > 4) ? 4 : uvCount;

        for(auto c=0u; c<uvCount; ++c) {
            assert(srcMesh->HasTextureCoords(c));
            dstMesh.TexCoords[c].resize(vertexCount);
        }

        for(auto i=0u; i<vertexCount; ++i)
        {
            auto& pos = srcMesh->mVertices[i];
            dstMesh.Positions[i] = asdx::Vector3(pos.x, pos.y, pos.z);

            if (srcMesh->HasNormals())
            {
                auto& normal = srcMesh->mNormals[i];
                dstMesh.Normals[i] = asdx::Vector3(normal.x, normal.y, normal.z);
            }

            if (srcMesh->HasTangentsAndBitangents())
            {
                auto& tangent = srcMesh->mTangents[i];
                dstMesh.Tangents[i] = asdx::Vector3(tangent.x, tangent.y, tangent.z);
            }
            if (srcMesh->HasVertexColors(0))
            {
                auto& color = srcMesh->mColors[0][i];
                dstMesh.Colors[i] = asdx::Vector4(color.r, color.g, color.b, color.a);
            }
            for(auto c=0u; c<uvCount; ++c)
            {
                auto& uv = srcMesh->mTextureCoords[c][i];
                dstMesh.TexCoords[c][i] = asdx::Vector2(uv.x, uv.y);
            }
        }

        // 頂点インデックスのメモリを確保.
        auto indexCount = srcMesh->mNumFaces * 3;
        dstMesh.Indices.resize(indexCount);

        for(size_t i=0; i<srcMesh->mNumFaces; ++i)
        {
            const auto& face = srcMesh->mFaces[i];
            assert(face.mNumIndices == 3);  // 三角形化しているので必ず3になっている.

            dstMesh.Indices[i * 3 + 0] = face.mIndices[0];
            dstMesh.Indices[i * 3 + 1] = face.mIndices[1];
            dstMesh.Indices[i * 3 + 2] = face.mIndices[2];
        }
    }
};