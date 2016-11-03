//-------------------------------------------------------------------------------------------------
// File : asdxResMesh.cpp
// Desc : Resource Mesh Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxResMesh.h>
#include <asdxLogger.h>
#include <asdxMisc.h>
#include "formats/asdxResMSH.h"


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// MeshFactory class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      リソースメッシュを生成します.
//-------------------------------------------------------------------------------------------------
bool MeshFactory::Create( const char16* filename, ResMesh* pResult )
{
    if ( filename == nullptr || pResult == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    auto ext = GetExt( filename );

    if ( ext == L"msh" )
    { return LoadResMeshFromMSH( filename, pResult ); }

    ELOG( "Error : Invalid File FOrmat. Extension is %s", ext.c_str() );;
    return false;
}

//-------------------------------------------------------------------------------------------------
//      リソースメッシュを破棄します.
//-------------------------------------------------------------------------------------------------
void MeshFactory::Dispose( ResMesh*& ptr )
{
    if ( ptr == nullptr )
    { return; }

    ptr->Positions    .clear();
    ptr->Normals      .clear();
    ptr->TexCoords    .clear();
    ptr->BoneIndices  .clear();
    ptr->BoneWeights  .clear();
    ptr->VertexIndices.clear();
    ptr->Subsets      .clear();
    ptr->Bones        .clear();

    SafeDelete( ptr );
}

} // namespace asdx
