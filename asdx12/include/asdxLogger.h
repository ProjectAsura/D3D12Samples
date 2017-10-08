//-------------------------------------------------------------------------------------------------
// File : asdxLogger.h
// Desc : Logger Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


#ifndef __ASDX_WIDE
#define __ASDX_WIDE( _string )      L ## _string
#endif//__ASDX_WIDE


#ifndef ASDX_WIDE
#define ASDX_WIDE( _string )        __ASDX_WIDE( _string )
#endif//ASDX_WIDE


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// LogLevel enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum LogLevel
{
    Verbose = 0,          //!< VERBOSEレベル (白).
    Info,                 //!< INFOレベル    (緑).
    Debug,                //!< DEBUGレベル   (青).
    Warning,              //!< WRARNINGレベル(黄).
    Error,                //!< ERRORレベル   (赤).
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// ILogger interface
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ILogger
{
    virtual ~ILogger()
    { /* DO_NOTHING */}

    virtual void LogA( const LogLevel level, const char* format, ... ) = 0;
    virtual void LogW( const LogLevel level, const wchar_t* format, ... ) = 0;
    virtual void SetFilter( const LogLevel filter ) = 0;
    virtual LogLevel GetFilter() = 0;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// SystemLogger class
///////////////////////////////////////////////////////////////////////////////////////////////////
class SystemLogger : public ILogger
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    static SystemLogger& GetInstance();
    void LogA( const LogLevel level, const char* format, ... ) override;
    void LogW( const LogLevel level, const wchar_t* format, ... ) override;
    void SetFilter( const LogLevel filter ) override;
    LogLevel GetFilter() override;

protected:
    //=============================================================================================
    // protected variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // protected methods.
    //=============================================================================================
    /* NOTHING */

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    static SystemLogger     s_Instance;     //!< シングルトンインスタンスです.
    LogLevel                m_Filter;       //!< フィルターです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    SystemLogger();
};


} // namespace asdx


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#ifndef DLOGA
  #if defined(DEBUG) || defined(_DEBUG)
    #define DLOGA( fmt, ... )      asdx::SystemLogger::GetInstance().LogA( asdx::LogLevel::Debug, "[File: %s, Line: %d] "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS__ )
  #else
    #define DLOGA( fmt, ... )      ((void)0)
  #endif//defined(DEBUG) || defined(_DEBUG)
#endif//DLOGA

#ifndef DLOGW
  #if defined(DEBUG) || defined(_DEBUG)
    #define DLOGW( fmt, ... )      asdx::SystemLogger::GetInstance().LogW( asdx::LogLevel::Debug, ASDX_WIDE("[File: %s, Line: %d] ") ASDX_WIDE(fmt) ASDX_WIDE("\n"), ASDX_WIDE(__FILE__), __LINE__, ##__VA_ARGS__ )
  #else
    #define DLOGW( fmt, ... )      ((void)0)
  #endif//defined(DEBUG) || defined(_DEBUG)
#endif//DLOGW

#ifndef ILOGA
#define ILOGA( fmt, ... )      asdx::SystemLogger::GetInstance().LogA( asdx::LogLevel::Info, fmt "\n", ##__VA_ARGS__ )
#endif//ILOGA

#ifndef ILOGW
#define ILOGW( fmt, ... )      asdx::SystemLogger::GetInstance().LogW( asdx::LogLevel::Info, ASDX_WIDE(fmt) ASDX_WIDE("\n"), ##__VA_ARGS__ );
#endif//ILOGW

#ifndef ELOGA
#define ELOGA( fmt, ... )      asdx::SystemLogger::GetInstance().LogA( asdx::LogLevel::Error, "[File: %s, Line: %d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOGA

#ifndef ELOGW
#define ELOGW( fmt, ... )      asdx::SystemLogger::GetInstance().LogW( asdx::LogLevel::Error, ASDX_WIDE("[File: %s, Line: %d] ") ASDX_WIDE(fmt) ASDX_WIDE("\n"), ASDX_WIDE(__FILE__), __LINE__, ##__VA_ARGS__ )
#endif//ELOGW

#if defined(UNICODE) || defined(_UNICODE)
    #define DLOG        DLOGW
    #define ILOG        ILOGW
    #define ELOG        ELOGW
#else
    #define DLOG        DLOGA
    #define ILOG        ILOGA
    #define ELOG        ELOGA
#endif

