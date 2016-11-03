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
bool SearchFilePath( const char16* filePath, std::wstring& result )
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
//      ファイルパスからディレクトリ名を取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetDirectoryPath( const char16* filePath )
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
//      実行ファイルのファイルパスを取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetExePath()
{
    char16 exePath[ 520 ] = { 0 };
    GetModuleFileNameW( nullptr, exePath, 520  );
    exePath[ 519 ] = 0; // null終端化.

    return asdx::GetDirectoryPath( exePath );
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスから拡張子を取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetExt( const char16* filePath )
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
//      指定されたファイルパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFilePath( const char16* filePath )
{
    if ( PathFileExistsW( filePath ) == TRUE )
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      指定されたフォルダパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFolderPath( const char16* folderPath )
{
    if ( PathFileExistsW ( folderPath ) == TRUE
      && PathIsDirectoryW( folderPath ) != FALSE ) // PathIsDirectoryW() は TRUE を返却しないので注意!!
    { return true; }

    return false;
}

} // namespace asdx
