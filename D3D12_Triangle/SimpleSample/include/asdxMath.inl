//-------------------------------------------------------------------------------------------------
// File : asdxMath.inl
// Desc : Math Module.
// Copyright(c) Project Asura All right reserved.
//-------------------------------------------------------------------------------------------------

#ifndef _ASDX_MATH_INL_
#define _ASDX_MATH_INL_

namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

ASDX_INLINE
f32 ToRadian( f32 degree )
{ return degree * ( F_PI / 180.0f ); }

ASDX_INLINE
f32 ToDegree( f32 radian )
{ return radian * ( 180.0f / F_PI ); }

ASDX_INLINE
bool IsZero( f32 value )
{ return fabs( value ) <= F_EPSILON; }

ASDX_INLINE
bool IsZero( f64 value )
{ return abs( value ) <= D_EPSILON; }

ASDX_INLINE
bool IsEqual( f32 value1, f32 value2 )
{ return fabs( value1 - value2 ) <= F_EPSILON; }

ASDX_INLINE
bool IsEqual( f64 value1, f64 value2 )
{ return abs( value1 - value2 ) <= D_EPSILON; }

ASDX_INLINE
bool IsNan( f32 value )
{ return ( value != value ); }

ASDX_INLINE
bool IsInf( f32 value )
{
    u32 f = *reinterpret_cast< u32* >( &value );
    if ( ( ( f & 0x7e000000 ) == 0x7e000000 ) && ( value == value ) )
    { return true; }
    return false;
}

ASDX_INLINE
u32 Fact( u32 number )
{
    register u32 result = 1;
    for( u32 i=1; i<=number; ++i )
    { result *= i; }
    return result;
}

ASDX_INLINE
u32 DblFact( u32 number )
{
    register u32 result = 1;
    u32 start = ( ( number % 2 ) == 0 ) ? 2 : 1;
    for( u32 i=start; i<=number; i+=2 )
    { result *= i; }
    return result;
}

ASDX_INLINE
u32 Perm( u32 n, u32 r )
{
    assert( n >= r );
    return Fact( n ) / Fact( n - r );
}

ASDX_INLINE
u32 Comb( u32 n, u32 r )
{
    assert( n >= r );
    return Fact( n ) / ( Fact( n - r ) * Fact( r ) );
}

ASDX_INLINE
f32 Fresnel( f32 n1, f32 n2, f32 cosTheta )
{
    register f32 a = n1 + n2;
    register f32 b = n1 - n2;
    register f32 R = ( a * a ) / ( b * b );
    return R + ( 1.0f - R ) * powf( 1.0f - cosTheta, 5.0f );
}

ASDX_INLINE
f64 Fresnel( f64 n1, f64 n2, f64 cosTheta )
{
    register f64 a = n1 + n2;
    register f64 b = n1 - n2;
    register f64 R = ( a * a ) / ( b * b );
    return R + ( 1.0 - R ) * pow( 1.0f - cosTheta, 5.0 );
}

ASDX_INLINE 
f16 F32ToF16( f32 value )
{
    f16 result;

    // ビット列を崩さないままu32型に変換.
    u32 bit = *reinterpret_cast<u32*>( &value );

    // f32表現の符号bitを取り出し.
    u32 sign   = ( bit & 0x80000000U) >> 16U;

    // 符号部を削ぎ落す.
    bit     = bit & 0x7FFFFFFFU;

    // f16として表現する際に値がデカ過ぎる場合は，無限大にクランプ.
    if ( bit > 0x47FFEFFFU)
    { result = 0x7FFFU; }
    else
    {
        // 正規化されたf16として表現するために小さすぎる値は正規化されていない値に変換.
        if ( bit < 0x38800000U)
        {
            u32 shift = 113U - ( bit >> 23U);
            bit    = (0x800000U | ( bit & 0x7FFFFFU)) >> shift;
        }
        else
        {
            // 正規化されたf16として表現するために指数部に再度バイアスをかける
            bit += 0xC8000000U;
        }

        // f16型表現にする.
        result = (( bit + 0x0FFFU + (( bit >> 13U) & 1U)) >> 13U) & 0x7FFFU; 
    }

    // 符号部を付け足して返却.
    return static_cast<f16>( result | sign );
}

ASDX_INLINE 
f32 F16ToF32( f16 value )
{
    u32 exponent;
    u32 result;

    // 仮数
    u32 mantissa = static_cast<u32>( value & 0x03FF );

    // 正規化済みの場合.
    if ( ( value & 0x7C00 ) != 0 )
    {
        // 指数部を計算.
        exponent = static_cast<u32>( ( value >> 10 ) & 0x1F );
    }
    // 正規化されていない場合.
    else if ( mantissa != 0 )
    {
        // 結果となるf32で値を正規化する.
        exponent = 1;

        do {
            exponent--;
            mantissa <<= 1;
        } while ( ( mantissa & 0x0400 ) == 0);

        mantissa &= 0x03FF;
    }
    // 値がゼロの場合.
    else
    {
        // 指数部を計算.
        exponent = (u32)-112;
    }

    result = ( ( value & 0x8000 ) << 16) | // 符号部.
             ( ( exponent + 112 ) << 23) | // 指数部.
             ( mantissa << 13 );           // 仮数部.

    return *reinterpret_cast<f32*>( &result );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector2 structure
///////////////////////////////////////////////////////////////////////////////////////////////////

ASDX_INLINE
Vector2::Vector2()
{ /* DO_NOTHING */ }

ASDX_INLINE
Vector2::Vector2( const f32* pf )
{
    assert( pf != 0 );
    x = pf[ 0 ];
    y = pf[ 1 ];
}

ASDX_INLINE
Vector2::Vector2( const f32 nx, const f32 ny )
{
    x = nx;
    y = ny;
}

ASDX_INLINE
Vector2::operator f32 *()
{ return (f32*)&x; }

ASDX_INLINE
Vector2::operator const f32 *() const
{ return (const f32*)&x; }

ASDX_INLINE
Vector2& Vector2::operator += ( const Vector2& v )
{
    x += v.x;
    y += v.y;
    return (*this);
}

ASDX_INLINE
Vector2& Vector2::operator -= ( const Vector2& v )
{
    x -= v.x;
    y -= v.y;
    return (*this);
}

ASDX_INLINE
Vector2& Vector2::operator *= ( f32 f )
{
    x *= f;
    y *= f;
    return (*this);
}

ASDX_INLINE
Vector2& Vector2::operator /= ( f32 f )
{
    assert( f != 0.0f );
    x /= f;
    y /= f;
    return (*this);
}

ASDX_INLINE
Vector2& Vector2::operator = ( const Vector2& value )
{
    x = value.x;
    y = value.y;
    return (*this);
}

ASDX_INLINE
Vector2 Vector2::operator + () const
{ return (*this); }

ASDX_INLINE
Vector2 Vector2::operator - () const
{ return Vector2( -x, -y ); }

ASDX_INLINE
Vector2 Vector2::operator + ( const Vector2& v ) const
{ return Vector2( x + v.x, y + v.y ); }

ASDX_INLINE
Vector2 Vector2::operator - ( const Vector2& v ) const
{ return Vector2( x - v.x, y - v.y ); }

ASDX_INLINE
Vector2 Vector2::operator * ( f32 f ) const
{ return Vector2( x * f, y * f ); }

ASDX_INLINE
Vector2 Vector2::operator / ( f32 f ) const
{
    assert( f != 0.0f );
    return Vector2( x / f, y / f );
}

ASDX_INLINE
Vector2 operator * ( f32 f, const Vector2& v )
{ return Vector2( f * v.x, f * v.y ); }

ASDX_INLINE
bool Vector2::operator == ( const Vector2& v ) const
{ 
    return ( x == v.x )
        && ( y == v.y );
}

ASDX_INLINE
bool Vector2::operator != ( const Vector2& v ) const
{
    return ( x != v.x )
        || ( y != v.y );
}

ASDX_INLINE
f32 Vector2::Length() const
{ return sqrtf( x * x + y * y ); }

ASDX_INLINE
f32 Vector2::LengthSq() const
{ return ( x * x + y * y ); }

ASDX_INLINE
Vector2& Vector2::Normalize()
{
    register f32 mag = Length();
    assert( mag > 0.0f );
    x /= mag;
    y /= mag;
    return (*this);
}

ASDX_INLINE
Vector2& Vector2::SafeNormalize( const Vector2& set )
{
    register f32 mag = Length();
    if ( mag > 0.0f )
    {
        x /= mag;
        y /= mag;
    }
    else
    {
        x = set.x;
        y = set.y;
    }
 
    return (*this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector2 Methods
///////////////////////////////////////////////////////////////////////////////////////////////////

ASDX_INLINE
Vector2 Vector2::Abs( const Vector2& value )
{ 
    return Vector2(
        fabs( value.x ), 
        fabs( value.y ) 
    );
}

ASDX_INLINE
void Vector2::Abs( const Vector2 &value, Vector2 &result )
{ 
    result.x = fabs( value.x );
    result.y = fabs( value.y );
}

ASDX_INLINE
Vector2 Vector2::Clamp( const Vector2& value, const Vector2& a, const Vector2& b )
{
    return Vector2(
        asdx::Clamp< f32 >( value.x, a.x, b.x ),
        asdx::Clamp< f32 >( value.y, a.y, b.y )
    );
}

ASDX_INLINE
void Vector2::Clamp( const Vector2& value, const Vector2& a, const Vector2& b, Vector2 &result )
{
    result.x = asdx::Clamp< f32 >( value.x, a.x, b.x );
    result.y = asdx::Clamp< f32 >( value.y, a.y, b.y );
}

ASDX_INLINE
Vector2 Vector2::Saturate( const Vector2& value )
{
    return Vector2(
        asdx::Saturate( value.x ),
        asdx::Saturate( value.y )
    );
}

ASDX_INLINE
void Vector2::Saturate( const Vector2& value, Vector2& result )
{
    result.x = asdx::Saturate( value.x );
    result.y = asdx::Saturate( value.y );
}

ASDX_INLINE
f32 Vector2::Distance( const Vector2& a, const Vector2& b )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    return sqrtf( X * X + Y * Y );
}

ASDX_INLINE
void Vector2::Distance( const Vector2 &a, const Vector2 &b, f32 &result )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    result = sqrtf( X * X + Y * Y );
}

ASDX_INLINE
f32 Vector2::DistanceSq( const Vector2& a, const Vector2& b )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    return X * X + Y * Y;
}

ASDX_INLINE
void Vector2::DistanceSq( const Vector2 &a, const Vector2 &b, f32 &result )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    result = X * X + Y * Y;
}

ASDX_INLINE
f32 Vector2::Dot( const Vector2& a, const Vector2& b )
{
    return ( a.x * b.x + a.y * b.y );
}

ASDX_INLINE
void Vector2::Dot( const Vector2 &a, const Vector2 &b, f32 &result )
{
    result = a.x * b.x + a.y * b.y;
}

ASDX_INLINE
Vector2 Vector2::Normalize( const Vector2& value )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y );
    assert( mag > 0.0f );
    return Vector2(
        value.x / mag,
        value.y / mag 
    );
}

ASDX_INLINE
void Vector2::Normalize( const Vector2 &value, Vector2 &result )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y );
    assert( mag > 0.0f );
    result.x = value.x / mag;
    result.y = value.y / mag;
}

ASDX_INLINE
Vector2 Vector2::SafeNormalize( const Vector2& value, const Vector2& set )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y );
    if ( mag > 0.0f )
    {
        return Vector2(
            value.x / mag,
            value.y / mag 
        );
    }
    else
    {
        return Vector2(
            set.x,
            set.y 
        );
    }
}

ASDX_INLINE
void Vector2::SafeNormalize( const Vector2& value, const Vector2& set, Vector2& result )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y );
    if ( mag > 0.0f )
    {
        result.x = value.x / mag;
        result.y = value.y / mag;
    }
    else
    {
        result.x = set.x;
        result.y = set.y;
    }
}

ASDX_INLINE
f32 Vector2::ComputeCrossingAngle( const Vector2& a, const Vector2& b )
{
    register f32 d = a.Length() * b.Length();
    if ( d <= 0.0f )
    { return 0.0f; }

    register f32 c = Vector2::Dot( a, b ) / d;
    if ( c >= 1.0f ) 
    { return 0.0f; }

    if ( c <= -1.0f )
    { return F_PI; }

    return acosf( c );
}

ASDX_INLINE
void Vector2::ComputeCrossingAngle( const Vector2 &a, const Vector2 &b, f32 &result )
{
    register f32 d = a.Length() * b.Length();
    if ( d <= 0.0f )
    {
        result = 0.0f;
        return;
    }

    register f32 c = Vector2::Dot( a, b ) / d;
    if ( c >= 1.0f ) 
    {
        result = 0.0f;
        return;
    }

    if ( c <= -1.0f )
    {
        result = F_PI;
        return;
    }

    result = acosf( c );
}

ASDX_INLINE
Vector2 Vector2::Min( const Vector2& a, const Vector2& b )
{ 
    return Vector2(
        asdx::Min< f32 >( a.x, b.x ),
        asdx::Min< f32 >( a.y, b.y )
    );
}

ASDX_INLINE
void Vector2::Min( const Vector2 &a, const Vector2 &b, Vector2 &result )
{
    result.x = asdx::Min< f32 >( a.x, b.x );
    result.y = asdx::Min< f32 >( a.y, b.y );
}

ASDX_INLINE
Vector2 Vector2::Max( const Vector2& a, const Vector2& b )
{
    return Vector2(
        asdx::Max< f32 >( a.x, b.x ),
        asdx::Max< f32 >( a.y, b.y )
    );
}

ASDX_INLINE
void Vector2::Max( const Vector2 &a, const Vector2 &b, Vector2 &result )
{
    result.x = asdx::Max< f32 >( a.x, b.x );
    result.y = asdx::Max< f32 >( a.y, b.y );
}

ASDX_INLINE
Vector2 Vector2::Reflect( const Vector2& i, const Vector2& n )
{
    register f32 dot = n.x * i.x + n.y * i.y;
    return Vector2(
        i.x - ( 2.0f * n.x ) * dot,
        i.y - ( 2.0f * n.y ) * dot 
    );
}

ASDX_INLINE
void Vector2::Reflect( const Vector2 &i, const Vector2 &n, Vector2 &result )
{
    register f32 dot = n.x * i.x + n.y * i.y;
    result.x = i.x - ( 2.0f * n.x ) * dot;
    result.y = i.y - ( 2.0f * n.y ) * dot;
}

ASDX_INLINE
Vector2 Vector2::Refract( const Vector2& i, const Vector2& n, const f32 eta )
{
    register f32 cosi   = ( -i.x * n.x ) + ( -i.y * n.y );
    register f32 cost2  = 1.0f - eta * eta * ( 1.0f - cosi * cosi );
    register f32 sign   = Sign< f32 >( cost2 );
    register f32 sqrtC2 = sqrtf( fabs( cost2 ) );
    register f32 coeff  = eta * cosi - sqrtC2;

    return Vector2(
        sign * ( eta * i.x + coeff * n.x ),
        sign * ( eta * i.y + coeff * n.y )
    );
}

ASDX_INLINE
void Vector2::Refract( const Vector2 &i, const Vector2 &n, const f32 eta, Vector2 &result )
{
    register f32 cosi   =  ( -i.x * n.x ) + ( -i.y * n.y );
    register f32 cost2  = 1.0f - eta * eta * ( 1.0f - cosi * cosi );
    register f32 sign   = Sign< f32 >( cost2 );
    register f32 sqrtC2 = sqrtf( fabs( cost2 ) );
    register f32 coeff  = eta * cosi - sqrtC2;

    result.x = sign * ( eta * i.x + coeff * n.x );
    result.y = sign * ( eta * i.y + coeff * n.y );
}

ASDX_INLINE
Vector2 Vector2::Barycentric( const Vector2& a, const Vector2& b, const Vector2& c, const f32 f, const f32 g )
{
    return Vector2(
        a.x + f * ( b.x - a.x ) + g * ( c.x - a.x ),
        a.y + f * ( b.y - a.y ) + g * ( c.y - a.y )
    );
}

ASDX_INLINE
void Vector2::Barycentric( const Vector2& a, const Vector2& b, const Vector2& c, const f32 f, const f32 g, Vector2 &result )
{
    result.x = a.x + f * ( b.x - a.x ) + g * ( c.x - a.x );
    result.y = a.y + f * ( b.y - a.y ) + g * ( c.y - a.y );
}

ASDX_INLINE
Vector2 Vector2::Hermite( const Vector2& a, const Vector2& t1, const Vector2& b, const Vector2& t2, const f32 amount )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    Vector2 result;
    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
    }
    return result;
}

ASDX_INLINE
void Vector2::Hermite( const Vector2& a, const Vector2& t1, const Vector2& b, const Vector2& t2, const f32 amount, Vector2& result )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
    }
}

ASDX_INLINE
Vector2 Vector2::CatmullRom( const Vector2& a, const Vector2& b, const Vector2& c, const Vector2& d, const f32 amount )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    return Vector2(
        ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) )
    );
}

ASDX_INLINE
void Vector2::CatmullRom( const Vector2& a, const Vector2& b, const Vector2& c, const Vector2& d, const f32 amount, Vector2& result )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    result.x = ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) );
    result.y = ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) );
}

ASDX_INLINE
Vector2 Vector2::Lerp( const Vector2& a, const Vector2& b, const f32 amount )
{
    return Vector2(
        a.x - amount * ( a.x - b.x ),
        a.y - amount * ( a.y - b.y )
    );
}

ASDX_INLINE
void Vector2::Lerp( const Vector2 &a, const Vector2 &b, const f32 amount, Vector2 &result )
{
    result.x = a.x - amount * ( a.x - b.x );
    result.y = a.y - amount * ( a.y - b.y );
}

