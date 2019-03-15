#pragma once

#include "nativecommon.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define LEVEL_MASK 3

//
// Push lock definitions
//
typedef struct _EX_PUSH_LOCK {
    //
    // LOCK bit is set for both exclusive and shared acquires
    //
#define EX_PUSH_LOCK_LOCK_V          ((ULONG_PTR)0x0)
#define EX_PUSH_LOCK_LOCK            ((ULONG_PTR)0x1)
    //
    // Waiting bit designates that the pointer has chained waiters
    //
#define EX_PUSH_LOCK_WAITING         ((ULONG_PTR)0x2)
    //
    // Waking bit designates that we are either traversing the list
    // to wake threads or optimizing the list
    //
#define EX_PUSH_LOCK_WAKING          ((ULONG_PTR)0x4)
    //
    // Set if the lock is held shared by multiple owners and there are waiters
    //
#define EX_PUSH_LOCK_MULTIPLE_SHARED ((ULONG_PTR)0x8)
    //
    // Total shared Acquires are incremented using this
    //
#define EX_PUSH_LOCK_SHARE_INC       ((ULONG_PTR)0x10)
#define EX_PUSH_LOCK_PTR_BITS        ((ULONG_PTR)0xf)
    union {
        struct {
            ULONG_PTR Locked : 1;
            ULONG_PTR Waiting : 1;
            ULONG_PTR Waking : 1;
            ULONG_PTR MultipleShared : 1;
            ULONG_PTR Shared : sizeof( ULONG_PTR ) * 8 - 4;
        };
        ULONG_PTR Value;
        PVOID Ptr;
    };
} EX_PUSH_LOCK, *PEX_PUSH_LOCK;

typedef struct _EX_PUSH_LOCK64 {
    union {
        struct {
            ULONGLONG Locked : 1;
            ULONGLONG Waiting : 1;
            ULONGLONG Waking : 1;
            ULONGLONG MultipleShared : 1;
            ULONGLONG Shared : sizeof( ULONGLONG ) * 8 - 4;
        };
        ULONGLONG Value;
        ULONGLONG Ptr; // PVOID
    };
} EX_PUSH_LOCK64, *PEX_PUSH_LOCK64;

//
// Rundown protection structure
//
typedef struct _EX_RUNDOWN_REF {
#define EX_RUNDOWN_ACTIVE      0x01
#define EX_RUNDOWN_COUNT_SHIFT 0x01
#define EX_RUNDOWN_COUNT_INC   (1 << EX_RUNDOWN_COUNT_SHIFT)
    union {
        ULONG_PTR Count;
        PVOID Ptr;
    };
} EX_RUNDOWN_REF, *PEX_RUNDOWN_REF;

typedef struct _EX_RUNDOWN_REF64 {
    union {
        ULONGLONG Count;
        ULONGLONG Ptr;  // PVOID
    };
} EX_RUNDOWN_REF64, *PEX_RUNDOWN_REF64;

//
// Executive Fast Reference Structure
//
typedef struct _EX_FAST_REF {
    union {
        PVOID Object;
        ULONG_PTR RefCnt : 3;
        ULONG_PTR Value;
    };
} EX_FAST_REF, *PEX_FAST_REF;

typedef struct _EX_FAST_REF64 {
    union {
        ULONGLONG Object;           // PVOID
        ULONGLONG RefCnt : 3;
        ULONGLONG Value;
    };
} EX_FAST_REF64, *PEX_FAST_REF64;


//
// Absolute maximum number of handles allowed
//
#define MAX_HANDLES         (1 << 24)

#define LOWLEVEL_COUNT      (PAGE_SIZE / sizeof( HANDLE_TABLE_ENTRY ))
#define MIDLEVEL_COUNT      (PAGE_SIZE / sizeof( PHANDLE_TABLE_ENTRY ))
#define HIGHLEVEL_COUNT     MAX_HANDLES / (LOWLEVEL_COUNT * MIDLEVEL_COUNT)

#define HANDLE_TAG_BITS     (2)
#define HANDLE_INDEX_BITS   (30)

//
//  The Ex/Ob handle table package uses a common handle definition. The actual
//  type definition for a handle is a pvoid and is declared in sdk/inc. This
//  package uses only the low 32 bits of the pvoid pointer.
//
//  For simplicity we declare a new typedef called an exhandle
//
//  The 2 bits of an EXHANDLE is available to the application and is
//  ignored by the system.  The next 24 bits store the handle table entry
//  index and is used to refer to a particular entry in a handle table.
//
//  Note that this format is immutable because there are outside programs with
//  hardwired code that already assumes the format of a handle.
//
typedef struct _EXHANDLE {
    union {
        struct {
            //
            //  Application available tag bits
            //
            ULONG TagBits : HANDLE_TAG_BITS;
            //
            //  The handle table entry index
            //
            ULONG Index : HANDLE_INDEX_BITS;
       };
        HANDLE GenericHandleOverlay;
#define HANDLE_VALUE_INC 4 // Amount to increment the Value to get to the next handle
        ULONG_PTR Value;
    };
} EXHANDLE, *PEXHANDLE;

typedef union _EXHANDLE64 {
    struct {
        ULONG TagBits : HANDLE_TAG_BITS;
        ULONG Index : HANDLE_INDEX_BITS;
    };
    ULONGLONG GenericHandleOverlay; // HANDLE
    ULONGLONG Value;                // ULONG_PTR
} EXHANDLE64, *PEXHANDLE64;

typedef struct _HANDLE_TABLE_ENTRY_INFO {
    ULONG AuditMask;                                        // 0x00
    ULONG MaxRelativeAccessMask;                            // 0x04
} HANDLE_TABLE_ENTRY_INFO, *PHANDLE_TABLE_ENTRY_INFO;

