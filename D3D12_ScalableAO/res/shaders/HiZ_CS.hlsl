//-----------------------------------------------------------------------------
// File : HiZ_CS.hlsl
// Desc : Generate Hierarchical Z-Buffer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#define NUM_THREADS         (8)
#define ENABLE_REVERSE_Z    (1)

///////////////////////////////////////////////////////////////////////////////
// HiZParam structure
///////////////////////////////////////////////////////////////////////////////
struct HiZParam
{
    uint2   Size;
    float   MipLevel;
    float   Reserved;
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
ConstantBuffer<HiZParam>    Param           : register(b0);
Texture2D<float>            SrcDepth        : register(t0);
RWTexture2D<float>          DstDepth        : register(u0);
SamplerState                PointSampler    : register(s0);

//-----------------------------------------------------------------------------
//      スレッドグループのタイリングを行いスレッドIDを再マッピングします.
//-----------------------------------------------------------------------------
uint2 RemapThreadId
(
    const uint2 threadGroupDim,     // [numthreads(A, B, 1)]のAとBの値. (8, 8, 1)の64で起動するのが推奨.
    uint2       dispatchDim,        // ID3D12GraphicsCommandList::Dipatch(X, Y, Z)で渡した(X, Y)の値.
    uint2       groupId,            // グループID(SV_GroupID).
    uint2       groupThreadId       // グループスレッドID(SV_GroupThreadID).
)
{
    // Louis Bavoil, "Optimizing Compute Shaders for L2 Locality using Thread-Group ID Swizzling"
    // https://developer.nvidia.com/blog/optimizing-compute-shaders-for-l2-locality-using-thread-group-id-swizzling/
    // July 16, 2020.
    
    const uint N = 16; // タイルの横幅.

    // 1タイル内のスレッドグループの総数.
    const uint countThreadGroups = N * dispatchDim.y;

    // 考えうる完全なタイルの数.
    const uint countPerfectTiles = dispatchDim.x / N;

    // 完全なタイルにおけるスレッドグループの総数.
    const uint totalThreadGroups = countPerfectTiles * N * dispatchDim.y - 1;
 
    // 1次元にする.
    const uint threadGroupIdFlattened = dispatchDim.x * groupId.y + groupId.x;

    // 現在のスレッドグループからタイルIDへのマッピング.
    const uint tileId             = threadGroupIdFlattened / countThreadGroups;
    const uint localThreadGroupId = threadGroupIdFlattened % countThreadGroups;

    // タイルの横幅で除算と剰余算を行って X と Y を算出.
    uint localThreadGroupIdY = localThreadGroupId / N;
    uint localThreadGroupIdX = localThreadGroupId % N;
 
    if(totalThreadGroups < threadGroupIdFlattened)
    {
        // 最後のタイルに不完全な次元があり、最後のタイルからのCTAが起動された場合にのみ実行されるパス.
        uint lastTile = dispatchDim.x % N;
        if (lastTile > 0)
        {
            localThreadGroupIdY = localThreadGroupId / lastTile;
            localThreadGroupIdX = localThreadGroupId % lastTile;
        }
    }

    // 1次元にする.
    const uint swizzledThreadGroupIdFlattened = tileId * N
      + localThreadGroupIdY * dispatchDim.x
      + localThreadGroupIdX;

    // 起動数で割り，グループIDを求める.
    uint2 swizzledThreadGroupId;
    swizzledThreadGroupId.y = swizzledThreadGroupIdFlattened / dispatchDim.x;
    swizzledThreadGroupId.x = swizzledThreadGroupIdFlattened % dispatchDim.x;

    // スレッドグループ数から起動スレッドIDを求める.
    uint2 swizzledThreadId;
    swizzledThreadId.x = threadGroupDim.x * swizzledThreadGroupId.x + groupThreadId.x;
    swizzledThreadId.y = threadGroupDim.y * swizzledThreadGroupId.y + groupThreadId.y;

    return swizzledThreadId;
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void main(uint2 groupId : SV_GroupID, uint2 groupThreadId : SV_GroupThreadID)
{
    uint2 dispatchArgs = uint2(
        (Param.Size.x + (NUM_THREADS - 1)) / NUM_THREADS,
        (Param.Size.y + (NUM_THREADS - 1)) / NUM_THREADS);

    uint2 dispatchId = RemapThreadId(
        uint2(NUM_THREADS, NUM_THREADS), dispatchArgs, groupId, groupThreadId);

    float2 uv = (float2(dispatchId) + 0.5f.xx) / float2(Param.Size);

    float z0 = SrcDepth.SampleLevel(PointSampler, uv, Param.MipLevel, int2( 0,  0));
    float z1 = SrcDepth.SampleLevel(PointSampler, uv, Param.MipLevel, int2( 0, -1));
    float z2 = SrcDepth.SampleLevel(PointSampler, uv, Param.MipLevel, int2(-1,  0));
    float z3 = SrcDepth.SampleLevel(PointSampler, uv, Param.MipLevel, int2(-1, -1));

    // 最も手前の深度を取得.
#if ENABLE_REVERSE_Z
    float z = max(max(z0, z1), max(z2, z3));
#else
    float z = min(min(z0, z1), min(z2, z3));
#endif

    DstDepth[dispatchId] = z;
}