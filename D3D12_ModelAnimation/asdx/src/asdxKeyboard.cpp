//-------------------------------------------------------------------------------------------------
// File : asdxKeyboard.cpp
// Desc : Keybaord Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxHid.h>
#include <Windows.h>
#include <cstring>
#include <cassert>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Keyboard class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Keyboard::Keyboard()
: m_Index( 0 )
{ memset( m_Keys, 0, sizeof( bool ) * MAX_KEYS * 2 ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Keyboard::~Keyboard()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      キーの状態を更新します.
//-------------------------------------------------------------------------------------------------
void Keyboard::UpdateState()
{
    m_Index = 1 - m_Index;

    u8 keys[ MAX_KEYS ];
    GetKeyboardState( keys );
    for( u32 i=0; i<MAX_KEYS; ++i )
    { m_Keys[ m_Index ][ i ] = ( ( keys[ i ] & 0x80 ) != 0 ) ? true : false; }
}

//-------------------------------------------------------------------------------------------------
//      キーが押されたかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool Keyboard::IsPush( const u32 keyCode ) const
{
    assert( keyCode < MAX_KEYS );
    u32 idx = m_Index;
    u32 key = ConvertKey( keyCode );
    return m_Keys[ idx ][ key ];
}

//-------------------------------------------------------------------------------------------------
//      キーが押されっぱなしかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool Keyboard::IsDown( const u32 keyCode ) const
{
    assert( keyCode < MAX_KEYS );
    u32 idx = m_Index;
    u32 key = ConvertKey( keyCode );
    return m_Keys[ idx ][ key ] & ( !m_Keys[ 1 - idx ][ key ] );
}

//-------------------------------------------------------------------------------------------------
//      キーコードを変換します.
//-------------------------------------------------------------------------------------------------
u32 Keyboard::ConvertKey( const u32 keyCode ) const
{
    u32 key = keyCode;
    u32 result = 0;
    if ( key >= 'a' && key <= 'z' )
    { key = 'A' + ( key - 'a' ); }

    bool isAlphaBet = ( key >= 'A' && key <= 'Z' );
    bool isNumber = ( key >= '0' && key <= '9' );
    if ( isAlphaBet || isNumber )
    { result = key; }

    if ( result == 0 )
    {
        switch( key )
        {
            case ' ':               { result = VK_SPACE; }      break;
            case '+':               { result = VK_OEM_PLUS; }   break;
            case ',':               { result = VK_OEM_COMMA; }  break;
            case '-':               { result = VK_OEM_MINUS; }  break;
            case '.':               { result = VK_OEM_PERIOD; } break;
            case asdx::KEY_RETURN:  { result = VK_RETURN; }     break;
            case asdx::KEY_TAB:     { result = VK_TAB; }        break;
            case asdx::KEY_ESC:     { result = VK_ESCAPE; }     break;
            case asdx::KEY_BACK:    { result = VK_BACK; }       break;
            case asdx::KEY_SHIFT:   { result = VK_SHIFT; }      break;
            case asdx::KEY_CONTROL: { result = VK_CONTROL; }    break;
            case asdx::KEY_ALT:     { result = VK_MENU; }       break;
            case asdx::KEY_F1:      { result = VK_F1; }         break;
            case asdx::KEY_F2:      { result = VK_F2; }         break;
            case asdx::KEY_F3:      { result = VK_F3; }         break;
            case asdx::KEY_F4:      { result = VK_F4; }         break;
            case asdx::KEY_F5:      { result = VK_F5; }         break;
            case asdx::KEY_F6:      { result = VK_F6; }         break;
            case asdx::KEY_F7:      { result = VK_F7; }         break;
            case asdx::KEY_F8:      { result = VK_F8; }         break;
            case asdx::KEY_F9:      { result = VK_F9; }         break;
            case asdx::KEY_F10:     { result = VK_F10; }        break;
            case asdx::KEY_F11:     { result = VK_F11; }        break;
            case asdx::KEY_F12:     { result = VK_F12; }        break;
            case asdx::KEY_UP:      { result = VK_UP; }         break;
            case asdx::KEY_DOWN:    { result = VK_DOWN; }       break;
            case asdx::KEY_LEFT:    { result = VK_LEFT; }       break;
            case asdx::KEY_RIGHT:   { result = VK_RIGHT; }      break;
            case asdx::KEY_NUM0:    { result = VK_NUMPAD0; }    break;
            case asdx::KEY_NUM1:    { result = VK_NUMPAD1; }    break;
            case asdx::KEY_NUM2:    { result = VK_NUMPAD2; }    break;
            case asdx::KEY_NUM3:    { result = VK_NUMPAD3; }    break;
            case asdx::KEY_NUM4:    { result = VK_NUMPAD4; }    break;
            case asdx::KEY_NUM5:    { result = VK_NUMPAD5; }    break;
            case asdx::KEY_NUM6:    { result = VK_NUMPAD6; }    break;
            case asdx::KEY_NUM7:    { result = VK_NUMPAD7; }    break;
            case asdx::KEY_NUM8:    { result = VK_NUMPAD8; }    break;
            case asdx::KEY_NUM9:    { result = VK_NUMPAD9; }    break;
            case asdx::KEY_INSERT:  { result = VK_INSERT; }     break;
            case asdx::KEY_DELETE:  { result = VK_DELETE; }     break;
            case asdx::KEY_HOME:    { result = VK_HOME; }       break;
            case asdx::KEY_END:     { result = VK_END; }        break;
        }
    }

    return result;
}

} // namespace asdx
