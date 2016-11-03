//-------------------------------------------------------------------------------------------------
// File : asdxConstantBuffer.cpp
// Desc : Constant Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxConstantBuffer.h>
#include <asdxLogger.h>
#include <cassert>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ConstantBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ConstantBuffer::ConstantBuffer()
: m_Resource()
, m_pDst( nullptr )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ConstantBuffer::~ConstantBuffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool ConstantBuffer::Init( ID3D12Device* pDevice, u64 size )
{
    if ( pDevice == nullptr || size == 0 )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    auto rest = size % 256;
    if ( rest != 0 )
    {
        ELOG( "Error : ConstantBuffer must be 256 byte alignment., (size %% 256) = %u", rest );
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

    hr = m_Resource->Map( 0, nullptr, reinterpret_cast<void**>( &m_pDst ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Resource::Map() Failed." );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void ConstantBuffer::Term()
{
    if ( m_pDst != nullptr )
    {
        m_Resource->Unmap( 0, nullptr );
        m_pDst = nullptr;
    }

    m_Resource.Reset();
}

//-------------------------------------------------------------------------------------------------
//      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
void ConstantBuffer::Update( const void* pSrc, u64 size )
{
    assert( pSrc != nullptr );
    assert( size != 0 );
    memcpy( m_pDst, pSrc, size_t(size) );
}

//-------------------------------------------------------------------------------------------------
//      サブリソースを更新します.
//-------------------------------------------------------------------------------------------------
void ConstantBuffer::Update( const void* pSrc, u64 size, u64 srcOffset, u64 dstOffset )
{
    assert( pSrc != nullptr );
    assert( size != 0 );
    memcpy( m_pDst + dstOffset, static_cast<const u8*>(pSrc) + srcOffset, size_t(size) );
}

//-------------------------------------------------------------------------------------------------
//      アロー演算子です.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* ConstantBuffer::operator -> () const
{ return m_Resource.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Resource* ConstantBuffer::GetResource() const
{ return m_Resource.GetPtr(); }


} // namespace asdx


