#pragma once

#include "nativecommon.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

///
/// < PEB Macros and Definitions >
///

#define FLS_BITMAP_BITS             (FLS_MAXIMUM_AVAILABLE / (sizeof( ULONG ) * 8))

#define GDI_HANDLE_BUFFER_SIZE32    34
#define GDI_HANDLE_BUFFER_SIZE64    60
#if defined(_WIN64)
#define GDI_HANDLE_BUFFER_SIZE GDI_HANDLE_BUFFER_SIZE64
#else
#define GDI_HANDLE_BUFFER_SIZE GDI_HANDLE_BUFFER_SIZE32
#endif

typedef ULONG GDI_HANDLE_BUFFER[GDI_HANDLE_BUFFER_SIZE];
typedef ULONG GDI_HANDLE_BUFFER32[GDI_HANDLE_BUFFER_SIZE32];
typedef ULONG GDI_HANDLE_BUFFER64[GDI_HANDLE_BUFFER_SIZE64];

///
/// < PEB Structures >
///

//
// PEB Loader Data
//

typedef struct _PEB_LDR_DATA32 {
    ULONG Length;                                   // 0x00
    UCHAR Initialized;                              // 0x04
    ULONG SsHandle;                                 // 0x08
    LIST_ENTRY32 InLoadOrderModuleList;             // 0x0C
    LIST_ENTRY32 InMemoryOrderModuleList;           // 0x14
    LIST_ENTRY32 InInitializationOrderModuleList;   // 0x1C
    ULONG EntryInProgress;                          // 0x24
    BOOLEAN ShutdownInProgress;                     // 0x28
    ULONG ShutdownThreadId;                         // 0x2C
} PEB_LDR_DATA32, *PPEB_LDR_DATA32;

typedef struct _PEB_LDR_DATA64 {
    ULONG Length;                                   // 0x00
    UCHAR Initialized;                              // 0x04
    ULONGLONG SsHandle;                             // 0x08
    LIST_ENTRY64 InLoadOrderModuleList;             // 0x10
    LIST_ENTRY64 InMemoryOrderModuleList;           // 0x20
    LIST_ENTRY64 InInitializationOrderModuleList;   // 0x30
    ULONGLONG EntryInProgress;                      // 0x40
    BOOLEAN ShutdownInProgress;                     // 0x48
    ULONGLONG ShutdownThreadId;                     // 0x50
} PEB_LDR_DATA64, *PPEB_LDR_DATA64;

typedef struct _PEB_LDR_DATA {
    ULONG Length;                                   // 0x00 0x00
    UCHAR Initialized;                              // 0x04 0x04
    PVOID SsHandle;                                 // 0x08 0x08
    LIST_ENTRY InLoadOrderModuleList;               // 0x10 0x0C
    LIST_ENTRY InMemoryOrderModuleList;             // 0x20 0x14
    LIST_ENTRY InInitializationOrderModuleList;     // 0x30 0x1C
    PVOID EntryInProgress;                          // 0x40 0x24
    UCHAR ShutdownInProgress;                       // 0x48 0x28
    PVOID ShutdownThreadId;                         // 0x50 0x2C
} PEB_LDR_DATA, *PPEB_LDR_DATA;


//
// PEB Structures
//

