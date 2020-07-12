//-----------------------------------------------------------------------------
// File : SimpleMeshShader.hlsl
// Desc : Simple Mesh Shader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VertexInput structure
///////////////////////////////////////////////////////////////////////////////
struct VertexInput
{
    float3 Position;    // ���_���W.
    float4 Color;       // ���_�J���[.
};

///////////////////////////////////////////////////////////////////////////////
// VertexOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VertexOutput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
};

///////////////////////////////////////////////////////////////////////////////
// PrimitiveOutput structure
///////////////////////////////////////////////////////////////////////////////
struct PrimitiveOutput
{
    uint PrimitiveId : INDEX0;
};

//-----------------------------------------------------------------------------
// Resources and Samplers.
//-----------------------------------------------------------------------------
StructuredBuffer<VertexInput>   Vertices    : register(t0);
StructuredBuffer<uint3>         Indices     : register(t1);

//-----------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
[numthreads(32, 1, 1)]
[outputtopology("triangle")]    // �V�����A�g���r���[�g "line" �܂��� "triangle" ���w��ł���.
void main
(
    uint groupIndex : SV_GroupIndex,
    out vertices VertexOutput verts[3],         // vertices�͕K�{����.
    out indices uint3 tris[1],                  // indices���K�{����.
    out primitives PrimitiveOutput prims[1]     // primitives�̓I�v�V��������.
)
{
    // �X���b�h�O���[�v�̒��_�ƃv���~�e�B�u�̐���ݒ�. ���̊֐��̌Ăяo���͕K�{.
    SetMeshOutputCounts(3, 1);

    if (groupIndex < 1)
    {
        tris[groupIndex] = Indices[groupIndex];         // ���_�C���f�b�N�X��ݒ�.
        prims[groupIndex].PrimitiveId = groupIndex;     // �v���~�e�B�u�C���f�b�N�X��ݒ�.
    }

    if (groupIndex < 3)
    {
        VertexOutput vout;
        vout.Position   = float4(Vertices[groupIndex].Position, 1.0f);
        vout.Color      = Vertices[groupIndex].Color;

        verts[groupIndex] = vout;   // ���_���o��.
    }
}
