//-------------------------------------------------------------------------------------------------
// File : Model.cpp
// Desc : Model Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <Model.h>
#include <asdxLogger.h>
#include <asdxMisc.h>
#include <asdxResMaterial.h>



///////////////////////////////////////////////////////////////////////////////////////////////////
// Model class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Model::Model()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Model::~Model()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Model::Init
(
    asdx::Device& device, 
    asdx::DeviceContext& context,
    const char16* meshFile,
    const char16* materialFile
)
{
    asdx::ResMesh mesh;

    if ( !asdx::MeshFactory::Create( meshFile, &mesh ) )
    {
        ELOG( "Error : Mesh Load Failed." );
        return false;
    }

    {
        m_Vertices.resize( mesh.Positions.size() );

        for( size_t i=0; i<mesh.Positions.size(); ++i )
        {
            m_Vertices[i].Position = mesh.Positions[i];
            m_Vertices[i].Normal = mesh.Normals[i];
            m_Vertices[i].TexCoord = mesh.TexCoords[i];
            m_Vertices[i].BoneIndices.x = mesh.BoneIndices[i].x;
            m_Vertices[i].BoneIndices.y = mesh.BoneIndices[i].y;
            m_Vertices[i].BoneWeights.x = mesh.BoneWeights[i].x;
            m_Vertices[i].BoneWeights.y = mesh.BoneWeights[i].y;
        }

        m_Subsets = mesh.Subsets;
        m_Indices = mesh.VertexIndices;
        m_Bones   = mesh.Bones;
    }

    u32 materialCount = 0;
    {
        asdx::ResMaterial material;

        if ( !asdx::MaterialFactory::Create( materialFile, &material ) )
        {
            ELOG( "Error : Material Load Failed." );
            return false;
        }

        materialCount = static_cast<u32>( material.Phong.size() );

        m_Materials.resize( materialCount );
        for( u32 i=0; i<materialCount; ++i )
        {
            m_Materials[i].Diffuse   = material.Phong[i].Diffuse;
            m_Materials[i].Alpha     = material.Phong[i].Alpha;
            m_Materials[i].Specular  = material.Phong[i].Specular;
            m_Materials[i].Power     = material.Phong[i].Power;
            m_Materials[i].Emissive  = material.Phong[i].Emissive;
            m_Materials[i].TextureId = material.Phong[i].TextureId;
        }

        auto dir = asdx::GetDirectoryPath( materialFile );

        m_ResTextures.resize( material.Paths.size() );

        for( size_t i=0; i<material.Paths.size(); ++i )
        {
            std::wstring path;
            auto temp = dir + L"/" + material.Paths[i];
            if ( !asdx::SearchFilePath( temp.c_str(), path ) )
            {
                assert( false );
                continue;
            }

            asdx::TextureFactory::Create( path.c_str(), &m_ResTextures[i] );
        }
    }


    if ( !m_VB.Init( 
        device.GetDevice(),
        sizeof(SkinningVertex) * m_Vertices.size(),
        sizeof(SkinningVertex),
        &m_Vertices[0] ))
    {
        ELOG( "Error : VertexBuffer::Init() Failed." );
        return false;
    }

    if ( !m_IB.Init( 
        device.GetDevice(),
        sizeof(u32) * m_Indices.size(),
        DXGI_FORMAT_R32_UINT,
        &m_Indices[0]))
    {
        ELOG( "Error : IndexBuffer::Init() Failed." );
        return false;
    }

    // 定数バッファを生成.
    u32 cbSize = static_cast<u32>(sizeof(Material) * m_Materials.size() );
    if ( !m_CB.Init( device.GetDevice(), cbSize ))
    {
        ELOG( "Error : ConstantBuffer::Init() Failed." );
        return false;
    }

    m_CB.Update( &m_Materials[0], sizeof(Material) * m_Materials.size() );

    // 定数バッファビューの設定.
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = m_CB->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = sizeof(Material);

    m_CBV.resize( materialCount );

    // 定数バッファの生成.
    for( u32 i=0; i<materialCount; ++i )
    {
        m_CBV[i] = device.CreateCBV( &cbvDesc );
        cbvDesc.BufferLocation += sizeof(Material);
    }

    auto textureCount = static_cast<u32>( m_ResTextures.size() );
    m_Textures.resize( textureCount );
    m_SRV.resize( textureCount );
    for( u32 i=0; i<textureCount; ++i )
    {
        if ( !CreateTexture( device, m_ResTextures[i], m_Textures[i].GetAddress(), &m_SRV[i] ) )
        {
            ELOG( "Error : CreateTexture() Failed. index = %u", i );
            return false;
        }

        context.Clear( nullptr );

        D3D12_SUBRESOURCE_DATA subRes;
        subRes.pData      = m_ResTextures[i].pSurfaces->pPixels;
        subRes.RowPitch   = m_ResTextures[i].pSurfaces->RowPitch;
        subRes.SlicePitch = m_ResTextures[i].pSurfaces->SlicePitch;

        if ( !context.UpdateSubRes( m_Textures[i].GetPtr(), 0, 1, &subRes ) )
        {
            ELOG( "Error : DeviceContext::UpdateSubRes() Failed. index = %u", i );
            return false;
        }
    }

    // ダミーテクスチャ生成.
    {
        m_DummyResTexture.Width = 32;
        m_DummyResTexture.Height = 32;
        m_DummyResTexture.Depth = 0;
        m_DummyResTexture.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_DummyResTexture.MipMapCount = 1;
        m_DummyResTexture.SurfaceCount = 1;

        auto surface = new asdx::Surface();
        surface->Width = 32;
        surface->Height = 32;
        surface->RowPitch = 32 * 4;
        surface->SlicePitch = surface->RowPitch * surface->Height;
        surface->pPixels = new u8 [surface->SlicePitch];

        for( u32 i=0; i<surface->SlicePitch; ++i)
        { surface->pPixels[i] = 255; }

        m_DummyResTexture.pSurfaces = surface;

        if ( !CreateTexture( device, m_DummyResTexture, m_DummyTexture.GetAddress(), &m_DummySRV ) )
        {
            ELOG( "Error : CreateTexture() Failed." );
            return false;
        }

        context.Clear( nullptr );

        D3D12_SUBRESOURCE_DATA subRes;
        subRes.pData      = surface->pPixels;
        subRes.RowPitch   = surface->RowPitch;
        subRes.SlicePitch = surface->SlicePitch;

        if ( !context.UpdateSubRes( m_DummyTexture.GetPtr(), 0, 1, &subRes ) )
        {
            ELOG( "Error : DeviceContext::UpdateSubRes() Failed." );
            return false;
        }
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Model::Term()
{
    m_VB.Term();
    m_IB.Term();
    m_CB.Term();

    m_Vertices .clear();
    m_Indices  .clear();
    m_Materials.clear();
    m_Subsets  .clear();
    m_Bones    .clear();

    m_ResTextures.clear();
    m_Textures   .clear();

    m_CBV.clear();
    m_SRV.clear();
}

//-------------------------------------------------------------------------------------------------
//      テクスチャを生成します.
//-------------------------------------------------------------------------------------------------
bool Model::CreateTexture
(
    asdx::Device& device,
    const asdx::ResTexture& texture,
    ID3D12Resource** ppResource,
    asdx::DescHandle* pHandle
) const
{
    D3D12_RESOURCE_DESC desc = {
        D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        0,
        texture.Width,
        texture.Height,
        static_cast<u16>(texture.SurfaceCount),
        static_cast<u16>(texture.MipMapCount),
        static_cast<DXGI_FORMAT>(texture.Format),
        { 1, 0 },
        D3D12_TEXTURE_LAYOUT_UNKNOWN,
        D3D12_RESOURCE_FLAG_NONE
    };

    D3D12_HEAP_PROPERTIES props = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1,
        1,
    };

    auto hr = device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(ppResource));
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommitedResource() Failed." );
        return false;
    }

    auto mipLevel = ( desc.MipLevels == 0 ) ? -1 : desc.MipLevels;

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    if ( texture.SurfaceCount > 6 && ( texture.Option & asdx::RESTEXTURE_OPTION_CUBEMAP ) )
    {
        viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        viewDesc.TextureCubeArray.MipLevels = mipLevel;
        viewDesc.TextureCubeArray.NumCubes  = texture.SurfaceCount / 6;
    }
    else if ( texture.SurfaceCount == 6  && (texture.Option & asdx::RESTEXTURE_OPTION_CUBEMAP ) )
    {
        viewDesc.ViewDimension         = D3D12_SRV_DIMENSION_TEXTURECUBE;
        viewDesc.TextureCube.MipLevels = mipLevel;
    }
    else if ( texture.SurfaceCount > 1 )
    {
        viewDesc.ViewDimension            = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        viewDesc.Texture2DArray.ArraySize = texture.SurfaceCount;
        viewDesc.Texture2DArray.MipLevels = mipLevel;
    }
    else
    {
        viewDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipLevels       = mipLevel;
        viewDesc.Texture2D.MostDetailedMip = 0;
    }
    viewDesc.Format = static_cast<DXGI_FORMAT>(texture.Format);
    viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    (*pHandle) = device.CreateSRV( (*ppResource), &viewDesc );

    return true;
}

void Model::DrawCmd( ID3D12GraphicsCommandList* pCmd )
{
    assert( pCmd != nullptr );

    auto vbv = m_VB.GetView();
    auto ibv = m_IB.GetView();

    pCmd->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    pCmd->IASetVertexBuffers( 0, 1, &vbv );
    pCmd->IASetIndexBuffer( &ibv );

    for( size_t i=0; i<m_Subsets.size(); ++i )
    {
        auto materialId = m_Subsets[i].MaterialId;
        auto textureId = m_Materials[materialId].TextureId;

        auto handleSRV = ( textureId != U32_MAX ) ? m_SRV[textureId].GetHandleGpu() : m_DummySRV.GetHandleGpu();
        pCmd->SetGraphicsRootDescriptorTable( 1, handleSRV );
        pCmd->SetGraphicsRootDescriptorTable( 2, m_CBV[materialId].GetHandleGpu() );
        pCmd->DrawIndexedInstanced( m_Subsets[i].Count, 1, m_Subsets[i].Offset, 0, 0 );
    }
}

asdx::ResBone* Model::GetBones()
{ return &m_Bones[0]; }

u32 Model::GetBoneCount() const
{ return static_cast<u32>( m_Bones.size() ); }
