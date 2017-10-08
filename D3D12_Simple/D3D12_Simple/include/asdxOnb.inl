//----------------------------------------------------------------------------------
// File : asdxOnb.inl
// Desc : Asura Orthonormal Basis Module.
// Copyright(c) Project Asura. All right reserved.
//----------------------------------------------------------------------------------

#ifndef __ASDX_ONB_INL__
#define __ASDX_ONB_INL__


namespace /* anonymous */ {

//-----------------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------------
static const f32 ONB_EPSILON = 0.01f;

} // namespace /* anonymous */


namespace asdx {

////////////////////////////////////////////////////////////////////////////////////
// OrthonormalBasis structure
////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------------
//      コンストラクタです.
//----------------------------------------------------------------------------------
ASDX_INLINE
OrthonormalBasis::OrthonormalBasis()
{ /* DO_NOTHING */ }

//----------------------------------------------------------------------------------
//      引数付きコンストラクタです.
//----------------------------------------------------------------------------------
ASDX_INLINE
OrthonormalBasis::OrthonormalBasis
(
    const Vector3& nu,
    const Vector3& nv,
    const Vector3& nw 
)
: u( nu )
, v( nv )
, w( nw )
{ /* DO_NOTHING */ }

//----------------------------------------------------------------------------------
//      コピーコンストラクタです.
//----------------------------------------------------------------------------------
ASDX_INLINE
OrthonormalBasis::OrthonormalBasis( const OrthonormalBasis& value )
: u( value.u )
, v( value.v )
, w( value.w )
{ /* DO_NOTHING */ }

//----------------------------------------------------------------------------------
//      U方向から基底を構築します.
//----------------------------------------------------------------------------------
ASDX_INLINE
void OrthonormalBasis::InitFromU( const Vector3& value )
{
    Vector3 n( 1.0f, 0.0f, 0.0f );
    Vector3 m( 0.0f, 1.0f, 0.0f );
    
    u = Vector3::Normalize( value );
    v = Vector3::Cross( u, n );
    if ( v.Length() < ONB_EPSILON )
    { v = Vector3::Cross( u, m ); }
    w = Vector3::Cross( v, u );
}

//----------------------------------------------------------------------------------
//      V方向から基底を構築します.
//----------------------------------------------------------------------------------
ASDX_INLINE
void OrthonormalBasis::InitFromV( const Vector3& value )
{
    Vector3 n ( 1.0f, 0.0f, 0.0f );
    Vector3 m ( 0.0f, 1.0f, 0.0f );

    v = Vector3::Normalize( value );
    u = Vector3::Cross( v, n );
    if ( u.LengthSq() < ONB_EPSILON )
    { u = Vector3::Cross( v, m ); }
    w = Vector3::Cross( v, u );
}

//----------------------------------------------------------------------------------
//      W方向から基底を構築します.
//----------------------------------------------------------------------------------
ASDX_INLINE
void OrthonormalBasis::InitFromW( const Vector3& value )
{
    Vector3 n ( 1.0f, 0.0f, 0.0f );
    Vector3 m ( 0.0f, 1.0f, 0.0f );

    w = Vector3::Normalize( value );
    u = Vector3::Cross( w, n );
    if ( u.Length() < ONB_EPSILON )
    { u = Vector3::Cross( w, m ); }
    u.Normalize();

    v = Vector3::Cross( u, w );
    v.Normalize();
}

//----------------------------------------------------------------------------------
//      U方向とV方向から基底を構築します.
//----------------------------------------------------------------------------------
ASDX_INLINE
void OrthonormalBasis::InitFromUV( const Vector3& _u, const Vector3& _v )
{
    u = Vector3::Normalize( _u );
    w = Vector3::Normalize( Vector3::Cross( _u, _v ) );
    v = Vector3::Cross( w, v );
}

//----------------------------------------------------------------------------------
//      V方向とU方向から基底を構築します.
//----------------------------------------------------------------------------------
ASDX_INLINE
void OrthonormalBasis::InitFromVU( const Vector3& _v, const Vector3& _u )
{
    v = Vector3::Normalize( _v );
    w = Vector3::Normalize( Vector3::Cross( _u, _v ) );
    u = Vector3::Cross( v, w );
}

//----------------------------------------------------------------------------------
//      U方向とW方向から基底を構築します.
//----------------------------------------------------------------------------------
ASDX_INLINE
void OrthonormalBasis::InitFromUW( const Vector3& _u, const Vector3& _w )
{
    u = Vector3::Normalize( _u );
    v = Vector3::Normalize( Vector3::Cross( _w, _u ) );
    w = Vector3::Cross( u, v );
}

//----------------------------------------------------------------------------------
//      W方向とU方向から基底を構築します.
//----------------------------------------------------------------------------------
ASDX_INLINE
void OrthonormalBasis::InitFromWU( const Vector3& _w, const Vector3& _u )
{
    w = Vector3::Normalize( _w );
    v = Vector3::Normalize( Vector3::Cross( _w, _u ) );
    u = Vector3::Cross( v, w );
}

//----------------------------------------------------------------------------------
//      V方向とW方向から基底を構築します.
//----------------------------------------------------------------------------------
ASDX_INLINE
void OrthonormalBasis::InitFromVW( const Vector3& _v, const Vector3& _w )
{
    v = Vector3::Normalize( _v );
    u = Vector3::Normalize( Vector3::Cross( _v, _w ) );
    w = Vector3::Cross( u, v );
}

//----------------------------------------------------------------------------------
//      W方向とV方向から基底を構築します.
//----------------------------------------------------------------------------------
ASDX_INLINE
void OrthonormalBasis::InitFromWV( const Vector3& _w, const Vector3& _v )
{
    w = Vector3::Normalize( _w );
    u = Vector3::Normalize( Vector3::Cross( _v, _w ) );
    v = Vector3::Cross( w, u );
}

//----------------------------------------------------------------------------------
//      等価演算子です.
//----------------------------------------------------------------------------------
ASDX_INLINE
bool OrthonormalBasis::operator == ( const OrthonormalBasis& value ) const
{
    return ( u == value.u )
        && ( v == value.v )
        && ( w == value.w );
}

//----------------------------------------------------------------------------------
//      非等価演算子です.
//----------------------------------------------------------------------------------
ASDX_INLINE
bool OrthonormalBasis::operator != ( const OrthonormalBasis& value ) const
{
    return ( u != value.u )
        || ( v != value.v )
        || ( w != value.w );
}

} // namespace asdx

#endif//__ASDX_ONB_INL__
