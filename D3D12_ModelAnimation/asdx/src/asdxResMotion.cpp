//-------------------------------------------------------------------------------------------------
// File : asdxResMotion.cpp
// Desc : Resource Motion Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxResMotion.h>
#include <asdxLogger.h>
#include <asdxMisc.h>
#include "formats/asdxResMTN.h"


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// MotionFactory class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      モーションリソースを生成します.
//-------------------------------------------------------------------------------------------------
bool MotionFactory::Create( const char16* filename, ResMotion* pResult )
{
    if ( filename == nullptr || pResult == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    auto ext = GetExt( filename );

    if ( ext == L"mtn" )
    { return LoadResMotionFromMTN( filename, pResult ); }

    ELOG( "Error : Invalid File Format. Extension is %s", ext.c_str() );
    return false;
}

//-------------------------------------------------------------------------------------------------
//      モーションリソースを破棄します.
//-------------------------------------------------------------------------------------------------
void MotionFactory::Dispose( ResMotion*& ptr )
{
    if ( ptr == nullptr )
    { return; }

    for( size_t i=0; i<ptr->Bones.size(); ++i )
    {
        ptr->Bones[i].KeyFrames.clear();
    }
    ptr->Bones.clear();

    SafeDelete( ptr );
}

} // namespace asdx
