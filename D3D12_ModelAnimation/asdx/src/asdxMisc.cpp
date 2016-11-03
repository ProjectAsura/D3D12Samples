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
#include <locale>
#include <codecvt>


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
//      拡張子を取り除いたファイルパスを取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetPathWithoutExt( const char16* filePath )
{
    std::wstring path = filePath;
    auto idx = path.find_last_of( L"." );
    if ( idx != std::wstring::npos )
    {
        return path.substr( 0, idx );
    }

    return path;
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

//-------------------------------------------------------------------------------------------------
//      ワイド文字列に変換します.
//-------------------------------------------------------------------------------------------------
std::wstring ToStringW( const std::string& value )
{
    std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>,wchar_t> cv;
    return cv.from_bytes(value);
}

//-------------------------------------------------------------------------------------------------
//      マルチバイト文字列に変換します.
//-------------------------------------------------------------------------------------------------
std::string ToStringA( const std::wstring& value )
{
    // UTF8 に変換.
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cv;
    auto srcUTF8 = cv.to_bytes(value);

    auto lenghtUnicode = MultiByteToWideChar(CP_UTF8, 0, srcUTF8.c_str(),int(srcUTF8.size()) + 1, nullptr, 0);
 
    //必要な分だけUnicode文字列のバッファを確保
    auto bufUnicode = new wchar_t[lenghtUnicode];
 
    //UTF8からUnicodeへ変換
    MultiByteToWideChar(CP_UTF8, 0, srcUTF8.c_str(), int(srcUTF8.size()) + 1, bufUnicode, lenghtUnicode);
 
    //ShiftJISへ変換後の文字列長を得る
    auto lengthSJis = WideCharToMultiByte(CP_THREAD_ACP, 0, bufUnicode, -1, nullptr, 0, nullptr, nullptr);
 
    //必要な分だけShiftJIS文字列のバッファを確保
    auto bufShiftJis = new char[lengthSJis];
 
    //UnicodeからShiftJISへ変換
    WideCharToMultiByte(CP_THREAD_ACP, 0, bufUnicode, lenghtUnicode + 1, bufShiftJis, lengthSJis, nullptr, nullptr);
 
    std::string strSJis(bufShiftJis);
 
    // 不要なメモリを解放.
    delete[] bufUnicode;
    delete[] bufShiftJis;

    return strSJis;
}

} // namespace asdx