typedef struct _HANDLE_TABLE_ENTRY {
    union {
        struct {
            union {
                volatile LONGLONG VolatileLowValue;         // 0x00
                LONGLONG LowValue;                          // 0x00
                PHANDLE_TABLE_ENTRY_INFO InfoTable;         // 0x00
            };
            union {
                LONGLONG HighValue;                         // 0x08
                struct _HANDLE_TABLE_ENTRY *NextFreeHandleEntry; // 0x08
                EXHANDLE LeafHandleValue;                   // 0x08
            };
        };
        struct {
            union {
                LONGLONG RefCountField;                     // 0x00
                struct {
                    LONGLONG Unlocked : 1;                  // 0x00
                    LONGLONG RefCnt : 16;                   // 0x00
                    LONGLONG Attributes : 3;                // 0x00
                    LONGLONG ObjectPointerBits : 44;        // 0x00
                };
            };
            ULONG GrantedAccessBits : 25;                   // 0x08
            ULONG NoRightsUpgrade : 1;                      // 0x08
            ULONG Spare1 : 6;                               // 0x08
            ULONG Spare2;                                   // 0x0C
        };
    };
} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;

typedef struct _HANDLE_TABLE_ENTRY64 {
    union {
        struct {
            union {
                volatile LONGLONG VolatileLowValue;         // 0x00
                LONGLONG LowValue;                          // 0x00
                ULONGLONG InfoTable;                        // 0x00 PHANDLE_TABLE_ENTRY_INFO
            };
            union {
                LONGLONG HighValue;                         // 0x08
                ULONGLONG NextFreeHandleEntry;              // 0x08 struct _HANDLE_TABLE_ENTRY *
                EXHANDLE64 LeafHandleValue;                 // 0x08
            };
        };
        struct {
            union {
                LONGLONG RefCountField;                     // 0x00
                struct {
                    LONGLONG Unlocked : 1;                  // 0x00
                    LONGLONG RefCnt : 16;                   // 0x00
                    LONGLONG Attributes : 3;                // 0x00
                    LONGLONG ObjectPointerBits : 44;        // 0x00
                };
            };
            ULONG GrantedAccessBits : 25;                   // 0x08
            ULONG NoRightsUpgrade : 1;                      // 0x08
            ULONG Spare1 : 6;                               // 0x08
            ULONG Spare2;                                   // 0x0C
        };
    };
} HANDLE_TABLE_ENTRY64, *PHANDLE_TABLE_ENTRY64;

typedef struct _HANDLE_TABLE_FREE_LIST {
    EX_PUSH_LOCK FreeListLock;                              // 0x00
    PHANDLE_TABLE_ENTRY FirstFreeHandleEntry;               // 0x08
    PHANDLE_TABLE_ENTRY LastFreeHandleEntry;                // 0x10
    LONG HandleCount;                                       // 0x18
    ULONG HighWaterMark;                                    // 0x1C
} HANDLE_TABLE_FREE_LIST, *PHANDLE_TABLE_FREE_LIST;

typedef struct _HANDLE_TABLE_FREE_LIST64 {
    EX_PUSH_LOCK64 FreeListLock;                            // 0x00
    ULONGLONG FirstFreeHandleEntry;                         // 0x08 PHANDLE_TABLE_ENTRY64
    ULONGLONG LastFreeHandleEntry;                          // 0x10 PHANDLE_TABLE_ENTRY64
    LONG HandleCount;                                       // 0x18
    ULONG HighWaterMark;                                    // 0x1C
} HANDLE_TABLE_FREE_LIST64, *PHANDLE_TABLE_FREE_LIST64;

typedef struct _HANDLE_TRACE_DB_ENTRY {
    CLIENT_ID ClientId;                                     // 0x00
    HANDLE Handle;                                          // 0x10
#define HANDLE_TRACE_DB_OPEN    1
#define HANDLE_TRACE_DB_CLOSE   2
#define HANDLE_TRACE_DB_BADREF  3
    ULONG Type;                                             // 0x18
    PVOID StackTrace[16];                                   // 0x20
} HANDLE_TRACE_DB_ENTRY, *PHANDLE_TRACE_DB_ENTRY;

typedef struct _HANDLE_TRACE_DB_ENTRY64 {
    CLIENT_ID64 ClientId;                                   // 0x00
    ULONGLONG Handle;                                       // 0x10 HANDLE
    ULONG Type;                                             // 0x18
    ULONGLONG StackTrace[16];                               // 0x20 PVOID[16]
} HANDLE_TRACE_DB_ENTRY64, *PHANDLE_TRACE_DB_ENTRY64;

typedef struct _HANDLE_TRACE_DEBUG_INFO {
    LONG RefCount;                                          // 0x00
    ULONG TableSize;                                        // 0x04
    ULONG BitMaskFlags;                                     // 0x08
    FAST_MUTEX CloseCompactionLock;                         // 0x10
    ULONG CurrentStackIndex;                                // 0x48
    HANDLE_TRACE_DB_ENTRY TraceDb[1];                       // 0x50
} HANDLE_TRACE_DEBUG_INFO, *PHANDLE_TRACE_DEBUG_INFO;

typedef struct _HANDLE_TRACE_DEBUG_INFO64 {
    LONG RefCount;                                          // 0x00
    ULONG TableSize;                                        // 0x04
    ULONG BitMaskFlags;                                     // 0x08
    ULONG Spare0;                                           // 0x0C
    FAST_MUTEX64 CloseCompactionLock;                       // 0x10
    ULONG CurrentStackIndex;                                // 0x48
    HANDLE_TRACE_DB_ENTRY64 TraceDb[1];                     // 0x50
} HANDLE_TRACE_DEBUG_INFO64, *PHANDLE_TRACE_DEBUG_INFO64;

