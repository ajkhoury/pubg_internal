/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file ntapi.h
 * @author Aidan Khoury (ajkhoury)
 * @date 8/30/2018
 */

#ifndef _BLACKOUT_NTTRUST_H_
#define _BLACKOUT_NTTRUST_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include <ntdef.h>

typedef struct _WIN_CERTIFICATE {
    ULONG       dwLength;
    SHORT       wRevision;
    SHORT       wCertificateType;   // WIN_CERT_TYPE_xxx
    UCHAR       bCertificate[ANYSIZE_ARRAY];
} WIN_CERTIFICATE, *LPWIN_CERTIFICATE;

#define WIN_CERT_REVISION_1_0               (0x0100)
#define WIN_CERT_REVISION_2_0               (0x0200)

#define WIN_CERT_TYPE_X509                  (0x0001)   // bCertificate contains an X.509 Certificate
#define WIN_CERT_TYPE_PKCS_SIGNED_DATA      (0x0002)   // bCertificate contains a PKCS SignedData structure
#define WIN_CERT_TYPE_RESERVED_1            (0x0003)   // Reserved
#define WIN_CERT_TYPE_TS_STACK_SIGNED       (0x0004)   // Terminal Server Protocol Stack Certificate signing

#endif // _BLACKOUT_NTTRUST_H_