typedef struct _PEB32 {
    UCHAR InheritedAddressSpace;                        // 0x00
    UCHAR ReadImageFileExecOptions;                     // 0x01
    UCHAR BeingDebugged;                                // 0x02
    union {
        UCHAR BitField;                                 // 0x03
        struct {
            UCHAR ImageUsesLargePages : 1;
            UCHAR IsProtectedProcess : 1;
            UCHAR IsImageDynamicallyRelocated : 1;
            UCHAR SkipPatchingUser32Forwarders : 1;
            UCHAR IsPackagedProcess : 1;
            UCHAR IsAppContainer : 1;
            UCHAR IsProtectedProcessLight : 1;
            UCHAR SpareBits : 1;
        };
    };
    ULONG Mutant;                                       // 0x04 PVOID
    ULONG ImageBaseAddress;                             // 0x08 PVOID
    ULONG Ldr;                                          // 0x0C PPEB_LDR_DATA32
    ULONG ProcessParameters;                            // 0x10 PRTL_USER_PROCESS_PARAMETERS
    ULONG SubSystemData;                                // 0x14 PVOID
    ULONG ProcessHeap;                                  // 0x18 PVOID
    ULONG FastPebLock;                                  // 0x1C PRTL_CRITICAL_SECTION
    ULONG AtlThunkSListPtr;                             // 0x20 PVOID
    ULONG IFEOKey;                                      // 0x24
    union {
        ULONG CrossProcessFlags;                        // 0x28
        struct {
            ULONG ProcessInJob : 1;
            ULONG ProcessInitializing : 1;
            ULONG ProcessUsingVEH : 1;
            ULONG ProcessUsingVCH : 1;
            ULONG ProcessUsingFTH : 1;
            ULONG ReservedBits0 : 27;
        };
    };
    union {
        ULONG KernelCallbackTable;                      // 0x2C
        ULONG UserSharedInfoPtr;                        // 0x2C
    };
    ULONG SystemReserved[1];                            // 0x30
    ULONG AtlThunkSListPtr32;                           // 0x34
    ULONG ApiSetMap;                                    // 0x38 PVOID
    ULONG TlsExpansionCounter;                          // 0x3C
    ULONG TlsBitmap;                                    // 0x40
    ULONG TlsBitmapBits[2];                             // 0x44
    ULONG ReadOnlySharedMemoryBase;                     // 0x4C
    ULONG SparePvoid0;                                  // 0x50
    ULONG ReadOnlyStaticServerData;                     // 0x54
    ULONG AnsiCodePageData;                             // 0x58
    ULONG OemCodePageData;                              // 0x5C
    ULONG UnicodeCaseTableData;                         // 0x60
    ULONG NumberOfProcessors;                           // 0x64
    ULONG NtGlobalFlag;                                 // 0x68
    LARGE_INTEGER CriticalSectionTimeout;               // 0x70
    ULONG HeapSegmentReserve;                           // 0x78
    ULONG HeapSegmentCommit;                            // 0x7C
    ULONG HeapDeCommitTotalFreeThreshold;               // 0x80
    ULONG HeapDeCommitFreeBlockThreshold;               // 0x84
    ULONG NumberOfHeaps;                                // 0x88
    ULONG MaximumNumberOfHeaps;                         // 0x8C
    ULONG ProcessHeaps;                                 // 0x90
    ULONG GdiSharedHandleTable;                         // 0x94
    ULONG ProcessStarterHelper;                         // 0x98
    ULONG GdiDCAttributeList;                           // 0x9C
    ULONG LoaderLock;                                   // 0xA0
    ULONG OSMajorVersion;                               // 0xA4
    ULONG OSMinorVersion;                               // 0xA8
    USHORT OSBuildNumber;                               // 0xAC
    USHORT OSCSDVersion;                                // 0xAE
    ULONG OSPlatformId;                                 // 0xB0
    ULONG ImageSubsystem;                               // 0xB4
    ULONG ImageSubsystemMajorVersion;                   // 0xB8
    ULONG ImageSubsystemMinorVersion;                   // 0xBC
    ULONG ActiveProcessAffinityMask;                    // 0xC0
    ULONG GdiHandleBuffer[34];                          // 0xC4
    ULONG PostProcessInitRoutine;                       // 0x14C
    ULONG TlsExpansionBitmap;                           // 0x150
    ULONG TlsExpansionBitmapBits[32];                   // 0x154
    ULONG SessionId;                                    // 0x1D4
    ULARGE_INTEGER AppCompatFlags;                      // 0x1D8
    ULARGE_INTEGER AppCompatFlagsUser;                  // 0x1E0
    ULONG pShimData;                                    // 0x1E8
    ULONG AppCompatInfo;                                // 0x1EC
    UNICODE_STRING32 CSDVersion;                        // 0x1F0
    ULONG ActivationContextData;                        // 0x1F8
    ULONG ProcessAssemblyStorageMap;                    // 0x1FC
    ULONG SystemDefaultActivationContextData;           // 0x200
    ULONG SystemAssemblyStorageMap;                     // 0x204
    ULONG MinimumStackCommit;                           // 0x208
    ULONG FlsCallback;                                  // 0x20C
    LIST_ENTRY32 FlsListHead;                           // 0x210
    ULONG FlsBitmap;                                    // 0x218
    ULONG FlsBitmapBits[FLS_BITMAP_BITS];               // 0x21C
    ULONG FlsHighIndex;                                 // 0x22C
    // >= Windows Vista
    ULONG WerRegistrationData;                          // 0x230
    ULONG WerShipAssertPtr;                             // 0x234
    ULONG pUnused;                                      // 0x238
    ULONG pImageHeaderHash;                             // 0x23C
    union {
        ULONG TracingFlags;                             // 0x240
        struct {
            ULONG HeapTracingEnabled : 1;
            ULONG CritSecTracingEnabled : 1;
            ULONG LibLoaderTracingEnabled : 1;
            ULONG SpareTracingBits : 29;
        };
    };
    ULONG64 CsrServerReadOnlySharedMemoryBase;          // 0x248
    // >= Windows 8.1
    ULONG TppWorkerpListLock;                           // 0x250
    LIST_ENTRY32 TppWorkerpList;                        // 0x254
    ULONG WaitOnAddressHashTable[128];                  // 0x25C PVOID[128]
    // >= Windows 10 RS1
    ULONG TelemetryCoverageHeader;                      // 0x45C PVOID
    ULONG CloudFileFlags;                               // 0x460
} PEB32, *PPEB32;

