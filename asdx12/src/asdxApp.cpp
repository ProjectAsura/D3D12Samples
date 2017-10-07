//-------------------------------------------------------------------------------------------------
// File : asdxDesktopApp.cpp
// Desc : DeskTop Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <list>
#include <cassert>
#include <asdxApp.h>
#include <asdxLogger.h>


namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////////////////////////////////
// AppList class
///////////////////////////////////////////////////////////////////////////////////////////////////
class AppList
{
public:
    typedef std::list<asdx::AppBase*>                    app_list;
    typedef std::list<asdx::AppBase*>::iterator          iter;
    typedef std::list<asdx::AppBase*>::const_iterator    const_iter;

    AppList ()    { m_List.clear(); }
    ~AppList()    { m_List.clear(); }

    void        remove      ( asdx::AppBase* app )   { m_List.remove   ( app ); }
    void        push_back   ( asdx::AppBase* app)    { m_List.push_back( app ); }
    void        pop_back    ()                          { m_List.pop_back    (); }
    void        pop_front   ()                          { m_List.pop_front   (); }
    void        clear       ()                          { m_List.clear       (); }
    iter        begin       ()                          { return m_List.begin(); }
    const_iter  begin       () const                    { return m_List.begin(); }
    iter        end         ()                          { return m_List.end  (); }
    const_iter  end         () const                    { return m_List.end  (); }

private:
    app_list   m_List;
};

//-------------------------------------------------------------------------------------------------
// Global Variables.
//-------------------------------------------------------------------------------------------------
AppList             g_AppList;

//-------------------------------------------------------------------------------------------------
// Constant Values
//-------------------------------------------------------------------------------------------------
static const LPCWSTR WndClassName = L"asdxWindowClass";

} // namespace /* anonymous */


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// AppBase class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
AppBase::AppBase
(
    LPWSTR  title,
    int     width,
    int     height,
    HICON   hIcon,
    HMENU   hMenu,
    HACCEL  hAccel
)
: m_hInst               ( nullptr )
, m_hWnd                ( nullptr )
, m_StepTimer           ()
, m_Title               ( title )
, m_hIcon               ( hIcon )
, m_hMenu               ( hMenu )
, m_hAccel              ( hAccel )
, m_Width               ( width )
, m_Height              ( height )
, m_AspectRatio         ( float( width ) / float( height ) )
, m_FrameCount          ( 0 )
, m_FramePerSec         ( 0.0f )
, m_LastUpdateSec       ( 0 )
, m_IsStopDraw          ( false )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
AppBase::~AppBase()
{ TermApp(); }

//-------------------------------------------------------------------------------------------------
//      描画停止フラグを設定します.
//-------------------------------------------------------------------------------------------------
void AppBase::StopDraw( bool isStopDraw )
{ m_IsStopDraw = isStopDraw; }

//-------------------------------------------------------------------------------------------------
//      描画停止フラグを取得します.
//-------------------------------------------------------------------------------------------------
bool AppBase::IsStopDraw() const
{ return m_IsStopDraw; }

//-------------------------------------------------------------------------------------------------
//      フレームカウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t AppBase::GetFrameCount() const
{ return m_FrameCount; }

//-------------------------------------------------------------------------------------------------
//      FPSを取得します.
//-------------------------------------------------------------------------------------------------
float AppBase::GetFramePerSec() const
{ return m_FramePerSec; }

