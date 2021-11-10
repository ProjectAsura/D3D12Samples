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
//      �X���b�h�O���[�v�̃^�C�����O���s���X���b�hID���ă}�b�s���O���܂�.
//-----------------------------------------------------------------------------
uint2 RemapThreadId
(
    const uint2 threadGroupDim,     // [numthreads(A, B, 1)]��A��B�̒l. (8, 8, 1)��64�ŋN������̂�����.
    uint2       dispatchDim,        // ID3D12GraphicsCommandList::Dipatch(X, Y, Z)�œn����(X, Y)�̒l.
    uint2       groupId,            // �O���[�vID(SV_GroupID).
    uint2       groupThreadId       // �O���[�v�X���b�hID(SV_GroupThreadID).
)
{
    // Louis Bavoil, "Optimizing Compute Shaders for L2 Locality using Thread-Group ID Swizzling"
    // https://developer.nvidia.com/blog/optimizing-compute-shaders-for-l2-locality-using-thread-group-id-swizzling/
    // July 16, 2020.
    
    const uint N = 16; // �^�C���̉���.

    // 1�^�C�����̃X���b�h�O���[�v�̑���.
    const uint countThreadGroups = N * dispatchDim.y;

    // �l�����銮�S�ȃ^�C���̐�.
    const uint countPerfectTiles = dispatchDim.x / N;

    // ���S�ȃ^�C���ɂ�����X���b�h�O���[�v�̑���.
    const uint totalThreadGroups = countPerfectTiles * N * dispatchDim.y - 1;
 
    // 1�����ɂ���.
    const uint threadGroupIdFlattened = dispatchDim.x * groupId.y + groupId.x;

    // ���݂̃X���b�h�O���[�v����^�C��ID�ւ̃}�b�s���O.
    const uint tileId             = threadGroupIdFlattened / countThreadGroups;
    const uint localThreadGroupId = threadGroupIdFlattened % countThreadGroups;

    // �^�C���̉����ŏ��Z�Ə�]�Z���s���� X �� Y ���Z�o.
    uint localThreadGroupIdY = localThreadGroupId / N;
    uint localThreadGroupIdX = localThreadGroupId % N;
 
    if(totalThreadGroups < threadGroupIdFlattened)
    {
        // �Ō�̃^�C���ɕs���S�Ȏ���������A�Ō�̃^�C�������CTA���N�����ꂽ�ꍇ�ɂ̂ݎ��s�����p�X.
        uint lastTile = dispatchDim.x % N;
        if (lastTile > 0)
        {
            localThreadGroupIdY = localThreadGroupId / lastTile;
            localThreadGroupIdX = localThreadGroupId % lastTile;
        }
    }

    // 1�����ɂ���.
    const uint swizzledThreadGroupIdFlattened = tileId * N
      + localThreadGroupIdY * dispatchDim.x
      + localThreadGroupIdX;

    // �N�����Ŋ���C�O���[�vID�����߂�.
    uint2 swizzledThreadGroupId;
    swizzledThreadGroupId.y = swizzledThreadGroupIdFlattened / dispatchDim.x;
    swizzledThreadGroupId.x = swizzledThreadGroupIdFlattened % dispatchDim.x;

    // �X���b�h�O���[�v������N���X���b�hID�����߂�.
    uint2 swizzledThreadId;
    swizzledThreadId.x = threadGroupDim.x * swizzledThreadGroupId.x + groupThreadId.x;
    swizzledThreadId.y = threadGroupDim.y * swizzledThreadGroupId.y + groupThreadId.y;

    return swizzledThreadId;
}

//-----------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
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

    // �ł���O�̐[�x���擾.
#if ENABLE_REVERSE_Z
    float z = max(max(z0, z1), max(z2, z3));
#else
    float z = min(min(z0, z1), min(z2, z3));
#endif

    DstDepth[dispatchId] = z;
}