typedef struct _PEB64 {
    UCHAR InheritedAddressSpace;                        // 0x00
    UCHAR ReadImageFileExecOptions;                     // 0x01
    UCHAR BeingDebugged;                                // 0x02
    union {
        UCHAR BitField;                                 // 0x03
        struct {
            UCHAR ImageUsesLargePages : 1;
            UCHAR IsProtectedProcess : 1;
            UCHAR IsImageDynamicallyRelocated : 1;
            UCHAR SkipPatchingUser32Forwarders : 1;
            UCHAR IsPackagedProcess : 1;
            UCHAR IsAppContainer : 1;
            UCHAR IsProtectedProcessLight : 1;
            UCHAR SpareBits : 1;
        };
    };
    UCHAR Padding0[4];                                  // 0x04
    ULONG64 Mutant;                                     // 0x08 PVOID
    ULONG64 ImageBaseAddress;                           // 0x10 PVOID
    ULONG64 Ldr;                                        // 0x18 PPEB_LDR_DATA64
    ULONG64 ProcessParameters;                          // 0x20 PRTL_USER_PROCESS_PARAMETERS
    ULONG64 SubSystemData;                              // 0x28 PVOID
    ULONG64 ProcessHeap;                                // 0x30 PVOID
    ULONG64 FastPebLock;                                // 0x38 PRTL_CRITICAL_SECTION
    ULONG64 AtlThunkSListPtr;                           // 0x40 PVOID
    ULONG64 IFEOKey;                                    // 0x48 PVOID
    union {
        ULONG CrossProcessFlags;                        // 0x50
        struct {
            ULONG ProcessInJob : 1;
            ULONG ProcessInitializing : 1;
            ULONG ProcessUsingVEH : 1;
            ULONG ProcessUsingVCH : 1;
            ULONG ProcessUsingFTH : 1;
            ULONG ReservedBits0 : 27;
        };
    };
    UCHAR Padding1[4];                                  // 0x54
    union {
        ULONGLONG KernelCallbackTable;                  // 0x58
        ULONGLONG UserSharedInfoPtr;                    // 0x58
    };
    ULONG SystemReserved[1];                            // 0x60
    ULONG AtlThunkSListPtr32;                           // 0x64
    ULONG64 ApiSetMap;                                  // 0x68 PVOID
    ULONG TlsExpansionCounter;                          // 0x70
    UCHAR Padding2[4];                                  // 0x74
    ULONG64 TlsBitmap;                                  // 0x78 PVOID
    ULONG TlsBitmapBits[2];                             // 0x80
    ULONG64 ReadOnlySharedMemoryBase;                   // 0x88 PVOID
    ULONG64 SparePvoid0;                                // 0x90 PVOID
    ULONG64 ReadOnlyStaticServerData;                   // 0x98 PVOID*
    ULONG64 AnsiCodePageData;                           // 0xA0 PVOID
    ULONG64 OemCodePageData;                            // 0xA8 PVOID
    ULONG64 UnicodeCaseTableData;                       // 0xB0 PVOID
    ULONG NumberOfProcessors;                           // 0xB8
    ULONG NtGlobalFlag;                                 // 0xBC
    LARGE_INTEGER CriticalSectionTimeout;               // 0xC0
    ULONG64 HeapSegmentReserve;                         // 0xC8
    ULONG64 HeapSegmentCommit;                          // 0xD0
    ULONG64 HeapDeCommitTotalFreeThreshold;             // 0xD8
    ULONG64 HeapDeCommitFreeBlockThreshold;             // 0xE0
    ULONG NumberOfHeaps;                                // 0xE8
    ULONG MaximumNumberOfHeaps;                         // 0xEC
    ULONG64 ProcessHeaps;                               // 0xF0 PVOID*
    ULONG64 GdiSharedHandleTable;                       // 0xF8 PVOID
    ULONG64 ProcessStarterHelper;                       // 0x100 PVOID
    ULONG GdiDCAttributeList;                           // 0x108
    UCHAR Padding3[4];                                  // 0x10C
    ULONG64 LoaderLock;                                 // 0x110 PRTL_CRITICAL_SECTION
    ULONG OSMajorVersion;                               // 0x118
    ULONG OSMinorVersion;                               // 0x11C
    USHORT OSBuildNumber;                               // 0x120
    USHORT OSCSDVersion;                                // 0x122
    ULONG OSPlatformId;                                 // 0x124
    ULONG ImageSubsystem;                               // 0x128
    ULONG ImageSubsystemMajorVersion;                   // 0x12C
    ULONG ImageSubsystemMinorVersion;                   // 0x130
    UCHAR Padding4[4];                                  // 0x134
    ULONG64 ActiveProcessAffinityMask;                  // 0x138
    ULONG GdiHandleBuffer[60];                          // 0x140
    ULONG64 PostProcessInitRoutine;                     // 0x23 PVOID
    ULONG64 TlsExpansionBitmap;                         // 0x23 PVOID
    ULONG TlsExpansionBitmapBits[32];                   // 0x240
    ULONG SessionId;                                    // 0x2C0
    UCHAR Padding5[4];                                  // 0x2C4
    ULARGE_INTEGER AppCompatFlags;                      // 0x2C8
    ULARGE_INTEGER AppCompatFlagsUser;                  // 0x2D0
    ULONG64 pShimData;                                  // 0x2D8 PVOID
    ULONG64 AppCompatInfo;                              // 0x2E0 PVOID
    UNICODE_STRING64 CSDVersion;                        // 0x2E8
    ULONG64 ActivationContextData;                      // 0x2F8 PVOID
    ULONG64 ProcessAssemblyStorageMap;                  // 0x300 PVOID
    ULONG64 SystemDefaultActivationContextData;         // 0x308 PVOID
    ULONG64 SystemAssemblyStorageMap;                   // 0x310 PVOID
    ULONG64 MinimumStackCommit;                         // 0x318
    ULONG64 FlsCallback;                                // 0x320 PVOID
    LIST_ENTRY64 FlsListHead;                           // 0x328
    ULONG64 FlsBitmap;                                  // 0x338 PVOID
    ULONG FlsBitmapBits[FLS_BITMAP_BITS];               // 0x340
    ULONG FlsHighIndex;                                 // 0x350
    // >= Windows Vista
    ULONG64 WerRegistrationData;                        // 0x358 PVOID
    ULONG64 WerShipAssertPtr;                           // 0x360 PVOID
    ULONG64 pUnused;                                    // 0x368 PVOID
    ULONG64 pImageHeaderHash;                           // 0x370 PVOID
    union {
        ULONG TracingFlags;                             // 0x378
        struct {
            ULONG HeapTracingEnabled : 1;
            ULONG CritSecTracingEnabled : 1;
            ULONG LibLoaderTracingEnabled : 1;
            ULONG SpareTracingBits : 29;
        };
    };
    UCHAR Padding6[4];                                  // 0x37C
    ULONG64 CsrServerReadOnlySharedMemoryBase;          // 0x380
    // >= Windows 8.1
    ULONG64 TppWorkerpListLock;                         // 0x388
    LIST_ENTRY64 TppWorkerpList;                        // 0x390
    ULONG64 WaitOnAddressHashTable[128];                // 0x3A0 PVOID[128]
} PEB64, *PPEB64;

typedef struct _PEB {
    BOOLEAN InheritedAddressSpace;                      // 0x00 0x00
    BOOLEAN ReadImageFileExecOptions;                   // 0x01 0x01
    BOOLEAN BeingDebugged;                              // 0x02 0x02
    union {
        BOOLEAN BitField;                               // 0x03 0x03
        struct {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN IsProtectedProcess : 1;
            BOOLEAN IsImageDynamicallyRelocated : 1;
            BOOLEAN SkipPatchingUser32Forwarders : 1;
            BOOLEAN IsPackagedProcess : 1;
            BOOLEAN IsAppContainer : 1;
            BOOLEAN IsProtectedProcessLight : 1;
            BOOLEAN SpareBits : 1;
        };
    };
#if defined(_WIN64)
    UCHAR Padding0[4];                                  // 0x04
#endif
    HANDLE Mutant;                                      // 0x08 0x04
    PVOID ImageBaseAddress;                             // 0x10 0x08
    PPEB_LDR_DATA Ldr;                                  // 0x18 0x0C
    struct _RTL_USER_PROCESS_PARAMETERS *ProcessParameters; // 0x20 0x10
    PVOID SubSystemData;                                // 0x28 0x14
    PVOID ProcessHeap;                                  // 0x30 0x18
    struct _RTL_CRITICAL_SECTION *FastPebLock;          // 0x38 0x1C
    PVOID AtlThunkSListPtr;                             // 0x40 0x20
    PVOID IFEOKey;                                      // 0x48 0x24
    union {
        ULONG CrossProcessFlags;                        // 0x50 0x28
        struct {
            ULONG ProcessInJob : 1;
            ULONG ProcessInitializing : 1;
            ULONG ProcessUsingVEH : 1;
            ULONG ProcessUsingVCH : 1;
            ULONG ProcessUsingFTH : 1;
            ULONG ProcessPreviouslyThrottled : 1;
            ULONG ProcessCurrentlyThrottled : 1;
            ULONG ReservedBits0 : 25;
        };
    };
#if defined(_WIN64)
    UCHAR Padding1[4];                                  // 0x54
#endif
    union {
        PVOID KernelCallbackTable;                      // 0x58 0x2C
        PVOID UserSharedInfoPtr;                        // 0x58 0x2C
    };
    ULONG SystemReserved;                               // 0x60 0x30
    ULONG AtlThunkSListPtr32;                           // 0x64 0x34
    PVOID ApiSetMap;                                    // 0x68 0x38
    ULONG TlsExpansionCounter;                          // 0x70 0x3C
#if defined(_WIN64)
    UCHAR Padding2[4];                                  // 0x74 
#endif
    PVOID TlsBitmap;                                    // 0x78 0x40
    ULONG TlsBitmapBits[2];                             // 0x80 0x44
    PVOID ReadOnlySharedMemoryBase;                     // 0x88 0x4C
    PVOID SharedData;                                   // 0x90 0x50
    PVOID *ReadOnlyStaticServerData;                    // 0x98 0x54
    PVOID AnsiCodePageData;                             // 0xA0 0x58
    PVOID OemCodePageData;                              // 0xA8 0x5C
    PVOID UnicodeCaseTableData;                         // 0xB0 0x60

    ULONG NumberOfProcessors;                           // 0xB8 0x64
    ULONG NtGlobalFlag;                                 // 0xBC 0x68

    LARGE_INTEGER CriticalSectionTimeout;               // 0xC0 0x70
    SIZE_T HeapSegmentReserve;                          // 0xC8 0x78
    SIZE_T HeapSegmentCommit;                           // 0xD0 0x7C
    SIZE_T HeapDeCommitTotalFreeThreshold;              // 0xD8 0x80
    SIZE_T HeapDeCommitFreeBlockThreshold;              // 0xE0 0x84

    ULONG NumberOfHeaps;                                // 0xE8 0x88
    ULONG MaximumNumberOfHeaps;                         // 0xEC 0x8C
    PVOID *ProcessHeaps;                                // 0xF0 0x90

    PVOID GdiSharedHandleTable;                         // 0xF8 0x94
    PVOID ProcessStarterHelper;                         // 0x100 0x98
    ULONG GdiDCAttributeList;                           // 0x108 0x9C

#if defined(_WIN64)
    UCHAR Padding3[4];                                  // 0x10C 
#endif

    struct _RTL_CRITICAL_SECTION *LoaderLock;           // 0x110 0xA0

    ULONG OSMajorVersion;                               // 0x118 0xA4
    ULONG OSMinorVersion;                               // 0x11C 0xA8
    USHORT OSBuildNumber;                               // 0x120 0xAC
    USHORT OSCSDVersion;                                // 0x122 0xAE
    ULONG OSPlatformId;                                 // 0x124 0xB0

    ULONG ImageSubsystem;                               // 0x128 0xB4
    ULONG ImageSubsystemMajorVersion;                   // 0x12C 0xB8
    ULONG ImageSubsystemMinorVersion;                   // 0x130 0xBC
#if defined(_WIN64)
    UCHAR Padding4[4];                                  // 0x134 
#endif
    SIZE_T ActiveProcessAffinityMask;                   // 0x138 0xC0 
    GDI_HANDLE_BUFFER GdiHandleBuffer;                  // 0x140 0xC4
    PVOID PostProcessInitRoutine;                       // 0x230 0x14C

    PVOID TlsExpansionBitmap;                           // 0x238 0x150
    ULONG TlsExpansionBitmapBits[32];                   // 0x240 0x154

    ULONG SessionId;                                    // 0x2C0 0x1D4
#if defined(_WIN64)
    UCHAR Padding5[4];                                  // 0x2C4 
#endif

    ULARGE_INTEGER AppCompatFlags;                      // 0x2C8 0x1D8
    ULARGE_INTEGER AppCompatFlagsUser;                  // 0x2D0 0x1E0
    PVOID pShimData;                                    // 0x2D8 0x1E8
    PVOID AppCompatInfo;                                // 0x2E0 0x1EC

    UNICODE_STRING CSDVersion;                          // 0x2E8 0x1F0

    PVOID ActivationContextData;                        // 0x2F8 0x1F8
    PVOID ProcessAssemblyStorageMap;                    // 0x300 0x1FC
    PVOID SystemDefaultActivationContextData;           // 0x308 0x200
    PVOID SystemAssemblyStorageMap;                     // 0x310 0x204

    SIZE_T MinimumStackCommit;                          // 0x318 0x208

    PVOID *FlsCallback;                                 // 0x320 0x20C
    LIST_ENTRY FlsListHead;                             // 0x328 0x210
    PVOID FlsBitmap;                                    // 0x338 0x218
    ULONG FlsBitmapBits[FLS_BITMAP_BITS];               // 0x340 0x21C
    ULONG FlsHighIndex;                                 // 0x350 0x22C
    // >= Windows Vista
    PVOID WerRegistrationData;                          // 0x358 0x230
    PVOID WerShipAssertPtr;                             // 0x360 0x234
    union {
        PVOID pContextData;                             // 0x368 0x238
        PVOID pUnused;                                  // 0x368 0x238
    };
    PVOID pImageHeaderHash;                             // 0x370 0x23C
    union {
        ULONG TracingFlags;                             // 0x378 0x240
        struct {
            ULONG HeapTracingEnabled : 1;
            ULONG CritSecTracingEnabled : 1;
            ULONG LibLoaderTracingEnabled : 1;
            ULONG SpareTracingBits : 29;
        };
    };
    UCHAR Padding6[4];                                  // 0x37C 0x244
    ULONGLONG CsrServerReadOnlySharedMemoryBase;        // 0x380 0x248
    // >= Windows 8.1
    ULONG_PTR TppWorkerpListLock;                       // 0x388 0x250
    LIST_ENTRY TppWorkerpList;                          // 0x390 0x254
    PVOID WaitOnAddressHashTable[128];                  // 0x3A0 0x25C
    // >= Windows 10 RS1
    PVOID TelemetryCoverageHeader;                      // 0x7A0 0x45C
    ULONG CloudFileFlags;                               // 0x7A8 0x460
} PEB, *PPEB;


#ifdef __cplusplus
} // extern "C" 
#endif // __cplusplus