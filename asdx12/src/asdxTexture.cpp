//-------------------------------------------------------------------------------------------------
// File : asdxTexture.cpp
// Desc : Texture.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTexture.h>


namespace asdx {

TextureDesc::TextureDesc()
{
    Dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;
    Alignment = 0;
    Width = 0;
    Height = 1;
    DepthOrArraySize = 1;
    MipLevels = 1;
    Format = DXGI_FORMAT_UNKNOWN;
    SampleDesc.Count = 1;
    SampleDesc.Quality = 0;
    Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    Flags = D3D12_RESOURCE_FLAG_NONE;
}

TextureDesc::~TextureDesc()
{
}

void TextureDesc::Set1D(uint32_t w, DXGI_FORMAT f)
{
    Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    Width = w;
    Height = 1;
    DepthOrArraySize = 1;
    Format = f;
}

void TextureDesc::Set2D(uint32_t w, uint32_t h, DXGI_FORMAT f)
{
    Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    Width = w;
    Height = h;
    DepthOrArraySize = 1;
    Format = f;
}

void TextureDesc::Set3D(uint32_t w, uint32_t h, uint32_t d, DXGI_FORMAT f)
{
    Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    Width = w;
    Height = h;
    DepthOrArraySize = d;
    Format = f;
}

Texture::Texture()
{ /* DO_NOTHING */ }

Texture::~Texture()
{ Term(); }

bool Texture::Init(ID3D12Device* pDevice, const TextureDesc* pDesc)
{
    HRESULT hr = S_OK;

    {
        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1    
        };

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format = pDesc->Format;
        clearValue.Color[0] = 1.0f;
        clearValue.Color[1] = 1.0f;
        clearValue.Color[2] = 1.0f;
        clearValue.Color[3] = 1.0f;

        hr = pDevice->CreateCommittedResource( 
            &props,
            D3D12_HEAP_FLAG_NONE,
            pDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if ( FAILED( hr ) )
        { return false; }
    }

    {
        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        desc.Format             = pDesc->Format;
        desc.Texture2D.MipSlice = 0;

        m_pDescriptorTarget = pDesc->pHeapTarget->CreateDescriptor();
        if (m_pDescriptorTarget == nullptr)
        { return false; }

        pDevice->CreateRenderTargetView(m_pResource.GetPtr(), &desc, m_pDescriptorTarget->GetHandleCPU());
    }

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format = pDesc->Format;
        desc.Texture2D.MipLevels = 1;
        desc.Texture2D.MostDetailedMip = 0;

        m_SRV = device.CreateSRV( m_Resource.GetPtr(), &desc );
    }

    return true;
}

void Texture::Term()
{
    m_pDescriptorResource.Reset();
    m_pDescriptorTarget.Reset();
    m_pResource.Reset();
}

ID3D12Resource* Texture::GetResource() const
{ return m_pResource.GetPtr(); }

const Descriptor* Texture::GetDescriptorResource() const
{ return m_pDescriptorResource.GetPtr(); }

const Descriptor* Texture::GetDescriptorTarget() const
{ return m_pDescriptorTarget.GetPtr(); }

} // namespace asdx
