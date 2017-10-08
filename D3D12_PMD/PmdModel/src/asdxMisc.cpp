//-------------------------------------------------------------------------------------------------
// File : asdxMisc.cpp
// Desc : Utility Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxMisc.h>
#include <cstdio>
#include <cassert>
#include <algorithm>
#include <shlwapi.h>
#include <d3d12.h>


//-------------------------------------------------------------------------------------------------
// Linker
//-------------------------------------------------------------------------------------------------
#pragma comment( lib, "shlwapi.lib" )


namespace asdx {

//-------------------------------------------------------------------------------------------------
//      ファイルパスを検索します.
//-------------------------------------------------------------------------------------------------
bool SearchFilePathW( const char16* filePath, std::wstring& result )
{
    if ( filePath == nullptr )
    { return false; }

    if ( wcscmp( filePath, L" " ) == 0 || wcscmp( filePath, L"" ) == 0 )
    { return false; }

    char16 exePath[ 520 ] = { 0 };
    GetModuleFileNameW( nullptr, exePath, 520  );
    exePath[ 519 ] = 0; // null終端化.
    PathRemoveFileSpecW( exePath );

    char16 dstPath[ 520 ] = { 0 };

    wcscpy_s( dstPath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"..\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"..\\..\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"\\res\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\..\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\..\\..\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\res\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    return false;
}


//-------------------------------------------------------------------------------------------------
//      ファイルパスを検索します.
//-------------------------------------------------------------------------------------------------
bool SearchFilePathA( const char8* filePath, std::string& result )
{
    if ( filePath == nullptr )
    { return false; }

    if ( strcmp( filePath, " " ) == 0 || strcmp( filePath, "" ) == 0 )
    { return false; }

    char8 exePath[ 256 ] = { 0 };
    GetModuleFileNameA( nullptr, exePath, 255 );
    exePath[ 255 ] = 0; // null終端化.
    PathRemoveFileSpecA( exePath );

    char8 dstPath[ 256 ] = { 0 };

    strcpy_s( dstPath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "..\\%s", filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "..\\..\\%s", filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "\\res\\%s", filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\..\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\..\\..\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\res\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    return false;
}


//-------------------------------------------------------------------------------------------------
//      ファイルパスからディレクトリ名を取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetDirectoryPathW( const char16* filePath )
{
    std::wstring path = filePath;
    auto idx = path.find_last_of( L"/" );
    if ( idx != std::wstring::npos )
    {
        auto result = path.substr( 0, idx + 1 );
        return result;
    }

    idx = path.find_last_of( L"\\" );
    if ( idx != std::wstring::npos )
    {
        auto result = path.substr( 0, idx + 1 );
        return result;
    }

    return std::wstring();
}


//-------------------------------------------------------------------------------------------------
//      ファイルパスからディレクトリ名を取得します.
//-------------------------------------------------------------------------------------------------
std::string GetDirectoryPathA( const char8* filePath )
{
    std::string path = filePath;
    auto idx = path.find_last_of( "/" );
    if ( idx != std::string::npos )
    {
        auto result = path.substr( 0, idx + 1 );
        return result;
    }

    idx = path.find_last_of( "\\" );
    if ( idx != std::string::npos )
    {
        auto result = path.substr( 0, idx + 1 );
        return result;
    }

    return std::string();
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスから拡張子を取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetExtW( const char16* filePath )
{
    std::wstring path = filePath;
    auto idx = path.find_last_of( L"." );
    if ( idx != std::wstring::npos )
    {
        std::wstring result = path.substr( idx + 1 );

        // 小文字化.
        std::transform( result.begin(), result.end(), result.begin(), tolower );

        return result;
    }

    return std::wstring();
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスから拡張子を取得します.
//-------------------------------------------------------------------------------------------------
std::string GetExtA( const char8* filePath )
{
    std::string path = filePath;
    auto idx = path.find_last_of( "." );
    if ( idx != std::string::npos )
    {
        std::string result = path.substr( idx + 1 );

        // 小文字化.
        std::transform( result.begin(), result.end(), result.begin(), tolower );

        return result;
    }

    return std::string();
}

//-------------------------------------------------------------------------------------------------
//      実行ファイルのファイルパスを取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetExePathW()
{
    char16 exePath[ 520 ] = { 0 };
    GetModuleFileNameW( nullptr, exePath, 520  );
    exePath[ 519 ] = 0; // null終端化.

    return asdx::GetDirectoryPathW( exePath );
}

//-------------------------------------------------------------------------------------------------
//      実行ファイルのファイルパスを取得します.
//-------------------------------------------------------------------------------------------------
std::string GetExePathA()
{
    char8 exePath[ 256 ] = { 0 };
    GetModuleFileNameA( nullptr, exePath, 255  );
    exePath[ 255 ] = 0; // null終端化.

    return asdx::GetDirectoryPathA( exePath );
}

//-------------------------------------------------------------------------------------------------
//      指定されたファイルパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFilePathW( const char16* filePath )
{
    if ( PathFileExistsW( filePath ) == TRUE )
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      指定されたファイルパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFilePathA( const char8* filePath )
{
    if ( PathFileExistsA( filePath ) == TRUE )
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      指定されたフォルダパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFolderPathW( const char16* folderPath )
{
    if ( PathFileExistsW ( folderPath ) == TRUE
      && PathIsDirectoryW( folderPath ) != FALSE ) // PathIsDirectoryW() は TRUE を返却しないので注意!!
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      指定されたフォルダパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFolderPathA( const char8* folderPath )
{
    if ( PathFileExistsA ( folderPath ) == TRUE
      && PathIsDirectoryA( folderPath ) != FALSE ) // PathIsDirectoryA() は TRUE を返却しないので注意!!
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      ワイド文字列に変換します.
//-------------------------------------------------------------------------------------------------
std::wstring ToStringW( const std::string& value )
{
    char16 wcs[256];
    memset( wcs, 0, sizeof(char16) * 256 );
    mbstowcs( wcs, value.c_str(), 256 );
    std::wstring result( wcs );
    return result;
}

//-------------------------------------------------------------------------------------------------
//      マルチバイト文字列に変換します.
//-------------------------------------------------------------------------------------------------
std::string ToStringA( const std::wstring& value )
{
    char8 mbs[256];
    memset( mbs, 0, sizeof(char8) * 256 );
    wcstombs( mbs, value.c_str(), 256 );
    std::string result( mbs );
    return result;
}

} // namespace asdx
