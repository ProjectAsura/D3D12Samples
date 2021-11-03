//-----------------------------------------------------------------------------
// File : DescriptorHeap.h
// Desc : Descriptor Heap Warpper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d12.h>


struct IDescriptor
{
    virtual void Release() = 0;
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU() const = 0;
    virtual D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const = 0;
};

struct IDescriptorHeap
{
    virtual void Release() = 0;
    virtual IDescriptor* Alloc() = 0;
};

bool CreateDescriptorHeap(
    ID3D12Device*               device,
    D3D12_DESCRIPTOR_HEAP_TYPE  type,
    UINT                        count,
    IDescriptorHeap**           heap);