typedef struct _HANDLE_TABLE {
    ULONG NextHandleNeedingPool;                            // 0x00
    LONG ExtraInfoPages;                                    // 0x04
    ULONG_PTR TableCode;                                    // 0x08
    struct _EPROCESS *QuotaProcess;                         // 0x10
    LIST_ENTRY HandleTableList;                             // 0x18
    ULONG UniqueProcessId;                                  // 0x28             
    union {
        ULONG Flags;                                        // 0x2C
        struct {
            ULONG StrictFIFO : 1;
            ULONG EnableHandleExceptions : 1;
            ULONG Rundown : 1;
            ULONG Duplicated : 1;
            ULONG RaiseUMExceptionOnInvalidHandleClose : 1;
        };
    };
    EX_PUSH_LOCK HandleContentionEvent;                     // 0x30
    EX_PUSH_LOCK HandleTableLock;                           // 0x38
    union {
        HANDLE_TABLE_FREE_LIST FreeLists[1];                // 0x40
        UCHAR ActualEntry[32];                              // 0x40
    };
    PHANDLE_TRACE_DEBUG_INFO DebugInfo;                     // 0x60
} HANDLE_TABLE, *PHANDLE_TABLE;

typedef struct _HANDLE_TABLE64 {
    ULONG NextHandleNeedingPool;                            // 0x00
    LONG ExtraInfoPages;                                    // 0x04
    ULONGLONG TableCode;                                    // 0x08 ULONG_PTR
    ULONGLONG QuotaProcess;                                 // 0x10 PEPROCESS
    LIST_ENTRY64 HandleTableList;                           // 0x18
    ULONG UniqueProcessId;                                  // 0x28             
    union {
        ULONG Flags;                                        // 0x2C
        struct {
            ULONG StrictFIFO : 1;
            ULONG EnableHandleExceptions : 1;
            ULONG Rundown : 1;
            ULONG Duplicated : 1;
            ULONG RaiseUMExceptionOnInvalidHandleClose : 1;
        };
    };
    EX_PUSH_LOCK64 HandleContentionEvent;                   // 0x30
    EX_PUSH_LOCK64 HandleTableLock;                         // 0x38
    union {
        HANDLE_TABLE_FREE_LIST64 FreeLists[1];              // 0x40
        UCHAR ActualEntry[32];                              // 0x40
    };
    ULONGLONG DebugInfo;                                    // 0x60 PHANDLE_TRACE_DEBUG_INFO64
} HANDLE_TABLE64, *PHANDLE_TABLE64;


//
// Extended processor state configuration
//

typedef struct _XSTATE_CONFIGURATION_EXTENDED {
    // Mask of all enabled features
    ULONG64 EnabledFeatures;

    // Mask of volatile enabled features
    ULONG64 EnabledVolatileFeatures;

    // Total size of the save area for user states
    ULONG Size;

    // Control Flags
    union {
        ULONG ControlFlags;
        struct {
            ULONG OptimizedSave : 1;
            ULONG CompactionEnabled : 1;
        };
    };

    // List of features
    XSTATE_FEATURE Features[MAXIMUM_XSTATE_FEATURES];

    // Mask of all supervisor features
    ULONG64 EnabledSupervisorFeatures;

    // Mask of features that require start address to be 64 byte aligned
    ULONG64 AlignedFeatures;

    // Total size of the save area for user and supervisor states
    ULONG AllFeatureSize;

    // List which holds size of each user and supervisor state supported by CPU
    ULONG AllFeatures[MAXIMUM_XSTATE_FEATURES];

    // Mask of all supervisor features that are exposed to user-mode
    ULONG64 EnabledUserVisibleSupervisorFeatures;

} XSTATE_CONFIGURATION_EXTENDED, *PXSTATE_CONFIGURATION_EXTENDED;

#define PROCESSOR_INTEL_386     386
#define PROCESSOR_INTEL_486     486
#define PROCESSOR_INTEL_PENTIUM 586
#define PROCESSOR_INTEL_IA64    2200
#define PROCESSOR_AMD_X8664     8664
#define PROCESSOR_MIPS_R4000    4000    // incl R4101 & R3910 for Windows CE
#define PROCESSOR_ALPHA_21064   21064
#define PROCESSOR_PPC_601       601
#define PROCESSOR_PPC_603       603
#define PROCESSOR_PPC_604       604
#define PROCESSOR_PPC_620       620
#define PROCESSOR_HITACHI_SH3   10003   // Windows CE
#define PROCESSOR_HITACHI_SH3E  10004   // Windows CE
#define PROCESSOR_HITACHI_SH4   10005   // Windows CE
#define PROCESSOR_MOTOROLA_821  821     // Windows CE
#define PROCESSOR_SHx_SH3       103     // Windows CE
#define PROCESSOR_SHx_SH4       104     // Windows CE
#define PROCESSOR_STRONGARM     2577    // Windows CE - 0xA11
#define PROCESSOR_ARM720        1824    // Windows CE - 0x720
#define PROCESSOR_ARM820        2080    // Windows CE - 0x820
#define PROCESSOR_ARM920        2336    // Windows CE - 0x920
#define PROCESSOR_ARM_7TDMI     70001   // Windows CE
#define PROCESSOR_OPTIL         0x494f  // MSIL

