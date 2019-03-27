#pragma once

#include "types.h"

#include <string>
#include <map>

namespace unreal {

/** 
 * Unreal object size map of Objname -> Size.
 */
typedef std::map<std::wstring, size_t> ObjectSizeMapType;
extern ObjectSizeMapType ObjectSizeMap;
int InitializeUnrealObjectSizeMap();
const size_t GetObjectSize(const std::wstring& ObjectName);


/**
 * UObjects offsets and inline decryption globals.
 */
extern size_t ObjectsEncryptedOffset; /* = -1; */
extern int8_t ObjectsEncryptedRegister; /* = -1; */
extern uint8_t* ObjectsDecryptionBegin; /* = nullptr; */
extern uint8_t* ObjectsDecryptionEnd; /* = nullptr; */

extern size_t ObjectFlagsEncryptedOffset; /* = -1; */
extern int8_t ObjectFlagsEncryptedRegister; /* = -1; */
extern uint8_t* ObjectFlagsDecryptionBegin; /* = nullptr; */
extern uint8_t* ObjectFlagsDecryptionEnd; /* = nullptr; */

extern size_t ObjectOuterEncryptedOffset; /* = -1; */
extern int8_t ObjectOuterEncryptedRegister; /* = -1; */
extern uint8_t* ObjectOuterDecryptionBegin; /* = nullptr; */
extern uint8_t* ObjectOuterDecryptionEnd; /* = nullptr; */

extern size_t ObjectInternalIndexEncryptedOffset; /* = -1; */
extern int8_t ObjectInternalIndexEncryptedRegister; /* = -1; */
extern uint8_t* ObjectInternalIndexDecryptionBegin; /* = nullptr; */
extern uint8_t* ObjectInternalIndexDecryptionEnd; /* = nullptr; */

extern size_t ObjectClassEncryptedOffset; /* = -1; */
extern int8_t ObjectClassEncryptedRegister; /* = -1; */
extern uint8_t* ObjectClassDecryptionBegin; /* = nullptr; */
extern uint8_t* ObjectClassDecryptionEnd; /* = nullptr; */

extern size_t ObjectNameIndexEncryptedOffset; /* = -1; */
extern int8_t ObjectNameIndexEncryptedRegister; /* = -1; */
extern size_t ObjectNameNumberEncryptedOffset; /* = -1; */
extern int8_t ObjectNameNumberEncryptedRegister; /* = -1; */
extern uint8_t* ObjectNameDecryptionBegin; /* = nullptr; */
extern uint8_t* ObjectNameDecryptionEnd; /* = nullptr; */

extern size_t StructPropertiesSizeOffset; /* = -1; */
extern size_t StructMinAlignmentOffset; /* = -1; */
extern size_t StructChildrenOffset; /* = -1; */
extern size_t StructSuperStructOffset; /* = -1; */

extern size_t FunctionFlagsOffset; /* = -1; */

extern size_t PropertyElementSizeOffset; /* = -1; */
extern size_t PropertyArrayDimOffset; /* = -1; */
extern size_t PropertyOffsetInternalOffset; /* = -1; */
extern size_t PropertyFlagsOffset; /* = -1; */

extern size_t EnumNamesOffset; /* = -1; */
extern size_t EnumCppFormOffset; /* = -1; */

int DumpObjects();


/**
 * Map entry of structure field map.
 */
struct FieldMapEntry {
    FieldMapEntry() : Size(0) { memset(Name, 0, sizeof(Name) + sizeof(TypeName)); }

    FieldMapEntry(const char* InName, const char *InTypeName, size_t InSize) : Size(InSize)
    {
        strncpy_s(Name, InName, sizeof(Name));
        strncpy_s(TypeName, InTypeName, sizeof(TypeName));
    }

    FieldMapEntry& operator=(const FieldMapEntry& Other)
    {
        strncpy_s(Name, Other.Name, sizeof(Name));
        strncpy_s(TypeName, Other.TypeName, sizeof(TypeName));
        Size = Other.Size;
        return *this;
    }

    char Name[256];
    char TypeName[256];
    size_t Size;
};
#define FIELD_MAP_ENTRY(Name, Type) \
    FieldMapEntry(_XOR_(Name), _XOR_(#Type), sizeof(Type))

/**
 * Map of structure fields of Offset -> FieldMapEntry
 */
typedef std::map<size_t, FieldMapEntry> FieldMapType;

int DumpStructs();


/**
 * FNameEntry/Names offsets and inline decryption globals.
 */
extern int8_t NamesHashEntryRegister; /* = -1; */
extern size_t NamesEntryHashNextOffset; /* = -1; */
extern size_t NamesEntryIndexEncryptedOffset; /* = -1; */
extern int8_t NamesEntryIndexEncryptedRegister; /* = -1; */
extern uint8_t* NamesEntryIndexDecryptionBegin; /* = nullptr; */
extern uint8_t* NamesEntryIndexDecryptionEnd; /* = nullptr; */

extern uint8_t* NamesGetEncryptedNamesRoutine; /* = nullptr; */
extern int8_t NamesEncryptedRegister; /* = -1; */
extern uint8_t* NamesDecryptionBegin; /* = nullptr; */
extern uint8_t* NamesDecryptionEnd; /* = nullptr; */

extern uint8_t* NamesEntryArrayAddZeroedRoutine; /* = nullptr; */
extern size_t NamesEntryArrayElementsPerChunk; /* = -1; */
extern size_t NamesEntryArrayChunksEncryptedOffset; /* = -1; */
extern int8_t NamesEntryArrayChunksEncryptedRegister; /* = -1; */
extern uint8_t* NamesEntryArrayChunksDecryptionBegin; /* = nullptr; */
extern uint8_t* NamesEntryArrayChunksDecryptionEnd; /* = nullptr; */

extern size_t NamesEntryArrayNumElementsEncryptedOffset; /* = -1; */
extern int8_t NamesEntryArrayNumElementsEncryptedRegister; /* = -1; */
extern uint8_t* NamesEntryArrayNumElementsDecryptionBegin; /* = nullptr; */
extern uint8_t* NamesEntryArrayNumElementsDecryptionEnd; /* = nullptr; */

int DumpNames();


/**
 * UWorld offsets and inline decryption globals.
 */
extern size_t WorldEncryptedOffset; /* = -1; */
extern int8_t WorldEncryptedRegister; /* = -1; */
extern size_t WorldEncryptedStackOffset; /* = -1; */
extern int8_t WorldEncryptedStackRegister; /* = -1; */
extern uint8_t* WorldDecryptionBegin; /* = nullptr; */
extern uint8_t* WorldDecryptionEnd; /* = nullptr; */

int DumpWorld();

}
