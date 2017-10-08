//-------------------------------------------------------------------------------------------------
// File : asdxIndexBuffer.cpp
// Desc : Index Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxIndexBuffer.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// IndexBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
IndexBuffer::IndexBuffer()
: m_Resource()
{
    m_View.BufferLocation = 0;
    m_View.SizeInBytes    = 0;
    m_View.Format         = DXGI_FORMAT_UNKNOWN;
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
IndexBuffer::~IndexBuffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool IndexBuffer::Init( ID3D12Device* pDevice, u64 size, DXGI_FORMAT format, const void* pIndices )
{
    if ( pDevice == nullptr || pIndices == nullptr || size == 0 )
    {
        ELOG( "Error : Invalid Argument." );
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

    memcpy( pDst, pIndices, size_t(size) );

    m_Resource->Unmap( 0, nullptr );

    m_View.BufferLocation = m_Resource->GetGPUVirtualAddress();
    m_View.SizeInBytes    = static_cast<u32>( size );
    m_View.Format         = format;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void IndexBuffer::Term()
{
    m_Resource.Reset();

    m_View.BufferLocation = 0;
    m_View.SizeInBytes    = 0;
    m_View.Format         = DXGI_FORMAT_UNKNOWN;
}

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* IndexBuffer::GetResource() const
{ return m_Resource.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      インデックスバッファビューを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_INDEX_BUFFER_VIEW IndexBuffer::GetView() const
{ return m_View; }

} // namespace asdx