#define PROCESSOR_ARCHITECTURE_INTEL            0
#define PROCESSOR_ARCHITECTURE_MIPS             1
#define PROCESSOR_ARCHITECTURE_ALPHA            2
#define PROCESSOR_ARCHITECTURE_PPC              3
#define PROCESSOR_ARCHITECTURE_SHX              4
#define PROCESSOR_ARCHITECTURE_ARM              5
#define PROCESSOR_ARCHITECTURE_IA64             6
#define PROCESSOR_ARCHITECTURE_ALPHA64          7
#define PROCESSOR_ARCHITECTURE_MSIL             8
#define PROCESSOR_ARCHITECTURE_AMD64            9
#define PROCESSOR_ARCHITECTURE_IA32_ON_WIN64    10
#define PROCESSOR_ARCHITECTURE_NEUTRAL          11

#define PROCESSOR_ARCHITECTURE_UNKNOWN 0xFFFF

//
// Max number of processor features
//

#define PROCESSOR_FEATURE_MAX 64

//
// Defined processor features
//

#define PF_FLOATING_POINT_PRECISION_ERRATA       0
#define PF_FLOATING_POINT_EMULATED               1
#define PF_COMPARE_EXCHANGE_DOUBLE               2
#define PF_MMX_INSTRUCTIONS_AVAILABLE            3
#define PF_PPC_MOVEMEM_64BIT_OK                  4
#define PF_ALPHA_BYTE_INSTRUCTIONS               5
#define PF_XMMI_INSTRUCTIONS_AVAILABLE           6
#define PF_3DNOW_INSTRUCTIONS_AVAILABLE          7
#define PF_RDTSC_INSTRUCTION_AVAILABLE           8
#define PF_PAE_ENABLED                           9
#define PF_XMMI64_INSTRUCTIONS_AVAILABLE        10
#define PF_SSE_DAZ_MODE_AVAILABLE               11
#define PF_NX_ENABLED                           12
#define PF_SSE3_INSTRUCTIONS_AVAILABLE          13
#define PF_COMPARE_EXCHANGE128                  14
#define PF_COMPARE64_EXCHANGE128                15
#define PF_CHANNELS_ENABLED                     16
#define PF_XSAVE_ENABLED                        17
#define PF_ARM_VFP_32_REGISTERS_AVAILABLE       18
#define PF_ARM_NEON_INSTRUCTIONS_AVAILABLE      19
#define PF_SECOND_LEVEL_ADDRESS_TRANSLATION     20
#define PF_VIRT_FIRMWARE_ENABLED                21
#define PF_RDWRFSGSBASE_AVAILABLE               22
#define PF_FASTFAIL_AVAILABLE                   23
#define PF_ARM_DIVIDE_INSTRUCTION_AVAILABLE     24
#define PF_ARM_64BIT_LOADSTORE_ATOMIC           25
#define PF_ARM_EXTERNAL_CACHE_AVAILABLE         26
#define PF_ARM_FMAC_INSTRUCTIONS_AVAILABLE      27
#define PF_RDRAND_INSTRUCTION_AVAILABLE         28
#define PF_ARM_V8_INSTRUCTIONS_AVAILABLE        29
#define PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE 30
#define PF_ARM_V8_CRC32_INSTRUCTIONS_AVAILABLE  31
#define PF_RDTSCP_INSTRUCTION_AVAILABLE         32

typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE {
    StandardDesign,                 // None == 0 == standard design
    NEC98x86,                       // NEC PC98xx series on X86
    EndAlternatives                 // past end of known alternatives
} ALTERNATIVE_ARCHITECTURE_TYPE;

#define MAX_WOW64_SHARED_ENTRIES 16

//
// Define NX support policy values.
//

#define NX_SUPPORT_POLICY_ALWAYSOFF     0
#define NX_SUPPORT_POLICY_ALWAYSON      1
#define NX_SUPPORT_POLICY_OPTIN         2
#define NX_SUPPORT_POLICY_OPTOUT        3

//
// SEH chain validation policies.
//
// N.B. These constants must not be changed because the ldr relies on their
//      semantic meaning.
//

#define SEH_VALIDATION_POLICY_ON        0
#define SEH_VALIDATION_POLICY_OFF       1
#define SEH_VALIDATION_POLICY_TELEMETRY 2
#define SEH_VALIDATION_POLICY_DEFER     3

//
// Global shared data flags and manipulation macros.
//

#define SHARED_GLOBAL_FLAGS_ERROR_PORT_V                0x0
#define SHARED_GLOBAL_FLAGS_ERROR_PORT                  \
    (1UL << SHARED_GLOBAL_FLAGS_ERROR_PORT_V)

#define SHARED_GLOBAL_FLAGS_ELEVATION_ENABLED_V         0x1
#define SHARED_GLOBAL_FLAGS_ELEVATION_ENABLED           \
    (1UL << SHARED_GLOBAL_FLAGS_ELEVATION_ENABLED_V)

#define SHARED_GLOBAL_FLAGS_VIRT_ENABLED_V              0x2
#define SHARED_GLOBAL_FLAGS_VIRT_ENABLED                \
    (1UL << SHARED_GLOBAL_FLAGS_VIRT_ENABLED_V)

#define SHARED_GLOBAL_FLAGS_INSTALLER_DETECT_ENABLED_V  0x3
#define SHARED_GLOBAL_FLAGS_INSTALLER_DETECT_ENABLED    \
    (1UL << SHARED_GLOBAL_FLAGS_INSTALLER_DETECT_ENABLED_V)

#define SHARED_GLOBAL_FLAGS_LKG_ENABLED_V               0x4
#define SHARED_GLOBAL_FLAGS_LKG_ENABLED                 \
    (1UL << SHARED_GLOBAL_FLAGS_LKG_ENABLED_V)