//-------------------------------------------------------------------------------------------------
//      アプリを初期化します.
//-------------------------------------------------------------------------------------------------
bool AppBase::InitApp()
{
    // COMライブラリを初期化します.
    auto hr = CoInitialize( nullptr );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : Com Library Initialize Failed." );
        return false;
    }

    // COMライブラリのセキュリティレベルを設定.
    hr = CoInitializeSecurity(
        nullptr,
        -1,
        nullptr,
        nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE,
        nullptr);
    if ( FAILED( hr ) )
    {
        ELOG( "Error : Com Librarary Initialize Secutrity Failed." );
        return false;
    }

    // ウィンドウの初期化.
    if ( !InitWnd() )
    {
        ELOG( "Error : InitWnd() Failed." );
        return false;
    }

    // アプリケーション固有の初期化.
    if ( !OnInit() )
    {
        ELOG( "Error : OnInit() Failed." );
        return false;
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      アプリケーションの終了処理.
//-------------------------------------------------------------------------------------------------
void AppBase::TermApp()
{
    // アプリケーション固有の終了処理.
    OnTerm();

    // ウィンドウの終了処理.
    TermWnd();

    // COMライブラリの終了処理.
    CoUninitialize();
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウの初期化処理.
//-------------------------------------------------------------------------------------------------
bool AppBase::InitWnd()
{
    // インスタンスハンドルを取得.
    auto hInst = GetModuleHandle( nullptr );
    if ( hInst == nullptr )
    {
        ELOG( "Error : GetModuleHandle() Failed." );
        return false;
    }

    // アイコンなしの場合はデフォルトのアイコンを使用.
    if ( m_hIcon == nullptr )
    { m_hIcon = LoadIcon( hInst, IDI_APPLICATION ); }

    // 拡張ウィンドウクラスの設定.
    WNDCLASSEX wc;
    wc.cbSize           = sizeof(wc);
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MsgProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = m_hIcon;
    wc.hCursor          = LoadCursor( nullptr, IDC_ARROW );
    wc.hbrBackground    = (HBRUSH)( COLOR_WINDOW + 1 );
    wc.lpszMenuName     = nullptr;
    wc.lpszClassName    = WndClassName;
    wc.hIconSm          = m_hIcon;

    // ウィンドウクラスを登録します.
    if ( !RegisterClassExW( &wc ) )
    {
        ELOG( "Error : RegisterClassExW() Failed." );
        return false;
    }

    // インスタンスハンドルを設定.
    m_hInst = hInst;

    // 矩形を設定.
    RECT rc = {  0, 0, LONG(m_Width), LONG(m_Height) };

    // 指定されたクライアント領域を確保するために必要なウィンドウ座標を計算します.
    auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
    AdjustWindowRect( &rc, style, FALSE );

    // ウィンドウを生成.
    m_hWnd = CreateWindowW(
        WndClassName,
        m_Title,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (rc.right - rc.left),
        (rc.bottom - rc.top),
        nullptr,
        m_hMenu,
        m_hInst,
        nullptr );
    if ( m_hWnd == nullptr )
    {
        ELOG( "Error : CreateWindowW() Failed" );
        return false;
    }

    // ウィンドウを表示します.
    ShowWindow( m_hWnd, SW_SHOWNORMAL );
    UpdateWindow( m_hWnd );

    // フォーカスを設定.
    SetFocus( m_hWnd );

    // アプリケーション管理リストに登録.
    g_AppList.push_back( this );

    // タイマーを開始.
    m_StepTimer.Start();

    // 開始時刻を取得.
    m_LastUpdateSec = m_StepTimer.GetElapsedSec();

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウの終了処理.
//-------------------------------------------------------------------------------------------------
void AppBase::TermWnd()
{
    m_StepTimer.Stop();

    // ウィンドウクラスの登録を解除.
    if ( m_hInst != nullptr )
    { UnregisterClassW( WndClassName, m_hInst ); }

    // アクセレレータテーｂるを破棄.
    if ( m_hAccel != nullptr )
    { DestroyAcceleratorTable( m_hAccel ); }

    // アイコンを破棄.
    if ( m_hIcon != nullptr )
    { DestroyIcon( m_hIcon ); }

    // ポインタクリア.
    m_Title  = nullptr;
    m_hInst  = nullptr;
    m_hWnd   = nullptr;
    m_hIcon  = nullptr;
    m_hMenu  = nullptr;
    m_hAccel = nullptr;

    // アプリケーション管理リストから削除.
    g_AppList.remove( this );
}

//-------------------------------------------------------------------------------------------------
//      メインループ処理.
//-------------------------------------------------------------------------------------------------
void AppBase::MainLoop()
{
    MSG msg = { 0 };

    FrameEventArgs args;
    auto frameCount = 0;

    while( WM_QUIT != msg.message )
    {
        auto hasMsg = PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE );
        if ( hasMsg )
        {
            auto ret = TranslateAccelerator( m_hWnd, m_hAccel, &msg );
            if ( 0 == ret )
            {
                TranslateMessage( &msg );
                DispatchMessage ( &msg );
            }
        }
        else
        {
            double uptimeSec;
            double absTimeSec;
            double elapsedTimeSec;
            m_StepTimer.GetValues( uptimeSec, absTimeSec, elapsedTimeSec );

            auto interval = float( uptimeSec - m_LastUpdateSec );
            if ( interval > 0.5 )
            {
                m_FramePerSec   = frameCount / interval;
                m_LastUpdateSec = uptimeSec;
                frameCount      = 0;
            }

            args.UpTimeSec   = uptimeSec;
            args.FramePerSec = 1.0f / static_cast<float>(elapsedTimeSec);
            args.ElapsedSec  = elapsedTimeSec;
            args.IsStopDraw  = m_IsStopDraw;

            OnFrameMove( args );

            if ( !IsStopDraw() )
            {
                OnFrameRender( args );
                m_FrameCount++;
            }

            frameCount++;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      アプリケーションを実行します.
//-------------------------------------------------------------------------------------------------
void AppBase::Run()
{
    if ( InitApp() )
    { MainLoop(); }

    TermApp();
}

//-------------------------------------------------------------------------------------------------
//      キー処理.
//-------------------------------------------------------------------------------------------------
void AppBase::DoKeyEvent( const KeyEventArgs& args )
{ OnKey( args ); }

//-------------------------------------------------------------------------------------------------
//      リサイズ処理.
//-------------------------------------------------------------------------------------------------
void AppBase::DoResizeEvent( const ResizeEventArgs& args )
{
    // パラメータ設定.
    m_Width       = args.Width;
    m_Height      = args.Height;
    m_AspectRatio = args.AspectRatio;

    OnResize( args );
}

//-------------------------------------------------------------------------------------------------
//      マウス処理.
//-------------------------------------------------------------------------------------------------
void AppBase::DoMouseEvent( const MouseEventArgs& args )
{ OnMouse( args ); }

//-------------------------------------------------------------------------------------------------
//      ドロップ時の処理.
//-------------------------------------------------------------------------------------------------
void AppBase::DoDropEvent( const DropEventArgs& args )
{ OnDrop( args ); }

//-------------------------------------------------------------------------------------------------
//      ウィンドウプロシージャです.
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK AppBase::MsgProc( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp )
{
    if ( ( msg == WM_KEYDOWN )
      || ( msg == WM_SYSKEYDOWN )
      || ( msg == WM_KEYUP )
      || ( msg == WM_SYSKEYUP ) )
    {
        auto isKeyDown = ( msg == WM_KEYDOWN ) || ( msg == WM_SYSKEYDOWN );
        uint32_t mask = ( 1 << 29 );
        auto isAltDown = ( ( lp & mask ) != 0 );

        KeyEventArgs args;
        args.KeyCode   = uint32_t( wp );
        args.IsAltDown = isAltDown;
        args.IsKeyDown = isKeyDown;

        for( auto app : g_AppList )
        { app->DoKeyEvent( args ); }
    }

    const UINT OLD_WM_MOUSEWHEEL = 0x020A;

    if ( ( msg == WM_LBUTTONDOWN ) 
      || ( msg == WM_LBUTTONUP )
      || ( msg == WM_LBUTTONDBLCLK )
      || ( msg == WM_MBUTTONDOWN )
      || ( msg == WM_MBUTTONUP )
      || ( msg == WM_MBUTTONDBLCLK )
      || ( msg == WM_RBUTTONDOWN )
      || ( msg == WM_RBUTTONUP )
      || ( msg == WM_RBUTTONDBLCLK )
      || ( msg == WM_XBUTTONDOWN )
      || ( msg == WM_XBUTTONUP )
      || ( msg == WM_XBUTTONDBLCLK )
      || ( msg == WM_MOUSEHWHEEL )
      || ( msg == WM_MOUSEMOVE )
      || ( msg == OLD_WM_MOUSEWHEEL ) )
    {
        auto x = static_cast<int>( (int16_t)LOWORD( lp ) );
        auto y = static_cast<int>( (int16_t)HIWORD( lp ) );

        auto wheelDelta = 0;
        POINT pt = { x, y };
        ScreenToClient( hWnd, &pt );
        x = pt.x;
        y = pt.y;

        if ( ( msg == WM_MOUSEHWHEEL ) || ( msg == OLD_WM_MOUSEWHEEL ) )
        {
            wheelDelta += static_cast<int16_t>( wp );
        }

        auto state = LOWORD( wp );
        auto isDownL  = ( ( state & MK_LBUTTON )  != 0 );
        auto isDownR  = ( ( state & MK_RBUTTON )  != 0 );
        auto isDownM  = ( ( state & MK_MBUTTON )  != 0 );
        auto isDownX1 = ( ( state & MK_XBUTTON1 ) != 0 );
        auto isDownX2 = ( ( state & MK_XBUTTON2 ) != 0 );

        MouseEventArgs args;
        args.CursorX      = x;
        args.CursorY      = y;
        args.IsLeftDown   = isDownL;
        args.IsMiddleDown = isDownM;
        args.IsRightDown  = isDownR;
        args.IsSide1Down  = isDownX1;
        args.IsSide2Down  = isDownX2;

        for( auto app : g_AppList )
        { app->DoMouseEvent( args ); }
    }

    switch( msg )
    {
    case WM_CREATE:
        { DragAcceptFiles( hWnd, TRUE ); }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
        }
        break;

    case WM_DESTROY:
        { PostQuitMessage( 0 ); }
        break;

    case WM_SIZE:
        {
            auto w = static_cast<uint32_t>( LOWORD( lp ) );
            auto h = static_cast<uint32_t>( HIWORD( lp ) );

            ResizeEventArgs args;
            args.Width  = ( w > 8 ) ? w : 8;        // マルチサンプリングの対策.
            args.Height = ( h > 8 ) ? h : 8;        // マルチサンプリングの対策.
            args.AspectRatio = float( args.Width ) / float( args.Height );

            for( auto app : g_AppList )
            { app->DoResizeEvent( args ); }
        }
        break;

    case WM_DROPFILES:
        {
            auto fileCount = DragQueryFileW( (HDROP)wp, 0xFFFFFFFF, nullptr, 0 );
            if (fileCount <= 0)
            { break; }

            DropEventArgs args;
            args.Files = new wchar_t* [fileCount];
            args.FileCount = fileCount;
            for( uint32_t i=0; i<fileCount; ++i )
            {
                wchar_t* file = new wchar_t [ MAX_PATH ];
                DragQueryFileW( (HDROP)wp, i, file, MAX_PATH );
                args.Files[i] = file;
            }

            for ( auto app : g_AppList )
            { app->DoDropEvent( args ); }

            for( uint32_t i=0; i<fileCount; ++i )
            {
                if( args.Files[i] != nullptr)
                {
                    delete[] args.Files[i];
                    args.Files[i] = nullptr;
                }
            }

            if (args.Files != nullptr)
            {
                delete[] args.Files;
                args.Files = nullptr;
            }

            DragFinish( (HDROP)wp );
        }
        break;
    }

    for( auto app : g_AppList )
    { app->OnMsgProc( hWnd, msg, wp, lp ); }

    return DefWindowProc( hWnd, msg, wp, lp );
}

//-------------------------------------------------------------------------------------------------
//      アプリケーション固有の初期化時の処理.
//-------------------------------------------------------------------------------------------------
bool AppBase::OnInit()
{
    /* DO_NOTHING */
    return true;
}

//-------------------------------------------------------------------------------------------------
//      アプリケーション固有の終了時の処理.
//-------------------------------------------------------------------------------------------------
void AppBase::OnTerm()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      フレーム遷移時の処理.
//-------------------------------------------------------------------------------------------------
void AppBase::OnFrameMove( const FrameEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      フレーム描画時の処理.
//-------------------------------------------------------------------------------------------------
void AppBase::OnFrameRender( const FrameEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      リサイズ時の処理
//-------------------------------------------------------------------------------------------------
void AppBase::OnResize( const ResizeEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      キーの処理
//-------------------------------------------------------------------------------------------------
void AppBase::OnKey( const KeyEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      マウスの処理.
//-------------------------------------------------------------------------------------------------
void AppBase::OnMouse( const MouseEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      ドロップ時の処理.
//-------------------------------------------------------------------------------------------------
void AppBase::OnDrop( const DropEventArgs& )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      メッセージプロシージャです.
//-------------------------------------------------------------------------------------------------
void AppBase::OnMsgProc( HWND, UINT, WPARAM, LPARAM )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      フォーカスを持つかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool AppBase::HasFocus() const
{ return ( GetActiveWindow() == m_hWnd ); }


} // namespace asdx
