//-------------------------------------------------------------------------------------------------
// File : asdxConnector.cpp
// Desc : Connector Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxConnnector.h>
#include <asdxLogger.h>
#include <WinSock2.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Connector class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Connector::Connector()
: m_IsConnected ( false )
, m_IsReady     ( false )
, m_SrcSocket   ( 0 )
, m_DstSocket   ( 0 )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Connector::~Connector()
{ Close(); }

//-------------------------------------------------------------------------------------------------
//      接続処理を行います
//-------------------------------------------------------------------------------------------------
bool Connector::Connect( const TargetInfo& info )
{
    sockaddr_in addr;
    sockaddr_in client;
    s32 len = sizeof(client);
    auto ret = 0;

    WSADATA wsaData;

    if ( !m_IsReady )
    {
        ret = WSAStartup( 0x0202, &wsaData );
        if ( ret != 0 )
        {
            ELOG( "Error : WSAStartup() Failed." );
            return false;
        }

        // TCP通信の設定でソケットを生成.
        m_SrcSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

        addr.sin_family           = AF_INET;
        addr.sin_port             = htons( info.Port );
        addr.sin_addr.S_un.S_addr = info.Address;

        // サーバーソケットに名前を付けます.
        ret = bind( m_SrcSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr) );
        if ( ret != 0 )
        {
            auto errcode = WSAGetLastError();
            if ( errcode != WSAEADDRINUSE )
            {
                ELOG( "Error : WSAGetLastError() errorCode = %d", errcode );
                return false;
            }

            // 準備済みフラグを立てえる.
            m_IsReady = true;
        }
    }

    // 非ブロッキングモードにする.
    u_long val = 1;
    ioctlsocket( m_SrcSocket, FIOASYNC, &val );

    // ソケットを受信待機モードにして，保留接続キューのサイズを確保します.
    ret = listen( m_SrcSocket, 5 );
    if ( ret == SOCKET_ERROR )
    {
        ELOG( "Error : listen() Failed." );
        return false;
    }

    // サーバーソケットを接続要求待ち状態にします.
    m_DstSocket = accept( m_SrcSocket, reinterpret_cast<sockaddr*>(&client), &len );
    if ( m_DstSocket == INVALID_SOCKET )
    {
        ELOG( "Error : accepct() Failed." );
        return false;
    }

    // 接続済みフラグを立てます.
    m_IsConnected = true;

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      切断処理を行います.
//-------------------------------------------------------------------------------------------------
void Connector::Close()
{
    if ( !m_IsConnected && !m_IsReady )
    { return; }

    // 切断通知.
    shutdown( m_SrcSocket, SD_BOTH );
    shutdown( m_DstSocket, SD_BOTH );

    // ソケットを解放.
    closesocket( m_SrcSocket );
    closesocket( m_DstSocket );

    // 終了処理.
    WSACleanup();

    // フラグをクリア.
    m_IsConnected = false;
    m_IsReady     = false;
}

//-------------------------------------------------------------------------------------------------
//      接続しているかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool Connector::IsConnect()
{
    if ( m_DstSocket == INVALID_SOCKET )
    {
        m_IsConnected = false;
        return false;
    }

    // PINGデータを送って応答を確かめる.
    u32 ping[4] = { 0, 0, 0, 0 };
    return Write( ping, sizeof(u32) * 4 );
}

//-------------------------------------------------------------------------------------------------
//      送信処理を行います.
//-------------------------------------------------------------------------------------------------
bool Connector::Write( const void* pBuffer, u32 size )
{
    // 引数チェック.
    if ( pBuffer == nullptr || size == 0 )
    { return false; }

    // 送信処理.
    auto len = send( m_DstSocket, static_cast<const char*>(pBuffer), size, 0 );
    if ( len == SOCKET_ERROR )
    {
        auto code = WSAGetLastError();
        if ( code == WSAENOTCONN  ||
             code == WSAESHUTDOWN ||
             code == WSAENETDOWN)
        { m_IsConnected = false; }
        return false;
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      受信処理を行います.
//-------------------------------------------------------------------------------------------------
bool Connector::Read( void* pBuffer, u32 size )
{
    // 受信処理.
    auto status = recv( m_DstSocket, static_cast<char*>(pBuffer), size, 0 );

    // エラー.
    if ( status == SOCKET_ERROR )
    {
        ELOG( "Error : recv() Failed." );
        return false;
    }

    // ソケットが閉じられた場合.
    if ( status == 0 )
    {
        m_IsConnected = false;
        DLOG( "Error : Socket is already closed." );
        return false;
    }

    // 正常終了.
    return true;
}

} // namespace asdx