#define SHARED_GLOBAL_FLAGS_DYNAMIC_PROC_ENABLED_V      0x5
#define SHARED_GLOBAL_FLAGS_DYNAMIC_PROC_ENABLED        \
    (1UL << SHARED_GLOBAL_FLAGS_DYNAMIC_PROC_ENABLED_V)

#define SHARED_GLOBAL_FLAGS_CONSOLE_BROKER_ENABLED_V    0x6
#define SHARED_GLOBAL_FLAGS_CONSOLE_BROKER_ENABLED      \
    (1UL << SHARED_GLOBAL_FLAGS_CONSOLE_BROKER_ENABLED_V)

#define SHARED_GLOBAL_FLAGS_SECURE_BOOT_ENABLED_V       0x7
#define SHARED_GLOBAL_FLAGS_SECURE_BOOT_ENABLED         \
    (1UL << SHARED_GLOBAL_FLAGS_SECURE_BOOT_ENABLED_V)

#define SHARED_GLOBAL_FLAGS_MULTI_SESSION_SKU_V         0x8
#define SHARED_GLOBAL_FLAGS_MULTI_SESSION_SKU           \
    (1UL << SHARED_GLOBAL_FLAGS_MULTI_SESSION_SKU_V)

#define SHARED_GLOBAL_FLAGS_MULTIUSERS_IN_SESSION_SKU_V 0x9
#define SHARED_GLOBAL_FLAGS_MULTIUSERS_IN_SESSION_SKU   \
    (1UL << SHARED_GLOBAL_FLAGS_MULTIUSERS_IN_SESSION_SKU_V)

#define SHARED_GLOBAL_FLAGS_STATE_SEPARATION_ENABLED_V 0xA
#define SHARED_GLOBAL_FLAGS_STATE_SEPARATION_ENABLED   \
    (1UL << SHARED_GLOBAL_FLAGS_STATE_SEPARATION_ENABLED_V)

//
// Define legal values for the SystemCall member.
//
#define SYSTEM_CALL_SYSCALL 0
#define SYSTEM_CALL_INT_2E  1

//
// Define flags for QPC bypass information. None of these flags may be set
// unless bypass is enabled. This is for compat with existing code which
// compares this value to zero to detect bypass enablement.
//
#define SHARED_GLOBAL_FLAGS_QPC_BYPASS_ENABLED (0x01)
#define SHARED_GLOBAL_FLAGS_QPC_BYPASS_USE_MFENCE (0x10)
#define SHARED_GLOBAL_FLAGS_QPC_BYPASS_USE_LFENCE (0x20)
#define SHARED_GLOBAL_FLAGS_QPC_BYPASS_A73_ERRATA (0x40)
#define SHARED_GLOBAL_FLAGS_QPC_BYPASS_USE_RDTSCP (0x80)


//
// Define data shared between kernel and user mode.
//
// N.B. User mode has read only access to this data.
//
// WARNING: This structure must have exactly the same layout for 32- and
//    64-bit systems. The layout of this structure cannot change and new
//    fields can only be added at the end of the structure (unless a gap
//    can be exploited). Deprecated fields cannot be deleted. Platform
//    specific fields are included on all systems.
//
//    Layout exactness is required for Wow64 support of 32-bit applications
//    on Win64 systems.
//
//    The layout itself cannot change since this structure has been exported
//    in ntddk, ntifs.h, and nthal.h for some time.
//