ASDX_INLINE
Vector2 Vector2::SmoothStep( const Vector2& a, const Vector2& b, const f32 amount )
{
    register f32 s = asdx::Clamp< f32 >( amount, 0.0f, 1.0f );
    register f32 u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    return Vector2(
        a.x - u * ( a.x - b.x ),
        a.y - u * ( a.y - b.y )
    );
}

ASDX_INLINE
void Vector2::SmoothStep( const Vector2 &a, const Vector2 &b, const f32 t, Vector2 &result )
{
    register f32 s = asdx::Clamp< f32 >( t, 0.0f, 1.0f );
    register f32 u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    result.x = a.x - u * ( a.x - b.x );
    result.y = a.y - u * ( a.y - b.y );
}

ASDX_INLINE
Vector2 Vector2::Transform( const Vector2& position, const Matrix& matrix )
{
    return Vector2(
        ((position.x * matrix._11) + (position.y * matrix._21)) + matrix._41,
        ((position.x * matrix._12) + (position.y * matrix._22)) + matrix._42 );
}

ASDX_INLINE
void Vector2::Transform( const Vector2 &position, const Matrix &matrix, Vector2 &result )
{
    result.x = ((position.x * matrix._11) + (position.y * matrix._21)) + matrix._41;
    result.y = ((position.x * matrix._12) + (position.y * matrix._22)) + matrix._42;
}

ASDX_INLINE
Vector2 Vector2::TransformNormal( const Vector2& normal, const Matrix& matrix )
{
    return Vector2(
        (normal.x * matrix._11) + (normal.y * matrix._21),
        (normal.x * matrix._12) + (normal.y * matrix._22) );
}

ASDX_INLINE
void Vector2::TransformNormal( const Vector2 &normal, const Matrix &matrix, Vector2 &result )
{
    result.x = (normal.x * matrix._11) + (normal.y * matrix._21);
    result.y = (normal.x * matrix._12) + (normal.y * matrix._22);
}

ASDX_INLINE
Vector2 Vector2::TransformCoord( const Vector2& coords, const Matrix& matrix )
{
    register f32 X = ( ( ((coords.x * matrix._11) + (coords.y * matrix._21)) ) + matrix._41);
    register f32 Y = ( ( ((coords.x * matrix._12) + (coords.y * matrix._22)) ) + matrix._42);
    register f32 W = ( ( ((coords.x * matrix._14) + (coords.y * matrix._24)) ) + matrix._44);
    return Vector2(
        X / W,
        Y / W 
    );
}

ASDX_INLINE
void Vector2::TransformCoord( const Vector2 &coords, const Matrix &matrix, Vector2 &result )
{
    register f32 X = ( ( ((coords.x * matrix._11) + (coords.y * matrix._21)) ) + matrix._41);
    register f32 Y = ( ( ((coords.x * matrix._12) + (coords.y * matrix._22)) ) + matrix._42);
    register f32 W = ( ( ((coords.x * matrix._14) + (coords.y * matrix._24)) ) + matrix._44);

    result.x = X / W;
    result.y = Y / W;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector3 structure
///////////////////////////////////////////////////////////////////////////////////////////////////

ASDX_INLINE
Vector3::Vector3()
{ /* DO_NOTHING */ }

ASDX_INLINE
Vector3::Vector3( const f32* pf )
{
    assert( pf != 0 );
    x = pf[ 0 ];
    y = pf[ 1 ];
    z = pf[ 2 ];
}

ASDX_INLINE
Vector3::Vector3( const Vector2& value, const f32 nz )
{
    x = value.x;
    y = value.y;
    z = nz;
}

ASDX_INLINE
Vector3::Vector3( const f32 nx, const f32 ny, const f32 nz )
{
    x = nx;
    y = ny;
    z = nz;
}

ASDX_INLINE
Vector3::operator f32 *()
{ return (f32*)&x; }

ASDX_INLINE
Vector3::operator const f32 *() const
{ return (const f32*)&x; }

ASDX_INLINE
Vector3& Vector3::operator += ( const Vector3& v )
{
    x += v.x;
    y += v.y;
    z += v.z;
    return (*this);
}

ASDX_INLINE
Vector3& Vector3::operator -= ( const Vector3& v )
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return (*this);
}

ASDX_INLINE
Vector3& Vector3::operator *= ( f32 f )
{
    x *= f;
    y *= f;
    z *= f;
    return (*this);
}

ASDX_INLINE
Vector3& Vector3::operator /= ( f32 f )
{
    assert( f != 0.0f );
    x /= f;
    y /= f;
    z /= f;
    return (*this);
}

ASDX_INLINE
Vector3& Vector3::operator = ( const Vector3& value )
{
    x = value.x;
    y = value.y;
    z = value.z;
    return (*this);
}

ASDX_INLINE
Vector3 Vector3::operator + () const
{ return (*this); }

ASDX_INLINE
Vector3 Vector3::operator - () const
{ return Vector3( -x, -y, -z ); }

ASDX_INLINE
Vector3 Vector3::operator + ( const Vector3& v ) const
{ return Vector3( x + v.x, y + v.y, z + v.z ); }

ASDX_INLINE
Vector3 Vector3::operator - ( const Vector3& v ) const
{ return Vector3( x - v.x, y - v.y, z - v.z ); }

ASDX_INLINE
Vector3 Vector3::operator * ( f32 f ) const
{ return Vector3( x * f, y * f, z * f ); }

ASDX_INLINE
Vector3 Vector3::operator / ( f32 f ) const
{
    assert( f != 0.0f );
    return Vector3( x / f, y / f, z / f );
}

ASDX_INLINE
Vector3 operator * ( f32 f, const Vector3& v )
{ return Vector3( f * v.x, f * v.y, f * v.z ); }

ASDX_INLINE
bool Vector3::operator == ( const Vector3& v ) const
{
    return ( x == v.x )
        && ( y == v.y )
        && ( z == v.z );
}

ASDX_INLINE
bool Vector3::operator != ( const Vector3& v ) const
{ 
    return ( x != v.x )
        || ( y != v.y )
        || ( z != v.z );
}

ASDX_INLINE
f32 Vector3::Length() const
{ return sqrtf( x * x + y * y + z * z); }

ASDX_INLINE
f32 Vector3::LengthSq() const
{ return ( x * x + y * y + z * z); }

ASDX_INLINE
Vector3& Vector3::Normalize()
{
    register f32 mag = Length();
    assert( mag > 0.0f );
    x /= mag;
    y /= mag;
    z /= mag;
    return (*this);
}

ASDX_INLINE
Vector3& Vector3::SafeNormalize( const Vector3& set )
{
    register f32 mag = Length();
    if ( mag > 0.0f )
    {
        x /= mag;
        y /= mag;
        z /= mag;
    }
    else
    {
        x = set.x;
        y = set.y;
        z = set.z;
    }

    return (*this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector3 methods
///////////////////////////////////////////////////////////////////////////////////////////////////
ASDX_INLINE
Vector3 Vector3::Abs( const Vector3& v )
{ 
    return Vector3(
        fabs( v.x ),
        fabs( v.y ),
        fabs( v.z ) 
     );
}

ASDX_INLINE
void Vector3::Abs( const Vector3 &value, Vector3 &result )
{ 
    result.x = fabs( value.x );
    result.y = fabs( value.y );
    result.z = fabs( value.z );
}

ASDX_INLINE
Vector3 Vector3::Clamp( const Vector3& value, const Vector3& a, const Vector3& b )
{
    return Vector3( 
        asdx::Clamp< f32 >( value.x, a.x, b.x ),
        asdx::Clamp< f32 >( value.y, a.y, b.y ),
        asdx::Clamp< f32 >( value.z, a.z, b.z )
    );
}

ASDX_INLINE
void Vector3::Clamp( const Vector3 &value, const Vector3 &a, const Vector3 &b, Vector3 &result )
{
    result.x = asdx::Clamp< f32 >( value.x, a.x, b.x );
    result.y = asdx::Clamp< f32 >( value.y, a.y, b.y );
    result.z = asdx::Clamp< f32 >( value.z, a.z, b.z );
}

ASDX_INLINE
Vector3 Vector3::Saturate( const Vector3& value )
{
    return Vector3(
        asdx::Saturate( value.x ),
        asdx::Saturate( value.y ),
        asdx::Saturate( value.z )
    );
}

ASDX_INLINE
void Vector3::Saturate( const Vector3& value, Vector3& result )
{
    result.x = asdx::Saturate( value.x );
    result.y = asdx::Saturate( value.y );
    result.z = asdx::Saturate( value.z );
}

ASDX_INLINE
f32 Vector3::Distance( const Vector3& a, const Vector3& b )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    register f32 Z = b.z - a.z;
    return sqrtf( X * X + Y * Y + Z * Z );
}

ASDX_INLINE
void Vector3::Distance( const Vector3 &a, const Vector3 &b, f32 &result )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    register f32 Z = b.z - a.z;
    result = sqrtf( X * X + Y * Y + Z * Z );
}

ASDX_INLINE
f32 Vector3::DistanceSq( const Vector3& a, const Vector3& b )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    register f32 Z = b.z - a.z;
    return X * X + Y * Y + Z * Z;
}


ASDX_INLINE
void Vector3::DistanceSq( const Vector3 &a, const Vector3 &b, f32 &result )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    register f32 Z = b.z - a.z;
    result = X * X + Y * Y + Z * Z;
}

ASDX_INLINE
f32 Vector3::Dot( const Vector3& a, const Vector3& b )
{
    return ( a.x * b.x + a.y * b.y + a.z * b.z );
}

ASDX_INLINE
void Vector3::Dot( const Vector3 &a, const Vector3 &b, f32 &result )
{
    result = a.x * b.x + a.y * b.y + a.z * b.z;
}

ASDX_INLINE
Vector3 Vector3::Cross( const Vector3& a, const Vector3& b )
{
    return Vector3( 
        ( a.y * b.z ) - ( a.z * b.y ),
        ( a.z * b.x ) - ( a.x * b.z ),
        ( a.x * b.y ) - ( a.y * b.x )
    );
}

ASDX_INLINE
void Vector3::Cross( const Vector3 &a, const Vector3 &b, Vector3 &result )
{
    result.x = ( a.y * b.z ) - ( a.z * b.y );
    result.y = ( a.z * b.x ) - ( a.x * b.z );
    result.z = ( a.x * b.y ) - ( a.y * b.x );
}


ASDX_INLINE
Vector3 Vector3::Normalize( const Vector3& value )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y + value.z * value.z );
    assert( mag > 0.0f );
    return Vector3(
        value.x / mag,
        value.y / mag,
        value.z / mag 
    );
}

ASDX_INLINE
void Vector3::Normalize( const Vector3& value, Vector3 &result )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y + value.z * value.z );
    assert( mag > 0.0f );
    result.x = value.x / mag;
    result.y = value.y / mag;
    result.z = value.z / mag;
}

ASDX_INLINE
Vector3 Vector3::SafeNormalize( const Vector3& value, const Vector3& set )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y + value.z * value.z );
    if ( mag > 0.0f )
    {
        return Vector3(
            value.x / mag,
            value.y / mag,
            value.z / mag
        );
    }
    else
    {
        return Vector3(
            set.x,
            set.y,
            set.z 
        );
    }
}

ASDX_INLINE
void Vector3::SafeNormalize( const Vector3& value, const Vector3& set, Vector3& result )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y + value.z * value.z );
    if ( mag > 0.0f )
    {
        result.x = value.x / mag;
        result.y = value.y / mag;
        result.z = value.z / mag;
    }
    else
    {
        result.x = set.x;
        result.y = set.y;
        result.z = set.z;
    }
}

ASDX_INLINE
Vector3 Vector3::ComputeNormal( const Vector3& p1, const Vector3& p2, const Vector3& p3 )
{
    register Vector3 v1 = p2 - p1;
    register Vector3 v2 = p3 - p1;
    Vector3 result = Vector3::Cross( v1, v2 );
    return result.Normalize();
}

ASDX_INLINE
void Vector3::ComputeNormal( const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, Vector3 &result )
{
    register Vector3 v1 = p2 - p1;
    register Vector3 v2 = p3 - p1;
    Vector3::Cross( v1, v2, result );
    result.Normalize();
}

ASDX_INLINE
Vector3 Vector3::ComputeQuadNormal( const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4 )
{
    Vector3 result;
    Vector3 n1a = Vector3::ComputeNormal( p1, p2, p3 );
    Vector3 n1b = Vector3::ComputeNormal( p1, p3, p4 );
    Vector3 n2a = Vector3::ComputeNormal( p2, p3, p4 );
    Vector3 n2b = Vector3::ComputeNormal( p2, p4, p1 );
    if ( Vector3::Dot( n1a, n1b ) > Vector3::Dot( n2a, n2b ) )
    {
        result = n1a + n1b;
        result.Normalize();
    }
    else
    {
        result = n2a + n2b;
        result.Normalize();
    }
    return result;
}

ASDX_INLINE
void Vector3::ComputeQuadNormal( const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &p4, Vector3 &result )
{
    Vector3 n1a = Vector3::ComputeNormal( p1, p2, p3 );
    Vector3 n1b = Vector3::ComputeNormal( p1, p3, p4 );
    Vector3 n2a = Vector3::ComputeNormal( p2, p3, p4 );
    Vector3 n2b = Vector3::ComputeNormal( p2, p4, p1 );
    if ( Vector3::Dot( n1a, n1b ) > Vector3::Dot( n2a, n2b ) )
    {
        result = n1a + n1b;
        result.Normalize();
    }
    else
    {
        result = n2a + n2b;
        result.Normalize();
    }
}

ASDX_INLINE
f32 Vector3::ComputeCrossingAngle( const Vector3& a, const Vector3& b )
{
    register f32 d = a.Length() * b.Length();
    if ( d <= 0.0f ) 
    { return 0.0f; }

    register f32 c = Vector3::Dot( a, b ) / d;
    if ( c >= 1.0f )
    { return 0.0f; }

    if ( c <= -1.0f )
    { return F_PI; }

    return acosf( c );
}

ASDX_INLINE
void Vector3::ComputeCrossingAngle( const Vector3 &a, const Vector3 &b, f32 &result )
{
    register f32 d = a.Length() * b.Length();
    if ( d <= 0.0f )
    {
        result = 0.0f;
        return;
    }

    register f32 c = Vector3::Dot( a, b ) / d;
    if ( c >= 1.0f ) 
    {
        result = 0.0f;
        return;
    }

    if ( c <= -1.0f )
    {
        result = F_PI;
        return;
    }

    result = acosf( c );
}

ASDX_INLINE
Vector3 Vector3::Min( const Vector3& a, const Vector3& b )
{ 
    return Vector3( 
        asdx::Min< f32 >( a.x, b.x ),
        asdx::Min< f32 >( a.y, b.y ),
        asdx::Min< f32 >( a.z, b.z )
    );
}

ASDX_INLINE
void Vector3::Min( const Vector3 &a, const Vector3 &b, Vector3 &result )
{
    result.x = asdx::Min< f32 >( a.x, b.x );
    result.y = asdx::Min< f32 >( a.y, b.y );
    result.z = asdx::Min< f32 >( a.z, b.z );
}

ASDX_INLINE
Vector3 Vector3::Max( const Vector3& a, const Vector3& b )
{
    return Vector3(
        asdx::Max< f32 >( a.x, b.x ),
        asdx::Max< f32 >( a.y, b.y ),
        asdx::Max< f32 >( a.z, b.z )
    );
}

ASDX_INLINE
void Vector3::Max( const Vector3 &a, const Vector3 &b, Vector3 &result )
{
    result.x = asdx::Max< f32 >( a.x, b.x );
    result.y = asdx::Max< f32 >( a.y, b.y );
    result.z = asdx::Max< f32 >( a.z, b.z );
}

ASDX_INLINE
Vector3 Vector3::Reflect( const Vector3& i, const Vector3& n )
{
    register f32 dot = n.x * i.x + n.y * i.y + n.z * i.z;
    return Vector3(
        i.x - ( 2.0f * n.x ) * dot,
        i.y - ( 2.0f * n.y ) * dot,
        i.z - ( 2.0f * n.z ) * dot
    );
}

ASDX_INLINE
void Vector3::Reflect( const Vector3 &i, const Vector3 &n, Vector3 &result )
{
    register f32 dot = n.x * i.x + n.y * i.y + n.z * i.z;
    result.x = i.x - ( 2.0f * n.x ) * dot;
    result.y = i.y - ( 2.0f * n.y ) * dot;
    result.z = i.z - ( 2.0f * n.z ) * dot;
}

ASDX_INLINE
Vector3 Vector3::Refract( const Vector3& i, const Vector3& n, const f32 eta )
{
    register f32 cosi   = ( -i.x * n.x ) + ( -i.y * n.y ) + ( -i.z * n.z );
    register f32 cost2  = 1.0f - eta * eta * ( 1.0f - cosi * cosi );
    register f32 sign   = Sign< f32 >( cost2 );
    register f32 sqrtC2 = sqrtf( fabs( cost2 ) );
    register f32 coeff  = eta * cosi - sqrtC2;

    return Vector3(
        sign * ( eta * i.x + coeff * n.x ),
        sign * ( eta * i.y + coeff * n.y ),
        sign * ( eta * i.z + coeff * n.z )
    );
}

ASDX_INLINE
void Vector3::Refract( const Vector3 &i, const Vector3 &n, const f32 eta, Vector3 &result )
{
    register f32 cosi   =  ( -i.x * n.x ) + ( -i.y * n.y ) + ( -i.z * n.z );
    register f32 cost2  = 1.0f - eta * eta * ( 1.0f - cosi * cosi );
    register f32 sign   = Sign< f32 >( cost2 );
    register f32 sqrtC2 = sqrtf( fabs( cost2 ) );
    register f32 coeff  = eta * cosi - sqrtC2;

    result.x = sign * ( eta * i.x + coeff * n.x );
    result.y = sign * ( eta * i.y + coeff * n.y );
    result.z = sign * ( eta * i.z + coeff * n.z );
}

