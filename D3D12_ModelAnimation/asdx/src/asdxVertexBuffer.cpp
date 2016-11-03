//-------------------------------------------------------------------------------------------------
// File : asdxVertexBuffer.cpp
// Desc : Vertex Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxVertexBuffer.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// VertexBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
VertexBuffer::VertexBuffer()
: m_Resource()
{
    m_View.BufferLocation = 0;
    m_View.SizeInBytes    = 0;
    m_View.StrideInBytes  = 0;
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool VertexBuffer::Init( ID3D12Device* pDevice, u64 size, u32 stride, const void* pVertices )
{
    if ( pDevice == nullptr || pVertices == nullptr || size == 0 || stride == 0 )
    {
        ELOG( "Error : Invalid Arguments." );
        return false;
    }

    D3D12_HEAP_PROPERTIES props = {
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1,
        1
    };

    D3D12_RESOURCE_DESC desc = {
        D3D12_RESOURCE_DIMENSION_BUFFER,
        0,
        size,
        1,
        1,
        1,
        DXGI_FORMAT_UNKNOWN,
        { 1, 0 },
        D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        D3D12_RESOURCE_FLAG_NONE
    };

    auto hr = pDevice->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_Resource.GetAddress()));
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed." );
        return false;
    }

    u8* pDst;
    hr = m_Resource->Map( 0, nullptr, reinterpret_cast<void**>( &pDst ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Resource::Map() Failed." );
        return false;
    }

    memcpy( pDst, pVertices, size_t(size) );

    m_Resource->Unmap( 0, nullptr );

    m_View.BufferLocation = m_Resource->GetGPUVirtualAddress();
    m_View.SizeInBytes    = static_cast<u32>(size);
    m_View.StrideInBytes  = stride;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void VertexBuffer::Term()
{
    m_Resource.Reset();

    m_View.BufferLocation = 0;
    m_View.SizeInBytes    = 0;
    m_View.StrideInBytes  = 0;
}

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* VertexBuffer::GetResource() const
{ return m_Resource.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      頂点バッファビューを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetView() const
{ return m_View; }

} // namespace asdx
