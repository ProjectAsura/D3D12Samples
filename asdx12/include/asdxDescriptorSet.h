//-------------------------------------------------------------------------------------------------
// File : asdxDescriptorSet.h
// Desc : Descriptor Set.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <d3d12.h>
#include <vector>
#include <map>
#include <asdxRefPtr.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ShaderStage enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum ShaderStage
{
    VS,     // Vertex Shader
    PS,     // Pixel Shader
    DS,     // Domain Shader
    HS,     // Hull Shader
    GS,     // Geometry Shader
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSetLayout class
///////////////////////////////////////////////////////////////////////////////////////////////////
class DescriptorLayout
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    friend class DescriptorSet;

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    DescriptorLayout();
    ~DescriptorLayout();
    void AddCBV(ShaderStage stage, uint32_t reg);
    void AddSRV(ShaderStage stage, uint32_t reg);
    void AddUAV(ShaderStage stage, uint32_t reg);
    void AddSmp(ShaderStage stage, uint32_t reg);
    void SetFlags(D3D12_ROOT_SIGNATURE_FLAGS value);

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::vector<D3D12_DESCRIPTOR_RANGE*> m_Range;
    std::vector<D3D12_ROOT_PARAMETER>    m_Param;
    std::vector<uint32_t>                m_Hash;
    D3D12_ROOT_SIGNATURE_FLAGS           m_Flags;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSet class
///////////////////////////////////////////////////////////////////////////////////////////////////
class DescriptorSet
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    DescriptorSet();
    ~DescriptorSet();
    bool Init(ID3D12Device* pDevice, const DescriptorLayout& layout);
    void Term();

    bool SetCBV(ShaderStage stage, uint32_t reg, D3D12_GPU_DESCRIPTOR_HANDLE handle);
    bool SetSRV(ShaderStage stage, uint32_t reg, D3D12_GPU_DESCRIPTOR_HANDLE handle);
    bool SetUAV(ShaderStage stage, uint32_t reg, D3D12_GPU_DESCRIPTOR_HANDLE handle);
    bool SetSmp(ShaderStage stage, uint32_t reg, D3D12_GPU_DESCRIPTOR_HANDLE handle);
    void MakeCommand(ID3D12GraphicsCommandList* pCmdList) const;

    ID3D12RootSignature* GetRootSignature() const;

private:
    struct Table
    {
        uint32_t                    Index;
        D3D12_GPU_DESCRIPTOR_HANDLE Handle;
    };
    //=============================================================================================
    // private variables.
    //=============================================================================================
    RefPtr<ID3D12RootSignature>     m_pRootSignature;
    std::map<uint32_t, Table>       m_Tables;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace asdx
