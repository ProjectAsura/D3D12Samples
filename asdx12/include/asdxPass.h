//-------------------------------------------------------------------------------------------------
// File : asdxPass.h
// Desc : Render Pass.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdint>
#include <asdxTexture.h>
#include <map>


namespace asdx {

class PassGraph;


struct PassResource
{
    enum Kind
    {
        Color,
        Depth,
        Compute,
    };

    int Handle;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// IPass structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct IPass
{
    virtual ~IPass()
    { /* DO_NOTHING */ }

    virtual void OnInit(const PassGraph& graph, ID3D12Device* pDevice) = 0;
    virtual void OnExecute(const PassGraph& graph, ID3D12GraphicsCommandList* pCmdList) = 0;
    virtual void OnTerm() = 0;
    virtual uint32_t GetInputCount() const = 0;
    virtual uint32_t GetOutputCount() const = 0;
    virtual PassResource GetOutput(uint32_t index) const = 0;
    virtual void SetInput(uint32_t index, const PassResource& value) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PassBase class
///////////////////////////////////////////////////////////////////////////////////////////////////
template<uint32_t InputCount, uint32_t OutputCount>
class PassBase : public IPass
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
    virtual ~PassBase()
    { /* DO_NOTHING */ }

    uint32_t GetInputCount() const override
    { return InputCount; }

    uint32_t GetOutputCount() const override
    { return OutputCount; }

    PassResource GetOutput(uint32_t index) const override
    { return m_Output[index].Texture; }

    void SetInput(uint32_t index, const PassResource& value) override
    { m_Input[index] = handle; }

protected:
    //=============================================================================================
    // protected variables.
    //=============================================================================================
    PassResource    m_Input[InputCount];
    PassResource    m_Output[OutputCount];

    //=============================================================================================
    // protected methods.
    //=============================================================================================
    /* NOTHING */
};

class PassGraph
{
public:
    PassGraph();
    ~PassGraph();

    void AddPass(const std::string& tag, IPass* pass);
    bool FindPass(const std::string& tag, IPass** pass);

    void Init(ID3D12Device* pDevice, uint32_t maxThreadCount);
    void Term();
    void Execute();

private:
    std::map<std::string, IPass*>   m_Pass;
};


} // namespace asdx
