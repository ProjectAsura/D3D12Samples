//-----------------------------------------------------------------------------
// File : App.cpp
// Desc : Sample Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <App.h>
#include <fnd/asdxLogger.h>
#include <edit/asdxGuiMgr.h>
#include <gfx/asdxDevice.h>


namespace {


} // namespace


///////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
App::App()
: asdx::Application(L"TangentCompress", 1920, 1080, nullptr, nullptr, nullptr)
{
    m_SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_ClearDepth      = 0.0f;
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
App::~App()
{
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool App::OnInit()
{




    auto pos = asdx::Vector3(1000.0f, 100.0f, 0.0f);
    auto at  = asdx::Vector3(0.0f, 100.0f, 0.0f);
    auto up  = asdx::Vector3(0.0f, 1.0f, 0.0f);
    m_Camera.Init(pos, at, up, 0.1f, 10000.0f);
    m_Camera.SetMoveGain(0.1f);

    auto pCmd = m_GfxCmdList.Reset();

    if (!asdx::GuiMgr::Instance().Init(
        pCmd,
        m_hWnd,
        m_Width,
        m_Height,
        m_SwapChainFormat,
        "../res/fonts/07やさしさゴシック.ttf"))
    {
        ELOG("Error : GuiMgr::Init() Failed.");
        return false;
    }

    pCmd->Close();

    ID3D12CommandList* lists[] = {
        pCmd
    };

    auto queue = asdx::GetGraphicsQueue();
    queue->Execute(1, lists);
    m_WaitPoint = queue->Signal();
    queue->Sync(m_WaitPoint);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void App::OnTerm()
{
    asdx::GuiMgr::Instance().Term();
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void App::OnFrameRender(asdx::FrameEventArgs& args)
{
    // コマンド記録開始.
    auto pCmd = m_GfxCmdList.Reset();

    auto idx  = GetCurrentBackBufferIndex();


    {

    }



    // コマンド記録終了.
    pCmd->Close();

    auto pQueue = asdx::GetGraphicsQueue();

    // 前フレームのコマンドが終了していなければ待機.
    pQueue->Sync(m_WaitPoint);

    ID3D12CommandList* pLists[] = {
        pCmd
    };

    // コマンド実行.
    pQueue->Execute(_countof(pLists), pLists);

    // 待機点を記録.
    m_WaitPoint = pQueue->Signal();
    
    // 画面に表示.
    Present( 0 );
}

//-----------------------------------------------------------------------------
//      キー処理を行います.
//-----------------------------------------------------------------------------
void App::OnKey(const asdx::KeyEventArgs& args)
{
    m_Camera.OnKey(args.KeyCode, args.IsKeyDown, args.IsAltDown);
    asdx::GuiMgr::Instance().OnKey(args.IsKeyDown, args.IsAltDown, args.KeyCode);
}

//-----------------------------------------------------------------------------
//      マウス所入りを行います.
//-----------------------------------------------------------------------------
void App::OnMouse(const asdx::MouseEventArgs& args)
{
    bool isAltDown = GetAsyncKeyState(VK_MENU) & 0x1;
    if (isAltDown) {
        m_Camera.OnMouse(
            args.X,
            args.Y,
            args.WheelDelta,
            args.IsLeftButtonDown,
            args.IsRightButtonDown,
            args.IsMiddleButtonDown,
            args.IsSideButton1Down,
            args.IsSideButton2Down);
    }
    else
    {
        asdx::GuiMgr::Instance().OnMouse(
            args.X,
            args.Y,
            args.WheelDelta,
            args.IsLeftButtonDown,
            args.IsMiddleButtonDown,
            args.IsRightButtonDown);
    }
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void App::OnTyping(uint32_t keyCode)
{
    asdx::GuiMgr::Instance().OnTyping(keyCode);
}