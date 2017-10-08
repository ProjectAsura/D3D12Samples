//-------------------------------------------------------------------------------------------------
// File : asdxDescriptorSet.cpp
// Desc : Descriptor Set.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxDescriptorSet.h>


namespace {

//------------------------------------------------------------------------------------------------
// Constant Values.
//------------------------------------------------------------------------------------------------
constexpr uint8_t kTypeCBV = 0;
constexpr uint8_t kTypeSRV = 1;
constexpr uint8_t kTypeUAV = 2;
constexpr uint8_t kTypeSmp = 3;

//-------------------------------------------------------------------------------------------------
//      ハッシュ値を計算します.
//-------------------------------------------------------------------------------------------------
uint32_t CalcHash(asdx::ShaderStage stage, uint8_t type, uint32_t reg)
{ return (uint8_t(type) << 24) | (uint8_t(stage) << 16) | reg; }

//-------------------------------------------------------------------------------------------------
//      シェーダビジビリティを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_SHADER_VISIBILITY GetShaderVisiblity(asdx::ShaderStage stage)
{
    switch(stage)
    {
    case asdx::ShaderStage::VS: return D3D12_SHADER_VISIBILITY_VERTEX;
    case asdx::ShaderStage::PS: return D3D12_SHADER_VISIBILITY_PIXEL;
    case asdx::ShaderStage::DS: return D3D12_SHADER_VISIBILITY_DOMAIN;
    case asdx::ShaderStage::HS: return D3D12_SHADER_VISIBILITY_HULL;
    case asdx::ShaderStage::GS: return D3D12_SHADER_VISIBILITY_GEOMETRY;
    }

    return D3D12_SHADER_VISIBILITY_ALL;
}

} // namespace


