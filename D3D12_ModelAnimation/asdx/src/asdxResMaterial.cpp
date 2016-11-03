//-------------------------------------------------------------------------------------------------
// File : asdxResMaterial.cpp
// Desc : Project Asura MaterialSet Data Format (*.mts) Loader 
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxResMaterial.h>
#include <asdxLogger.h>
#include <asdxMisc.h>
#include "formats/asdxResMAT.h"


namespace asdx {

//-------------------------------------------------------------------------------------------------
//      マテリアルを生成します.
//-------------------------------------------------------------------------------------------------
bool MaterialFactory::Create( const char16* filename, ResMaterial* pResult )
{
    if ( filename == nullptr || pResult == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    auto ext = GetExt( filename );

    if ( ext == L"mat" )
    { return LoadResMaterialFromMAT( filename, pResult ); }

    ELOG( "Error : Invalid File Format. Extension is %s", ext.c_str() );
    return false;
}

//-------------------------------------------------------------------------------------------------
//      マテリアルセットを破棄します.
//-------------------------------------------------------------------------------------------------
void MaterialFactory::Dispose( ResMaterial*& ptr )
{
    if ( ptr == nullptr )
    { return; }

    ptr->Paths .clear();
    ptr->Phong .clear();
    ptr->Disney.clear();

    SafeDelete( ptr );
}

} // namespace asdx