ASDX_INLINE
Vector3 Vector3::Barycentric( const Vector3& a, const Vector3& b, const Vector3& c, const f32 f, const f32 g )
{
    return Vector3(
        a.x + f * ( b.x - a.x ) + g * ( c.x - a.x ),
        a.y + f * ( b.y - a.y ) + g * ( c.y - a.y ),
        a.z + f * ( b.z - a.z ) + g * ( c.z - a.z )
    );
}

ASDX_INLINE
void Vector3::Barycentric( const Vector3 &a, const Vector3 &b, const Vector3 &c, const f32 f, const f32 g, Vector3 &result )
{
    result.x = a.x + f * ( b.x - a.x ) + g * ( c.x - a.x );
    result.y = a.y + f * ( b.y - a.y ) + g * ( c.y - a.y );
    result.z = a.z + f * ( b.z - a.z ) + g * ( c.z - a.z );
}

ASDX_INLINE
Vector3 Vector3::Hermite( const Vector3& a, const Vector3& t1, const Vector3& b, const Vector3& t2, const f32 amount )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    Vector3 result;
    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
        result.z = a.z;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
        result.z = b.z;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
        result.z = ( 2.0f * a.z - 2.0f * b.z + t2.z + t1.z ) * c3 + ( 3.0f * b.z - 3.0f * a.z - 2.0f * t1.z - t2.z ) * c3 + t1.y * amount + a.z;
    }
    return result;
}

ASDX_INLINE
void Vector3::Hermite( const Vector3& a, const Vector3& t1, const Vector3& b, const Vector3& t2, const f32 amount, Vector3& result )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
        result.z = a.z;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
        result.z = b.z;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
        result.z = ( 2.0f * a.z - 2.0f * b.z + t2.z + t1.z ) * c3 + ( 3.0f * b.z - 3.0f * a.z - 2.0f * t1.z - t2.z ) * c3 + t1.z * amount + a.z;
    }
}

ASDX_INLINE
Vector3 Vector3::CatmullRom( const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const f32 amount )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    return Vector3(
        ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.z + ( c.z - a.z ) * amount + ( 2.0f * a.z - 5.0f * b.z + 4.0f * c.z - d.z ) * c2 + ( 3.0f * b.z - a.z - 3.0f * c.z + d.z ) * c3 ) )
    );
}

ASDX_INLINE
void Vector3::CatmullRom( const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const f32 amount, Vector3& result )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    result.x = ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) );
    result.y = ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) );
    result.z = ( 0.5f * ( 2.0f * b.z + ( c.z - a.z ) * amount + ( 2.0f * a.z - 5.0f * b.z + 4.0f * c.z - d.z ) * c2 + ( 3.0f * b.z - a.z - 3.0f * c.z + d.z ) * c3 ) );
}

ASDX_INLINE
Vector3 Vector3::Lerp( const Vector3& a, const Vector3& b, const f32 amount )
{
    return Vector3(
        a.x - amount * ( a.x - b.x ),
        a.y - amount * ( a.y - b.y ),
        a.z - amount * ( a.z - b.z ) 
    );
}

ASDX_INLINE
void Vector3::Lerp( const Vector3 &a, const Vector3 &b, const f32 amount, Vector3 &result )
{
    result.x = a.x - amount * ( a.x - b.x );
    result.y = a.y - amount * ( a.y - b.y );
    result.z = a.z - amount * ( a.z - b.z );
}

ASDX_INLINE
Vector3 Vector3::SmoothStep( const Vector3& a, const Vector3& b, const f32 amount )
{
    register f32 s = asdx::Clamp< f32 >( amount, 0.0f, 1.0f );
    register f32 u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    return Vector3(
        a.x - u * ( a.x - b.x ),
        a.y - u * ( a.y - b.y ),
        a.z - u * ( a.z - b.z )
    );
}

ASDX_INLINE
void Vector3::SmoothStep( const Vector3 &a, const Vector3 &b, const f32 amount, Vector3 &result )
{ 
    register f32 s = asdx::Clamp< f32 >( amount, 0.0f, 1.0f );
    register f32 u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    result.x = a.x - u * ( a.x - b.x );
    result.y = a.y - u * ( a.y - b.y );
    result.z = a.z - u * ( a.z - b.z );
}

ASDX_INLINE
Vector3 Vector3::Transform( const Vector3& position, const Matrix& matrix )
{
    return Vector3(
        ( ((position.x * matrix._11) + (position.y * matrix._21)) + (position.z * matrix._31)) + matrix._41,
        ( ((position.x * matrix._12) + (position.y * matrix._22)) + (position.z * matrix._32)) + matrix._42,
        ( ((position.x * matrix._13) + (position.y * matrix._23)) + (position.z * matrix._33)) + matrix._43 );
}

ASDX_INLINE
void Vector3::Transform( const Vector3 &position, const Matrix &matrix, Vector3 &result )
{
    result.x = ( ((position.x * matrix._11) + (position.y * matrix._21)) + (position.z * matrix._31)) + matrix._41;
    result.y = ( ((position.x * matrix._12) + (position.y * matrix._22)) + (position.z * matrix._32)) + matrix._42;
    result.z = ( ((position.x * matrix._13) + (position.y * matrix._23)) + (position.z * matrix._33)) + matrix._43;
}

ASDX_INLINE
Vector3 Vector3::TransformNormal( const Vector3& normal, const Matrix& matrix )
{
    return Vector3(
        ((normal.x * matrix._11) + (normal.y * matrix._21)) + (normal.z * matrix._31),
        ((normal.y * matrix._12) + (normal.y * matrix._22)) + (normal.z * matrix._32),
        ((normal.z * matrix._13) + (normal.y * matrix._23)) + (normal.z * matrix._33) );
}

ASDX_INLINE
void Vector3::TransformNormal( const Vector3 &normal, const Matrix &matrix, Vector3 &result )
{
    result.x = ((normal.x * matrix._11) + (normal.y * matrix._21)) + (normal.z * matrix._31);
    result.y = ((normal.y * matrix._12) + (normal.y * matrix._22)) + (normal.z * matrix._32);
    result.z = ((normal.z * matrix._13) + (normal.y * matrix._23)) + (normal.z * matrix._33);
}

ASDX_INLINE
Vector3 Vector3::TransformCoord( const Vector3& coords, const Matrix& matrix )
{
    register f32 X = ( ( ((coords.x * matrix._11) + (coords.y * matrix._21)) + (coords.z * matrix._31) ) + matrix._41);
    register f32 Y = ( ( ((coords.x * matrix._12) + (coords.y * matrix._22)) + (coords.z * matrix._32) ) + matrix._42);
    register f32 Z = ( ( ((coords.x * matrix._13) + (coords.y * matrix._23)) + (coords.z * matrix._33) ) + matrix._43);
    register f32 W = ( ( ((coords.x * matrix._14) + (coords.y * matrix._24)) + (coords.z * matrix._34) ) + matrix._44);
    return Vector3(
        X / W,
        Y / W,
        Z / W 
    );
}

ASDX_INLINE
void Vector3::TransformCoord( const Vector3 &coords, const Matrix &matrix, Vector3 &result )
{
    register f32 X = ( ( ((coords.x * matrix._11) + (coords.y * matrix._21)) + (coords.z * matrix._31) ) + matrix._41);
    register f32 Y = ( ( ((coords.x * matrix._12) + (coords.y * matrix._22)) + (coords.z * matrix._32) ) + matrix._42);
    register f32 Z = ( ( ((coords.x * matrix._13) + (coords.y * matrix._23)) + (coords.z * matrix._33) ) + matrix._43);
    register f32 W = ( ( ((coords.x * matrix._14) + (coords.y * matrix._24)) + (coords.z * matrix._34) ) + matrix._44);

    result.x = X / W;
    result.y = Y / W;
    result.z = Z / W;
}


ASDX_INLINE
f32 Vector3::ScalarTriple( const Vector3& a, const Vector3& b, const Vector3& c )
{
    register f32 crossX = ( b.y * c.z ) - ( b.z * c.y );
    register f32 crossY = ( b.z * c.x ) - ( b.x * c.z );
    register f32 crossZ = ( b.x * c.y ) - ( b.y * c.x );

    return ( a.x * crossX ) + ( a.y * crossY ) + ( a.z * crossZ );
}

ASDX_INLINE
void Vector3::ScalarTriple( const Vector3& a, const Vector3& b, const Vector3& c, f32& result )
{
    register f32 crossX = ( b.y * c.z ) - ( b.z * c.y );
    register f32 crossY = ( b.z * c.x ) - ( b.x * c.z );
    register f32 crossZ = ( b.x * c.y ) - ( b.y * c.x );

    result = ( a.x * crossX ) + ( a.y * crossY ) + ( a.z * crossZ );
}

ASDX_INLINE
Vector3 Vector3::VectorTriple( const Vector3& a, const Vector3& b, const Vector3& c )
{
    register f32 crossX = ( b.y * c.z ) - ( b.z * c.y );
    register f32 crossY = ( b.z * c.x ) - ( b.x * c.z );
    register f32 crossZ = ( b.x * c.y ) - ( b.y * c.x );

    return Vector3(
        ( ( a.y * crossZ ) - ( a.z * crossY ) ),
        ( ( a.z * crossX ) - ( a.x * crossZ ) ),
        ( ( a.x * crossY ) - ( a.y * crossX ) )
    );
}

ASDX_INLINE
void Vector3::VectorTriple( const Vector3& a, const Vector3& b, const Vector3& c, Vector3& result )
{
    register f32 crossX = ( b.y * c.z ) - ( b.z * c.y );
    register f32 crossY = ( b.z * c.x ) - ( b.x * c.z );
    register f32 crossZ = ( b.x * c.y ) - ( b.y * c.x );

    result.x = ( a.y * crossZ ) - ( a.z * crossY );
    result.y = ( a.z * crossX ) - ( a.x * crossZ );
    result.z = ( a.x * crossY ) - ( a.y * crossX );
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector4 structure
///////////////////////////////////////////////////////////////////////////////////////////////////

ASDX_INLINE
Vector4::Vector4()
{ /* DO_NOTHING */ }

ASDX_INLINE
Vector4::Vector4( const f32* pf )
{
    assert( pf != 0 );
    x = pf[ 0 ];
    y = pf[ 1 ];
    z = pf[ 2 ];
    w = pf[ 3 ];
}

ASDX_INLINE
Vector4::Vector4( const Vector2& value, const f32 nz, const f32 nw )
{
    x = value.x;
    y = value.y;
    z = nz;
    w = nw;
}

ASDX_INLINE
Vector4::Vector4( const Vector3& value, const f32 nw )
{
    x = value.x;
    y = value.y;
    z = value.z;
    w = nw;
}

ASDX_INLINE
Vector4::Vector4( const f32 nx, const f32 ny, const f32 nz, const f32 nw )
{
    x = nx;
    y = ny;
    z = nz;
    w = nw;
}

ASDX_INLINE
Vector4::operator f32 *()
{ return (f32*)&x; }

ASDX_INLINE
Vector4::operator const f32 *() const
{ return (const f32*)&x; }

ASDX_INLINE
Vector4& Vector4::operator += ( const Vector4& v )
{
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return (*this);
}

ASDX_INLINE
Vector4& Vector4::operator -= ( const Vector4& v )
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return (*this);
}

ASDX_INLINE
Vector4& Vector4::operator *= ( f32 f )
{
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return (*this);
}

ASDX_INLINE
Vector4& Vector4::operator /= ( f32 f )
{
    assert( f != 0.0f );
    x /= f;
    y /= f;
    z /= f;
    w /= f;
    return (*this);
}

ASDX_INLINE
Vector4& Vector4::operator = ( const Vector4& value )
{
    x = value.x;
    y = value.y;
    z = value.z;
    w = value.w;
    return (*this);
}

ASDX_INLINE
Vector4 Vector4::operator + () const
{ return (*this); }

ASDX_INLINE
Vector4 Vector4::operator - () const
{ return Vector4( -x, -y, -z, -w ); }

ASDX_INLINE
Vector4 Vector4::operator + ( const Vector4& v ) const
{ return Vector4( x + v.x, y + v.y, z + v.z, w + v.w ); }

ASDX_INLINE
Vector4 Vector4::operator - ( const Vector4& v ) const
{ return Vector4( x - v.x, y - v.y, z - v.z, w - v.w ); }

ASDX_INLINE
Vector4 Vector4::operator * ( f32 f ) const
{ return Vector4( x * f, y * f, z * f, w * f ); }

ASDX_INLINE
Vector4 Vector4::operator / ( f32 f ) const
{
    assert( f != 0.0f );
    return Vector4( x / f, y / f, z / f, w / f );
}

ASDX_INLINE
Vector4 operator * ( f32 f, const Vector4& v )
{ return Vector4( f * v.x, f * v.y, f * v.z, f * v.w ); }

ASDX_INLINE
bool Vector4::operator == ( const Vector4& v ) const
{
    return ( x == v.x )
        && ( y == v.y )
        && ( z == v.z )
        && ( w == v.z );
}

ASDX_INLINE
bool Vector4::operator != ( const Vector4& v ) const
{ 
    return ( x != v.x )
        || ( y != v.y )
        || ( z != v.z )
        || ( w != v.w );
}

ASDX_INLINE
f32 Vector4::Length() const
{ return sqrtf( x * x + y * y + z * z + w * w ); }

ASDX_INLINE
f32 Vector4::LengthSq() const
{ return ( x * x + y * y + z * z + w * w ); }

ASDX_INLINE
Vector4& Vector4::Normalize()
{
    register f32 mag = Length();
    assert( mag > 0.0f );
    x /= mag;
    y /= mag;
    z /= mag;
    w /= mag;
    return (*this);
}

ASDX_INLINE
Vector4& Vector4::SafeNormalize( const Vector4& set )
{
    register f32 mag = Length();
    if ( mag > 0.0f )
    {
        x /= mag;
        y /= mag;
        z /= mag;
        w /= mag;
    }
    else
    {
        x = set.x;
        y = set.y;
        z = set.z;
        w = set.w;
    }

    return (*this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector4  Methods
///////////////////////////////////////////////////////////////////////////////////////////////////
ASDX_INLINE
Vector4 Vector4::Abs( const Vector4& value )
{ 
    return Vector4( 
        fabs( value.x ),
        fabs( value.y ),
        fabs( value.z ),
        fabs( value.w )
    );
}

ASDX_INLINE
void Vector4::Abs( const Vector4 &value, Vector4 &result )
{ 
    result.x = fabs( value.x );
    result.y = fabs( value.y );
    result.z = fabs( value.z );
    result.w = fabs( value.w );
}


ASDX_INLINE
Vector4 Vector4::Clamp( const Vector4& value, const Vector4& a, const Vector4& b )
{
    return Vector4( 
        asdx::Clamp< f32 >( value.x, a.x, b.x ),
        asdx::Clamp< f32 >( value.y, a.y, b.y ),
        asdx::Clamp< f32 >( value.z, a.z, b.z ),
        asdx::Clamp< f32 >( value.w, a.w, b.w )
    );
}

ASDX_INLINE
void Vector4::Clamp( const Vector4 &value, const Vector4 &a, const Vector4 &b, Vector4 &result )
{
    result.x = asdx::Clamp< f32 >( value.x, a.x, b.x );
    result.y = asdx::Clamp< f32 >( value.y, a.y, b.y );
    result.z = asdx::Clamp< f32 >( value.z, a.z, b.z );
    result.w = asdx::Clamp< f32 >( value.w, a.w, b.w );
}

ASDX_INLINE
Vector4 Vector4::Saturate( const Vector4& value )
{
    return Vector4(
        asdx::Saturate( value.x ),
        asdx::Saturate( value.y ),
        asdx::Saturate( value.z ),
        asdx::Saturate( value.w )
    );
}

ASDX_INLINE
void Vector4::Saturate( const Vector4& value, Vector4& result )
{
    result.x = asdx::Saturate( value.x );
    result.y = asdx::Saturate( value.y );
    result.z = asdx::Saturate( value.z );
    result.w = asdx::Saturate( value.w );
}

ASDX_INLINE
f32 Vector4::Distance( const Vector4& a, const Vector4& b )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    register f32 Z = b.z - a.z;
    register f32 W = b.w - a.w;
    return sqrtf( X * X + Y * Y + Z * Z + W * W );
}

ASDX_INLINE
void Vector4::Distance( const Vector4 &a, const Vector4 &b, f32 &result )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    register f32 Z = b.z - a.z;
    register f32 W = b.w - a.w;
    result = sqrtf( X * X + Y * Y + Z * Z + W * W );
}

ASDX_INLINE
f32 Vector4::DistanceSq( const Vector4& a, const Vector4& b )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    register f32 Z = b.z - a.z;
    register f32 W = b.w - a.w;
    return X * X + Y * Y + Z * Z + W * W;
}

ASDX_INLINE
void Vector4::DistanceSq( const Vector4 &a, const Vector4 &b, f32 &result )
{
    register f32 X = b.x - a.x;
    register f32 Y = b.y - a.y;
    register f32 Z = b.z - a.z;
    register f32 W = b.w - a.w;
    result = X * X + Y * Y + Z * Z + W * W;
}

ASDX_INLINE
f32 Vector4::Dot( const Vector4& a, const Vector4& b )
{ return ( a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w ); }

ASDX_INLINE
void Vector4::Dot( const Vector4 &a, const Vector4 &b, f32 &result )
{
    result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

ASDX_INLINE
Vector4 Vector4::Normalize( const Vector4& value )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w );
    assert( mag > 0.0f );
    return Vector4(
        value.x / mag,
        value.y / mag,
        value.z / mag,
        value.w / mag
    );
}

ASDX_INLINE
void Vector4::Normalize( const Vector4 &value, Vector4 &result )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w );
    assert( mag > 0.0f );
    result.x = value.x / mag;
    result.y = value.y / mag;
    result.z = value.z / mag;
    result.w = value.w / mag;
}