typedef struct _KUSER_SHARED_DATA {

    //
    // Current low 32-bit of tick count and tick count multiplier.
    //
    // N.B. The tick count is updated each time the clock ticks.
    //
    UINT32 TickCountLowDeprecated;
    UINT32 TickCountMultiplier;

    //
    // Current 64-bit interrupt time in 100ns units.
    //
    volatile struct _KSYSTEM_TIME InterruptTime;

    //
    // Current 64-bit system time in 100ns units.
    //
    volatile struct _KSYSTEM_TIME SystemTime;

    //
    // Current 64-bit time zone bias.
    //
    volatile struct _KSYSTEM_TIME TimeZoneBias;

    //
    // Support image magic number range for the host system.
    //
    // N.B. This is an inclusive range.
    //
    UINT16 ImageNumberLow;
    UINT16 ImageNumberHigh;

    //
    // Copy of system root in unicode.
    //
    // N.B. This field must be accessed via the RtlGetNtSystemRoot API for
    //      an accurate result.
    //
    WCHAR NtSystemRoot[MAX_PATH];

    //
    // Maximum stack trace depth if tracing enabled.
    //
    UINT32 MaxStackTraceDepth;

    //
    // Crypto exponent value.
    //
    UINT32 CryptoExponent;

    //
    // Time zone ID.
    //
    UINT32 TimeZoneId;
    UINT32 LargePageMinimum;

    //
    // This value controls the AIT Sampling rate.
    //
    UINT32 AitSamplingValue;

    //
    // This value controls switchback processing.
    //
    UINT32 AppCompatFlag;

    //
    // Current Kernel Root RNG state seed version
    //
    UINT64 RNGSeedVersion;

    //
    // This value controls assertion failure handling.
    //
    UINT32 GlobalValidationRunlevel;

    volatile INT32 TimeZoneBiasStamp;

    //
    // The shared collective build number undecorated with C or F.
    // GetVersionEx hides the real number
    //
    UINT32 NtBuildNumber;

    //
    // Product type.
    //
    // N.B. This field must be accessed via the RtlGetNtProductType API for
    //      an accurate result.
    //
    NT_PRODUCT_TYPE NtProductType;
    BOOLEAN ProductTypeIsValid;
    BOOLEAN Reserved0[1];
    UINT16 NativeProcessorArchitecture;

    //
    // The NT Version.
    //
    // N. B. Note that each process sees a version from its PEB, but if the
    //       process is running with an altered view of the system version,
    //       the following two fields are used to correctly identify the
    //       version
    //
    UINT32 NtMajorVersion;
    UINT32 NtMinorVersion;

    //
    // Processor features.
    //
    BOOLEAN ProcessorFeatures[PROCESSOR_FEATURE_MAX];

    //
    // Reserved fields - do not use.
    //
    UINT32 Reserved1;
    UINT32 Reserved3;

    //
    // Time slippage while in debugger.
    //
    volatile UINT32 TimeSlip;

    //
    // Alternative system architecture, e.g., NEC PC98xx on x86.
    //
    ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;

    //
    // Boot sequence, incremented for each boot attempt by the OS loader.
    //
    UINT32 BootId;

    //
    // If the system is an evaluation unit, the following field contains the
    // date and time that the evaluation unit expires. A value of 0 indicates
    // that there is no expiration. A non-zero value is the UTC absolute time
    // that the system expires.
    //
    LARGE_INTEGER SystemExpirationDate;

    //
    // Suite support.
    //
    // N.B. This field must be accessed via the RtlGetSuiteMask API for
    //      an accurate result.
    //
    UINT32 SuiteMask;

    //
    // TRUE if a kernel debugger is connected/enabled.
    //
    BOOLEAN KdDebuggerEnabled;

    //
    // Mitigation policies.
    //
    union {
        UINT8 MitigationPolicies;
        struct {
            UINT8 NXSupportPolicy : 2;
            UINT8 SEHValidationPolicy : 2;
            UINT8 CurDirDevicesSkippedForDlls : 2;
            UINT8 Reserved : 2;
        };
    };

    //
    // Two bytes of padding here -- offsets 0x2d6, 0x2d7
    //
    UINT8 Reserved6[2];

    //
    // Current console session Id. Always zero on non-TS systems.
    //
    // N.B. This field must be accessed via the RtlGetActiveConsoleId API for an
    //      accurate result.
    //
    volatile UINT32 ActiveConsoleId;

    //
    // Force-dismounts cause handles to become invalid. Rather than always
    // probe handles, a serial number of dismounts is maintained that clients
    // can use to see if they need to probe handles.
    //
    volatile UINT32 DismountCount;

    //
    // This field indicates the status of the 64-bit COM+ package on the
    // system. It indicates whether the Itermediate Language (IL) COM+
    // images need to use the 64-bit COM+ runtime or the 32-bit COM+ runtime.
    //
    UINT32 ComPlusPackage;

    //
    // Time in tick count for system-wide last user input across all terminal
    // sessions. For MP performance, it is not updated all the time (e.g. once
    // a minute per session). It is used for idle detection.
    //
    UINT32 LastSystemRITEventTickCount;

    //
    // Number of physical pages in the system. This can dynamically change as
    // physical memory can be added or removed from a running system.
    //
    UINT32 NumberOfPhysicalPages;

    //
    // True if the system was booted in safe boot mode.
    //
    BOOLEAN SafeBootMode;

    //
    // Virtualization flags
    //
    UINT8 VirtualizationFlags;

    //
    // Reserved (available for reuse).
    //
    UINT8 Reserved12[2];

    //
    // This is a packed bitfield that contains various flags concerning
    // the system state. They must be manipulated using interlocked
    // operations.
    //
    // N.B. DbgMultiSessionSku must be accessed via the RtlIsMultiSessionSku
    //      API for an accurate result
    //
    union {
        UINT32 SharedDataFlags;
        struct {
            //
            // The following bit fields are for the debugger only. Do not use.
            // Use the bit definitions instead.
            //
            UINT32 DbgErrorPortPresent : 1;
            UINT32 DbgElevationEnabled : 1;
            UINT32 DbgVirtEnabled : 1;
            UINT32 DbgInstallerDetectEnabled : 1;
            UINT32 DbgLkgEnabled : 1;
            UINT32 DbgDynProcessorEnabled : 1;
            UINT32 DbgConsoleBrokerEnabled : 1;
            UINT32 DbgSecureBootEnabled : 1;
            UINT32 DbgMultiSessionSku : 1;
            UINT32 DbgMultiUsersInSessionSku : 1;
            UINT32 DbgStateSeparationEnabled : 1;
            UINT32 SpareBits : 21;
        } DUMMYSTRUCTNAME2;
    } DUMMYUNIONNAME2;

    UINT32 DataFlagsPad[1];

    //
    // Depending on the processor, the code for fast system call will differ,
    // Stub code is provided pointers below to access the appropriate code.
    //
    // N.B. The following field is only used on 32-bit systems.
    //
    UINT64 TestRetInstruction;
    INT64 QpcFrequency;

    //
    // On AMD64, this value is initialized to a nonzero value if the system
    // operates with an altered view of the system service call mechanism.
    //
    UINT32 SystemCall;

    //
    // Reserved, available for reuse.
    //
    UINT32 SystemCallPad0;
    UINT64 SystemCallPad[2];

    //
    // The 64-bit tick count.
    //
    union {
        volatile KSYSTEM_TIME TickCount;
        volatile UINT64 TickCountQuad;
        struct {
            UINT32 ReservedTickCountOverlay[3];
            UINT32 TickCountPad[1];
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME3;

    //
    // Cookie for encoding pointers system wide.
    //
    UINT32 Cookie;
    UINT32 CookiePad[1];

    //
    // Client id of the process having the focus in the current
    // active console session id.
    //
    // N.B. This field must be accessed via the
    //      RtlGetConsoleSessionForegroundProcessId API for an accurate result.
    //
    INT64 ConsoleSessionForegroundProcessId;

    //
    // N.B. The following data is used to implement the precise time
    //      services. It is aligned on a 64-byte cache-line boundary and
    //      arranged in the order of typical accesses.
    //
    // Placeholder for the (internal) time update lock.
    //
    UINT64 TimeUpdateLock;

    //
    // The performance counter value used to establish the current system time.
    //
    UINT64 BaselineSystemTimeQpc;

    //
    // The performance counter value used to compute the last interrupt time.
    //
    UINT64 BaselineInterruptTimeQpc;

    //
    // The scaled number of system time seconds represented by a single
    // performance count (this value may vary to achieve time synchronization).
    //
    UINT64 QpcSystemTimeIncrement;

    //
    // The scaled number of interrupt time seconds represented by a single
    // performance count (this value is constant after the system is booted).
    //
    UINT64 QpcInterruptTimeIncrement;

    //
    // The scaling shift count applied to the performance counter system time
    // increment.
    //
    UINT8 QpcSystemTimeIncrementShift;

    //
    // The scaling shift count applied to the performance counter interrupt time
    // increment.
    //
    UINT8 QpcInterruptTimeIncrementShift;

    //
    // The count of unparked processors.
    //
    UINT16 UnparkedProcessorCount;

    //
    // A bitmask of enclave features supported on this system.
    //
    UINT32 EnclaveFeatureMask[4];

    //
    // Current coverage round for telemetry based coverage.
    //
    UINT32 TelemetryCoverageRound;

    //
    // The following field is used for ETW user mode global logging
    // (UMGL).
    //
    UINT16 UserModeGlobalLogger[16];

    //
    // Settings that can enable the use of Image File Execution Options
    // from HKCU in addition to the original HKLM.
    //
    UINT32 ImageFileExecutionOptions;

    //
    // Generation of the kernel structure holding system language information
    //
    UINT32 LangGenerationCount;

    //
    // Reserved (available for reuse).
    //
    UINT64 Reserved4;

    //
    // Current 64-bit interrupt time bias in 100ns units.
    //
    volatile UINT64 InterruptTimeBias;

    //
    // Current 64-bit performance counter bias, in performance counter units
    // before the shift is applied.
    //
    volatile UINT64 QpcBias;

    //
    // Number of active processors and groups.
    //
    UINT32 ActiveProcessorCount;
    volatile UINT8 ActiveGroupCount;

    //
    // Reserved (available for re-use).
    //
    UINT8 Reserved9;

    union {
        UINT16 QpcData;
        struct {

            //
            // A boolean indicating whether performance counter queries
            // can read the counter directly (bypassing the system call).
            //
            volatile UINT8 QpcBypassEnabled;

            //
            // Shift applied to the raw counter value to derive the
            // QPC count.
            //
            UINT8 QpcShift;
        };
    };

    LARGE_INTEGER TimeZoneBiasEffectiveStart;
    LARGE_INTEGER TimeZoneBiasEffectiveEnd;

    //
    // Extended processor state configuration
    //
    XSTATE_CONFIGURATION_EXTENDED XState;

} KUSER_SHARED_DATA, *PKUSER_SHARED_DATA;


//
// Mostly enforce earlier comment about the stability and
// architecture-neutrality of this struct.
//
#if !defined(__midl) && !defined(MIDL_PASS)

//
// Assembler logic assumes a zero value for syscall and a nonzero value for
// int 2e, and that no other values exist presently for the SystemCall field.
//
C_ASSERT(SYSTEM_CALL_SYSCALL == 0);
C_ASSERT(SYSTEM_CALL_INT_2E == 1);

//
// The overall size can change, but it must be the same for all architectures.
//
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TickCountLowDeprecated) == 0x0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TickCountMultiplier) == 0x4);
C_ASSERT(__alignof(KSYSTEM_TIME) == 4);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, InterruptTime) == 0x08);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemTime) == 0x014);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TimeZoneBias) == 0x020);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ImageNumberLow) == 0x02c);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ImageNumberHigh) == 0x02e);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NtSystemRoot) == 0x030);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, MaxStackTraceDepth) == 0x238);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, CryptoExponent) == 0x23c);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TimeZoneId) == 0x240);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, LargePageMinimum) == 0x244);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, AitSamplingValue) == 0x248);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, AppCompatFlag) == 0x24c);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, RNGSeedVersion) == 0x250);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, GlobalValidationRunlevel) == 0x258);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TimeZoneBiasStamp) == 0x25c);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NtBuildNumber) == 0x260);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NtProductType) == 0x264);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ProductTypeIsValid) == 0x268);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NativeProcessorArchitecture) == 0x26a);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NtMajorVersion) == 0x26c);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NtMinorVersion) == 0x270);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ProcessorFeatures) == 0x274);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, Reserved1) == 0x2b4);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, Reserved3) == 0x2b8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TimeSlip) == 0x2bc);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, AlternativeArchitecture) == 0x2c0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemExpirationDate) == 0x2c8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SuiteMask) == 0x2d0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, KdDebuggerEnabled) == 0x2d4);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, MitigationPolicies) == 0x2d5);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ActiveConsoleId) == 0x2d8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, DismountCount) == 0x2dc);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ComPlusPackage) == 0x2e0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, LastSystemRITEventTickCount) == 0x2e4);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NumberOfPhysicalPages) == 0x2e8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SafeBootMode) == 0x2ec);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, VirtualizationFlags) == 0x2ed);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, Reserved12) == 0x2ee);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TestRetInstruction) == 0x2f8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, QpcFrequency) == 0x300);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemCall) == 0x308);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemCallPad0) == 0x30c);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemCallPad) == 0x310);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, Cookie) == 0x330);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ConsoleSessionForegroundProcessId) == 0x338);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TimeUpdateLock) == 0x340);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, BaselineSystemTimeQpc) == 0x348);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, BaselineInterruptTimeQpc) == 0x350);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, QpcSystemTimeIncrement) == 0x358);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, QpcInterruptTimeIncrement) == 0x360);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, QpcSystemTimeIncrementShift) == 0x368);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, QpcInterruptTimeIncrementShift) == 0x369);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, UnparkedProcessorCount) == 0x36a);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, EnclaveFeatureMask) == 0x36c);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TelemetryCoverageRound) == 0x37c);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, UserModeGlobalLogger) == 0x380);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ImageFileExecutionOptions) == 0x3a0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, LangGenerationCount) == 0x3a4);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, Reserved4) == 0x3a8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, InterruptTimeBias) == 0x3b0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, QpcBias) == 0x3b8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ActiveProcessorCount) == 0x3c0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ActiveGroupCount) == 0x3c4);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, Reserved9) == 0x3c5);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, QpcData) == 0x3c6);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, QpcBypassEnabled) == 0x3c6);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, QpcShift) == 0x3c7);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TimeZoneBiasEffectiveStart) == 0x3c8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TimeZoneBiasEffectiveEnd) == 0x3d0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, XState) == 0x3d8);
C_ASSERT(sizeof(KUSER_SHARED_DATA) == 0x710);

