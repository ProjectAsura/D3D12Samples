# D3D12_DRED
Device Remoded Extended Data Sample Program.

![キャプチャー画像](./result.png)

## Sample Log

~~~~
ErrorCode : 0x887a0005 - GPU デバイス インスタンスが中断されています。GetDeviceRemovedReason を使用して適切なアクションを確認してください。
Device Removed Reason : 0x887a0006 - GPU が以降のコマンドに応答しません。原因として、呼び出し元のアプリケーションから無効なコマンドが渡されたことが考えられます。
Breadcrumb Node (0x000001C96ABDB128) : 
    State              : Not Started
    BreadcrumbCount    : 0
    LastBreadcrumValue : 0
    Has Next           : Yes
    ContextCount       : 0
    CommandQueue       :
        Pointer    = 0x000001C971A94560
        Type       = DIRECT
        Priority   = 0
        Flags      = 0x0
        NodeMask   = 1
        DebugNameA = (null)
        DebugNameW = (null)
    CommandHistory     : None
    BreadcrumbContext  : None
Breadcrumb Node (0x000001C96ABDA328) : 
    State              : Not Started
    BreadcrumbCount    : 0
    LastBreadcrumValue : 0
    Has Next           : Yes
    ContextCount       : 0
    CommandQueue       :
        Pointer    = 0x000001C971A94560
        Type       = DIRECT
        Priority   = 0
        Flags      = 0x0
        NodeMask   = 1
        DebugNameA = (null)
        DebugNameW = (null)
    CommandHistory     : None
    BreadcrumbContext  : None
Breadcrumb Node (0x000001C96ABDBF28) : 
    State              : Not Completed.
    BreadcrumbCount    : 5
    LastBreadcrumValue : 3
    Has Next           : No
    ContextCount       : 0
    CommandQueue       :
        Pointer    = 0x000001C971A94560
        Type       = DIRECT
        Priority   = 0
        Flags      = 0x0
        NodeMask   = 1
        DebugNameA = (null)
        DebugNameW = (null)
    CommandHistory     : 
        00000: [OK] Op = RESOURCEBARRIER
        00001: [OK] Op = CLEARRENDERTARGETVIEW
        00002: [OK] Op = CLEARDEPTHSTENCILVIEW
        00003: [NG] Op = DRAWINSTANCED
        00004: [  ] Op = RESOURCEBARRIER
    BreadcrumbContext  : None
PageFaultOutput : 
    PageFaultVA : 0x0
--- Existing Allocation Node  ---
None
---------------------------------
--- Recent Freed Allocation Node ---
None
---------------------------------
~~~~

Licence
--------------------

This software is distributed under MIT. For details, see LICENCE file.