ASDX_INLINE
Vector4 Vector4::SafeNormalize( const Vector4& value, const Vector4& set )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w );
    if ( mag > 0.0f )
    {
        return Vector4(
            value.x / mag,
            value.y / mag,
            value.z / mag,
            value.w / mag
        );
    }
    else
    {
        return Vector4(
            set.x,
            set.y,
            set.z,
            set.w 
        );
    }
}

ASDX_INLINE
void Vector4::SafeNormalize( const Vector4& value, const Vector4& set, Vector4& result)
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w );
    if ( mag > 0.0f )
    {
        result.x = value.x / mag;
        result.y = value.y / mag;
        result.z = value.z / mag;
        result.w = value.w / mag;
    }
    else
    {
        result.x = set.x;
        result.y = set.y;
        result.z = set.z;
        result.w = set.w;
    }
}

ASDX_INLINE
f32 Vector4::ComputeCrossingAngle( const Vector4& a, const Vector4& b )
{
    register f32 d = a.Length() * b.Length();
    if ( d <= 0.0f )
    { return 0.0f; }

    register f32 c = Vector4::Dot( a, b ) / d;
    if ( c >= 1.0f )
    { return 0.0f; }

    if ( c <= -1.0f ) 
    { return F_PI; }

    return acosf( c );
}

ASDX_INLINE
void Vector4::ComputeCrossingAngle( const Vector4 &a, const Vector4 &b, f32 &result )
{
    register f32 d = a.Length() * b.Length();
    if ( d <= 0.0f )
    {
        result = 0.0f;
        return;
    }

    register f32 c = Vector4::Dot( a, b ) / d;
    if ( c >= 1.0f ) 
    {
        result = 0.0f;
        return;
    }

    if ( c <= -1.0f )
    {
        result = F_PI;
        return;
    }

    result = acosf( c );
}

ASDX_INLINE
Vector4 Vector4::Min( const Vector4& a, const Vector4& b )
{ 
    return Vector4( 
        asdx::Min< f32 >( a.x, b.x ),
        asdx::Min< f32 >( a.y, b.y ),
        asdx::Min< f32 >( a.z, b.z ),
        asdx::Min< f32 >( a.w, b.w )
    );
}

ASDX_INLINE
void Vector4::Min( const Vector4 &a, const Vector4 &b, Vector4 &result )
{
    result.x = asdx::Min< f32 >( a.x, b.x );
    result.y = asdx::Min< f32 >( a.y, b.y );
    result.z = asdx::Min< f32 >( a.z, b.z );
    result.w = asdx::Min< f32 >( a.w, b.w );
}

ASDX_INLINE
Vector4 Vector4::Max( const Vector4& a, const Vector4& b )
{
    return Vector4( 
        asdx::Max< f32 >( a.x, b.x ),
        asdx::Max< f32 >( a.y, b.y ),
        asdx::Max< f32 >( a.z, b.z ),
        asdx::Max< f32 >( a.w, b.w )
    );
}

ASDX_INLINE
void Vector4::Max( const Vector4 &a, const Vector4 &b, Vector4 &result )
{
    result.x = asdx::Max< f32 >( a.x, b.x );
    result.y = asdx::Max< f32 >( a.y, b.y );
    result.z = asdx::Max< f32 >( a.z, b.z );
    result.w = asdx::Max< f32 >( a.w, b.w );
}

ASDX_INLINE
Vector4 Vector4::Barycentric( const Vector4& a, const Vector4& b, const Vector4& c, const f32 f, const f32 g )
{
    return Vector4(
        a.x + f * ( b.x - a.x ) + g * ( c.x - a.x ),
        a.y + f * ( b.y - a.y ) + g * ( c.y - a.y ),
        a.z + f * ( b.z - a.z ) + g * ( c.z - a.z ),
        a.w + f * ( b.w - a.w ) + g * ( c.w - a.w )
    );
}

ASDX_INLINE
void Vector4::Barycentric( const Vector4 &a, const Vector4 &b, const Vector4 &c, const f32 f, const f32 g , Vector4 &result )
{
    result.x = a.x + f * ( b.x - a.x ) + g * ( c.x - a.x );
    result.y = a.y + f * ( b.y - a.y ) + g * ( c.y - a.y );
    result.z = a.z + f * ( b.z - a.z ) + g * ( c.z - a.z );
    result.w = a.w + f * ( b.w - a.w ) + g * ( c.w - a.w );
}

ASDX_INLINE
Vector4 Vector4::Hermite( const Vector4& a, const Vector4& t1, const Vector4& b, const Vector4& t2, const f32 amount )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    Vector4 result;
    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
        result.z = a.z;
        result.w = a.w;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
        result.z = b.z;
        result.w = b.w;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
        result.z = ( 2.0f * a.z - 2.0f * b.z + t2.z + t1.z ) * c3 + ( 3.0f * b.z - 3.0f * a.z - 2.0f * t1.z - t2.z ) * c3 + t1.y * amount + a.z;
        result.w = ( 2.0f * a.w - 2.0f * b.w + t2.w + t1.w ) * c3 + ( 3.0f * b.w - 3.0f * a.w - 2.0f * t1.w - t2.w ) * c3 + t1.w * amount + a.w;
    }
    return result;
}

ASDX_INLINE
void Vector4::Hermite( const Vector4& a, const Vector4& t1, const Vector4& b, const Vector4& t2, const f32 amount, Vector4& result )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
        result.z = a.z;
        result.w = a.w;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
        result.z = b.z;
        result.w = b.w;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
        result.z = ( 2.0f * a.z - 2.0f * b.z + t2.z + t1.z ) * c3 + ( 3.0f * b.z - 3.0f * a.z - 2.0f * t1.z - t2.z ) * c3 + t1.z * amount + a.z;
        result.w = ( 2.0f * a.w - 2.0f * b.w + t2.w + t1.w ) * c3 + ( 3.0f * b.w - 3.0f * a.w - 2.0f * t1.w - t2.w ) * c3 + t1.w * amount + a.w;
    }
}

ASDX_INLINE
Vector4 Vector4::CatmullRom( const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d, const f32 amount )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    return Vector4(
        ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.z + ( c.z - a.z ) * amount + ( 2.0f * a.z - 5.0f * b.z + 4.0f * c.z - d.z ) * c2 + ( 3.0f * b.z - a.z - 3.0f * c.z + d.z ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.w + ( c.w - a.w ) * amount + ( 2.0f * a.w - 5.0f * b.w + 4.0f * c.w - d.w ) * c2 + ( 3.0f * b.w - a.w - 3.0f * c.w + d.w ) * c3 ) )
    );
}

ASDX_INLINE
void Vector4::CatmullRom( const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d, const f32 amount, Vector4& result )
{
    register f32 c2 = amount * amount;
    register f32 c3 = c2 * amount;

    result.x = ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) );
    result.y = ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) );
    result.z = ( 0.5f * ( 2.0f * b.z + ( c.z - a.z ) * amount + ( 2.0f * a.z - 5.0f * b.z + 4.0f * c.z - d.z ) * c2 + ( 3.0f * b.z - a.z - 3.0f * c.z + d.z ) * c3 ) );
    result.w = ( 0.5f * ( 2.0f * b.w + ( c.w - a.w ) * amount + ( 2.0f * a.w - 5.0f * b.w + 4.0f * c.w - d.w ) * c2 + ( 3.0f * b.w - a.w - 3.0f * c.w + d.w ) * c3 ) );
}

ASDX_INLINE
Vector4 Vector4::Lerp( const Vector4& a, const Vector4& b, const f32 amount )
{
    return Vector4(
        a.x - amount * ( a.x - b.x ),
        a.y - amount * ( a.y - b.y ),
        a.z - amount * ( a.z - b.z ),
        a.w - amount * ( a.w - b.w )
    );
}

ASDX_INLINE
void Vector4::Lerp( const Vector4 &a, const Vector4 &b, const f32 amount, Vector4 &result )
{
    result.x = a.x - amount * ( a.x - b.x );
    result.y = a.y - amount * ( a.y - b.y );
    result.z = a.z - amount * ( a.z - b.z );
    result.w = a.w - amount * ( a.w - b.w );
}

ASDX_INLINE
Vector4 Vector4::SmoothStep( const Vector4& a, const Vector4& b, const f32 amount )
{
    register f32 s = asdx::Clamp< f32 >( amount, 0.0f, 1.0f );
    register f32 u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    return Vector4(
        a.x - u * ( a.x - b.x ),
        a.y - u * ( a.y - b.y ),
        a.z - u * ( a.z - b.z ),
        a.w - u * ( a.w - b.w )
    );
}

ASDX_INLINE
void Vector4::SmoothStep( const Vector4 &a, const Vector4 &b, const f32 amount, Vector4 &result )
{
    register f32 s = asdx::Clamp< f32 >( amount, 0.0f, 1.0f );
    register f32 u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    result.x = a.x - u * ( a.x - b.x );
    result.y = a.y - u * ( a.y - b.y );
    result.z = a.z - u * ( a.z - b.z );
    result.w = a.w - u * ( a.w - b.w );
}

ASDX_INLINE
Vector4 Vector4::Transform( const Vector4& position, const Matrix& matrix )
{
    return Vector4(
        ( ( ((position.x * matrix._11) + (position.y * matrix._21)) + (position.z * matrix._31) ) + (position.w * matrix._41)),
        ( ( ((position.x * matrix._12) + (position.y * matrix._22)) + (position.z * matrix._32) ) + (position.w * matrix._42)),
        ( ( ((position.x * matrix._13) + (position.y * matrix._23)) + (position.z * matrix._33) ) + (position.w * matrix._43)),
        ( ( ((position.x * matrix._14) + (position.y * matrix._24)) + (position.z * matrix._43) ) + (position.w * matrix._44)) );
}

