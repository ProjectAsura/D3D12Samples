//-------------------------------------------------------------------------------------------------
// File : asdxresMTN.cpp
// Desc : Project Asura Motion Format (*.mtn) Loader
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxLogger.h>
#include "asdxResMTN.h"


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//  Constant Values.
//-------------------------------------------------------------------------------------------------
static constexpr u32 MTN_VERSION = 0x000001;


///////////////////////////////////////////////////////////////////////////////////////////////////
// MTN_FILE_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MTN_FILE_HEADER
{
    u8      Magic[4];       //!< 'M', 'T', 'N', '\0'
    u32     Version;        //!< ファイルバージョン.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MTN_KEYFRAME structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MTN_KEYFRAME
{
    u32              Time;          //!< フレーム番号です.
    asdx::Vector3    Location;      //!< 位置座標です.
    asdx::Quaternion Rotation;      //!< 回転量です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MTN_KEYFRAME_SET structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MTN_KEYFRAME_SET
{
    char16  BoneName[32];       //!< ボーン番号です.
    u32     KeyFrameCount;      //!< キーフレーム数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MTN_MOTION structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MTN_MOTION
{
    u32     Duration;           //!< 継続時間です.
    u32     KeyFrameSetCount;   //!< キーフレームセット数です.
};

} // namespace /* anonymous */


namespace asdx {

//-------------------------------------------------------------------------------------------------
//      MTNファイルから読込を行います.
//-------------------------------------------------------------------------------------------------
bool LoadResMotionFromMTN( const char16* filename, ResMotion* pResult )
{
    if ( filename == nullptr || pResult == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    FILE* pFile;
    auto err = _wfopen_s( &pFile, filename, L"rb" );
    if ( err != 0 )
    {
        ELOG( "Error : File Open Failed. filename = %s", filename );
        return false;
    }

    MTN_FILE_HEADER header;
    fread( &header, sizeof(header), 1, pFile );

    if ( header.Magic[0] != 'M' ||
         header.Magic[1] != 'T' ||
         header.Magic[2] != 'N' ||
         header.Magic[3] != '\0' )
    {
        ELOG( "Error : Invalid File." );
        fclose( pFile );
        return false;
    }

    if ( header.Version != MTN_VERSION )
    {
        ELOG( "Error : Invalid File Version." );
        fclose( pFile );
        return false;
    }

    MTN_MOTION motion;
    fread( &motion, sizeof(motion), 1, pFile );

    (*pResult).Duration = motion.Duration;
    (*pResult).Bones.resize( motion.KeyFrameSetCount );

    for( u32 i=0; i<motion.KeyFrameSetCount; ++i )
    {
        MTN_KEYFRAME_SET keyFrameSet;
        fread( &keyFrameSet, sizeof(keyFrameSet), 1, pFile );

        (*pResult).Bones[i].BoneName = keyFrameSet.BoneName;
        (*pResult).Bones[i].KeyFrames.resize( keyFrameSet.KeyFrameCount );

        for( u32 j=0; j<keyFrameSet.KeyFrameCount; ++j )
        {
            MTN_KEYFRAME keyFrame;
            fread( &keyFrame, sizeof(keyFrame), 1, pFile );

            (*pResult).Bones[i].KeyFrames[j].Time      = keyFrame.Time;
            (*pResult).Bones[i].KeyFrames[j].Transform = asdx::Matrix::CreateFromQuaternion(keyFrame.Rotation);
            (*pResult).Bones[i].KeyFrames[j].Transform._41 = keyFrame.Location.x;
            (*pResult).Bones[i].KeyFrames[j].Transform._42 = keyFrame.Location.y;
            (*pResult).Bones[i].KeyFrames[j].Transform._43 = keyFrame.Location.z;
            (*pResult).Bones[i].KeyFrames[j].Transform._44 = 1.0f;
        }
    }

    fclose( pFile );
    pFile = nullptr;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      MTNファイルに保存します.
//-------------------------------------------------------------------------------------------------
bool SaveResMotionToMTN( const char16* filename, const ResMotion* pMotion )
{
    if ( filename == nullptr || pMotion == nullptr )
    {
        ELOG( "Error : Invalid Arugment." );
        return false;
    }

    FILE* pFile;
    auto err = _wfopen_s( &pFile, filename, L"wb" );
    if ( err != 0 )
    {
        ELOG( "Error : File Open Failed." );
        return false;
    }

    MTN_FILE_HEADER header;
    header.Magic[0] = 'M';
    header.Magic[1] = 'T';
    header.Magic[2] = 'N';
    header.Magic[3] = '\0';
    header.Version = MTN_VERSION;

    fwrite( &header, sizeof(header), 1, pFile );

    MTN_MOTION motion;
    motion.Duration         = pMotion->Duration;
    motion.KeyFrameSetCount = static_cast<u32>( pMotion->Bones.size() );

    fwrite( &motion, sizeof(motion), 1, pFile );

    for( u32 i=0; i<motion.KeyFrameSetCount; ++i )
    {
        MTN_KEYFRAME_SET keyFrameSet;
        wcscpy_s(keyFrameSet.BoneName, pMotion->Bones[i].BoneName.c_str());
        keyFrameSet.KeyFrameCount = static_cast<u32>( pMotion->Bones[i].KeyFrames.size() );

        fwrite( &keyFrameSet, sizeof(keyFrameSet), 1, pFile );

        for( u32 j=0; j<keyFrameSet.KeyFrameCount; ++j )
        {
            auto matrix = pMotion->Bones[i].KeyFrames[j].Transform;

            MTN_KEYFRAME key;
            key.Time     = pMotion->Bones[i].KeyFrames[j].Time;
            key.Location.x = matrix._41;
            key.Location.y = matrix._42;
            key.Location.z = matrix._43;

            matrix._41 = 0.0f;
            matrix._42 = 0.0f;
            matrix._43 = 0.0f;

            key.Rotation = asdx::Quaternion::CreateFromRotationMatrix( matrix );

            fwrite( &key, sizeof(key), 1, pFile );
        }
    }

    fclose( pFile );
    pFile = nullptr;

    return true;
}

} // namespace asdx

