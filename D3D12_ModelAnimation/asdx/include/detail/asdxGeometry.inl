//-------------------------------------------------------------------------------------------------
// File : asdxGeometry.inl
// Desc : Geometry Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Ray structure
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
Ray::Ray( const Vector3& position, const Vector3& direction )
{ Update( position, direction ); }

//-------------------------------------------------------------------------------------------------
//      コピーコンストラクタです.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
Ray::Ray( const Ray& value )
: pos   ( value.pos )
, dir   ( value.dir )
, invDir( value.invDir )
, sign  ( value.sign )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      レイを更新します.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
void Ray::Update( const Vector3& position, const Vector3& direction )
{
    pos = position;
    dir = direction;
    invDir.x = 1.0f / dir.x;
    invDir.y = 1.0f / dir.y;
    invDir.z = 1.0f / dir.z;
    sign.x = ( dir.x > 0.0f ) ? 0 : 1;
    sign.y = ( dir.y > 0.0f ) ? 0 : 1;
    sign.z = ( dir.z > 0.0f ) ? 0 : 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// BoundingBox structures
////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------
//      コンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingBox::BoundingBox()
: mini(  F32_MAX,  F32_MAX,  F32_MAX )
, maxi( -F32_MAX, -F32_MAX, -F32_MAX )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      引数付きコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingBox::BoundingBox( const asdx::Vector3& _min, const asdx::Vector3& _max )
: mini( _min )
, maxi( _max )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      コピーコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingBox::BoundingBox( const BoundingBox& value )
: mini( value.mini )
, maxi( value.maxi )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      中心座標を取得します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Vector3 BoundingBox::GetCenter() const
{ return ( maxi + mini ) * 0.5f; }

//--------------------------------------------------------------------------------------------------
//      8角の頂点を取得します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
std::array<Vector3, 8> BoundingBox::GetCorners() const
{
    std::array<Vector3, 8> result;

    // 手前.
    result[ 0 ] = Vector3( mini.x, mini.y, mini.z );    // 左下.
    result[ 1 ] = Vector3( mini.x, maxi.y, mini.z );    // 左上.
    result[ 2 ] = Vector3( maxi.x, maxi.y, mini.z );    // 右上.
    result[ 3 ] = Vector3( maxi.x, mini.y, mini.z );    // 右下.

    // 奥側.
    result[ 4 ] = Vector3( mini.x, mini.y, maxi.z );    // 左下.
    result[ 5 ] = Vector3( mini.x, maxi.y, maxi.z );    // 左上.
    result[ 6 ] = Vector3( maxi.x, maxi.y, maxi.z );    // 右上.
    result[ 7 ] = Vector3( maxi.x, mini.y, maxi.z );    // 右下.
    return result;
}

//-------------------------------------------------------------------------------------------------
//      点が含まれるかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool BoundingBox::Contains( const Vector3& point ) const
{
    if ( mini.x > point.x || point.x > maxi.x
      || mini.y > point.y || point.y > maxi.y
      || mini.z > point.z || point.z > maxi.z )
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      点群が含まれるかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool BoundingBox::Contains( const Vector3* pVertices, const u32 count ) const
{
    for( u32 i=0; i<count; ++i )
    {
        if ( !Contains(pVertices[i]) )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      バウンディングスフィアが含まれるかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool BoundingBox::Contains( const BoundingSphere& sphere ) const
{
    auto v = Vector3::Clamp( sphere.center, mini, maxi );
    auto dist = Vector3::DistanceSq( sphere.center, v );
    return ( dist <= sphere.radius * sphere.radius );
}

//-------------------------------------------------------------------------------------------------
//      バウンディングボックスが含まれるかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool BoundingBox::Contains( const BoundingBox& box ) const
{
    if ( maxi.x < box.mini.x || mini.x > box.maxi.x )
    { return false; }

    if ( maxi.y < box.mini.y || mini.y > box.maxi.y )
    { return false; }

    if ( maxi.z < box.mini.z || mini.z > box.maxi.z )
    { return false; }

    return true;
}

//--------------------------------------------------------------------------------------------------
//      マージします.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
void BoundingBox::Merge( const asdx::Vector3& value )
{
    maxi = asdx::Vector3::Max( maxi, value );
    mini = asdx::Vector3::Min( mini, value );
}

//--------------------------------------------------------------------------------------------------
//      マージします.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingBox BoundingBox::Merge( const BoundingBox& a, const BoundingBox& b )
{
    return BoundingBox(
        asdx::Vector3::Min( a.mini, b.mini ),
        asdx::Vector3::Max( a.maxi, b.maxi ) );
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// BoundingSphere structure
////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------
//      コンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingSphere::BoundingSphere()
: center( 0.0f, 0.0f, 0.0f )
, radius( F32_MAX )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      引数付きコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingSphere::BoundingSphere( const asdx::Vector3& _center, const f32 _radius )
: center( _center )
, radius( _radius )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      引数付きコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingSphere::BoundingSphere( const BoundingBox& value )
{
    center = ( value.mini + value.maxi ) * 0.5f;
    radius = asdx::Vector3::Distance( value.mini, value.maxi ) * 0.5f;
}

//--------------------------------------------------------------------------------------------------
//      コピーコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingSphere::BoundingSphere( const BoundingSphere& value )
: center( value.center )
, radius( value.radius )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      点を含むかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool BoundingSphere::Contains( const Vector3& point ) const
{
    auto dist = Vector3::DistanceSq( point, center );
    return ( dist <= radius * radius );
}

//-------------------------------------------------------------------------------------------------
//      点群を含むかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool BoundingSphere::Contains( const Vector3* pVertices, const u32 count ) const
{
    auto r2 = radius * radius;
    for( u32 i=0; i<count; ++i )
    {
        auto dist = Vector3::DistanceSq( pVertices[i], center );
        if ( dist > r2 )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      バウンディングボックスを含むかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool BoundingSphere::Contains( const BoundingBox& box ) const
{
    auto corners = box.GetCorners();
    auto r2 = radius * radius;
    for( u32 i=0; i<8; ++i )
    {
        auto dist = Vector3::DistanceSq( corners[i], center );
        if ( dist > r2 )
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      バウンディングスフィアを含むかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool BoundingSphere::Contains( const BoundingSphere& sphere ) const
{
    auto dist = Vector3::Distance(center, sphere.center);
    return ( dist <= radius + sphere.radius );
}

//--------------------------------------------------------------------------------------------------
//      マージします.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingSphere BoundingSphere::Merge( const BoundingSphere& a, const BoundingSphere& b )
{
    asdx::Vector3 dif( 
        b.center.x - a.center.x,
        b.center.y - a.center.y,
        b.center.z - a.center.z );

    auto len = dif.Length();
    
    if ( a.radius + b.radius >= len )
    {
        if ( a.radius - b.radius >= len )
        { return a; }

        if ( b.radius - a.radius >= len )
        { return b; }
    }

    assert( len > 0.0f );
    auto v = dif * ( 1.0f / len );
    auto fmin = asdx::Min<f32>( -a.radius, len - b.radius );
    auto fmax = asdx::Max<f32>(  a.radius, len + b.radius );
    auto fmag = ( fmax - fmin ) * 0.5f;

    return BoundingSphere(
        asdx::Vector3( a.center.x + v.x * ( fmag + fmin ),
                       a.center.y + v.y * ( fmag + fmin ),
                       a.center.z + v.z * ( fmag + fmin )
        ),
        fmag
    );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ViewFrustum class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
ViewFrustum::ViewFrustum()
: m_Position( 0.0f, 0.0f, 0.0f )
, m_Forward ( 0.0f, 0.0f, 1.0f )
, m_Right   ( 1.0f, 0.0f, 0.0f )
, m_Upward  ( 0.0f, 1.0f, 0.0f )
, m_FactorR ( 0.0f )
, m_FactorU ( 0.0f )
, m_NearClip( 0.0f )
, m_FarClip ( 0.0f )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      透視変換パラメータを設定します.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
void ViewFrustum::SetPerspective
(
    const f32 fieldOfView,
    const f32 aspectRatio,
    const f32 nearClip,
    const f32 farClip
)
{
    m_FactorR  = tanf( fieldOfView / 2.0f );
    m_FactorU  = m_FactorR * aspectRatio;
    m_NearClip = nearClip;
    m_FarClip  = farClip;
}

//-------------------------------------------------------------------------------------------------
//      ビュー変換パラメータを設定します.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
void ViewFrustum::SetView
(
    const Vector3& position,
    const Vector3& forward,
    const Vector3& right,
    const Vector3& upward
)
{
    m_Position = position;
    m_Forward  = forward;
    m_Right    = right;
    m_Upward   = upward;
}

//-------------------------------------------------------------------------------------------------
//      ビュー変換パラメータを設定します.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
void ViewFrustum::SetLookAt
(
    const Vector3& position,
    const Vector3& target,
    const Vector3& upward
)
{
    m_Position = position;
    m_Forward  = Vector3::Normalize(position - target);
    m_Right    = Vector3::Normalize(Vector3::Cross(upward, m_Forward));
    m_Upward   = Vector3::Normalize(Vector3::Cross(m_Forward, m_Right));
}

//-------------------------------------------------------------------------------------------------
//      ビュー変換パラメータを設定します.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
void ViewFrustum::SetLookTo
(
    const Vector3& position,
    const Vector3& direction,
    const Vector3& upward
)
{
    m_Position = position;
    m_Forward  = -direction;
    m_Right    = Vector3::Normalize(Vector3::Cross(upward, m_Forward));
    m_Upward   = Vector3::Normalize(Vector3::Cross(m_Forward, m_Right));
}

//-------------------------------------------------------------------------------------------------
//      点が含まれるかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool ViewFrustum::Contains( const Vector3& point ) const
{
    auto op = point - m_Position;
    auto f = Vector3::Dot(op,  m_Forward);
    if ( f < m_NearClip || m_FarClip < f )
    { return false; }

    auto r = Vector3::Dot(op, m_Right);
    auto rLimit = m_FactorR * f;
    if ( r < -rLimit || rLimit < r )
    { return false; }

    auto u = Vector3::Dot(op, m_Upward);
    auto uLimit = m_FactorU * f;
    if ( u < -uLimit || uLimit < u )
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      点群が含まれるかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool ViewFrustum::Contains( const Vector3* pVertices, const u32 count ) const
{
    auto behindLeft   = 0;
    auto behindRight  = 0;
    auto behindFar    = 0;
    auto behindNear   = 0;
    auto behindTop    = 0;
    auto behindBottom = 0;

    auto inForward = false;
    auto inRight   = false;
    auto inUp      = false;
    for( u32 i=0; i<count; ++i )
    {
        inForward = inRight = inUp = false;

        auto op = pVertices[i] - m_Position;
        auto f = Vector3::Dot(op, m_Forward);
        auto r = Vector3::Dot(op, m_Right);
        auto u = Vector3::Dot(op, m_Upward);

        auto rLimit = m_FactorR * f;
        auto uLimit = m_FactorU * f;

        if ( f < m_NearClip )
        { ++behindNear; }
        else if ( f < m_FarClip )
        { ++behindFar; }
        else
        { inForward = true; }

        if ( r < -rLimit )
        { ++behindLeft; }
        else if ( r > rLimit )
        { ++behindRight; }
        else
        { inRight = true; }

        if ( u < -uLimit )
        { ++behindBottom; }
        else if ( u > uLimit )
        { ++behindTop; }
        else 
        { inUp = true; }

        if ( inForward && inRight && inUp )
        { return true; }
    }

    if ( behindLeft   == 8
      || behindRight  == 8 
      || behindFar    == 8 
      || behindNear   == 8 
      || behindTop    == 8
      || behindBottom == 8 )
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      バウンディングスフィアが含まれるかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool ViewFrustum::Contains(const BoundingSphere& sphere) const
{
    auto op = sphere.center - m_Position;
    auto f = Vector3::Dot(op, m_Forward);
    if ( f < m_NearClip - sphere.radius || m_FarClip + sphere.radius < f )
    { return false; }

    auto r = Vector3::Dot(op, m_Right);
    auto rLimit = m_FactorR * f;
    auto rTop = rLimit + sphere.radius;
    if ( r < -rTop || rTop < r )
    { return false; }

    auto u = Vector3::Dot(op, m_Upward);
    auto uLimit = m_FactorU * f;
    auto uTop = uLimit + sphere.radius;
    if ( u < -uTop || uTop < u )
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      バウンディングボックスを含むかどうかチェックします.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
bool ViewFrustum::Contains( const BoundingBox& box ) const
{
    Vector3 p;
    auto outOfLeft   = 0;
    auto outOfRight  = 0;
    auto outOfFar    = 0;
    auto outOfNear   = 0;
    auto outOfTop    = 0;
    auto outOfBottom = 0;

    Vector3 corners[2];
    corners[0] = box.mini;
    corners[1] = box.maxi;

    auto isInRightTest = false;
    auto isInUpTest    = false;
    auto isInFrontTest = false;

    for( auto i=0; i<8; ++i )
    {
        isInRightTest = isInUpTest = isInFrontTest = false;
        p.x = corners[(i & 1)].x;
        p.y = corners[(i >> 2) & 1].y;
        p.z = corners[(i >> 1) & 1].z;

        auto r = m_Right.x   * p.x + m_Right.y   * p.y + m_Right.z   * p.z;
        auto u = m_Upward.x  * p.x + m_Upward.y  * p.y + m_Upward.z  * p.z;
        auto f = m_Forward.x * p.x + m_Forward.y * p.y + m_Forward.z * p.z;

        if ( r < -m_FactorR * f )
        { outOfLeft++; }
        else if ( r > m_FactorR * f )
        { outOfRight++; }
        else
        { isInRightTest = true; }

        if ( u < -m_FactorU * f )
        { outOfBottom++; }
        else if ( u > m_FactorU * f )
        { outOfTop++; }
        else
        { isInUpTest = true; }

        if ( f < m_NearClip )
        { outOfNear++; }
        else if ( f > m_FarClip )
        { outOfFar++; }
        else
        { isInFrontTest = true; }

        if ( isInRightTest && isInFrontTest && isInUpTest )
        { return true; }
    }

    if ( outOfLeft   == 8
      || outOfRight  == 8
      || outOfFar    == 8 
      || outOfNear   == 8
      || outOfTop    == 8 
      || outOfBottom == 8 )
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      8角の頂点を取得します.
//-------------------------------------------------------------------------------------------------
std::array<Vector3, 8> ViewFrustum::GetCorners() const
{
    std::array<Vector3, 8> result;

    auto n = m_Forward * m_NearClip;
    auto f = m_Forward * m_FarClip;

    // 手前.
    result[ 0 ] = n - m_Right * m_NearClip * m_FactorR - m_Upward * m_NearClip * m_FactorU;   // 左下.
    result[ 1 ] = n - m_Right * m_NearClip * m_FactorR + m_Upward * m_NearClip * m_FactorU;   // 左上.
    result[ 2 ] = n + m_Right * m_NearClip * m_FactorR + m_Upward * m_NearClip * m_FactorU;   // 右上.
    result[ 3 ] = n + m_Right * m_NearClip * m_FactorR - m_Upward * m_NearClip * m_FactorU;   // 右下.

    // 奥側.
    result[ 4 ] = f - m_Right * m_FarClip * m_FactorR - m_Upward * m_FarClip * m_FactorU;     // 左下.
    result[ 5 ] = f - m_Right * m_FarClip * m_FactorR + m_Upward * m_FarClip * m_FactorU;     // 左上.
    result[ 6 ] = f + m_Right * m_FarClip * m_FactorR + m_Upward * m_FarClip * m_FactorU;     // 右上.
    result[ 7 ] = f + m_Right * m_FarClip * m_FactorR - m_Upward * m_FarClip * m_FactorU;     // 右下.

    // 視点位置まで移動.
    for( auto i=0; i<8; ++i )
    { result[ i ] += m_Position; }

    return result;
}

} // namespace asdx