ASDX_INLINE
void Vector4::Transform( const Vector4 &position, const Matrix &matrix, Vector4 &result )
{
    result.x = ( ( ((position.x * matrix._11) + (position.y * matrix._21)) + (position.z * matrix._31) ) + (position.w * matrix._41));
    result.y = ( ( ((position.x * matrix._12) + (position.y * matrix._22)) + (position.z * matrix._32) ) + (position.w * matrix._42));
    result.z = ( ( ((position.x * matrix._13) + (position.y * matrix._23)) + (position.z * matrix._33) ) + (position.w * matrix._43));
    result.w = ( ( ((position.x * matrix._14) + (position.y * matrix._24)) + (position.z * matrix._43) ) + (position.w * matrix._44));
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector2A structure
///////////////////////////////////////////////////////////////////////////////////////////////////
ASDX_INLINE
Vector2A::Vector2A()
{ /* DO_NOTHING */ }

ASDX_INLINE
Vector2A::Vector2A( const f32 nx, const f32 ny )
: Vector2( nx, ny )
{ /* DO_NOTHING */ }

ASDX_INLINE
Vector2A::Vector2A( const Vector2& value )
: Vector2( value )
{ /* DO_NOTHING */ }

ASDX_INLINE
Vector2A::Vector2A( const Vector2A& value )
: Vector2( value.x, value.y )
{ /* DO_NOTHING */ }

ASDX_INLINE
Vector2A& Vector2A::operator = ( const Vector2& value )
{
    x = value.x;
    y = value.y;
    return (*this);
}

ASDX_INLINE
Vector2A& Vector2A::operator = ( const Vector2A& value )
{
    x = value.x;
    y = value.y;
    return (*this);
}

ASDX_INLINE
Vector2 Vector2A::ToVector2() const
{ return Vector2( x, y ); }


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector3A structure
///////////////////////////////////////////////////////////////////////////////////////////////////
ASDX_INLINE
Vector3A::Vector3A()
{ /* DO_NOTHING */ }

ASDX_INLINE
Vector3A::Vector3A( const f32 nx, const f32 ny, const f32 nz )
: Vector3( nx, ny, nz )
{ /* DO_NOTHING */ }

ASDX_INLINE
Vector3A::Vector3A( const Vector3& value )
: Vector3( value )
{ /* DO_NOTHING */ }

ASDX_INLINE
Vector3A::Vector3A( const Vector3A& value )
: Vector3( value.x, value.y, value.z )
{ /* DO_NOTHING */ }

ASDX_INLINE
Vector3A& Vector3A::operator = ( const Vector3& value )
{
    x = value.x;
    y = value.y;
    z = value.z;
    return (*this);
}

ASDX_INLINE
Vector3A& Vector3A::operator = ( const Vector3A& value )
{
    x = value.x;
    y = value.y;
    z = value.z;
    return (*this);
}

ASDX_INLINE
Vector3 Vector3A::ToVector3() const
{ return Vector3( x, y, z ); }


///////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix structure (row-major)
///////////////////////////////////////////////////////////////////////////////////////////////////
ASDX_INLINE
Matrix::Matrix()
{ /* DO_NOTHING */ }

ASDX_INLINE
Matrix::Matrix( const f32* pf )
{
    assert( pf != 0 );
    memcpy( &_11, pf, sizeof(Matrix) );
}

ASDX_INLINE
Matrix::Matrix( f32 _f11, f32 _f12, f32 _f13, f32 _f14,
                f32 _f21, f32 _f22, f32 _f23, f32 _f24,
                f32 _f31, f32 _f32, f32 _f33, f32 _f34,
                f32 _f41, f32 _f42, f32 _f43, f32 _f44 )
{
    _11 = _f11; _12 = _f12; _13 = _f13; _14 = _f14;
    _21 = _f21; _22 = _f22; _23 = _f23; _24 = _f24;
    _31 = _f31; _32 = _f32; _33 = _f33; _34 = _f34;
    _41 = _f41; _42 = _f42; _43 = _f43; _44 = _f44;
}

ASDX_INLINE 
f32& Matrix::operator () ( u32 iRow, u32 iCol )
{ return m[iRow][iCol]; }

ASDX_INLINE 
f32 Matrix::operator () ( u32 iRow, u32 iCol ) const
{ return m[iRow][iCol]; }

ASDX_INLINE
Matrix::operator f32* ()
{ return (f32*)&_11; }

ASDX_INLINE
Matrix::operator const f32* () const 
{ return (const f32*)&_11; }

ASDX_INLINE 
Matrix& Matrix::operator *= ( const Matrix &mat )
{
    Matrix tmp = (*this);
    (*this) = Multiply( tmp, mat );
    return (*this);
}

ASDX_INLINE 
Matrix& Matrix::operator += ( const Matrix& mat )
{
    _11 += mat._11; _12 += mat._12; _13 += mat._13; _14 += mat._14;
    _21 += mat._21; _22 += mat._22; _23 += mat._23; _24 += mat._24;
    _31 += mat._31; _32 += mat._32; _33 += mat._33; _34 += mat._34;
    _41 += mat._41; _42 += mat._42; _43 += mat._43; _44 += mat._44;
    return (*this);
}

ASDX_INLINE 
Matrix& Matrix::operator -= ( const Matrix& mat )
{
    _11 -= mat._11; _12 -= mat._12; _13 -= mat._13; _14 -= mat._14;
    _21 -= mat._21; _22 -= mat._22; _23 -= mat._23; _24 -= mat._24;
    _31 -= mat._31; _32 -= mat._32; _33 -= mat._33; _34 -= mat._34;
    _41 -= mat._41; _42 -= mat._42; _43 -= mat._43; _44 -= mat._44;
    return (*this);
}

ASDX_INLINE
Matrix& Matrix::operator *= ( f32 f )
{
    _11 *= f; _12 *= f; _13 *= f; _14 *= f;
    _21 *= f; _22 *= f; _23 *= f; _24 *= f;
    _31 *= f; _32 *= f; _33 *= f; _34 *= f;
    _41 *= f; _42 *= f; _43 *= f; _44 *= f;
    return (*this);
}

ASDX_INLINE 
Matrix& Matrix::operator /= ( f32 f )
{
    assert( f != 0.0f );
    _11 /= f; _12 /= f; _13 /= f; _14 /= f;
    _21 /= f; _22 /= f; _23 /= f; _24 /= f;
    _31 /= f; _32 /= f; _33 /= f; _34 /= f;
    _41 /= f; _42 /= f; _43 /= f; _44 /= f;
    return (*this);
}

ASDX_INLINE
Matrix& Matrix::operator = ( const Matrix& value )
{
    memcpy( &_11, &value._11, sizeof(Matrix) );
    return (*this);
}

ASDX_INLINE
Matrix Matrix::operator + () const
{ 
    return Matrix( _11, _12, _13, _14,
                   _21, _22, _23, _24,
                   _31, _32, _33, _34,
                   _41, _42, _43, _44 );
}

ASDX_INLINE
Matrix Matrix::operator - () const
{
    return Matrix( -_11, -_12, -_13, -_14,
                   -_21, -_22, -_23, -_24,
                   -_31, -_32, -_33, -_34,
                   -_41, -_42, -_43, -_44 );
}

ASDX_INLINE 
Matrix Matrix::operator * ( const Matrix& value ) const
{
#if 0
    Matrix tmp = (*this);
    Matrix result = Multiply( tmp, value );
    return result;
#else
    return Matrix(
        ( _11 * value._11 ) + ( _12 * value._21 ) + ( _13 * value._31 ) + ( _14 * value._41 ),
        ( _11 * value._12 ) + ( _12 * value._22 ) + ( _13 * value._32 ) + ( _14 * value._42 ),
        ( _11 * value._13 ) + ( _12 * value._23 ) + ( _13 * value._33 ) + ( _14 * value._43 ),
        ( _11 * value._14 ) + ( _12 * value._24 ) + ( _13 * value._34 ) + ( _14 * value._44 ),

        ( _21 * value._11 ) + ( _22 * value._21 ) + ( _23 * value._31 ) + ( _24 * value._41 ),
        ( _21 * value._12 ) + ( _22 * value._22 ) + ( _23 * value._32 ) + ( _24 * value._42 ),
        ( _21 * value._13 ) + ( _22 * value._23 ) + ( _23 * value._33 ) + ( _24 * value._43 ),
        ( _21 * value._14 ) + ( _22 * value._24 ) + ( _23 * value._34 ) + ( _24 * value._44 ),

        ( _31 * value._11 ) + ( _32 * value._21 ) + ( _33 * value._31 ) + ( _34 * value._41 ),
        ( _31 * value._12 ) + ( _32 * value._22 ) + ( _33 * value._32 ) + ( _34 * value._42 ),
        ( _31 * value._13 ) + ( _32 * value._23 ) + ( _33 * value._33 ) + ( _34 * value._43 ),
        ( _31 * value._14 ) + ( _32 * value._24 ) + ( _33 * value._34 ) + ( _34 * value._44 ),

        ( _41 * value._11 ) + ( _42 * value._21 ) + ( _43 * value._31 ) + ( _44 * value._41 ),
        ( _41 * value._12 ) + ( _42 * value._22 ) + ( _43 * value._32 ) + ( _44 * value._42 ),
        ( _41 * value._13 ) + ( _42 * value._23 ) + ( _43 * value._33 ) + ( _44 * value._43 ),
        ( _41 * value._14 ) + ( _42 * value._24 ) + ( _43 * value._34 ) + ( _44 * value._44 )
    );
#endif
}

ASDX_INLINE 
Matrix Matrix::operator + ( const Matrix& mat ) const
{
    return Matrix( _11 + mat._11, _12 + mat._12, _13 + mat._13, _14 + mat._14,
                   _21 + mat._21, _22 + mat._22, _23 + mat._23, _24 + mat._24,
                   _31 + mat._31, _32 + mat._32, _33 + mat._33, _34 + mat._34,
                   _41 + mat._41, _42 + mat._42, _43 + mat._43, _44 + mat._44 );
}

ASDX_INLINE 
Matrix Matrix::operator - ( const Matrix& mat ) const
{
    return Matrix( _11 - mat._11, _12 - mat._12, _13 - mat._13, _14 - mat._14,
                   _21 - mat._21, _22 - mat._22, _23 - mat._23, _24 - mat._24,
                   _31 - mat._31, _32 - mat._32, _33 - mat._33, _34 - mat._34,
                   _41 - mat._41, _42 - mat._42, _43 - mat._43, _44 - mat._44 );
}

ASDX_INLINE
Matrix Matrix::operator * ( f32 f ) const
{
    return Matrix( _11 * f, _12 * f, _13 * f, _14 * f,
                   _21 * f, _22 * f, _23 * f, _24 * f,
                   _31 * f, _32 * f, _33 * f, _34 * f,
                   _41 * f, _42 * f, _43 * f, _44 * f );
}

ASDX_INLINE
Matrix Matrix::operator / ( f32 f ) const
{
    assert( f != 0.0f );
    return Matrix( _11 / f, _12 / f, _13 / f, _14 / f,
                   _21 / f, _22 / f, _23 / f, _24 / f,
                   _31 / f, _32 / f, _33 / f, _34 / f,
                   _41 / f, _42 / f, _43 / f, _44 / f);
}

ASDX_INLINE 
Matrix operator * ( f32 f, const Matrix& mat )
{
    return Matrix( f * mat._11, f * mat._12, f * mat._13, f * mat._14,
                   f * mat._21, f * mat._22, f * mat._23, f * mat._24,
                   f * mat._31, f * mat._32, f * mat._33, f * mat._34,
                   f * mat._41, f * mat._42, f * mat._43, f * mat._44 );
}

ASDX_INLINE 
bool Matrix::operator == ( const Matrix& mat ) const
{ return ( 0 == memcmp( this, &mat, sizeof( Matrix ) ) ); }

ASDX_INLINE 
bool Matrix::operator != ( const Matrix& mat ) const
{ return ( 0 != memcmp( this, &mat, sizeof( Matrix ) ) ); }

ASDX_INLINE 
f32 Matrix::Determinant() const
{
    f32 det =
        _11*_22*_33*_44 + _11*_23*_34*_42 +
        _11*_24*_32*_43 + _12*_21*_34*_43 +
        _12*_23*_31*_44 + _12*_24*_33*_41 +
        _13*_21*_32*_44 + _13*_22*_34*_41 +
        _13*_24*_31*_42 + _14*_21*_33*_42 +
        _14*_22*_31*_43 + _14*_23*_32*_41 -
        _11*_22*_34*_43 - _11*_23*_32*_44 -
        _11*_24*_33*_42 - _12*_21*_33*_44 -
        _12*_23*_34*_41 - _12*_24*_31*_43 -
        _13*_21*_34*_42 - _13*_22*_31*_44 -
        _13*_24*_32*_41 - _14*_21*_32*_43 -
        _14*_22*_33*_41 - _14*_23*_31*_42;
    return det;
}

ASDX_INLINE 
Matrix& Matrix::Neutral()
{
    _11 = _22 = _33 = _44 = 1.0f;
    _12 = _13 = _14 =
    _21 = _23 = _24 =
    _31 = _32 = _34 =
    _41 = _42 = _43 = 0.0f;
    return (*this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix Methods (row-major)
///////////////////////////////////////////////////////////////////////////////////////////////////

ASDX_INLINE
Matrix Matrix::Identity()
{
    return Matrix(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f );
}

ASDX_INLINE
bool Matrix::IsIdentity( const Matrix &value )
{
    return (
        IsEqual( value.m[0][0], 1.0f ) && IsZero( value.m[0][1] ) && IsZero( value.m[0][2] ) && IsZero( value.m[0][3] ) &&
        IsZero( value.m[1][0] ) && IsEqual( value.m[1][1], 1.0f ) && IsZero( value.m[1][2] ) && IsZero( value.m[1][3] ) &&
        IsZero( value.m[2][0] ) && IsZero( value.m[2][1] ) && IsEqual( value.m[2][2] , 1.0f ) && IsZero( value.m[2][3] ) &&
        IsZero( value.m[3][0] ) && IsZero( value.m[3][1] ) && IsZero( value.m[3][2] ) && IsEqual( value.m[3][3], 1.0f ) );
}

ASDX_INLINE
Matrix Matrix::Transpose( const Matrix& value )
{
    return Matrix(
        value._11, value._21, value._31, value._41,
        value._12, value._22, value._32, value._42,
        value._13, value._23, value._33, value._43,
        value._14, value._24, value._34, value._44 );
}

ASDX_INLINE
void Matrix::Transpose( const Matrix &value, Matrix &result )
{
    result._11 = value._11;
    result._12 = value._21;
    result._13 = value._31;
    result._14 = value._41;

    result._21 = value._12;
    result._22 = value._22;
    result._23 = value._32;
    result._24 = value._42;

    result._31 = value._13;
    result._32 = value._23;
    result._33 = value._33;
    result._34 = value._34;

    result._41 = value._14;
    result._42 = value._24;
    result._43 = value._34;
    result._44 = value._44;
}

ASDX_INLINE
Matrix Matrix::Multiply( const Matrix& a, const Matrix& b )
{
#if 0
    Matrix tmp;
    for ( s32 i=0; i<4; i++ )
        for ( s32 j=0; j<4; j++ )
            tmp.m[i][j] = a.m[i][0] * b.m[0][j] +
                          a.m[i][1] * b.m[1][j] +
                          a.m[i][2] * b.m[2][j] +
                          a.m[i][3] * b.m[3][j];
    return tmp;
#else
    return Matrix(
        ( a._11 * b._11 ) + ( a._12 * b._21 ) + ( a._13 * b._31 ) + ( a._14 * b._41 ),
        ( a._11 * b._12 ) + ( a._12 * b._22 ) + ( a._13 * b._32 ) + ( a._14 * b._42 ),
        ( a._11 * b._13 ) + ( a._12 * b._23 ) + ( a._13 * b._33 ) + ( a._14 * b._43 ),
        ( a._11 * b._14 ) + ( a._12 * b._24 ) + ( a._13 * b._34 ) + ( a._14 * b._44 ),

        ( a._21 * b._11 ) + ( a._22 * b._21 ) + ( a._23 * b._31 ) + ( a._24 * b._41 ),
        ( a._21 * b._12 ) + ( a._22 * b._22 ) + ( a._23 * b._32 ) + ( a._24 * b._42 ),
        ( a._21 * b._13 ) + ( a._22 * b._23 ) + ( a._23 * b._33 ) + ( a._24 * b._43 ),
        ( a._21 * b._14 ) + ( a._22 * b._24 ) + ( a._23 * b._34 ) + ( a._24 * b._44 ),

        ( a._31 * b._11 ) + ( a._32 * b._21 ) + ( a._33 * b._31 ) + ( a._34 * b._41 ),
        ( a._31 * b._12 ) + ( a._32 * b._22 ) + ( a._33 * b._32 ) + ( a._34 * b._42 ),
        ( a._31 * b._13 ) + ( a._32 * b._23 ) + ( a._33 * b._33 ) + ( a._34 * b._43 ),
        ( a._31 * b._14 ) + ( a._32 * b._24 ) + ( a._33 * b._34 ) + ( a._34 * b._44 ),

        ( a._41 * b._11 ) + ( a._42 * b._21 ) + ( a._43 * b._31 ) + ( a._44 * b._41 ),
        ( a._41 * b._12 ) + ( a._42 * b._22 ) + ( a._43 * b._32 ) + ( a._44 * b._42 ),
        ( a._41 * b._13 ) + ( a._42 * b._23 ) + ( a._43 * b._33 ) + ( a._44 * b._43 ),
        ( a._41 * b._14 ) + ( a._42 * b._24 ) + ( a._43 * b._34 ) + ( a._44 * b._44 )
    );
#endif
}

ASDX_INLINE
void Matrix::Multiply( const Matrix &a, const Matrix &b, Matrix &result )
{
#if 0
    for ( s32 i=0; i<4; i++ )
        for ( s32 j=0; j<4; j++ )
            result.m[i][j] = a.m[i][0] * b.m[0][j] +
                             a.m[i][1] * b.m[1][j] +
                             a.m[i][2] * b.m[2][j] +
                             a.m[i][3] * b.m[3][j];
#else
    result._11 = ( a._11 * b._11 ) + ( a._12 * b._21 ) + ( a._13 * b._31 ) + ( a._14 * b._41 );
    result._12 = ( a._11 * b._12 ) + ( a._12 * b._22 ) + ( a._13 * b._32 ) + ( a._14 * b._42 );
    result._13 = ( a._11 * b._13 ) + ( a._12 * b._23 ) + ( a._13 * b._33 ) + ( a._14 * b._43 );
    result._14 = ( a._11 * b._14 ) + ( a._12 * b._24 ) + ( a._13 * b._34 ) + ( a._14 * b._44 );

    result._21 = ( a._21 * b._11 ) + ( a._22 * b._21 ) + ( a._23 * b._31 ) + ( a._24 * b._41 );
    result._22 = ( a._21 * b._12 ) + ( a._22 * b._22 ) + ( a._23 * b._32 ) + ( a._24 * b._42 );
    result._23 = ( a._21 * b._13 ) + ( a._22 * b._23 ) + ( a._23 * b._33 ) + ( a._24 * b._43 );
    result._24 = ( a._21 * b._14 ) + ( a._22 * b._24 ) + ( a._23 * b._34 ) + ( a._24 * b._44 );

    result._31 = ( a._31 * b._11 ) + ( a._32 * b._21 ) + ( a._33 * b._31 ) + ( a._34 * b._41 );
    result._32 = ( a._31 * b._12 ) + ( a._32 * b._22 ) + ( a._33 * b._32 ) + ( a._34 * b._42 );
    result._33 = ( a._31 * b._13 ) + ( a._32 * b._23 ) + ( a._33 * b._33 ) + ( a._34 * b._43 );
    result._34 = ( a._31 * b._14 ) + ( a._32 * b._24 ) + ( a._33 * b._34 ) + ( a._34 * b._44 );

    result._41 = ( a._41 * b._11 ) + ( a._42 * b._21 ) + ( a._43 * b._31 ) + ( a._44 * b._41 );
    result._42 = ( a._41 * b._12 ) + ( a._42 * b._22 ) + ( a._43 * b._32 ) + ( a._44 * b._42 );
    result._43 = ( a._41 * b._13 ) + ( a._42 * b._23 ) + ( a._43 * b._33 ) + ( a._44 * b._43 );
    result._44 = ( a._41 * b._14 ) + ( a._42 * b._24 ) + ( a._43 * b._34 ) + ( a._44 * b._44 );
#endif
}

ASDX_INLINE
Matrix Matrix::Multiply( const Matrix& value, const f32 scaleFactor )
{
    return Matrix(
        value._11*scaleFactor, value._12*scaleFactor, value._13*scaleFactor, value._14*scaleFactor,
        value._21*scaleFactor, value._22*scaleFactor, value._23*scaleFactor, value._24*scaleFactor,
        value._31*scaleFactor, value._32*scaleFactor, value._33*scaleFactor, value._34*scaleFactor,
        value._41*scaleFactor, value._42*scaleFactor, value._43*scaleFactor, value._44*scaleFactor
        );
}

ASDX_INLINE
void Matrix::Multiply( const Matrix &value, const f32 scaleFactor, Matrix &result )
{
    result._11 = value._11 * scaleFactor;
    result._12 = value._12 * scaleFactor;
    result._13 = value._13 * scaleFactor;
    result._14 = value._14 * scaleFactor;

    result._21 = value._21 * scaleFactor;
    result._22 = value._22 * scaleFactor;
    result._23 = value._23 * scaleFactor;
    result._24 = value._24 * scaleFactor;

    result._31 = value._31 * scaleFactor;
    result._32 = value._32 * scaleFactor;
    result._33 = value._33 * scaleFactor;
    result._34 = value._34 * scaleFactor;

    result._41 = value._41 * scaleFactor;
    result._42 = value._42 * scaleFactor;
    result._43 = value._43 * scaleFactor;
    result._44 = value._44 * scaleFactor;
}

ASDX_INLINE
Matrix Matrix::MultiplyTranspose( const Matrix& a, const Matrix& b )
{
#if 0
    Matrix result;
    result = Multiply( a, b );
    result = Transpose( result );
    return result;
#else
    return Matrix(
        ( a._11 * b._11 ) + ( a._12 * b._21 ) + ( a._13 * b._31 ) + ( a._14 * b._41 ),
        ( a._11 * b._12 ) + ( a._12 * b._22 ) + ( a._13 * b._32 ) + ( a._14 * b._42 ),
        ( a._11 * b._13 ) + ( a._12 * b._23 ) + ( a._13 * b._33 ) + ( a._14 * b._43 ),
        ( a._11 * b._14 ) + ( a._12 * b._24 ) + ( a._13 * b._34 ) + ( a._14 * b._44 ),

        ( a._21 * b._11 ) + ( a._22 * b._21 ) + ( a._23 * b._31 ) + ( a._24 * b._41 ),
        ( a._21 * b._12 ) + ( a._22 * b._22 ) + ( a._23 * b._32 ) + ( a._24 * b._42 ),
        ( a._21 * b._13 ) + ( a._22 * b._23 ) + ( a._23 * b._33 ) + ( a._24 * b._43 ),
        ( a._21 * b._14 ) + ( a._22 * b._24 ) + ( a._23 * b._34 ) + ( a._24 * b._44 ),

        ( a._31 * b._11 ) + ( a._32 * b._21 ) + ( a._33 * b._31 ) + ( a._34 * b._41 ),
        ( a._31 * b._12 ) + ( a._32 * b._22 ) + ( a._33 * b._32 ) + ( a._34 * b._42 ),
        ( a._31 * b._13 ) + ( a._32 * b._23 ) + ( a._33 * b._33 ) + ( a._34 * b._43 ),
        ( a._31 * b._14 ) + ( a._32 * b._24 ) + ( a._33 * b._34 ) + ( a._34 * b._44 ),

        ( a._41 * b._11 ) + ( a._42 * b._21 ) + ( a._43 * b._31 ) + ( a._44 * b._41 ),
        ( a._41 * b._12 ) + ( a._42 * b._22 ) + ( a._43 * b._32 ) + ( a._44 * b._42 ),
        ( a._41 * b._13 ) + ( a._42 * b._23 ) + ( a._43 * b._33 ) + ( a._44 * b._43 ),
        ( a._41 * b._14 ) + ( a._42 * b._24 ) + ( a._43 * b._34 ) + ( a._44 * b._44 )
    );
#endif
}

ASDX_INLINE
void Matrix::MultiplyTranspose( const Matrix &a, const Matrix &b, Matrix &result )
{
#if 0
    result = Multiply( a, b );
    result = Transpose( result );
#else
    result._11 = ( a._11 * b._11 ) + ( a._12 * b._21 ) + ( a._13 * b._31 ) + ( a._14 * b._41 );
    result._21 = ( a._11 * b._12 ) + ( a._12 * b._22 ) + ( a._13 * b._32 ) + ( a._14 * b._42 );
    result._31 = ( a._11 * b._13 ) + ( a._12 * b._23 ) + ( a._13 * b._33 ) + ( a._14 * b._43 );
    result._41 = ( a._11 * b._14 ) + ( a._12 * b._24 ) + ( a._13 * b._34 ) + ( a._14 * b._44 );

    result._12 = ( a._21 * b._11 ) + ( a._22 * b._21 ) + ( a._23 * b._31 ) + ( a._24 * b._41 );
    result._22 = ( a._21 * b._12 ) + ( a._22 * b._22 ) + ( a._23 * b._32 ) + ( a._24 * b._42 );
    result._32 = ( a._21 * b._13 ) + ( a._22 * b._23 ) + ( a._23 * b._33 ) + ( a._24 * b._43 );
    result._42 = ( a._21 * b._14 ) + ( a._22 * b._24 ) + ( a._23 * b._34 ) + ( a._24 * b._44 );

    result._13 = ( a._31 * b._11 ) + ( a._32 * b._21 ) + ( a._33 * b._31 ) + ( a._34 * b._41 );
    result._23 = ( a._31 * b._12 ) + ( a._32 * b._22 ) + ( a._33 * b._32 ) + ( a._34 * b._42 );
    result._33 = ( a._31 * b._13 ) + ( a._32 * b._23 ) + ( a._33 * b._33 ) + ( a._34 * b._43 );
    result._43 = ( a._31 * b._14 ) + ( a._32 * b._24 ) + ( a._33 * b._34 ) + ( a._34 * b._44 );

    result._14 = ( a._41 * b._11 ) + ( a._42 * b._21 ) + ( a._43 * b._31 ) + ( a._44 * b._41 );
    result._24 = ( a._41 * b._12 ) + ( a._42 * b._22 ) + ( a._43 * b._32 ) + ( a._44 * b._42 );
    result._34 = ( a._41 * b._13 ) + ( a._42 * b._23 ) + ( a._43 * b._33 ) + ( a._44 * b._43 );
    result._44 = ( a._41 * b._14 ) + ( a._42 * b._24 ) + ( a._43 * b._34 ) + ( a._44 * b._44 );

#endif
}

ASDX_INLINE 
Matrix Matrix::Invert( const Matrix& value )
{
    Matrix result;
    register f32 det = value.Determinant();
    assert( det != 0.0f );

    result._11 = value._22*value._33*value._44 + value._23*value._34*value._42 + value._24*value._32*value._43 - value._22*value._34*value._43 - value._23*value._32*value._44 - value._24*value._33*value._42;
    result._12 = value._12*value._34*value._43 + value._13*value._32*value._44 + value._14*value._33*value._42 - value._12*value._33*value._44 - value._13*value._34*value._42 - value._14*value._32*value._43;
    result._13 = value._12*value._23*value._44 + value._13*value._24*value._42 + value._14*value._22*value._43 - value._12*value._24*value._43 - value._13*value._22*value._44 - value._14*value._23*value._42;
    result._14 = value._12*value._24*value._33 + value._13*value._22*value._34 + value._14*value._23*value._32 - value._12*value._23*value._34 - value._13*value._24*value._32 - value._14*value._22*value._33;

    result._21 = value._21*value._34*value._43 + value._23*value._31*value._44 + value._24*value._33*value._41 - value._21*value._33*value._44 - value._23*value._34*value._41 - value._24*value._31*value._43;
    result._22 = value._11*value._33*value._44 + value._13*value._34*value._41 + value._14*value._31*value._43 - value._11*value._34*value._43 - value._13*value._31*value._44 - value._14*value._33*value._41;
    result._23 = value._11*value._24*value._43 + value._13*value._21*value._44 + value._14*value._23*value._41 - value._11*value._23*value._44 - value._13*value._24*value._41 - value._14*value._21*value._43;
    result._24 = value._11*value._23*value._34 + value._13*value._24*value._31 + value._14*value._21*value._33 - value._11*value._24*value._33 - value._13*value._21*value._34 - value._14*value._23*value._31;

    result._31 = value._21*value._32*value._44 + value._22*value._34*value._41 + value._24*value._31*value._42 - value._21*value._34*value._42 - value._22*value._31*value._44 - value._24*value._32*value._41;
    result._32 = value._11*value._34*value._42 + value._12*value._31*value._44 + value._14*value._32*value._41 - value._11*value._32*value._44 - value._12*value._34*value._41 - value._14*value._31*value._42;
    result._33 = value._11*value._22*value._44 + value._12*value._24*value._41 + value._14*value._21*value._42 - value._11*value._24*value._42 - value._12*value._21*value._44 - value._14*value._22*value._41;
    result._34 = value._11*value._24*value._32 + value._12*value._21*value._34 + value._14*value._22*value._31 - value._11*value._22*value._34 - value._12*value._24*value._31 - value._14*value._21*value._32;

    result._41 = value._21*value._33*value._42 + value._22*value._31*value._43 + value._23*value._32*value._41 - value._21*value._32*value._43 - value._22*value._33*value._41 - value._23*value._31*value._42;
    result._42 = value._11*value._32*value._43 + value._12*value._33*value._41 + value._13*value._31*value._42 - value._11*value._33*value._42 - value._12*value._31*value._43 - value._13*value._32*value._41;
    result._43 = value._11*value._23*value._42 + value._12*value._21*value._43 + value._13*value._22*value._41 - value._11*value._22*value._43 - value._12*value._23*value._41 - value._13*value._21*value._42;
    result._44 = value._11*value._22*value._33 + value._12*value._23*value._31 + value._13*value._21*value._32 - value._11*value._23*value._32 - value._12*value._21*value._33 - value._13*value._22*value._31;

    result._11 /= det;
    result._12 /= det;
    result._13 /= det;
    result._14 /= det;

    result._21 /= det;
    result._22 /= det;
    result._23 /= det;
    result._24 /= det;

    result._31 /= det;
    result._32 /= det;
    result._33 /= det;
    result._34 /= det;

    result._41 /= det;
    result._42 /= det;
    result._43 /= det;
    result._44 /= det;

    return result;
}

ASDX_INLINE
void Matrix::Invert( const Matrix &value, Matrix &result )
{ 
    register f32 det = value.Determinant();
    assert( det != 0.0f );

    result._11 = value._22*value._33*value._44 + value._23*value._34*value._42 + value._24*value._32*value._43 - value._22*value._34*value._43 - value._23*value._32*value._44 - value._24*value._33*value._42;
    result._12 = value._12*value._34*value._43 + value._13*value._32*value._44 + value._14*value._33*value._42 - value._12*value._33*value._44 - value._13*value._34*value._42 - value._14*value._32*value._43;
    result._13 = value._12*value._23*value._44 + value._13*value._24*value._42 + value._14*value._22*value._43 - value._12*value._24*value._43 - value._13*value._22*value._44 - value._14*value._23*value._42;
    result._14 = value._12*value._24*value._33 + value._13*value._22*value._34 + value._14*value._23*value._32 - value._12*value._23*value._34 - value._13*value._24*value._32 - value._14*value._22*value._33;

    result._21 = value._21*value._34*value._43 + value._23*value._31*value._44 + value._24*value._33*value._41 - value._21*value._33*value._44 - value._23*value._34*value._41 - value._24*value._31*value._43;
    result._22 = value._11*value._33*value._44 + value._13*value._34*value._41 + value._14*value._31*value._43 - value._11*value._34*value._43 - value._13*value._31*value._44 - value._14*value._33*value._41;
    result._23 = value._11*value._24*value._43 + value._13*value._21*value._44 + value._14*value._23*value._41 - value._11*value._23*value._44 - value._13*value._24*value._41 - value._14*value._21*value._43;
    result._24 = value._11*value._23*value._34 + value._13*value._24*value._31 + value._14*value._21*value._33 - value._11*value._24*value._33 - value._13*value._21*value._34 - value._14*value._23*value._31;

    result._31 = value._21*value._32*value._44 + value._22*value._34*value._41 + value._24*value._31*value._42 - value._21*value._34*value._42 - value._22*value._31*value._44 - value._24*value._32*value._41;
    result._32 = value._11*value._34*value._42 + value._12*value._31*value._44 + value._14*value._32*value._41 - value._11*value._32*value._44 - value._12*value._34*value._41 - value._14*value._31*value._42;
    result._33 = value._11*value._22*value._44 + value._12*value._24*value._41 + value._14*value._21*value._42 - value._11*value._24*value._42 - value._12*value._21*value._44 - value._14*value._22*value._41;
    result._34 = value._11*value._24*value._32 + value._12*value._21*value._34 + value._14*value._22*value._31 - value._11*value._22*value._34 - value._12*value._24*value._31 - value._14*value._21*value._32;

    result._41 = value._21*value._33*value._42 + value._22*value._31*value._43 + value._23*value._32*value._41 - value._21*value._32*value._43 - value._22*value._33*value._41 - value._23*value._31*value._42;
    result._42 = value._11*value._32*value._43 + value._12*value._33*value._41 + value._13*value._31*value._42 - value._11*value._33*value._42 - value._12*value._31*value._43 - value._13*value._32*value._41;
    result._43 = value._11*value._23*value._42 + value._12*value._21*value._43 + value._13*value._22*value._41 - value._11*value._22*value._43 - value._12*value._23*value._41 - value._13*value._21*value._42;
    result._44 = value._11*value._22*value._33 + value._12*value._23*value._31 + value._13*value._21*value._32 - value._11*value._23*value._32 - value._12*value._21*value._33 - value._13*value._22*value._31;

    result._11 /= det;
    result._12 /= det;
    result._13 /= det;
    result._14 /= det;

    result._21 /= det;
    result._22 /= det;
    result._23 /= det;
    result._24 /= det;

    result._31 /= det;
    result._32 /= det;
    result._33 /= det;
    result._34 /= det;

    result._41 /= det;
    result._42 /= det;
    result._43 /= det;
    result._44 /= det;
}

ASDX_INLINE
Matrix Matrix::CreateScale( const f32 scale )
{
    return Matrix(
        scale, 0.0f, 0.0f, 0.0f,
        0.0f, scale, 0.0f, 0.0f,
        0.0f, 0.0f, scale, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f );
}

ASDX_INLINE 
void Matrix::CreateScale( const f32 scale, Matrix &result )
{
    result._11 = scale;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = scale;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = scale;
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;
}

ASDX_INLINE 
Matrix Matrix::CreateScale( const f32 xScale, const f32 yScale, const f32 zScale )
{
    return Matrix(
        xScale, 0.0f, 0.0f, 0.0f,
        0.0f, yScale, 0.0f, 0.0f,
        0.0f, 0.0f, zScale, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f );
}

ASDX_INLINE
void Matrix::CreateScale( const f32 xScale, const f32 yScale, const f32 zScale, Matrix &result )
{
    result._11 = xScale;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = yScale;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = zScale;
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;
}

ASDX_INLINE
Matrix Matrix::CreateScale( const Vector3& scales )
{
    return Matrix(
        scales.x, 0.0f, 0.0f, 0.0f,
        0.0f, scales.y, 0.0f, 0.0f,
        0.0f, 0.0f, scales.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f ); 
}

ASDX_INLINE
void Matrix::CreateScale( const Vector3 &scales, Matrix &result )
{
    result._11 = scales.x;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = scales.y;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = scales.z;
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;
}

ASDX_INLINE 
Matrix Matrix::CreateTranslation( const f32 xPos, const f32 yPos, const f32 zPos )
{
    return Matrix(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        xPos, yPos, zPos, 1.0f );
}

ASDX_INLINE
void Matrix::CreateTranslation( const f32 xPos, const f32 yPos, const f32 zPos, Matrix &result )
{
    result._11 = 1.0f;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 1.0f;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = 1.0f;
    result._34 = 0.0f;

    result._41 = xPos;
    result._42 = yPos;
    result._43 = zPos;
    result._44 = 1.0f;
}

ASDX_INLINE
Matrix Matrix::CreateTranslation( const Vector3& pos )
{
    return Matrix(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        pos.x, pos.y, pos.z, 1.0f );
}

ASDX_INLINE
void Matrix::CreateTranslation( const Vector3 &pos, Matrix &result )
{
    result._11 = 1.0f;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 1.0f;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = 1.0f;
    result._34 = 0.0f;

    result._41 = pos.x;
    result._42 = pos.y;
    result._43 = pos.z;
    result._44 = 1.0f;
}

ASDX_INLINE 
Matrix Matrix::CreateRotationX( const f32 radian )
{
    register f32 cosRad = cosf(radian);
    register f32 sinRad = sinf(radian);
    return Matrix(
        1.0f,   0.0f,   0.0f,   0.0f,
        0.0f,   cosRad, sinRad, 0.0f,
        0.0f,  -sinRad, cosRad, 0.0f,
        0.0f,   0.0f,   0.0f,   1.0f );
}

ASDX_INLINE 
void Matrix::CreateRotationX( const f32 radian, Matrix &result )
{
    register f32 cosRad = cosf( radian );
    register f32 sinRad = sinf( radian );

    result._11 = 1.0f;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = cosRad;
    result._23 = sinRad;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = -sinRad;
    result._33 = cosRad;
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;
}

ASDX_INLINE 
Matrix Matrix::CreateRotationY( const f32 radian )
{
    register f32 cosRad = cosf( radian );
    register f32 sinRad = sinf( radian );

    return Matrix(
        cosRad, 0.0f,  -sinRad, 0.0f,
        0.0f,   1.0f,   0.0f,   0.0f,
        sinRad, 0.0f,   cosRad, 0.0f,
        0.0f,   0.0f,   0.0f,   1.0f );
}

ASDX_INLINE
void Matrix::CreateRotationY( const f32 radian, Matrix &result )
{
    register f32 cosRad = cosf( radian );
    register f32 sinRad = sinf( radian );

    result._11 = cosRad;
    result._12 = 0.0f;
    result._13 = -sinRad;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 1.0f;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = sinRad;
    result._32 = 0.0f;
    result._33 = cosRad;
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;
}

ASDX_INLINE
Matrix Matrix::CreateRotationZ( const f32 radian )
{
    register f32 cosRad = cosf( radian );
    register f32 sinRad = sinf( radian );

    return Matrix( 
        cosRad, sinRad, 0.0f, 0.0f,
       -sinRad, cosRad, 0.0f, 0.0f,
        0.0f,   0.0f,   1.0f, 0.0f,
        0.0f,   0.0f,   0.0f, 1.0f );
}

ASDX_INLINE
void Matrix::CreateRotationZ( const f32 radian, Matrix &result )
{
    register f32 cosRad = cosf( radian );
    register f32 sinRad = sinf( radian );

    result._11 = cosRad;
    result._12 = sinRad;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = -sinRad;
    result._22 = cosRad;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = 1.0f;
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;
}

ASDX_INLINE
Matrix Matrix::CreateFromQuaternion( const Quaternion& qua )
{
    Matrix result;
    register f32 xx = qua.x * qua.x;
    register f32 yy = qua.y * qua.y;
    register f32 zz = qua.z * qua.z;
    register f32 xy = qua.x * qua.y;
    register f32 yw = qua.y * qua.w;
    register f32 yz = qua.y * qua.z;
    register f32 xw = qua.x * qua.w;
    register f32 zx = qua.z * qua.x;
    register f32 zw = qua.z * qua.w;

    result._11 = 1.0f - (2.0f * (yy + zz));
    result._12 = 2.0f * (xy + zw);
    result._13 = 2.0f * (zx - yz);
    result._14 = 0.0f;

    result._21 = 2.0f * (xy - zw);
    result._22 = 1.0f - (2.0f * (zz + xx)); 
    result._23 = 2.0f * (yz + xw);
    result._24 = 0.0f;

    result._31 = 2.0f * (zx + yw); 
    result._32 = 2.0f * (yz - xw);
    result._33 = 1.0f - (2.0f * (yy + xx));
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;

    return result;
}

ASDX_INLINE
void Matrix::CreateFromQuaternion( const Quaternion &qua, Matrix &result )
{
    register f32 xx = qua.x * qua.x;
    register f32 yy = qua.y * qua.y;
    register f32 zz = qua.z * qua.z;
    register f32 xy = qua.x * qua.y;
    register f32 yw = qua.y * qua.w;
    register f32 yz = qua.y * qua.z;
    register f32 xw = qua.x * qua.w;
    register f32 zx = qua.z * qua.x;
    register f32 zw = qua.z * qua.w;

    result._11 = 1.0f - (2.0f * (yy + zz));
    result._12 = 2.0f * (xy + zw);
    result._13 = 2.0f * (zx - yz);
    result._14 = 0.0f;

    result._21 = 2.0f * (xy - zw);
    result._22 = 1.0f - (2.0f * (zz + xx)); 
    result._23 = 2.0f * (yz + xw);
    result._24 = 0.0f;

    result._31 = 2.0f * (zx + yw); 
    result._32 = 2.0f * (yz - xw);
    result._33 = 1.0f - (2.0f * (yy + xx));
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;
}

ASDX_INLINE
Matrix Matrix::CreateFromAxisAngle( const Vector3& axis, const f32 radian )
{
    Matrix result;
    register f32 sinRad = sinf(radian);
    register f32 cosRad = cosf(radian);
    register f32 a = 1.0f -cosRad;
    
    register f32 ab = axis.x * axis.y * a;
    register f32 bc = axis.y * axis.z * a;
    register f32 ca = axis.z * axis.x * a;
    register f32 tx = axis.x * axis.x;
    register f32 ty = axis.y * axis.y;
    register f32 tz = axis.z * axis.z;

    result._11 = tx + cosRad * (1.0f - tx);
    result._12 = ab + axis.z * sinRad;
    result._13 = ca - axis.y * sinRad;
    result._14 = 0.0f;

    result._21 = ab - axis.z * sinRad;
    result._22 = ty + cosRad * (1.0f - ty);
    result._23 = bc + axis.x * sinRad;
    result._24 = 0.0f;

    result._31 = ca + axis.y * sinRad;
    result._32 = bc - axis.x * sinRad;
    result._33 = tz + cosRad * (1.0f - tz);
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;

    return result;
}

ASDX_INLINE
void Matrix::CreateFromAxisAngle( const Vector3 &axis, const f32 radian, Matrix &result )
{
    register f32 sinRad = sinf(radian);
    register f32 cosRad = cosf(radian);
    register f32 a = 1.0f -cosRad;
    
    register f32 ab = axis.x * axis.y * a;
    register f32 bc = axis.y * axis.z * a;
    register f32 ca = axis.z * axis.x * a;
    register f32 tx = axis.x * axis.x;
    register f32 ty = axis.y * axis.y;
    register f32 tz = axis.z * axis.z;

    result._11 = tx + cosRad * (1.0f - tx);
    result._12 = ab + axis.z * sinRad;
    result._13 = ca - axis.y * sinRad;
    result._14 = 0.0f;

    result._21 = ab - axis.z * sinRad;
    result._22 = ty + cosRad * (1.0f - ty);
    result._23 = bc + axis.x * sinRad;
    result._24 = 0.0f;

    result._31 = ca + axis.y * sinRad;
    result._32 = bc - axis.x * sinRad;
    result._33 = tz + cosRad * (1.0f - tz);
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;
}

ASDX_INLINE
Matrix Matrix::CreateRotationFromYawPitchRoll( const f32 yaw, const f32 pitch, const f32 roll )
{
    Quaternion value = Quaternion::CreateFromYawPitchRoll( yaw, pitch, roll );
    return Matrix::CreateFromQuaternion( value );
}

ASDX_INLINE
void Matrix::CreateRotationFromYawPitchRoll( const f32 yaw, const f32 pitch, const f32 roll, Matrix& result )
{
    Quaternion value = Quaternion::CreateFromYawPitchRoll( yaw, pitch, roll );
    Matrix::CreateFromQuaternion( value, result );
}

ASDX_INLINE
Matrix Matrix::CreateLookAt( const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector )
{
    Matrix result;
    Vector3 zaxis = cameraPosition - cameraTarget;
    zaxis.Normalize();

    Vector3 xaxis = Vector3::Cross(cameraUpVector, zaxis);
    xaxis.Normalize();

    Vector3 yaxis = Vector3::Cross(zaxis, xaxis);
    yaxis.Normalize();

    result._11 = xaxis.x;
    result._12 = yaxis.x;
    result._13 = zaxis.x;
    result._14 = 0.0f;

    result._21 = xaxis.y;
    result._22 = yaxis.y;
    result._23 = zaxis.y;
    result._24 = 0.0f;

    result._31 = xaxis.z;
    result._32 = yaxis.z;
    result._33 = zaxis.z;
    result._34 = 0.0f;

    result._41 = -Vector3::Dot(xaxis, cameraPosition);
    result._42 = -Vector3::Dot(yaxis, cameraPosition);
    result._43 = -Vector3::Dot(zaxis, cameraPosition);
    result._44 = 1.0f;

    return result;
}

ASDX_INLINE
void Matrix::CreateLookAt( const Vector3 &cameraPosition, const Vector3 &cameraTarget, const Vector3 &cameraUpVector, Matrix &result )
{
    Vector3 zaxis = cameraPosition - cameraTarget;
    zaxis.Normalize();

    Vector3 xaxis = Vector3::Cross(cameraUpVector, zaxis);
    xaxis.Normalize();

    Vector3 yaxis = Vector3::Cross(zaxis, xaxis);
    yaxis.Normalize();

    result._11 = xaxis.x;
    result._12 = yaxis.x;
    result._13 = zaxis.x;
    result._14 = 0.0f;

    result._21 = xaxis.y;
    result._22 = yaxis.y;
    result._23 = zaxis.y;
    result._24 = 0.0f;

    result._31 = xaxis.z;
    result._32 = yaxis.z;
    result._33 = zaxis.z;
    result._34 = 0.0f;

    result._41 = -Vector3::Dot(xaxis, cameraPosition);
    result._42 = -Vector3::Dot(yaxis, cameraPosition);
    result._43 = -Vector3::Dot(zaxis, cameraPosition);
    result._44 = 1.0f;
}

ASDX_INLINE
Matrix Matrix::CreateLookTo( const Vector3& cameraPosition, const Vector3& cameraDir, const Vector3& cameraUpVector )
{
    Matrix result;
    Vector3 zaxis = -cameraDir;
    zaxis.Normalize();

    Vector3 xaxis = Vector3::Cross(cameraUpVector, zaxis);
    xaxis.Normalize();

    Vector3 yaxis = Vector3::Cross(zaxis, xaxis);
    yaxis.Normalize();

    result._11 = xaxis.x;
    result._12 = yaxis.x;
    result._13 = zaxis.x;
    result._14 = 0.0f;

    result._21 = xaxis.y;
    result._22 = yaxis.y;
    result._23 = zaxis.y;
    result._24 = 0.0f;

    result._31 = xaxis.z;
    result._32 = yaxis.z;
    result._33 = zaxis.z;
    result._34 = 0.0f;

    result._41 = -Vector3::Dot(xaxis, cameraPosition);
    result._42 = -Vector3::Dot(yaxis, cameraPosition);
    result._43 = -Vector3::Dot(zaxis, cameraPosition);
    result._44 = 1.0f;

    return result;
}

ASDX_INLINE
void Matrix::CreateLookTo( const Vector3 &cameraPosition, const Vector3 &cameraDir, const Vector3 &cameraUpVector, Matrix &result )
{
    Vector3 zaxis = -cameraDir;
    zaxis.Normalize();

    Vector3 xaxis = Vector3::Cross(cameraUpVector, zaxis);
    xaxis.Normalize();

    Vector3 yaxis = Vector3::Cross(zaxis, xaxis);
    yaxis.Normalize();

    result._11 = xaxis.x;
    result._12 = yaxis.x;
    result._13 = zaxis.x;
    result._14 = 0.0f;

    result._21 = xaxis.y;
    result._22 = yaxis.y;
    result._23 = zaxis.y;
    result._24 = 0.0f;

    result._31 = xaxis.z;
    result._32 = yaxis.z;
    result._33 = zaxis.z;
    result._34 = 0.0f;

    result._41 = -Vector3::Dot(xaxis, cameraPosition);
    result._42 = -Vector3::Dot(yaxis, cameraPosition);
    result._43 = -Vector3::Dot(zaxis, cameraPosition);
    result._44 = 1.0f;
}

ASDX_INLINE 
Matrix Matrix::CreatePerspective( const f32 width, const f32 height, const f32 nearClip, const f32 farClip )
{
    assert( width  != 0.0f );
    assert( height != 0.0f );
    register f32 diff = nearClip - farClip;
    assert( diff != 0.0f );
    Matrix result;
    result._11 = 2.0f * nearClip / width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 2.0f * nearClip / height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = farClip / diff;
    result._34 = -1.0f;
    
    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = (nearClip * farClip) / diff;
    result._44 = 0.0f;

    return result;
}

ASDX_INLINE
void Matrix::CreatePerspective( const f32 width, const f32 height, const f32 nearClip, const f32 farClip, Matrix &result )
{
    assert( width  != 0.0f );
    assert( height != 0.0f );
    register f32 diff = nearClip - farClip;
    assert( diff != 0.0f );

    result._11 = 2.0f * nearClip / width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 2.0f * nearClip / height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = farClip / diff;
    result._34 = -1.0f;
    
    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = (nearClip * farClip) / diff;
    result._44 = 0.0f;
}

ASDX_INLINE
Matrix Matrix::CreatePerspectiveFieldOfView( const f32 fieldOfView, const f32 aspectRatio, const f32 nearClip, const f32 farClip )
{
    assert( aspectRatio != 0.0f );
    register f32 diff = nearClip - farClip;
    assert( diff != 0.0f );
    Matrix result;
    register f32 yScale = 1.0f / tanf( fieldOfView / 2.0f );
    register f32 xScale = yScale / aspectRatio;
    result._11 = xScale;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = yScale;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = farClip / diff;
    result._34 = -1.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = (nearClip * farClip) / diff;
    result._44 = 0.0f;

    return result;
}

ASDX_INLINE
void Matrix::CreatePerspectiveFieldOfView( const f32 fieldOfView, const f32 aspectRatio, const f32 nearClip, const f32 farClip, Matrix &result )
{
    assert( aspectRatio != 0.0f );
    register f32 diff = nearClip - farClip;
    assert( diff != 0.0f );
    register f32 yScale = 1.0f / tanf( fieldOfView / 2.0f );
    register f32 xScale = yScale / aspectRatio;

    result._11 = xScale;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = yScale;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = farClip / diff;
    result._34 = -1.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = (nearClip * farClip) / diff;
    result._44 = 0.0f;
}

ASDX_INLINE
Matrix Matrix::CreatePerspectiveOffcenter( const f32 left, const f32 right, const f32 bottom, const f32 top, const f32 nearClip, const f32 farClip )
{
    register f32 diffRL = right - left;
    register f32 diffTB = top - bottom;
    register f32 diffNF = nearClip - farClip;
    assert( diffRL != 0.0f );
    assert( diffTB != 0.0f );
    assert( diffNF != 0.0f );

    Matrix result;
    result._11 = 2.0f * nearClip / diffRL;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 2.0f * nearClip / diffTB;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = (left + right) / diffRL;
    result._32 = (top + bottom) / diffTB;
    result._33 = farClip / diffNF;
    result._34 = -1.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = nearClip * farClip/ diffNF;
    result._44 = 0.0f;

    return result;
}

ASDX_INLINE
void Matrix::CreatePerspectiveOffcenter( const f32 left, const f32 right, const f32 bottom, const f32 top, const f32 nearClip, const f32 farClip, Matrix &result )
{
    register f32 diffRL = right - left;
    register f32 diffTB = top - bottom;
    register f32 diffNF = nearClip - farClip;
    assert( diffRL != 0.0f );
    assert( diffTB != 0.0f );
    assert( diffNF != 0.0f );

    result._11 = 2.0f * nearClip / diffRL;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 2.0f * nearClip / diffTB;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = (left + right) / diffRL;
    result._32 = (top + bottom) / diffTB;
    result._33 = farClip / diffNF;
    result._34 = -1.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = nearClip * farClip/ diffNF;
    result._44 = 0.0f;
}

ASDX_INLINE
Matrix Matrix::CreateOrthographic( const f32 width, const f32 height, const f32 nearClip, const f32 farClip )
{
    assert( width  != 0.0f );
    assert( height != 0.0f );
    register f32 diffNF = nearClip - farClip;
    assert( diffNF != 0.0f );

    Matrix result;
    result._11 = 2.0f / width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;
    
    result._21 = 0.0f;
    result._22 = 2.0f / height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = 1.0f / diffNF;
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = nearClip / diffNF;
    result._44 = 1.0f;

    return result;
}

ASDX_INLINE
void Matrix::CreateOrthographic( const f32 width, const f32 height, const f32 nearClip, const f32 farClip, Matrix &result )
{
    assert( width  != 0.0f );
    assert( height != 0.0f );
    register f32 diffNF = nearClip - farClip;
    assert( diffNF != 0.0f );

    result._11 = 2.0f / width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;
    
    result._21 = 0.0f;
    result._22 = 2.0f / height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = 1.0f / diffNF;
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = nearClip / diffNF;
    result._44 = 1.0f;
}

ASDX_INLINE
Matrix Matrix::CreateOrthographicOffcenter( const f32 left, const f32 right, const f32 bottom, const f32 top, const f32 nearClip, const f32 farClip )
{
    register f32 width  = right - left;
    register f32 height = bottom - top;
    register f32 depth  = farClip - nearClip;
    assert( width  != 0.0f );
    assert( height != 0.0f );
    assert( depth  != 0.0f );

    return Matrix(
        2.0f / width,
        0.0f,
        0.0f,
        0.0f,

        0.0f,
        2.0f / height,
        0.0f,
        0.0f,

        0.0f,
        0.0f,
        1.0f / depth,
        0.0f,

        (left + right) / width,
        (top + bottom) / height,
        nearClip / depth,
        1.0f
    );
}

ASDX_INLINE
void Matrix::CreateOrthographicOffcenter( const f32 left, const f32 right, const f32 bottom, const f32 top, const f32 nearClip, const f32 farClip, Matrix& result )
{
    register f32 width  = right - left;
    register f32 height = bottom - top;
    register f32 depth  = nearClip - farClip;
    assert( width  != 0.0f );
    assert( height != 0.0f );
    assert( depth  != 0.0f );

    result._11 = 2.0f / width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 2.0f / height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = 1.0f / depth;
    result._34 = 0.0f;

    result._41 = (left + right) / width;
    result._42 = (top + bottom) / height;
    result._43 = nearClip / depth;
    result._44 = 1.0f;
}

ASDX_INLINE
Matrix Matrix::Lerp( const Matrix& a, const Matrix& b, const f32 amount )
{
    return Matrix(
        a._11 - amount * ( a._11 - b._11 ),
        a._12 - amount * ( a._12 - b._12 ),
        a._13 - amount * ( a._13 - b._13 ),
        a._14 - amount * ( a._14 - b._14 ),

        a._21 - amount * ( a._21 - b._21 ),
        a._22 - amount * ( a._22 - b._22 ),
        a._23 - amount * ( a._23 - b._23 ),
        a._24 - amount * ( a._24 - b._24 ),

        a._31 - amount * ( a._31 - b._31 ),
        a._32 - amount * ( a._32 - b._32 ),
        a._33 - amount * ( a._33 - b._33 ),
        a._34 - amount * ( a._34 - b._34 ),

        a._41 - amount * ( a._41 - b._41 ),
        a._42 - amount * ( a._42 - b._42 ),
        a._43 - amount * ( a._43 - b._43 ),
        a._44 - amount * ( a._44 - b._44 )
    );
}

ASDX_INLINE
void Matrix::Lerp( const Matrix& a, const Matrix& b, const f32 amount, Matrix &result )
{
    result._11 = a._11 - amount * ( a._11 - b._11 );
    result._12 = a._12 - amount * ( a._12 - b._12 );
    result._13 = a._13 - amount * ( a._13 - b._13 );
    result._14 = a._14 - amount * ( a._14 - b._14 );

    result._21 = a._21 - amount * ( a._21 - b._21 );
    result._22 = a._22 - amount * ( a._22 - b._22 );
    result._23 = a._23 - amount * ( a._23 - b._23 );
    result._24 = a._24 - amount * ( a._24 - b._24 );

    result._31 = a._31 - amount * ( a._31 - b._31 );
    result._32 = a._32 - amount * ( a._32 - b._32 );
    result._33 = a._33 - amount * ( a._33 - b._33 );
    result._34 = a._34 - amount * ( a._34 - b._34 );

    result._41 = a._41 - amount * ( a._41 - b._41 );
    result._42 = a._42 - amount * ( a._42 - b._42 );
    result._43 = a._43 - amount * ( a._43 - b._43 );
    result._44 = a._44 - amount * ( a._44 - b._44 );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Quaternion
///////////////////////////////////////////////////////////////////////////////////////////////////

ASDX_INLINE
Quaternion::Quaternion()
{ /* DO_NOTHING */ }

ASDX_INLINE
Quaternion::Quaternion( const f32* pf )
{
    assert( pf != 0 );
    x = pf[ 0 ];
    y = pf[ 1 ];
    z = pf[ 2 ];
    w = pf[ 3 ];
}

ASDX_INLINE
Quaternion::Quaternion( const f32 nx, const f32 ny, const f32 nz, const f32 nw )
{ 
    x = nx;
    y = ny;
    z = nz;
    w = nw;
}

ASDX_INLINE
Quaternion::operator f32* ()
{ return (f32*)&x; }

ASDX_INLINE
Quaternion::operator const f32* () const
{ return (const f32*)&x; }

ASDX_INLINE Quaternion&
Quaternion::operator += ( const Quaternion& q )
{
    x += q.x;
    y += q.y;
    z += q.z;
    w += q.w;
    return (*this);
}

ASDX_INLINE 
Quaternion& Quaternion::operator -= ( const Quaternion& q )
{
    x -= q.x;
    y -= q.y;
    z -= q.z;
    w -= q.w;
    return (*this);
}

ASDX_INLINE
Quaternion& Quaternion::operator *= ( const Quaternion& q )
{
    register f32 f12 = ( y * q.z ) - ( z * q.y );
    register f32 f11 = ( z * q.x ) - ( x * q.z );
    register f32 f10 = ( x * q.y ) - ( y * q.x );
    register f32 f09 = ( x * q.x ) + ( y * q.y ) + ( z * q.z );

    x = ( x * q.w ) + ( q.x * w ) + f12;
    y = ( y * q.w ) + ( q.y * w ) + f11;
    z = ( z * q.w ) + ( q.z * w ) + f10;
    w = ( w * q.w ) - f09;
    return (*this);
}

ASDX_INLINE 
Quaternion& Quaternion::operator *= ( f32 f )
{
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return (*this);
}

ASDX_INLINE 
Quaternion& Quaternion::operator /= ( f32 f )
{
    assert( f != 0.0f );
    x /= f;
    y /= f;
    z /= f;
    w /= f;
    return (*this);
}

ASDX_INLINE 
Quaternion Quaternion::operator + () const
{ return (*this); }

ASDX_INLINE 
Quaternion Quaternion::operator - () const
{ return Quaternion( -x, -y, -z, -w ); }

ASDX_INLINE 
Quaternion Quaternion::operator + ( const Quaternion& q ) const
{ return Quaternion( x + q.x, y + q.y, z + q.z, w + q.z ); }

ASDX_INLINE 
Quaternion Quaternion::operator - ( const Quaternion& q ) const
{ return Quaternion( x - q.x, y - q.y, z - q.z, w - q.z ); }

ASDX_INLINE 
Quaternion Quaternion::operator * ( const Quaternion& q ) const
{ 
    register f32 f12 = ( y * q.z ) - ( z * q.y );
    register f32 f11 = ( z * q.x ) - ( x * q.z );
    register f32 f10 = ( x * q.y ) - ( y * q.x );
    register f32 f09 = ( x * q.x ) + ( y * q.y ) + ( z * q.z );

    return Quaternion(
        ( x * q.w ) + ( q.x * w ) + f12,
        ( y * q.w ) + ( q.x * w ) + f11,
        ( z * q.w ) + ( q.z * w ) + f10,
        ( w * q.w ) - f09 );
}

ASDX_INLINE 
Quaternion Quaternion::operator * ( f32 f ) const
{ return Quaternion( x * f, y * f, z * f, w *f ); }

ASDX_INLINE 
Quaternion Quaternion::operator / ( f32 f ) const
{ 
    assert( f != 0.0f );
    return Quaternion( x / f, y / f, z / f, w / f ); 
}

ASDX_INLINE 
Quaternion operator * ( f32 f, const Quaternion& q )
{ return Quaternion( f * q.x, f * q.y, f * q.z, f * q.w ); }

ASDX_INLINE 
bool Quaternion::operator == ( const Quaternion& q ) const
{ return ( (x==q.x) && (y==q.y) && (z==q.z) && (w==q.w) ); }

ASDX_INLINE
bool Quaternion::operator != ( const Quaternion& q ) const
{ return ( (x!=q.x) || (y!=q.y) || (z!=q.z) || (w!=q.w) ); }

ASDX_INLINE
f32 Quaternion::Length() const
{ return sqrtf( x * x + y * y + z * z + w * w ); }

ASDX_INLINE
f32 Quaternion::LengthSq() const
{ return ( x * x + y * y + z * z + w * w ); }

ASDX_INLINE 
Quaternion& Quaternion::Conjugate()
{ 
    x = -x;
    y = -y;
    z = -z;
    return (*this);
}

ASDX_INLINE 
Quaternion& Quaternion::Concatenate( const Quaternion& value )
{
    register f32 nx = (( value.x * w ) + ( x * value.w )) + ( value.y * z ) - ( value.z * y );
    register f32 ny = (( value.y * w ) + ( y * value.w )) + ( value.z * x ) - ( value.x * z );
    register f32 nz = (( value.z * w ) + ( z * value.w )) + ( value.x * y ) - ( value.y * x );
    register f32 nw = ( value.w * w ) - (( value.x * x ) + ( value.y * y )) + ( value.z * z );
    x = nx;
    y = ny;
    z = nz;
    w = nw;
    return (*this);
}


ASDX_INLINE 
Quaternion& Quaternion::Normalize()
{
    register f32 mag = sqrtf( x * x + y * y + z * z + w * w );
    assert( mag != 0.0f );
    x /= mag;
    y /= mag;
    z /= mag;
    w /= mag;
    return (*this);
}

ASDX_INLINE
Quaternion& Quaternion::SafeNormalize( const Quaternion& set )
{
    register f32 mag = sqrtf( x * x + y * y + z * z + w * w );
    if ( mag != 0.0f )
    {
        x /= mag;
        y /= mag;
        z /= mag;
        w /= mag;
    }
    else
    {
        x = set.x;
        y = set.y;
        z = set.z;
        w = set.w;
    }
    return (*this);
}

ASDX_INLINE
Quaternion& Quaternion::Identity()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 1.0f;
    return (*this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Quaternion Methods
///////////////////////////////////////////////////////////////////////////////////////////////////

ASDX_INLINE
void Quaternion::Identity( Quaternion& value )
{
    value.x = 0.0f;
    value.y = 0.0f;
    value.z = 0.0f;
    value.w = 1.0f;
}

ASDX_INLINE
bool Quaternion::IsIdentity( const Quaternion& value )
{
    return ( value.x == 0.0f )
        && ( value.y == 0.0f )
        && ( value.z == 0.0f )
        && ( value.w == 1.0f );
}

ASDX_INLINE
Quaternion Quaternion::Multiply( const Quaternion& a, const Quaternion& b )
{
    register f32 f12 = ( a.y * b.z ) - ( a.z * b.y );
    register f32 f11 = ( a.z * b.x ) - ( a.x * b.z );
    register f32 f10 = ( a.x * b.y ) - ( a.y * b.x );
    register f32 f09 = ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );

    return Quaternion(
        ( a.x * b.w ) + ( b.x * a.w ) + f12,
        ( a.y * b.w ) + ( b.x * a.w ) + f11,
        ( a.z * b.w ) + ( b.z * a.w ) + f10,
        ( a.w * b.w ) - f09 );
}

ASDX_INLINE
void Quaternion::Multiply( const Quaternion& a, const Quaternion& b, Quaternion& result )
{
    register f32 f12 = ( a.y * b.z ) - ( a.z * b.y );
    register f32 f11 = ( a.z * b.x ) - ( a.x * b.z );
    register f32 f10 = ( a.x * b.y ) - ( a.y * b.x );
    register f32 f09 = ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );

    result.x = ( a.x * b.w ) + ( b.x * a.w ) + f12;
    result.y = ( a.y * b.w ) + ( b.x * a.w ) + f11;
    result.z = ( a.z * b.w ) + ( b.z * a.w ) + f10;
    result.w = ( a.w * b.w ) - f09;
}

ASDX_INLINE
f32 Quaternion::Dot( const Quaternion& a, const Quaternion& b )
{
    return ( a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w );
}

ASDX_INLINE
void Quaternion::Dot( const Quaternion &a, const Quaternion &b, f32 &result )
{
    result = ( a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w );
}

ASDX_INLINE
Quaternion Quaternion::Conjugate( const Quaternion& value )
{ return Quaternion( -value.x, -value.y, -value.z, value.w ); }

ASDX_INLINE
void Quaternion::Conjugate( const Quaternion &value, Quaternion &result )
{
    result.x = -value.x;
    result.y = -value.y;
    result.z = -value.z;
    result.w = value.w;
}

ASDX_INLINE
Quaternion Quaternion::Concatenate( const Quaternion& a, const Quaternion& b )
{
    return Quaternion(
        (( b.x * a.w ) + ( a.x * b.w )) + ( b.y * a.z ) - ( b.z * a.y ),
        (( b.y * a.w ) + ( a.y * b.w )) + ( b.z * a.x ) - ( b.x * a.z ),
        (( b.z * a.w ) + ( a.z * b.w )) + ( b.x * a.y ) - ( b.y * a.x ),
        ( b.w * a.w ) - (( b.x * a.x ) + ( b.y * a.y )) + ( b.z * a.z )
   );
}

ASDX_INLINE
void Quaternion::Concatenate( const Quaternion& a, const Quaternion& b, Quaternion& result )
{
    result.x = (( b.x * a.w ) + ( a.x * b.w )) + ( b.y * a.z ) - ( b.z * a.y );
    result.y = (( b.y * a.w ) + ( a.y * b.w )) + ( b.z * a.x ) - ( b.x * a.z );
    result.z = (( b.z * a.w ) + ( a.z * b.w )) + ( b.x * a.y ) - ( b.y * a.x );
    result.w = ( b.w * a.w ) - (( b.x * a.x ) + ( b.y * a.y )) + ( b.z * a.z );
}

ASDX_INLINE
Quaternion Quaternion::Normalize( const Quaternion& value )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w );
    assert( mag != 0.0f );
    return Quaternion(
        value.x / mag,
        value.y / mag,
        value.z / mag,
        value.w / mag
    );
}

ASDX_INLINE
void Quaternion::Normalize( const Quaternion& value, Quaternion &result )
{
    register f32 mag = sqrtf( value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w );
    assert( mag != 0.0f );
    result.x /= mag;
    result.y /= mag;
    result.z /= mag;
    result.w /= mag;
}

ASDX_INLINE
Quaternion  Quaternion::SafeNormalize( const Quaternion& value, const Quaternion& set )
{
    register f32 mag = sqrtf( ( value.x * value.x )
                            + ( value.y * value.y )
                            + ( value.z * value.z )
                            + ( value.w * value.w ) );
    if ( mag != 0.0f )
    {
        return Quaternion(
            value.x / mag,
            value.y / mag,
            value.z / mag,
            value.w / mag
        );
    }
    else
    {
        return Quaternion(
            set.x,
            set.y,
            set.z,
            set.w 
        );
    }
}

ASDX_INLINE
void    Quaternion::SafeNormalize
(
    const Quaternion& value,
    const Quaternion& set,
    Quaternion& result
)
{
    register f32 mag = sqrtf( ( value.x * value.x )
                            + ( value.y * value.y )
                            + ( value.z * value.z )
                            + ( value.w * value.w ) );
    if ( mag != 0.0f )
    {
        result.x = value.x / mag;
        result.y = value.y / mag;
        result.z = value.z / mag;
        result.w = value.w / mag;
    }
    else
    {
        result.x = set.x;
        result.y = set.y;
        result.z = set.z;
        result.w = set.w;
    }
}

ASDX_INLINE
Quaternion Quaternion::Inverse( const Quaternion& value )
{
    Quaternion result( -value.x, -value.y, -value.z, value.w );
    result.Normalize();
    return result;
}

ASDX_INLINE
void Quaternion::Inverse( const Quaternion &value, Quaternion &result )
{
    result.x = -value.x;
    result.y = -value.y;
    result.z = -value.z;
    result.w = value.w;

    result.Normalize();
}

ASDX_INLINE
Quaternion Quaternion::CreateFromYawPitchRoll( const f32 yaw, const f32 pitch, const f32 roll )
{
    Quaternion result;
    f32 sr = sinf( roll  * 0.5f );
    f32 cr = cosf( roll  * 0.5f );
    f32 sp = sinf( pitch * 0.5f );
    f32 cp = cosf( pitch * 0.5f );
    f32 sy = sinf( yaw   * 0.5f );
    f32 cy = cosf( yaw   * 0.5f );

    result.x = -(sy * sp * cr) + (cy * cp * sr);
    result.y =  (cy * sp * cr) + (sy * cp * sr);
    result.z = -(cy * sp * sr) + (sy * cp * cr);
    result.w =  (cy * cp * cr) + (sy * sp * sr);
    return result;
}

ASDX_INLINE
void Quaternion::CreateFromYawPitchRoll( const f32 yaw, const f32 pitch, const f32 roll, Quaternion &result )
{
    f32 sr = sinf( roll  * 0.5f );
    f32 cr = cosf( roll  * 0.5f );
    f32 sp = sinf( pitch * 0.5f );
    f32 cp = cosf( pitch * 0.5f );
    f32 sy = sinf( yaw   * 0.5f );
    f32 cy = cosf( yaw   * 0.5f );

    result.x = -(sy * sp * cr) + (cy * cp * sr);
    result.y =  (cy * sp * cr) + (sy * cp * sr);
    result.z = -(cy * sp * sr) + (sy * cp * cr);
    result.w =  (cy * cp * cr) + (sy * sp * sr);
}

ASDX_INLINE
Quaternion  Quaternion::CreateFromAxisAngle( const Vector3& axis, const f32 radian )
{
    register f32 halfRad = radian * 0.5f;
    register f32 sinX = sinf( halfRad );
    return Quaternion(
        axis.x * sinX,
        axis.y * sinX,
        axis.z * sinX,
        cosf( halfRad )
   );
}

ASDX_INLINE
void    Quaternion::CreateFromAxisAngle
(
    const Vector3&  axis,
    const f32       radian,
    Quaternion&     result
)
{
    register f32 halfRad = radian * 0.5f;
    register f32 sinX = sinf( halfRad );
    result.x = axis.x * sinX;
    result.y = axis.y * sinX;
    result.z = axis.z * sinX;
    result.w = cosf( halfRad );
}

ASDX_INLINE
Quaternion  Quaternion::CreateFromRotationMatrix( const Matrix& value )
{
    if ( ( value._11 + value._22 + value._33 ) > 0.0f )
    {
        register f32 M1 = sqrtf( value._11 + value._22 + value._33 + 1.0f );
        register f32 W = M1 * 0.5f;
        assert( M1 != 0.0f );
        M1 = 0.5f / M1;
        return Quaternion(
            ( value._23 - value._32 ) * M1,
            ( value._31 - value._13 ) * M1,
            ( value._12 - value._21 ) * M1,
            W 
        );
    }
    if ( ( value._11 >= value._22 ) && ( value._11 >= value._33 ) )
    {
        register f32 M2 = sqrtf( 1.0f + value._11 - value._22 - value._33 );
        assert( M2 != 0.0f );
        register f32 M3 = 0.5f / M2;
        return Quaternion(
            0.5f * M2,
            ( value._12 + value._21 ) * M3,
            ( value._13 + value._31 ) * M3,
            ( value._23 - value._32 ) * M3
        );
    }
    if ( value._22 > value._33 )
    {
        register f32 M4 = sqrtf( 1.0f + value._22 - value._11 - value._33 );
        assert( M4 != 0.0f );
        register f32 M5 = 0.5f / M4;
        return Quaternion(
            ( value._21 + value._12 ) * M5,
            0.5f * M4,
            ( value._32 + value._23 ) * M5,
            ( value._31 - value._13 ) * M5
        );
    }
    register f32 M6 = sqrtf( 1.0f + value._33 - value._11 - value._22 );
    assert( M6 != 0.0f );
    register f32 M7 = 0.5f / M6;
    return Quaternion(
        ( value._31 + value._13 ) * M7,
        ( value._32 + value._23 ) * M7,
        0.5f * M6,
        ( value._12 - value._21 ) * M7
    );
}

ASDX_INLINE
void    Quaternion::CreateFromRotationMatrix( const Matrix& value, Quaternion& result )
{
    if ( ( value._11 + value._22 + value._33 ) > 0.0f )
    {
        register f32 M1 = sqrtf( value._11 + value._22 + value._33 + 1.0f );
        register f32 W = M1 * 0.5f;
        assert( M1 != 0.0f );
        M1 = 0.5f / M1;
        result.x = ( value._23 - value._32 ) * M1;
        result.y = ( value._31 - value._13 ) * M1;
        result.z = ( value._12 - value._21 ) * M1;
        result.w = W;
        return;
    }
    if ( ( value._11 >= value._22 ) && ( value._11 >= value._33 ) )
    {
        register f32 M2 = sqrtf( 1.0f + value._11 - value._22 - value._33 );
        assert( M2 != 0.0f );
        register f32 M3 = 0.5f / M2;
        result.x = 0.5f * M2;
        result.y = ( value._12 + value._21 ) * M3;
        result.z = ( value._13 + value._31 ) * M3;
        result.w = ( value._23 - value._32 ) * M3;
        return;
    }
    if ( value._22 > value._33 )
    {
        register f32 M4 = sqrtf( 1.0f + value._22 - value._11 - value._33 );
        assert( M4 != 0.0f );
        register f32 M5 = 0.5f / M4;
        result.x = ( value._21 + value._12 ) * M5;
        result.y = 0.5f * M4;
        result.z = ( value._32 + value._23 ) * M5;
        result.w = ( value._31 - value._13 ) * M5;
        return;
    }
    register f32 M6 = sqrtf( 1.0f + value._33 - value._11 - value._22 );
    assert( M6 != 0.0f );
    register f32 M7 = 0.5f / M6;
    result.x = ( value._31 + value._13 ) * M7;
    result.y = ( value._32 + value._23 ) * M7;
    result.z = 0.5f * M6;
    result.w = ( value._12 - value._21 ) * M7;
}

ASDX_INLINE
Quaternion Quaternion::Slerp( const Quaternion& a, const Quaternion& b, const f32 amount )
{
    if ( amount <= 0.0f )
    { return a; }

    if ( amount >= 1.0f ) 
    { return b; }

    register f32 cosOmega = Quaternion::Dot(a, b);
    bool flag = false;

    if ( cosOmega < 0.0f )
    {
        flag = true;
        cosOmega -= cosOmega;
    }

    f32 k1, k2;
    if ( cosOmega > 0.9999f )
    {
        k1 = 1.0f - amount;
        k2 = ( flag ) ? -amount : amount;
    }
    else
    {
        register f32 q5 = acosf( cosOmega );
        register f32 q6 = 1.0f / sinf( q5 );
        k1 = sinf( ( 1.0f - amount ) * q5 ) * q6;
        k2 = ( flag ) ? -sinf( amount * q5 ) * q6 : sinf( amount * q5 ) * q6;
    }
    return Quaternion( 
        ( k1 * a.x ) + ( k2 * a.x ),
        ( k1 * a.y ) + ( k2 * a.y ),
        ( k1 * a.z ) + ( k2 * a.z ),
        ( k1 * a.w ) + ( k2 * a.w )
    );
}

ASDX_INLINE
void Quaternion::Slerp( const Quaternion &a, const Quaternion &b, const f32 amount, Quaternion &result )
{
    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
        result.z = a.z;
        result.w = a.w;
        return;
    }

    if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
        result.z = b.z;
        result.w = b.w;
        return;
    }

    register f32 cosOmega = Quaternion::Dot(a, b);
    bool flag = false;

    if ( cosOmega < 0.0f )
    {
        flag = true;
        cosOmega -= cosOmega;
    }

    f32 k1, k2;
    if ( cosOmega > 0.9999f )
    {
        k1 = 1.0f - amount;
        k2 = ( flag ) ? -amount : amount;
    }
    else
    {
        register f32 q5 = acosf( cosOmega );
        register f32 q6 = 1.0f / sinf( q5 );
        k1 = sinf( ( 1.0f - amount ) * q5 ) * q6;
        k2 = ( flag ) ? -sinf( amount * q5 ) * q6 : sinf( amount * q5 ) * q6;
    }
  
    result.x = ( k1 * a.x ) + ( k2 * a.x );
    result.y = ( k1 * a.y ) + ( k2 * a.y );
    result.z = ( k1 * a.z ) + ( k2 * a.z );
    result.w = ( k1 * a.w ) + ( k2 * a.w );
}

ASDX_INLINE
Quaternion Quaternion::Squad( const Quaternion& q, const Quaternion& a, const Quaternion& b, const Quaternion& c, const f32 amount )
{
    register Quaternion d = Quaternion::Slerp( q, c, amount );
    register Quaternion e = Quaternion::Slerp( a, b, amount );
    return Quaternion::Slerp( d, e, 2.0f * amount * ( 1.0f - amount ) );
}

ASDX_INLINE
void Quaternion::Squad( const Quaternion &q, const Quaternion &a, const Quaternion &b, const Quaternion &c, const f32 amount, Quaternion &result )
{
    register Quaternion d = Quaternion::Slerp( q, c, amount );
    register Quaternion e = Quaternion::Slerp( a, b, amount );
    Quaternion::Slerp( d, e, 2.0f * amount * ( 1.0f - amount ), result );
}

} // namespace asdx

#endif// _ASDX_MATH_INL_
