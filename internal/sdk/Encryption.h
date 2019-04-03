#pragma once

#include <intrin.h>

union CryptValue {

/* Byte Values */

    unsigned char Byte;
    struct {
        unsigned char Byte1;
        unsigned char Byte2;
        unsigned char Byte3;
        unsigned char Byte4;
        unsigned char Byte5;
        unsigned char Byte6;
        unsigned char Byte7;
        unsigned char Byte8;
    };

/* Word Values */

    unsigned short Word;
    struct {
        unsigned short Word1;
        unsigned short Word2;
        unsigned short Word3;
        unsigned short Word4;
    };

/* Double Word Values */

    unsigned long Dword;
    struct {
        unsigned long LoDword;
        unsigned long HiDword;
    };
    struct {
        unsigned long Dword1;
        unsigned long Dword2;
    };

/* Quad Word Value */

    unsigned __int64 Qword;

/* Pointer Value */

    void *Pointer;
};

#ifndef __ROL__
#define __ROL__(value,count)    _rotl((unsigned int)(value),count)
#endif
#ifndef __ROR__
#define __ROR__(value,count)    _rotr((unsigned int)(value),count)
#endif
#ifndef __ROL1__
#define __ROL1__(value,count)   _rotl8((unsigned char)(value),count)
#endif
#ifndef __ROR1__
#define __ROR1__(value,count)   _rotr8((unsigned char)(value),count)
#endif
#ifndef __ROL2__
#define __ROL2__(value,count)   _rotl16((unsigned short)(value),count)
#endif
#ifndef __ROR2__
#define __ROR2__(value,count)   _rotr16((unsigned short)(value),count)
#endif
#ifndef __ROL4__
#define __ROL4__(value,count)   __ROL__(value,count)
#endif
#ifndef __ROR4__
#define __ROR4__(value,count)   __ROR__(value,count)
#endif
#ifndef __ROL8__
#define __ROL8__(value,count)   _rotl64((unsigned __int64)(value),count)
#endif
#ifndef __ROR8__
#define __ROR8__(value,count)   _rotr64((unsigned __int64)(value),count)
#endif

#ifndef __PAIR64__
#define __PAIR64__(hi, low) ((unsigned __int64)(((unsigned __int64)(hi) << 32) | (unsigned __int32)(low)))
#endif 

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Names asm decryption routines.
 */
unsigned __int64 DecryptNamesAsm(unsigned __int64 NamesEncrypted);
unsigned __int64 DecryptChunksAsm(unsigned __int64 ChunksEncrypted);
unsigned __int64 DecryptNumElementsAsm(unsigned __int64 NumElementsEncrypted);
unsigned __int64 DecryptNameEntryIndexAsm(unsigned __int64 IndexEncrypted);

/**
 * Objects asm decryption routines.
 */
unsigned __int64 DecryptObjectsAsm(unsigned __int64 ObjectsEncrypted);
unsigned int DecryptObjectFlagsAsm(unsigned int ObjectFlagsEncrypted);
unsigned int DecryptObjectIndexAsm(unsigned int InternalIndexEncrypted);
unsigned __int64 DecryptObjectClassAsm(unsigned __int64 ClassEncrypted);
unsigned __int64 DecryptObjectOuterAsm(unsigned __int64 OuterEncrypted);
void DecryptObjectFNameAsm(const int InNameIndexEncrypted,
                           const int InNameNumberEncrypted,
                           int* OutNameIndex,
                           int* OutNameNumber);

/**
 * World asm decryption routines.
 */
unsigned __int64 DecryptPersistentLevelAsm(unsigned __int64 PersistentLevelEncrypted);
unsigned __int64 DecryptActorsAsm(unsigned __int64 ActorsEncrypted);

#ifdef __cplusplus
}
#endif