namespace asdx{

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorLayout class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorLayout::DescriptorLayout()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorLayout::~DescriptorLayout()
{
    for(size_t i=0; i<m_Range.size(); ++i)
    {
        if (m_Range[i] != nullptr)
        {
            delete m_Range[i];
            m_Range[i] = nullptr;
        }
    }
    m_Range.clear();
    m_Param.clear();
}

//-------------------------------------------------------------------------------------------------
//      定数バッファを追加します.
//-------------------------------------------------------------------------------------------------
void DescriptorLayout::AddCBV(ShaderStage stage, uint32_t reg)
{
    auto range = new D3D12_DESCRIPTOR_RANGE();
    range->RangeType            = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    range->NumDescriptors       = 1;
    range->BaseShaderRegister   = reg;
    range->RegisterSpace        = 0;
    range->OffsetInDescriptorsFromTableStart = 0;

    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges   = 1;
    param.DescriptorTable.pDescriptorRanges     = range;
    param.ShaderVisibility                      = GetShaderVisiblity(stage);

    auto hash = CalcHash(stage, kTypeCBV, reg);

    m_Range.push_back(range);
    m_Param.push_back(param);
    m_Hash.push_back(hash);
}

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを追加します.
//-------------------------------------------------------------------------------------------------
void DescriptorLayout::AddSRV(ShaderStage stage, uint32_t reg)
{
    auto range = new D3D12_DESCRIPTOR_RANGE();
    range->RangeType            = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range->NumDescriptors       = 1;
    range->BaseShaderRegister   = reg;
    range->RegisterSpace        = 0;
    range->OffsetInDescriptorsFromTableStart = 0;

    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges   = 1;
    param.DescriptorTable.pDescriptorRanges     = range;
    param.ShaderVisibility                      = GetShaderVisiblity(stage);

    auto hash = CalcHash(stage, kTypeSRV, reg);

    m_Range.push_back(range);
    m_Param.push_back(param);
    m_Hash.push_back(hash);
}

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスビューを追加します.
//-------------------------------------------------------------------------------------------------
void DescriptorLayout::AddUAV(ShaderStage stage, uint32_t reg)
{
    auto range = new D3D12_DESCRIPTOR_RANGE();
    range->RangeType            = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    range->NumDescriptors       = 1;
    range->BaseShaderRegister   = reg;
    range->RegisterSpace        = 0;
    range->OffsetInDescriptorsFromTableStart = 0;

    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges   = 1;
    param.DescriptorTable.pDescriptorRanges     = range;
    param.ShaderVisibility                      = GetShaderVisiblity(stage);

    auto hash = CalcHash(stage, kTypeUAV, reg);

    m_Range.push_back(range);
    m_Param.push_back(param);
    m_Hash.push_back(hash);
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを追加します.
//-------------------------------------------------------------------------------------------------
void DescriptorLayout::AddSmp(ShaderStage stage, uint32_t reg)
{
    auto range = new D3D12_DESCRIPTOR_RANGE();
    range->RangeType            = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    range->NumDescriptors       = 1;
    range->BaseShaderRegister   = reg;
    range->RegisterSpace        = 0;
    range->OffsetInDescriptorsFromTableStart = 0;

    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges   = 1;
    param.DescriptorTable.pDescriptorRanges     = range;
    param.ShaderVisibility                      = GetShaderVisiblity(stage);

    auto hash = CalcHash(stage, kTypeSmp, reg);

    m_Range.push_back(range);
    m_Param.push_back(param);
    m_Hash.push_back(hash);
}

//-------------------------------------------------------------------------------------------------
//      フラグを設定します.
//------------- ------------------------------------------------------------------------------------
void DescriptorLayout::SetFlags(D3D12_ROOT_SIGNATURE_FLAGS value)
{ m_Flags = value; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSet class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSet::DescriptorSet()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DescriptorSet::~DescriptorSet()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::Init(ID3D12Device* pDevice, const DescriptorLayout& layout)
{
    if (pDevice == nullptr)
    { return false; }

    for(size_t i=0; i<layout.m_Param.size(); ++i)
    { m_Tables[layout.m_Hash[i]].Index = uint32_t(i); }

    // ルートシグニチャの設定.
    D3D12_ROOT_SIGNATURE_DESC desc;
    desc.NumParameters     = uint32_t(layout.m_Param.size());
    desc.pParameters       = layout.m_Param.data();
    desc.NumStaticSamplers = 0;
    desc.pStaticSamplers   = nullptr;
    desc.Flags             = layout.m_Flags;

    RefPtr<ID3DBlob> pSignature;
    RefPtr<ID3DBlob> pError;

    // シリアライズする.
    auto hr = D3D12SerializeRootSignature(
        &desc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        pSignature.GetAddress(),
        pError.GetAddress() );
    if ( FAILED( hr ) )
    { return false; }

    // ルートシグニチャを生成.
    hr = pDevice->CreateRootSignature(
        0,
        pSignature->GetBufferPointer(),
        pSignature->GetBufferSize(),
        IID_PPV_ARGS(m_pRootSignature.GetAddress()) );
    if ( FAILED( hr ) )
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::Term()
{
    m_Tables.clear();
    m_pRootSignature.Reset();
}

//-------------------------------------------------------------------------------------------------
//      定数バッファを設定します.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::SetCBV(ShaderStage stage, uint32_t reg, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
    auto hash = CalcHash(stage, kTypeCBV, reg);
    if (m_Tables.find(hash) == m_Tables.end())
    { return false; }

    m_Tables[hash].Handle = handle;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::SetSRV(ShaderStage stage, uint32_t reg, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
    auto hash = CalcHash(stage, kTypeSRV, reg);
    if (m_Tables.find(hash) == m_Tables.end())
    { return false; }

    m_Tables[hash].Handle = handle;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスビューを設定します.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::SetUAV(ShaderStage stage, uint32_t reg, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
    auto hash = CalcHash(stage, kTypeUAV, reg);
    if (m_Tables.find(hash) == m_Tables.end())
    { return false; }

    m_Tables[hash].Handle = handle;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを設定します.
//-------------------------------------------------------------------------------------------------
bool DescriptorSet::SetSmp(ShaderStage stage, uint32_t reg, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
    auto hash = CalcHash(stage, kTypeSmp, reg);
    if (m_Tables.find(hash) == m_Tables.end())
    { return false; }

    m_Tables[hash].Handle = handle;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      コマンドを生成します.
//-------------------------------------------------------------------------------------------------
void DescriptorSet::MakeCommand(ID3D12GraphicsCommandList* pCmdList) const
{
    for(auto& item : m_Tables)
    { pCmdList->SetGraphicsRootDescriptorTable(item.second.Index, item.second.Handle); }

    pCmdList->SetGraphicsRootSignature(m_pRootSignature.GetPtr());
}

//-------------------------------------------------------------------------------------------------
//      ルートシグニチャを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12RootSignature* DescriptorSet::GetRootSignature() const
{ return m_pRootSignature.GetPtr(); }


} // namespace asdx
