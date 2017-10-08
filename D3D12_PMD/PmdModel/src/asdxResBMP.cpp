//-------------------------------------------------------------------------------------------------
// File : asdxResBMP.cpp
// Desc : Bitmap Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxResBMP.h>
#include <asdxLogger.h>
#include <asdxHash.h>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <new>


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      1-Bit モノクロビットマップを解析します.
//-------------------------------------------------------------------------------------------------
void Parse1Bits( FILE* pFile, u8* pColorMap, s32 size, bool isWin, u8* pResult )
{
    auto pixSize = ( isWin ) ? 4 : 3;

    for( auto i=0; i<size; )
    {
        auto color = (u8)fgetc( pFile );
        for( auto j=7; j>=0; --j, ++i )
        {
            // 各ビットを見て 0 か 1 を判定する.
            u8 idx = (u8)(( color & ( 0x1 << j ) ) > 0);
            auto idxR = ( i * 3 );
            auto idxC = ( idx * pixSize );

            pResult[ idxR + 2 ] = pColorMap[ idxC + 0 ];
            pResult[ idxR + 1 ] = pColorMap[ idxC + 1 ];
            pResult[ idxR + 0 ] = pColorMap[ idxC + 2 ];
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      4-Bit 16色ビットマップを解析します.
//-------------------------------------------------------------------------------------------------
void Parse4Bits( FILE* pFile, u8* pColorMap, s32 size, bool isWin, u8* pResult )
{
    auto pixSize = ( isWin ) ? 4 : 3;

    for( auto i=0; i<size; i+=2 )
    {
        auto color = (u8)fgetc( pFile );
        auto idx = ( color >> 4 );
        auto idxR = ( i * 3 );
        auto idxC = ( idx * pixSize );

        pResult[ idxR + 2 ] = pColorMap[ idxC + 0 ];
        pResult[ idxR + 1 ] = pColorMap[ idxC + 1 ];
        pResult[ idxR + 0 ] = pColorMap[ idxC + 2 ];

        idx = ( color & 0x0f );
        idxC = ( idx * pixSize );

        pResult[ idxR + 5 ] = pColorMap[ idxC + 0 ];
        pResult[ idxR + 4 ] = pColorMap[ idxC + 1 ];
        pResult[ idxR + 3 ] = pColorMap[ idxC + 2 ];
    }
}

//-------------------------------------------------------------------------------------------------
//      8-Bit 256色ビットマップを解析します.
//-------------------------------------------------------------------------------------------------
void Parse8Bits( FILE* pFile, u8* pColorMap, s32 size, bool isWin, u8* pResult )
{
    auto pixSize = ( isWin ) ? 4 : 3;

    for( auto i=0; i<size; ++i )
    {
        auto color = (u8)fgetc( pFile );
        auto idxR = ( i * 3 );
        auto idxC = ( color * pixSize );

        pResult[ idxR + 2 ] = pColorMap[ idxC + 0 ];
        pResult[ idxR + 1 ] = pColorMap[ idxC + 1 ];
        pResult[ idxR + 0 ] = pColorMap[ idxC + 2 ];
    }
}

//-------------------------------------------------------------------------------------------------
//      24-Bit フルカラービットマップを解析します.
//-------------------------------------------------------------------------------------------------
void Parse24Bits( FILE* pFile, s32 size, u8* pResult )
{
    for( auto i=0; i<size; ++i )
    {
        auto idxR = ( i * 3 );
        pResult[ idxR + 2 ] = (u8)fgetc( pFile );
        pResult[ idxR + 1 ] = (u8)fgetc( pFile );
        pResult[ idxR + 0 ] = (u8)fgetc( pFile );
    }
}

//-------------------------------------------------------------------------------------------------
//      32-Bit フルカラービットマップを解析します.
//-------------------------------------------------------------------------------------------------
void Parse32Bits( FILE* pFile, s32 size, u8* pResult )
{
    for( auto i=0; i<size; ++i )
    {
        auto idxR = ( i * 4 );
        pResult[ idxR + 2 ] = (u8)fgetc( pFile );
        pResult[ idxR + 1 ] = (u8)fgetc( pFile );
        pResult[ idxR + 0 ] = (u8)fgetc( pFile );
        pResult[ idxR + 3 ] = (u8)fgetc( pFile );
    }
}

//-------------------------------------------------------------------------------------------------
//      8-Bit ランレングス圧縮ビットマップを解析します.
//-------------------------------------------------------------------------------------------------
void Parse8BitsRLE( FILE* pFile, u8* pColorMap, s32 size, u8* pResult )
{
    auto ptr = pResult;

    while( ptr < pResult + size * 3 )
    {
        auto byte1 = (u8)fgetc( pFile );
        auto byte2 = (u8)fgetc( pFile );

        // RLE_COMMAND ?
        if ( byte1 == 0 )
        {
            for( auto i=0; i<byte2; ++i, ptr+=3 )
            {
                auto color = (u8)fgetc( pFile );
                auto idxC = color * 4;
                ptr[ 0 ] = pColorMap[ idxC + 2 ];
                ptr[ 1 ] = pColorMap[ idxC + 1 ];
                ptr[ 2 ] = pColorMap[ idxC + 0 ];
            }

            if ( byte2 % 2 )
            {
                auto skip = (u8)fgetc( pFile );
                ASDX_UNUSED_VAR( skip );
            }
        }
        else
        {
            auto idxC = byte2 * 4;

            for ( auto i=0; i<byte1; ++i, ptr+=3 )
            {
                ptr[ 0 ] = pColorMap[ idxC + 2 ];
                ptr[ 1 ] = pColorMap[ idxC + 1 ];
                ptr[ 2 ] = pColorMap[ idxC + 0 ];
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      4-Bit ランレングス圧縮ビットマップを解析します.
//-------------------------------------------------------------------------------------------------
void Parse4BitsRLE( FILE* pFile, u8* pColorMap, s32 size, u8* pResult )
{
    auto ptr = pResult;
    auto byteRead = 0;
    auto dataByte = 0;
    u8 color = 0;

    while( ptr < pResult + size * 3 )
    {
        auto byte1 = (u8)fgetc( pFile );
        auto byte2 = (u8)fgetc( pFile );
        byteRead += 2;

        if ( byte1 == 0 )
        {
            dataByte = 0;
            for( auto i=0; i<byte2; ++i, ptr+=3 )
            {
                if ( i % 2 )
                {
                    color = ( dataByte & 0x0f );
                }
                else
                {
                    dataByte = (u8)fgetc( pFile );
                    byteRead++;
                    color = (dataByte >> 4);
                }

                auto idxC = color * 4;
                ptr[ 0 ] = pColorMap[ idxC + 2 ];
                ptr[ 1 ] = pColorMap[ idxC + 1 ];
                ptr[ 2 ] = pColorMap[ idxC + 0 ];
            }

            if ( byteRead % 2 )
            {
                auto skip = (u8)fgetc( pFile );
                ASDX_UNUSED_VAR( skip );
                byteRead++;
            }
        }
        else
        {
            for( auto i=0; i<byte1; ++i, ptr+=3 )
            {
                if ( i % 2 )
                { color = ( byte2 & 0x0f ); }
                else
                { color = ( byte2 >> 4 ); }

                auto idxC = color * 4;
                ptr[ 0 ] = pColorMap[ idxC + 2 ];
                ptr[ 1 ] = pColorMap[ idxC + 1 ];
                ptr[ 2 ] = pColorMap[ idxC + 0 ];
            }
        }
    }
}

} // namespace /* anonymous */

namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResBMP class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ResBMP::ResBMP()
: m_Width   ( 0 )
, m_Height  ( 0 )
, m_Format  ( 0 )
, m_pPixels ( nullptr )
, m_HashKey ( 0 )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      コピーコンストラクタです.
//-------------------------------------------------------------------------------------------------
ResBMP::ResBMP( const ResBMP& value )
: m_Width   ( value.m_Width )
, m_Height  ( value.m_Height )
, m_Format  ( value.m_Format )
, m_pPixels ( nullptr )
, m_HashKey ( value.m_HashKey )
{
    auto size = m_Width * m_Height * ( ( m_Format == Format_RGB ) ? 3 : 4 );
    m_pPixels = new (std::nothrow) u8 [ size ];
    assert( m_pPixels != nullptr );

    if ( m_pPixels )
    {
        memcpy( m_pPixels, value.m_pPixels, size * sizeof(u8) );
    }
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ResBMP::~ResBMP()
{
    Dispose();
}

//-------------------------------------------------------------------------------------------------
//      ファイルから読み込みを行います.
//-------------------------------------------------------------------------------------------------
bool ResBMP::Load( const wchar_t* filename )
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

    BMP_FILE_HEADER fh;
    fread( &fh, sizeof(fh), 1, pFile );

    if ( fh.Type != 'MB' )
    {
        ELOG( "Error : Invalid File." );
        fclose( pFile );
        return false;
    }

    BMP_INFO_HEADER ih;
    fread( &ih, sizeof(ih), 1, pFile );

    bool isWin = ( ih.Compression <= 3 );
    auto bitPerCount = 0;
    u32 compression;

    if ( isWin )
    {
        m_Width     = ih.Width;
        m_Height    = ih.Height;
        bitPerCount = ih.BitCount;
        compression = ih.Compression;
    }
    else
    {
        fseek( pFile, sizeof(fh), SEEK_SET );

        BMP_CORE_HEADER ch;
        fread( &ch, sizeof(ch), 1, pFile );
        m_Width     = ch.Width;
        m_Height    = ch.Height;
        bitPerCount = ch.BitCount;
        compression = 0;
    }

    auto colorMapSize = 0;
    u8* pColorMap = nullptr;
    if ( bitPerCount <= 8 )
    {
        colorMapSize = ( 1 << bitPerCount ) * ( isWin ? 4 : 3 );
        pColorMap = new (std::nothrow) u8 [ colorMapSize ];
        assert( pColorMap != nullptr );

        if ( pColorMap == nullptr )
        {
            ELOG( "Error : Out of Memory." );
            fclose( pFile );
            Dispose();
            return false;
        }

        fread( pColorMap, sizeof(u8), colorMapSize, pFile );
    }

    auto size = m_Width * m_Height;
    if ( bitPerCount == 32 )
    {
        m_Format = Format_RGBX;
        m_pPixels = new (std::nothrow) u8 [ size * 4 ];
    }
    else
    {
        m_Format = Format_RGB;
        m_pPixels = new (std::nothrow) u8 [ size * 3 ];
    }

    assert( m_pPixels != nullptr );
    if ( m_pPixels == nullptr )
    {
        ELOG( "Error : Out of Memory." );
        SafeDeleteArray( pColorMap );
        fclose( pFile );
        Dispose();
        return false;
    }

    m_HashKey = Crc32( filename ).GetHash();

    fseek( pFile, fh.OffBits, SEEK_SET );

    switch( compression )
    {
    case BMP_COMPRESSION_RGB:
        {
            switch( bitPerCount )
            {
                case 1:  { Parse1Bits( pFile, pColorMap, size, isWin, m_pPixels ); }  break;
                case 4:  { Parse4Bits( pFile, pColorMap, size, isWin, m_pPixels ); }  break;
                case 8:  { Parse8Bits( pFile, pColorMap, size, isWin, m_pPixels ); }  break;
                case 24: { Parse24Bits( pFile, size, m_pPixels ); } break;
                case 32: { Parse32Bits( pFile, size, m_pPixels ); } break;
            }
        }
        break;

    case BMP_COMPRESSION_RLE8:
        { Parse8BitsRLE( pFile, pColorMap, size, m_pPixels ); }
        break;

    case BMP_COMPRESSION_RLE4:
        { Parse4BitsRLE( pFile, pColorMap, size, m_pPixels ); }
        break;

    case BMP_COMPRESSION_BITFIELDS:
        { /* DO_NOTHING */}
        break;

    default:
        { assert( false ); }
        break;
    }

    SafeDeleteArray( pColorMap );

    fclose( pFile );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      メモリを解放します.
//-------------------------------------------------------------------------------------------------
void ResBMP::Dispose()
{
    SafeDeleteArray( m_pPixels );
    m_Width  = 0;
    m_Height = 0;
    m_Format = 0;
}

//-------------------------------------------------------------------------------------------------
//      画像の横幅を取得します.
//-------------------------------------------------------------------------------------------------
const u32 ResBMP::GetWidth() const
{ return m_Width; }

//-------------------------------------------------------------------------------------------------
//      画像の縦幅を取得します.
//-------------------------------------------------------------------------------------------------
const u32 ResBMP::GetHeight() const
{ return m_Height; }

//-------------------------------------------------------------------------------------------------
//      フォーマットを取得します.
//-------------------------------------------------------------------------------------------------
const u32 ResBMP::GetFormat() const
{ return m_Format; }

//-------------------------------------------------------------------------------------------------
//      ピクセルデータを取得します.
//-------------------------------------------------------------------------------------------------
const u8* ResBMP::GetPixels() const
{ return m_pPixels; }

//-------------------------------------------------------------------------------------------------
//      代入演算子です.
//-------------------------------------------------------------------------------------------------
ResBMP& ResBMP::operator = ( const ResBMP& value )
{
    m_Width   = value.m_Width;
    m_Height  = value.m_Height;
    m_Format  = value.m_Format;
    m_HashKey = value.m_HashKey;

    SafeDeleteArray( m_pPixels );
    auto size = m_Width * m_Height * ( ( m_Format == Format_RGB ) ? 3 : 4 );
    m_pPixels = new (std::nothrow) u8 [ size ];
    assert( m_pPixels != nullptr );

    if ( m_pPixels )
    {
        memcpy( m_pPixels, value.m_pPixels, size * sizeof(u8) );
    }

    return (*this);
}

//-------------------------------------------------------------------------------------------------
//      等価比較演算子です.
//-------------------------------------------------------------------------------------------------
bool ResBMP::operator == ( const ResBMP& value ) const
{
    if ( &value == this )
    { return true; }

    return m_HashKey == value.m_HashKey;
}

//-------------------------------------------------------------------------------------------------
//      非等価比較演算子です.
//-------------------------------------------------------------------------------------------------
bool ResBMP::operator!= ( const ResBMP& value ) const
{
    if ( &value == this )
    { return false; }

    return m_HashKey != value.m_HashKey;
}


} // namespace asdx