#endif /* __midl | MIDL_PASS */


//
// Kernel/User shared data.
//
// The pre-set address for access from kernel mode is defined symbolically as
// KI_USER_SHARED_DATA. It helps when debugging to remember that this is
// 0xFFDF0000 or 0xFFFFF780`00000000, respectively, in 32-bit and 64-bit Windows.
// Also defined is a convenient symbol, SharedUserData, which casts this constant
// address to a KUSER_SHARED_DATA pointer.
//
// The read-only user-mode address for the shared data is 0x7FFE0000, both in
// 32-bit and 64-bit Windows. The only formal definition among headers in the
// WDK or the Software Development Kit (SDK) is in assembly language headers:
// KS386.INC from the WDK and KSAMD64.INC from the SDK both define
// MM_SHARED_USER_DATA_VA for the user-mode address (and USER_SHARED_DATA for
// the kernel-mode).
//
//

#if defined(_M_AMD64)

#define KI_USER_SHARED_DATA (0xFFFFF78000000000ULL)
#define MM_SHARED_USER_DATA_VA 0x7FFE0000

#elif defined(_M_ARM)

#define KI_USER_SHARED_DATA 0xFFFF9000
#define MM_SHARED_USER_DATA_VA 0x7FFE0000

#elif defined(_M_ARM64) || defined(_CHPE_X86_ARM64_)

