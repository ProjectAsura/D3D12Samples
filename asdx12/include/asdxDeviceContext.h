//-------------------------------------------------------------------------------------------------
// File : asdxDeviceContext.h
// Desc : Device Context.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <d3d12.h>
#include <dxgi1_6.h>
#include <asdxRefPtr.h>
#include <asdxCommandQueue.h>
#include <asdxDescriptorHeap.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceDesc structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct DeviceContextDesc
{
    uint32_t    MaxCountRes;                //!< 最大SRV, CBV, UAV数.
    uint32_t    MaxCountSmp;                //!< 最大サンプラー数
    uint32_t    MaxCountRTV;                //!< 最大レンダーターゲットビュー数.
    uint32_t    MaxCountDSV;                //!< 最大深度ステンシルビュー数.
    uint32_t    MaxSubmitCountGraphics;     //!< グラフィックスキューの最大サブミット数.
    uint32_t    MaxSubmitCountCompute;      //!< コンピュートキューの最大サブミット数.
    uint32_t    MaxSubmitCountCopy;         //!< コピーキューの最大サブミット数.
    bool        EnableDebug;                //!< デバッグモードフラグ.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceContext class
///////////////////////////////////////////////////////////////////////////////////////////////////
class DeviceContext
{
    //=============================================================================================
    // list of friend classes and method.
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
    DeviceContext();
    ~DeviceContext();

    bool Init(const DeviceContextDesc& desc);
    void Term();

    DeviceContextDesc   GetDesc             () const;
    ID3D12Device*       GetDevice           () const;
    IDXGIFactory5*      GetDXGIFactory      () const;
    IDXGIAdapter4*      GetDXGIAdapter      () const;
    IDXGIOutput6*       GetDXGIOutput       () const;
    CommandQueue*       GetGraphicsQueue    ();
    CommandQueue*       GetComputeQueue     ();
    CommandQueue*       GetCopyQueue        ();
    DescriptorHeap*     GetDescriptorHeap   (uint32_t index);

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    DeviceContextDesc       m_Desc;                 //!< 構成設定です.
    RefPtr<ID3D12Device>    m_pDevice;              //!< デバイスです.
    RefPtr<IDXGIFactory5>   m_pFactory;             //!< DXGIファクトリです.
    RefPtr<IDXGIAdapter4>   m_pAdapter;             //!< DXGIアダプターです.
    RefPtr<IDXGIOutput6>    m_pOutput;              //!< DXGIアウトプットです.
    CommandQueue            m_Queue[3];             //!< コマンドキューです.
    DescriptorHeap          m_DescriptorHeap[4];    //!< ディスクリプタヒープです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace
