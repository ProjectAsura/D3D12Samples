//-----------------------------------------------------------------------------
// File : DredReport.cpp
// Desc : Device Removed Extented Data Reporter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <cstdarg>
#include <system_error>
#include <wrl/client.h>
#include <DredReport.h>
#include <d3d12video.h>

template<typename T>
using RefPtr = Microsoft::WRL::ComPtr<T>;

namespace {

//! パンくずタグ.
static const char* g_BreadcrumbTags[] = {
    "SETMARKER",
    "BEGINEVENT",
    "ENDEVENT",
    "DRAWINSTANCED",
    "DRAWINDEXEDINSTANCED",
    "EXECUTEINDIRECT",
    "DISPATCH",
    "COPYBUFFERREGION",
    "COPYTEXTUREREGION",
    "COPYRESOURCE",
    "COPYTILES",
    "RESOLVESUBRESOURCE",
    "CLEARRENDERTARGETVIEW",
    "CLEARUNORDEREDACCESSVIEW",
    "CLEARDEPTHSTENCILVIEW",
    "RESOURCEBARRIER",
    "EXECUTEBUNDLE",
    "PRESENT",
    "RESOLVEQUERYDATA",
    "BEGINSUBMISSION",
    "ENDSUBMISSION",
    "DECODEFRAME",
    "PROCESSFRAMES",
    "ATOMICCOPYBUFFERUINT",
    "ATOMICCOPYBUFFERUINT64",
    "RESOLVESUBRESOURCEREGION",
    "WRITEBUFFERIMMEDIATE",
    "DECODEFRAME1",
    "SETPROTECTEDRESOURCESESSION",
    "DECODEFRAME2",
    "PROCESSFRAMES1",
    "BUILDRAYTRACINGACCELERATIONSTRUCTURE",
    "EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO",
    "COPYRAYTRACINGACCELERATIONSTRUCTURE",
    "DISPATCHRAYS",
    "INITIALIZEMETACOMMAND",
    "EXECUTEMETACOMMAND",
    "ESTIMATEMOTION",
    "RESOLVEMOTIONVECTORHEAP",
    "SETPIPELINESTATE1",
    "INITIALIZEEXTENSIONCOMMAND",
    "EXECUTEEXTENSIONCOMMAND",
    "DISPATCHMESH",
    "ENCODEFRAME2",
    "RESOLVEENCODEROUTPUTMETADATA",
};

struct AllocPair
{
    D3D12_DRED_ALLOCATION_TYPE  Type;
    const char*                 Tag;
};

//! アロケーションタイプテーブル.
static const AllocPair g_AllocTypeTables[] = {
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_QUEUE              , "COMMAND_QUEUE"               },
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_ALLOCATOR          , "COMMAND_ALLOCATOR"           },
    { D3D12_DRED_ALLOCATION_TYPE_PIPELINE_STATE             , "PIPELINE_STATE"              },
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_LIST               , "COMMAND_LIST"                },
    { D3D12_DRED_ALLOCATION_TYPE_FENCE                      , "FENCE"                       },
    { D3D12_DRED_ALLOCATION_TYPE_DESCRIPTOR_HEAP            , "DESCRIPTOR_HEAP"             },
    { D3D12_DRED_ALLOCATION_TYPE_HEAP                       , "HEAP"                        },
    { D3D12_DRED_ALLOCATION_TYPE_QUERY_HEAP                 , "QUERY_HEAP"                  },
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_SIGNATURE          , "COMMAND_SIGNATURE"           },
    { D3D12_DRED_ALLOCATION_TYPE_PIPELINE_LIBRARY           , "PIPELINE_LIBRARAY"           },
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER              , "VIDEO_DECORDER"              },
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_PROCESSOR            , "VIDEO_PROCESSOR"             },
    { D3D12_DRED_ALLOCATION_TYPE_RESOURCE                   , "RESOURCE"                    },
    { D3D12_DRED_ALLOCATION_TYPE_PASS                       , "PASS"                        },
    { D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSION              , "CRYPTOSESSION"               },
    { D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSIONPOLICY        , "CRYPTOSESSIONPOLICY"         },
    { D3D12_DRED_ALLOCATION_TYPE_PROTECTEDRESOURCESESSION   , "PROTECTEDRESOURCESESSION"    },
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER_HEAP         , "VIDEO_DECODER_HEAP"          },
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_POOL               , "COMMAND_POOL"                },
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_RECORDER           , "COMMAND_RECORDER"            },
    { D3D12_DRED_ALLOCATION_TYPE_STATE_OBJECT               , "STATE_OBJECT"                },
    { D3D12_DRED_ALLOCATION_TYPE_METACOMMAND                , "METACOMMAND"                 },
    { D3D12_DRED_ALLOCATION_TYPE_SCHEDULINGGROUP            , "SCHEDULINGGROUP"             },
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_ESTIMATOR     , "VIDEO_MOTION_ESTIMATOR"      },
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_VECTOR_HEAP   , "VIDEO_MOTION_VECTOR_HEAP"    },
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_EXTENSION_COMMAND    , "VIDEO_EXTENSION_COMMAND"     },
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER              , "VIDEO_ENCODER"               },
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER_HEAP         , "VIDEO_ENCODER_HEAP"          },
    { D3D12_DRED_ALLOCATION_TYPE_INVALID                    , "INVALID"                     },
};

//-----------------------------------------------------------------------------
//! @brief      アロケーションタイプを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_DRED_ALLOCATION_TYPE value)
{
    auto count = sizeof(g_AllocTypeTables) / sizeof(g_AllocTypeTables[0]);
    for(auto i=0u; i<count; ++i)
    {
        if (value == g_AllocTypeTables[i].Type)
        { return g_AllocTypeTables[i].Tag; }
    }

    // バージョンアップとかで列挙体が増えた場合にここに来る可能性がある.
    return "UNKNOWN";
}

//-----------------------------------------------------------------------------
//! @brief      コマンドリストタイプを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_COMMAND_LIST_TYPE type)
{
    switch(type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        return "DIRECT";

    case D3D12_COMMAND_LIST_TYPE_BUNDLE:
        return "BUNDLE";

    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return "COMPUTE";

    case D3D12_COMMAND_LIST_TYPE_COPY:
        return "COPY";

    case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
        return "VIDEO_DECODE";

    case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
        return "VIDEO_PROCESS";

    case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
        return "VIDEO_ENCODE";

    case D3D12_COMMAND_LIST_TYPE_NONE:
        return "NONE";

    default:
        return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------
//! @brief      デバイス状態を文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_DRED_DEVICE_STATE state)
{
    switch(state)
    {
    case D3D12_DRED_DEVICE_STATE_UNKNOWN:
        return "UNKNOWN";

    case D3D12_DRED_DEVICE_STATE_HUNG:
        return "HUNG";

    case D3D12_DRED_DEVICE_STATE_FAULT:
        return "FAULT";

    case D3D12_DRED_DEVICE_STATE_PAGEFAULT:
        return "PAGEFAULT";

    default:
        return "UKNNOWN";
    }
}

//-----------------------------------------------------------------------------
//! @brief      ログを出力します.
//-----------------------------------------------------------------------------
void OutputLog(const char* format, ...)
{
    char msg[2048]= {};
    va_list arg = {};

    va_start( arg, format );
    vsprintf_s( msg, format, arg );
    va_end( arg );

    fprintf(stderr, "%s\n", msg );

    OutputDebugStringA( msg );
    OutputDebugStringA("\n");
}

//-----------------------------------------------------------------------------
//! @brief      自動パンくずリストをログ出力します.
//-----------------------------------------------------------------------------
void ReportBreadcrumbNode(const D3D12_AUTO_BREADCRUMB_NODE* pNode)
{
    if (pNode == nullptr)
        return;

    auto count     = pNode->BreadcrumbCount;        // パンくずの数.
    auto lastIndex = *pNode->pLastBreadcrumbValue;  // 最後に実行された番号.

    OutputLog("Breadcrumb Node (0x%p) : ", pNode);
    if (count == lastIndex && count > 0)
    { OutputLog("    State              : Completed."); }
    else if (lastIndex == 0)
    { OutputLog("    State              : Not Started"); }
    else 
    { OutputLog("    State              : Not Completed."); }
    OutputLog("    BreadcrumbCount    : %u", pNode->BreadcrumbCount);
    OutputLog("    LastBreadcrumValue : %u", *pNode->pLastBreadcrumbValue);
    OutputLog("    Has Next           : %s", (pNode->pNext == nullptr) ? "No" : "Yes");

    if (pNode->pCommandList != nullptr)
    {
        auto type = pNode->pCommandList->GetType();
        OutputLog("    CommandList        :");
        OutputLog("        Pointer    = 0x%p", pNode->pCommandList);
        OutputLog("        Type       = %s", ToString(type));
        OutputLog("        DebugNameA = %s", pNode->pCommandListDebugNameA);
        OutputLog("        DebugNameW = %ls", pNode->pCommandListDebugNameW);
    }

    if (pNode->pCommandQueue != nullptr)
    {
        auto desc = pNode->pCommandQueue->GetDesc();
        OutputLog("    CommandQueue       :");
        OutputLog("        Pointer    = 0x%p", pNode->pCommandQueue);
        OutputLog("        Type       = %s", ToString(desc.Type));
        OutputLog("        Priority   = %d", desc.Priority);
        OutputLog("        Flags      = 0x%x", desc.Flags);
        OutputLog("        NodeMask   = %u", desc.NodeMask);
        OutputLog("        DebugNameA = %s", pNode->pCommandQueueDebugNameA);
        OutputLog("        DebugNameW = %ls", pNode->pCommandQueueDebugNameW);
    }

    OutputLog("    CommandHistory     : %s", (count == 0) ? "None" : "");
    for(auto i=0u; i<count; ++i)
    {
        const char* mark = (i < lastIndex) ? "OK" : (i == lastIndex) ? "NG" : "  ";
        auto history = pNode->pCommandHistory[i];
        OutputLog("        Index:%04u [%s] Op = %s", i, mark, g_BreadcrumbTags[history]);
    }
}

//-----------------------------------------------------------------------------
//! @brief      自動パンくずリストをログ出力します.
//-----------------------------------------------------------------------------
void ReportBreadcrumbNode1(const D3D12_AUTO_BREADCRUMB_NODE1* pNode)
{
    if (pNode == nullptr)
        return;

    auto count     = pNode->BreadcrumbCount;        //!< パンくずの数.
    auto lastIndex = *pNode->pLastBreadcrumbValue;  //!< 最後に実行された番号.

    OutputLog("Breadcrumb Node (0x%p) : ", pNode);
    if (count == lastIndex && count > 0)
    { OutputLog("    State              : Completed."); }
    else if (lastIndex == 0)
    { OutputLog("    State              : Not Started"); }
    else 
    { OutputLog("    State              : Not Completed."); }
    OutputLog("    BreadcrumbCount    : %u", pNode->BreadcrumbCount);
    OutputLog("    LastBreadcrumValue : %u", *pNode->pLastBreadcrumbValue);
    OutputLog("    Has Next           : %s", (pNode->pNext == nullptr) ? "No" : "Yes");
    OutputLog("    ContextCount       : %u", pNode->BreadcrumbContextsCount);

    if (pNode->pCommandList != nullptr)
    {
        auto type = pNode->pCommandList->GetType();
        OutputLog("    CommandList        :");
        OutputLog("        Pointer    = 0x%p", pNode->pCommandList);
        OutputLog("        Type       = %s", ToString(type));
        OutputLog("        DebugNameA = %s", pNode->pCommandListDebugNameA);
        OutputLog("        DebugNameW = %ls", pNode->pCommandListDebugNameW);
    }

    if (pNode->pCommandQueue != nullptr)
    {
        auto desc = pNode->pCommandQueue->GetDesc();
        OutputLog("    CommandQueue       :");
        OutputLog("        Pointer    = 0x%p", pNode->pCommandQueue);
        OutputLog("        Type       = %s", ToString(desc.Type));
        OutputLog("        Priority   = %d", desc.Priority);
        OutputLog("        Flags      = 0x%x", desc.Flags);
        OutputLog("        NodeMask   = %u", desc.NodeMask);
        OutputLog("        DebugNameA = %s", pNode->pCommandQueueDebugNameA);
        OutputLog("        DebugNameW = %ls", pNode->pCommandQueueDebugNameW);
    }

    OutputLog("    CommandHistory     : %s", (count == 0) ? "None" : "");
    for(auto i=0u; i<count; ++i)
    {
        const char* mark = (i < lastIndex) ? "OK" : (i == lastIndex) ? "NG" : "  ";
        auto history = pNode->pCommandHistory[i];
        OutputLog("        %05u: [%s] Op = %s", i, mark, g_BreadcrumbTags[history]);
    }

    OutputLog("    BreadcrumbContext  : %s", (pNode->BreadcrumbContextsCount == 0) ? "None" : "");
    for(auto i=0u; i<pNode->BreadcrumbContextsCount; ++i)
    {
        OutputLog("        %05u: String=%ls", pNode->pBreadcrumbContexts[i].BreadcrumbIndex, pNode->pBreadcrumbContexts[i].pContextString);
    }
}

//-----------------------------------------------------------------------------
//! @brief      アロケーションノードをログ出力します.
//-----------------------------------------------------------------------------
void ReportAllocationNode(const D3D12_DRED_ALLOCATION_NODE* pNode)
{
    if (pNode == nullptr)
        return;

    OutputLog("AllocationNode 0x%x :", pNode);
    OutputLog("    ObjectName     : A(%s), W(%ls)", pNode->ObjectNameA, pNode->ObjectNameW);
    OutputLog("    AllocationType : %s", ToString(pNode->AllocationType));
    OutputLog("    Has Next       : %s", (pNode->pNext == nullptr) ? "No" : "Yes");
}

//-----------------------------------------------------------------------------
//! @brief      アロケーションノードをログ出力します.
//-----------------------------------------------------------------------------
void ReportAllocationNode1(const D3D12_DRED_ALLOCATION_NODE1* pNode)
{
    if (pNode == nullptr)
        return;

    OutputLog("AllocatioNode 0x%x : ", pNode);
    OutputLog("    ObjectName     : A(%s), W(%ls)", pNode->ObjectNameA, pNode->ObjectNameW);
    OutputLog("    AllocationType : %s", ToString(pNode->AllocationType));
    OutputLog("    Has Next       : %s", (pNode->pNext == nullptr) ? "No" : "Yes");
    OutputLog("    pObject        : 0x%p", pNode->pObject);
}

//-----------------------------------------------------------------------------
//! @brief      ページフォルトをログ出力します.
//-----------------------------------------------------------------------------
void ReportPageFaultOutput(const D3D12_DRED_PAGE_FAULT_OUTPUT& pageFaultOutput)
{
    OutputLog("PageFaultOutput : ");
    OutputLog("    PageFaultVA : 0x%p", pageFaultOutput.PageFaultVA);

    {
        OutputLog("--- Existing Allocation Node  ---");
        auto pNode = pageFaultOutput.pHeadExistingAllocationNode;
        if (pNode == nullptr)
        {
            OutputLog("None.");
        }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode(pNode);
                pNode = pNode->pNext;
            }
        }
        OutputLog("---------------------------------");
    }

    {
        OutputLog("--- Recent Freed Allocation Node ---");
        auto pNode = pageFaultOutput.pHeadRecentFreedAllocationNode;
        if (pNode == nullptr)
        {
            OutputLog("None.");
        }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode(pNode);
                pNode = pNode->pNext;
            }
        }
        OutputLog("---------------------------------");
    }
}

//-----------------------------------------------------------------------------
//! @brief      ページフォルト情報をログ出力します.
//-----------------------------------------------------------------------------
void ReportPageFaultOutput1(const D3D12_DRED_PAGE_FAULT_OUTPUT1& pageFaultOutput)
{
    OutputLog("PageFaultOutput : ");
    OutputLog("    PageFaultVA : 0x%x", pageFaultOutput.PageFaultVA);

    {
        OutputLog("--- Existing Allocation Node  ---");
        auto pNode = pageFaultOutput.pHeadExistingAllocationNode;
        if (pNode == nullptr)
        {
            OutputLog("None");
        }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode1(pNode);
                pNode = pNode->pNext;
            }
        }
        OutputLog("---------------------------------");
    }

    {
        OutputLog("--- Recent Freed Allocation Node ---");
        auto pNode = pageFaultOutput.pHeadRecentFreedAllocationNode;
        if (pNode == nullptr)
        {
            OutputLog("None");
        }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode1(pNode);
                pNode = pNode->pNext;
            }
        }
        OutputLog("---------------------------------");
    }
}

//-----------------------------------------------------------------------------
//! @brief      ページフォルト情報をログ出力します.
//-----------------------------------------------------------------------------
void ReportPageFaultOutput2(const D3D12_DRED_PAGE_FAULT_OUTPUT2& pageFaultOutput)
{
    OutputLog("PageFaultOutput : ");
    OutputLog("    PageFaultVA    : 0x%p", pageFaultOutput.PageFaultVA);
    OutputLog("    PgaeFaultFlags : 0x%x", pageFaultOutput.PageFaultFlags);

    {
        OutputLog("--- Existing Allocation Node  ---");
        auto pNode = pageFaultOutput.pHeadExistingAllocationNode;
        if (pNode == nullptr)
        {
            OutputLog(" None.");
        }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode1(pNode);
                pNode = pNode->pNext;
            }
        }
        OutputLog("---------------------------------");
    }

    {
        OutputLog("--- Recent Freed Allocation Node ---");
        auto pNode = pageFaultOutput.pHeadRecentFreedAllocationNode;
        if (pNode == nullptr)
        {
            OutputLog(" None.");
        }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode1(pNode);
                pNode = pNode->pNext;
            }
        }
        OutputLog("------------------------------------");
    }
}

//-----------------------------------------------------------------------------
//! @brief      デバイス削除拡張データをログ出力します.
//-----------------------------------------------------------------------------
bool ReportDeviceRemovedExtendedData(ID3D12Device* pDevice)
{
    RefPtr<ID3D12DeviceRemovedExtendedData> dred;
    auto hr = pDevice->QueryInterface(IID_PPV_ARGS(dred.GetAddressOf()));
    if (FAILED(hr))
    { return false; }

    D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbsOutput = {};
    hr = dred->GetAutoBreadcrumbsOutput(&breadcrumbsOutput);
    if (SUCCEEDED(hr))
    {
        auto pNode = breadcrumbsOutput.pHeadAutoBreadcrumbNode;
        while(pNode != nullptr)
        {
            ReportBreadcrumbNode(pNode);
            pNode = pNode->pNext;
        }
    }

    D3D12_DRED_PAGE_FAULT_OUTPUT pageFaultOutput = {};
    hr = dred->GetPageFaultAllocationOutput(&pageFaultOutput);
    if (SUCCEEDED(hr))
    {
        ReportPageFaultOutput(pageFaultOutput);
    }

    return true;
}

//-----------------------------------------------------------------------------
//! @brief      デバイス削除拡張データをログ出力します.
//-----------------------------------------------------------------------------
bool ReportDeviceRemovedExtendedData1(ID3D12Device* pDevice)
{
    RefPtr<ID3D12DeviceRemovedExtendedData1> dred;
    auto hr = pDevice->QueryInterface(IID_PPV_ARGS(dred.GetAddressOf()));
    if (FAILED(hr))
    { return false; }

    D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbsOutput1 = {};
    hr = dred->GetAutoBreadcrumbsOutput1(&breadcrumbsOutput1);
    if (SUCCEEDED(hr))
    {
        auto pNode = breadcrumbsOutput1.pHeadAutoBreadcrumbNode;
        while(pNode != nullptr)
        {
            ReportBreadcrumbNode1(pNode);
            pNode = pNode->pNext;
        }
    }
    else
    {
        D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbsOutput = {};
        hr = dred->GetAutoBreadcrumbsOutput(&breadcrumbsOutput);
        if (SUCCEEDED(hr))
        {
            auto pNode = breadcrumbsOutput.pHeadAutoBreadcrumbNode;
            while(pNode != nullptr)
            {
                ReportBreadcrumbNode(pNode);
                pNode = pNode->pNext;
            }
        }
    }

    D3D12_DRED_PAGE_FAULT_OUTPUT1 pageFaultOutput1 = {};
    hr = dred->GetPageFaultAllocationOutput1(&pageFaultOutput1);
    if (SUCCEEDED(hr))
    {
        ReportPageFaultOutput1(pageFaultOutput1);
    }
    else
    {
        D3D12_DRED_PAGE_FAULT_OUTPUT pageFaultOutput = {};
        hr = dred->GetPageFaultAllocationOutput(&pageFaultOutput);
        if (SUCCEEDED(hr))
        {
            ReportPageFaultOutput(pageFaultOutput);
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//! @brief      デバイス削除拡張データをログ出力します.
//-----------------------------------------------------------------------------
bool ReportDeviceRemovedExtendedData2(ID3D12Device* pDevice)
{
    RefPtr<ID3D12DeviceRemovedExtendedData2> dred;
    auto hr = pDevice->QueryInterface(IID_PPV_ARGS(dred.GetAddressOf()));
    if (FAILED(hr))
    { return false; }

    auto state = dred->GetDeviceState();
    OutputLog("DeviceState : %s", ToString(state));

    D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbsOutput1 = {};
    hr = dred->GetAutoBreadcrumbsOutput1(&breadcrumbsOutput1);
    if (SUCCEEDED(hr))
    {
        auto pNode = breadcrumbsOutput1.pHeadAutoBreadcrumbNode;
        while(pNode != nullptr)
        {
            ReportBreadcrumbNode1(pNode);
            pNode = pNode->pNext;
        }
    }
    else
    {
        D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadCrumbsOutput = {};
        hr = dred->GetAutoBreadcrumbsOutput(&breadCrumbsOutput);
        if (SUCCEEDED(hr))
        {
            auto pNode = breadCrumbsOutput.pHeadAutoBreadcrumbNode;
            while(pNode != nullptr)
            {
                ReportBreadcrumbNode(pNode);
                pNode = pNode->pNext;
            }
        }
    }

    D3D12_DRED_PAGE_FAULT_OUTPUT2 pageFalutOutput2 = {};
    hr = dred->GetPageFaultAllocationOutput2(&pageFalutOutput2);
    if (SUCCEEDED(hr))
    {
        ReportPageFaultOutput2(pageFalutOutput2);
    }
    else
    {
        D3D12_DRED_PAGE_FAULT_OUTPUT1 pageFaultOutput1 = {};
        hr = dred->GetPageFaultAllocationOutput1(&pageFaultOutput1);
        if (SUCCEEDED(hr))
        {
            ReportPageFaultOutput1(pageFaultOutput1);
        }
        else
        {
            D3D12_DRED_PAGE_FAULT_OUTPUT pageFaultOutput = {};
            hr = dred->GetPageFaultAllocationOutput(&pageFaultOutput);
            if (SUCCEEDED(hr))
            {
                ReportPageFaultOutput(pageFaultOutput);
            }
        }
    }

    return true;
}

} // namespace

//-----------------------------------------------------------------------------
//! @brief      DRED情報を出力します.
//-----------------------------------------------------------------------------
void ReportDRED(HRESULT hr, ID3D12Device* pDevice)
{
    OutputLog("ErrorCode : 0x%x - %s", hr, std::system_category().message(hr).c_str());
    if (pDevice == nullptr)
        return;

    auto reason = pDevice->GetDeviceRemovedReason();
    OutputLog("Device Removed Reason : 0x%x - %s", reason, std::system_category().message(reason).c_str());

    // 新しいバージョンから順に実行していき，実行出来たら終了.

    // DRED 1.2
    if (ReportDeviceRemovedExtendedData2(pDevice))
    { return; }

    // DRED 1.1
    if (ReportDeviceRemovedExtendedData1(pDevice))
    { return; }

    // DRED 1.0
    if (ReportDeviceRemovedExtendedData(pDevice))
    { return; }
}