#define KI_USER_SHARED_DATA (0xFFFFF78000000000ULL)
#define MM_SHARED_USER_DATA_VA 0x7FFE0000

#elif defined(_M_IX86)

#define KI_USER_SHARED_DATA 0xffdf0000
#define MM_SHARED_USER_DATA_VA 0x7FFE0000

#endif

#if defined(_WDM_)
#define SharedUserData ((struct _KUSER_SHARED_DATA * const) KI_USER_SHARED_DATA)
#else
#define SharedUserData ((struct _KUSER_SHARED_DATA * const) MM_SHARED_USER_DATA_VA)
#endif


///
/// < Exective Routines >
///

typedef
NTSTATUS
(NTAPI *PNT_DELAY_EXECUTION)(
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER DelayInterval
    );
NTSYSCALLAPI
NTSTATUS
NTAPI
NtDelayExecution(
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER DelayInterval
    );


#ifndef EVENT_QUERY_STATE
#define EVENT_QUERY_STATE 0x0001
#endif

typedef enum _EVENT_INFORMATION_CLASS {
    EventBasicInformation
} EVENT_INFORMATION_CLASS;

typedef struct _EVENT_BASIC_INFORMATION {
    EVENT_TYPE EventType;
    LONG EventState;
} EVENT_BASIC_INFORMATION, *PEVENT_BASIC_INFORMATION;

typedef
NTSTATUS
(NTAPI *PNT_CREATE_EVENT)(
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN EVENT_TYPE EventType,
    IN BOOLEAN InitialState
    );
NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateEvent(
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN EVENT_TYPE EventType,
    IN BOOLEAN InitialState
    );

typedef
NTSTATUS
(NTAPI *PNT_OPEN_EVENT)(
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenEvent(
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

typedef
NTSTATUS
(NTAPI *PNT_SET_EVENT)(
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );
NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetEvent(
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_SET_EVENT_BOOST_PRIORITY)(
    IN HANDLE EventHandle
    );
NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetEventBoostPriority(
    IN HANDLE EventHandle
    );

typedef
NTSTATUS
(NTAPI *PNT_CLEAR_EVENT)(
    IN HANDLE EventHandle
    );
NTSYSCALLAPI
NTSTATUS
NTAPI
NtClearEvent(
    IN HANDLE EventHandle
    );

typedef
NTSTATUS
(NTAPI *PNT_RESET_EVENT)(
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );
NTSYSCALLAPI
NTSTATUS
NTAPI
NtResetEvent(
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_PULSE_EVENT)(
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );
NTSYSCALLAPI
NTSTATUS
NTAPI
NtPulseEvent(
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_QUERY_EVENT)(
    IN HANDLE EventHandle,
    IN EVENT_INFORMATION_CLASS EventInformationClass,
    OUT PVOID EventInformation,
    IN ULONG EventInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryEvent(
    IN HANDLE EventHandle,
    IN EVENT_INFORMATION_CLASS EventInformationClass,
    OUT PVOID EventInformation,
    IN ULONG EventInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus