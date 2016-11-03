//-------------------------------------------------------------------------------------------------
// File : asdxMotionPlayer.cpp
// Desc : Motion Player Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxMotionPlayer.h>
#include <asdxResMesh.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// MotionPlayer
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
MotionPlayer::MotionPlayer()
: m_FrameTime      ( 0.0f )
, m_BoneCount      ( 0 )
, m_pBones         ( nullptr )
, m_pMotion        ( nullptr )
, m_BoneTransforms ()
, m_WorldTransforms()
, m_SkinTransforms ()
, m_IsLoop         ( false )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
MotionPlayer::~MotionPlayer()
{
    Unbind();
    m_pMotion = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      モーションを設定します.
//-------------------------------------------------------------------------------------------------
void MotionPlayer::SetMotion( const ResMotion* pMotion )
{ m_pMotion = pMotion; }

//-------------------------------------------------------------------------------------------------
//      ループ再生フラグを設定します.
//-------------------------------------------------------------------------------------------------
void MotionPlayer::SetLoop( bool isLoop )
{ m_IsLoop = isLoop; }

//-------------------------------------------------------------------------------------------------
//      ループ再生フラグを取得します.
//-------------------------------------------------------------------------------------------------
bool MotionPlayer::IsLoop() const
{ return m_IsLoop; }

//-------------------------------------------------------------------------------------------------
//      ボーンを関連付けします.
//-------------------------------------------------------------------------------------------------
void MotionPlayer::Bind( u32 boneCount, const ResBone* pBones )
{
    m_BoneCount = boneCount;
    m_pBones    = pBones;

    m_BoneTransforms .resize( boneCount );
    m_WorldTransforms.resize( boneCount );
    m_SkinTransforms .resize( boneCount );

    for( u32 i=0; i<boneCount; ++i )
    {
        m_BoneTransforms [i].Identity();
        m_WorldTransforms[i].Identity();
        m_SkinTransforms [i].Identity();
    }
}

//-------------------------------------------------------------------------------------------------
//      ボーンの関連付けを解除します.
//-------------------------------------------------------------------------------------------------
void MotionPlayer::Unbind()
{
    m_BoneTransforms .clear();
    m_WorldTransforms.clear();
    m_SkinTransforms .clear();

    m_BoneCount = 0;
    m_pBones    = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      再生時間を取得します.
//-------------------------------------------------------------------------------------------------
f32 MotionPlayer::GetFrameTime() const
{ return m_FrameTime; }

//-------------------------------------------------------------------------------------------------
//      変換行列の数を返却します.
//-------------------------------------------------------------------------------------------------
u32 MotionPlayer::GetTransformCount() const
{ return m_BoneCount; }

//-------------------------------------------------------------------------------------------------
//      ボーン行列を取得します.
//-------------------------------------------------------------------------------------------------
const Matrix* MotionPlayer::GetBoneTransforms() const
{ return ( m_BoneCount > 0 ) ? &m_BoneTransforms[0] : nullptr; }

//-------------------------------------------------------------------------------------------------
//      ワールド行列を取得します.
//-------------------------------------------------------------------------------------------------
const Matrix* MotionPlayer::GetWorldTransforms() const
{ return ( m_BoneCount > 0 ) ? &m_WorldTransforms[0] : nullptr; }

//-------------------------------------------------------------------------------------------------
//      スキニング行列を取得します.
//-------------------------------------------------------------------------------------------------
const Matrix* MotionPlayer::GetSkinTransforms() const
{ return ( m_BoneCount > 0 ) ? &m_SkinTransforms[0] : nullptr; }

//-------------------------------------------------------------------------------------------------
//      指定時間からボーン行列を計算します.
//-------------------------------------------------------------------------------------------------
Matrix MotionPlayer::CalcBoneMatrix( f32 time, const ResKeyFrameSet& bone ) const
{
    // 最後のフレームかどうかチェック.
    if ( time >= m_pMotion->Duration )
    {
        auto idx = static_cast<s32>(bone.KeyFrames.size()) - 1;
        idx = asdx::Max( idx, 0 );
        return bone.KeyFrames[idx].Transform;
    }

    u32 idx0 = 0;
    u32 idx1 = 0;

    // 指定時間に一番近いフレーム番号を求める.
    for( size_t i=0; i<bone.KeyFrames.size(); ++i )
    {
        if ( bone.KeyFrames[i].Time >= time )
        {
            idx1 = static_cast<u32>(i);
            idx0 = static_cast<u32>((i > 0) ? (i-1) : 0);
            break;
        }
    }

    // 同じであれば補間の必要はないので，フレーム番号に対応する行列をそのまま返す.
    if ( idx0 == idx1 )
    { return bone.KeyFrames[idx1].Transform; }

    // キーフレーム取得.
    const ResKeyFrame& key0 = bone.KeyFrames[idx0];
    const ResKeyFrame& key1 = bone.KeyFrames[idx1];

    // 補間係数を求める.
    auto ratio = asdx::Saturate( 
        ( time - static_cast<f32>(key0.Time) ) / 
        static_cast<f32>(key1.Time - key0.Time) );

    // ２つのフレームを補間する.
    return Matrix::Lerp( key0.Transform, key1.Transform, ratio );
}

//-------------------------------------------------------------------------------------------------
//      更新処理を行います.
//-------------------------------------------------------------------------------------------------
void MotionPlayer::Update( f32 elapsedTime )
{
    // モーションがなければ処理終了.
    if ( m_pMotion == nullptr )
    { return; }

    // 現在時間を進める.
    m_FrameTime += elapsedTime;

    if (m_FrameTime >= m_pMotion->Duration)
    {
        // ループ再生なら時間を戻す.
        if ( m_IsLoop )
        { m_FrameTime -= m_pMotion->Duration; }
        else
        { m_FrameTime = static_cast<f32>(m_pMotion->Duration); }
    }

    // 行列を更新.
    UpdateBoneTransforms ();
    UpdateWorldTransforms();
    UpdateSkinTransforms ();
}

//-------------------------------------------------------------------------------------------------
//      ボーン行列を更新します.
//-------------------------------------------------------------------------------------------------
void MotionPlayer::UpdateBoneTransforms()
{
    for( size_t i=0; i<m_pMotion->Bones.size(); ++i )
    { m_BoneTransforms[i] = CalcBoneMatrix( m_FrameTime, m_pMotion->Bones[i] ); }
}

//-------------------------------------------------------------------------------------------------
//      ワールド行列を更新します.
//-------------------------------------------------------------------------------------------------
void MotionPlayer::UpdateWorldTransforms()
{
    m_WorldTransforms[0] = m_BoneTransforms[0];

    for( u32 i=1; i<m_BoneCount; ++i )
    {
        auto parent = m_pBones[i].ParentId;

        // 親がいる場合は親のワールド行列をかける. いなければボーン行列をそのまま設定.
        if ( parent != U32_MAX )
        { m_WorldTransforms[i] = m_BoneTransforms[i] * m_WorldTransforms[parent]; }
        else
        { m_WorldTransforms[i] = m_BoneTransforms[i]; }
    }
}

//-------------------------------------------------------------------------------------------------
//      スキニング行列を更新します.
//-------------------------------------------------------------------------------------------------
void MotionPlayer::UpdateSkinTransforms()
{
    for( u32 i=0; i<m_BoneCount; ++i )
    { m_SkinTransforms[i] = m_pBones[i].InvBindPose * m_WorldTransforms[i]; }
}

} // namespace asdx

