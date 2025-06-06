/** @file
  X.509 Certificate Handler Wrapper Implementation over OpenSSL.

Copyright (c) 2010 - 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <crypto/asn1.h>
#include <openssl/asn1.h>
#include <openssl/rsa.h>

/* OID*/
#define OID_EXT_KEY_USAGE      { 0x55, 0x1D, 0x25 }
#define OID_BASIC_CONSTRAINTS  { 0x55, 0x1D, 0x13 }

static CONST UINT8  mOidExtKeyUsage[]      = OID_EXT_KEY_USAGE;
static CONST UINT8  mOidBasicConstraints[] = OID_BASIC_CONSTRAINTS;

#define CRYPTO_ASN1_TAG_CLASS_MASK  0xC0
#define CRYPTO_ASN1_TAG_PC_MASK     0x20
#define CRYPTO_ASN1_TAG_VALUE_MASK  0x1F

/**
  Construct a X509 object from DER-encoded certificate data.

  If Cert is NULL, then return FALSE.
  If SingleX509Cert is NULL, then return FALSE.

  @param[in]  Cert            Pointer to the DER-encoded certificate data.
  @param[in]  CertSize        The size of certificate data in bytes.
  @param[out] SingleX509Cert  The generated X509 object.

  @retval     TRUE            The X509 object generation succeeded.
  @retval     FALSE           The operation failed.

**/
BOOLEAN
EFIAPI
X509ConstructCertificate (
  IN   CONST UINT8  *Cert,
  IN   UINTN        CertSize,
  OUT  UINT8        **SingleX509Cert
  )
{
  X509         *X509Cert;
  CONST UINT8  *Temp;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (SingleX509Cert == NULL) || (CertSize > INT_MAX)) {
    return FALSE;
  }

  //
  // Read DER-encoded X509 Certificate and Construct X509 object.
  //
  Temp     = Cert;
  X509Cert = d2i_X509 (NULL, &Temp, (long)CertSize);
  if (X509Cert == NULL) {
    return FALSE;
  }

  *SingleX509Cert = (UINT8 *)X509Cert;

  return TRUE;
}

/**
  Construct a X509 stack object from a list of DER-encoded certificate data.

  If X509Stack is NULL, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in, out]  X509Stack  On input, pointer to an existing or NULL X509 stack object.
                              On output, pointer to the X509 stack object with new
                              inserted X509 certificate.
  @param[in]       Args       VA_LIST marker for the variable argument list.
                              A list of DER-encoded single certificate data followed
                              by certificate size. A NULL terminates the list. The
                              pairs are the arguments to X509ConstructCertificate().

  @retval     TRUE            The X509 stack construction succeeded.
  @retval     FALSE           The construction operation failed.
  @retval     FALSE           This interface is not supported.

**/
BOOLEAN
EFIAPI
X509ConstructCertificateStackV (
  IN OUT  UINT8    **X509Stack,
  IN      VA_LIST  Args
  )
{
  UINT8  *Cert;
  UINTN  CertSize;
  X509   *X509Cert;

  STACK_OF (X509)  *CertStack;
  BOOLEAN  Status;

  //
  // Check input parameters.
  //
  if (X509Stack == NULL) {
    return FALSE;
  }

  Status = FALSE;

  //
  // Initialize X509 stack object.
  //
  CertStack = (STACK_OF (X509) *)(*X509Stack);
  if (CertStack == NULL) {
    CertStack = sk_X509_new_null ();
    if (CertStack == NULL) {
      return Status;
    }
  }

  while (TRUE) {
    //
    // If Cert is NULL, then it is the end of the list.
    //
    Cert = VA_ARG (Args, UINT8 *);
    if (Cert == NULL) {
      break;
    }

    CertSize = VA_ARG (Args, UINTN);
    if (CertSize == 0) {
      break;
    }

    //
    // Construct X509 Object from the given DER-encoded certificate data.
    //
    X509Cert = NULL;
    Status   = X509ConstructCertificate (
                 (CONST UINT8 *)Cert,
                 CertSize,
                 (UINT8 **)&X509Cert
                 );
    if (!Status) {
      if (X509Cert != NULL) {
        X509_free (X509Cert);
      }

      break;
    }

    //
    // Insert the new X509 object into X509 stack object.
    //
    sk_X509_push (CertStack, X509Cert);
  }

  if (!Status) {
    sk_X509_pop_free (CertStack, X509_free);
  } else {
    *X509Stack = (UINT8 *)CertStack;
  }

  return Status;
}

/**
  Construct a X509 stack object from a list of DER-encoded certificate data.

  If X509Stack is NULL, then return FALSE.

  @param[in, out]  X509Stack  On input, pointer to an existing or NULL X509 stack object.
                              On output, pointer to the X509 stack object with new
                              inserted X509 certificate.
  @param           ...        A list of DER-encoded single certificate data followed
                              by certificate size. A NULL terminates the list. The
                              pairs are the arguments to X509ConstructCertificate().

  @retval     TRUE            The X509 stack construction succeeded.
  @retval     FALSE           The construction operation failed.

**/
BOOLEAN
EFIAPI
X509ConstructCertificateStack (
  IN OUT  UINT8  **X509Stack,
  ...
  )
{
  VA_LIST  Args;
  BOOLEAN  Result;

  VA_START (Args, X509Stack);
  Result = X509ConstructCertificateStackV (X509Stack, Args);
  VA_END (Args);
  return Result;
}

/**
  Release the specified X509 object.

  If X509Cert is NULL, then return FALSE.

  @param[in]  X509Cert  Pointer to the X509 object to be released.

**/
VOID
EFIAPI
X509Free (
  IN  VOID  *X509Cert
  )
{
  //
  // Check input parameters.
  //
  if (X509Cert == NULL) {
    return;
  }

  //
  // Free OpenSSL X509 object.
  //
  X509_free ((X509 *)X509Cert);
}

/**
  Release the specified X509 stack object.

  If X509Stack is NULL, then return FALSE.

  @param[in]  X509Stack  Pointer to the X509 stack object to be released.

**/
VOID
EFIAPI
X509StackFree (
  IN  VOID  *X509Stack
  )
{
  //
  // Check input parameters.
  //
  if (X509Stack == NULL) {
    return;
  }

  //
  // Free OpenSSL X509 stack object.
  //
  sk_X509_pop_free ((STACK_OF (X509) *) X509Stack, X509_free);
}

/**
  Retrieve the subject bytes from one X.509 certificate.

  @param[in]      Cert         Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize     Size of the X509 certificate in bytes.
  @param[out]     CertSubject  Pointer to the retrieved certificate subject bytes.
  @param[in, out] SubjectSize  The size in bytes of the CertSubject buffer on input,
                               and the size of buffer returned CertSubject on output.

  If Cert is NULL, then return FALSE.
  If SubjectSize is NULL, then return FALSE.

  @retval  TRUE   The certificate subject retrieved successfully.
  @retval  FALSE  Invalid certificate, or the SubjectSize is too small for the result.
                  The SubjectSize will be updated with the required size.

**/
BOOLEAN
EFIAPI
X509GetSubjectName (
  IN      CONST UINT8  *Cert,
  IN      UINTN        CertSize,
  OUT     UINT8        *CertSubject,
  IN OUT  UINTN        *SubjectSize
  )
{
  BOOLEAN    Status;
  X509       *X509Cert;
  X509_NAME  *X509Name;
  UINTN      X509NameSize;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (SubjectSize == NULL)) {
    return FALSE;
  }

  X509Cert = NULL;

  //
  // Read DER-encoded X509 Certificate and Construct X509 object.
  //
  Status = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    Status = FALSE;
    goto _Exit;
  }

  Status = FALSE;

  //
  // Retrieve subject name from certificate object.
  //
  X509Name = X509_get_subject_name (X509Cert);
  if (X509Name == NULL) {
    goto _Exit;
  }

  X509NameSize = i2d_X509_NAME (X509Name, NULL);
  if (*SubjectSize < X509NameSize) {
    *SubjectSize = X509NameSize;
    goto _Exit;
  }

  *SubjectSize = X509NameSize;
  if (CertSubject != NULL) {
    i2d_X509_NAME (X509Name, &CertSubject);
    Status = TRUE;
  }

_Exit:
  //
  // Release Resources.
  //
  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  return Status;
}

/**
  Retrieve a string from one X.509 certificate base on the Request_NID.

  @param[in]      Cert             Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize         Size of the X509 certificate in bytes.
  @param[in]      Request_NID      NID of string to obtain
  @param[out]     CommonName       Buffer to contain the retrieved certificate common
                                   name string (UTF8). At most CommonNameSize bytes will be
                                   written and the string will be null terminated. May be
                                   NULL in order to determine the size buffer needed.
  @param[in,out]  CommonNameSize   The size in bytes of the CommonName buffer on input,
                                   and the size of buffer returned CommonName on output.
                                   If CommonName is NULL then the amount of space needed
                                   in buffer (including the final null) is returned.

  @retval RETURN_SUCCESS           The certificate CommonName retrieved successfully.
  @retval RETURN_INVALID_PARAMETER If Cert is NULL.
                                   If CommonNameSize is NULL.
                                   If CommonName is not NULL and *CommonNameSize is 0.
                                   If Certificate is invalid.
  @retval RETURN_NOT_FOUND         If no NID Name entry exists.
  @retval RETURN_BUFFER_TOO_SMALL  If the CommonName is NULL. The required buffer size
                                   (including the final null) is returned in the
                                   CommonNameSize parameter.
  @retval RETURN_UNSUPPORTED       The operation is not supported.

**/
STATIC
RETURN_STATUS
InternalX509GetNIDName (
  IN      CONST UINT8  *Cert,
  IN      UINTN        CertSize,
  IN      INT32        Request_NID,
  OUT     CHAR8        *CommonName   OPTIONAL,
  IN OUT  UINTN        *CommonNameSize
  )
{
  RETURN_STATUS    ReturnStatus;
  BOOLEAN          Status;
  X509             *X509Cert;
  X509_NAME        *X509Name;
  INT32            Index;
  INTN             Length;
  X509_NAME_ENTRY  *Entry;
  ASN1_STRING      *EntryData;
  UINT8            *UTF8Name;

  ReturnStatus = RETURN_INVALID_PARAMETER;
  UTF8Name     = NULL;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (CertSize > INT_MAX) || (CommonNameSize == NULL)) {
    return ReturnStatus;
  }

  if ((CommonName != NULL) && (*CommonNameSize == 0)) {
    return ReturnStatus;
  }

  X509Cert = NULL;
  //
  // Read DER-encoded X509 Certificate and Construct X509 object.
  //
  Status = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    //
    // Invalid X.509 Certificate
    //
    goto _Exit;
  }

  Status = FALSE;

  //
  // Retrieve subject name from certificate object.
  //
  X509Name = X509_get_subject_name (X509Cert);
  if (X509Name == NULL) {
    //
    // Fail to retrieve subject name content
    //
    goto _Exit;
  }

  //
  // Retrive the string from X.509 Subject base on the Request_NID
  //
  Index = X509_NAME_get_index_by_NID (X509Name, Request_NID, -1);
  if (Index < 0) {
    //
    // No Request_NID name entry exists in X509_NAME object
    //
    *CommonNameSize = 0;
    ReturnStatus    = RETURN_NOT_FOUND;
    goto _Exit;
  }

  Entry = X509_NAME_get_entry (X509Name, Index);
  if (Entry == NULL) {
    //
    // Fail to retrieve name entry data
    //
    *CommonNameSize = 0;
    ReturnStatus    = RETURN_NOT_FOUND;
    goto _Exit;
  }

  EntryData = X509_NAME_ENTRY_get_data (Entry);
  if (EntryData == NULL) {
    //
    // Fail to retrieve name entry data
    //
    *CommonNameSize = 0;
    ReturnStatus    = RETURN_NOT_FOUND;
    goto _Exit;
  }

  Length = ASN1_STRING_to_UTF8 (&UTF8Name, EntryData);
  if (Length < 0) {
    //
    // Fail to convert the Name string
    //
    *CommonNameSize = 0;
    ReturnStatus    = RETURN_INVALID_PARAMETER;
    goto _Exit;
  }

  if (CommonName == NULL) {
    *CommonNameSize = Length + 1;
    ReturnStatus    = RETURN_BUFFER_TOO_SMALL;
  } else {
    *CommonNameSize = MIN ((UINTN)Length, *CommonNameSize - 1) + 1;
    CopyMem (CommonName, UTF8Name, *CommonNameSize - 1);
    CommonName[*CommonNameSize - 1] = '\0';
    ReturnStatus                    = RETURN_SUCCESS;
  }

_Exit:
  //
  // Release Resources.
  //
  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  if (UTF8Name != NULL) {
    OPENSSL_free (UTF8Name);
  }

  return ReturnStatus;
}

/**
  Retrieve the common name (CN) string from one X.509 certificate.

  @param[in]      Cert             Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize         Size of the X509 certificate in bytes.
  @param[out]     CommonName       Buffer to contain the retrieved certificate common
                                   name string. At most CommonNameSize bytes will be
                                   written and the string will be null terminated. May be
                                   NULL in order to determine the size buffer needed.
  @param[in,out]  CommonNameSize   The size in bytes of the CommonName buffer on input,
                                   and the size of buffer returned CommonName on output.
                                   If CommonName is NULL then the amount of space needed
                                   in buffer (including the final null) is returned.

  @retval RETURN_SUCCESS           The certificate CommonName retrieved successfully.
  @retval RETURN_INVALID_PARAMETER If Cert is NULL.
                                   If CommonNameSize is NULL.
                                   If CommonName is not NULL and *CommonNameSize is 0.
                                   If Certificate is invalid.
  @retval RETURN_NOT_FOUND         If no CommonName entry exists.
  @retval RETURN_BUFFER_TOO_SMALL  If the CommonName is NULL. The required buffer size
                                   (including the final null) is returned in the
                                   CommonNameSize parameter.
  @retval RETURN_UNSUPPORTED       The operation is not supported.

**/
RETURN_STATUS
EFIAPI
X509GetCommonName (
  IN      CONST UINT8  *Cert,
  IN      UINTN        CertSize,
  OUT     CHAR8        *CommonName   OPTIONAL,
  IN OUT  UINTN        *CommonNameSize
  )
{
  return InternalX509GetNIDName (Cert, CertSize, NID_commonName, CommonName, CommonNameSize);
}

/**
  Retrieve the organization name (O) string from one X.509 certificate.

  @param[in]      Cert             Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize         Size of the X509 certificate in bytes.
  @param[out]     NameBuffer       Buffer to contain the retrieved certificate organization
                                   name string. At most NameBufferSize bytes will be
                                   written and the string will be null terminated. May be
                                   NULL in order to determine the size buffer needed.
  @param[in,out]  NameBufferSize   The size in bytes of the Name buffer on input,
                                   and the size of buffer returned Name on output.
                                   If NameBuffer is NULL then the amount of space needed
                                   in buffer (including the final null) is returned.

  @retval RETURN_SUCCESS           The certificate Organization Name retrieved successfully.
  @retval RETURN_INVALID_PARAMETER If Cert is NULL.
                                   If NameBufferSize is NULL.
                                   If NameBuffer is not NULL and *CommonNameSize is 0.
                                   If Certificate is invalid.
  @retval RETURN_NOT_FOUND         If no Organization Name entry exists.
  @retval RETURN_BUFFER_TOO_SMALL  If the NameBuffer is NULL. The required buffer size
                                   (including the final null) is returned in the
                                   CommonNameSize parameter.
  @retval RETURN_UNSUPPORTED       The operation is not supported.

**/
RETURN_STATUS
EFIAPI
X509GetOrganizationName (
  IN      CONST UINT8  *Cert,
  IN      UINTN        CertSize,
  OUT     CHAR8        *NameBuffer   OPTIONAL,
  IN OUT  UINTN        *NameBufferSize
  )
{
  return InternalX509GetNIDName (Cert, CertSize, NID_organizationName, NameBuffer, NameBufferSize);
}

/**
  Retrieve the RSA Public Key from one DER-encoded X509 certificate.

  @param[in]  Cert         Pointer to the DER-encoded X509 certificate.
  @param[in]  CertSize     Size of the X509 certificate in bytes.
  @param[out] RsaContext   Pointer to new-generated RSA context which contain the retrieved
                           RSA public key component. Use RsaFree() function to free the
                           resource.

  If Cert is NULL, then return FALSE.
  If RsaContext is NULL, then return FALSE.

  @retval  TRUE   RSA Public Key was retrieved successfully.
  @retval  FALSE  Fail to retrieve RSA public key from X509 certificate.

**/
BOOLEAN
EFIAPI
RsaGetPublicKeyFromX509 (
  IN   CONST UINT8  *Cert,
  IN   UINTN        CertSize,
  OUT  VOID         **RsaContext
  )
{
  BOOLEAN   Status;
  EVP_PKEY  *Pkey;
  X509      *X509Cert;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (RsaContext == NULL)) {
    return FALSE;
  }

  Pkey     = NULL;
  X509Cert = NULL;

  //
  // Read DER-encoded X509 Certificate and Construct X509 object.
  //
  Status = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    Status = FALSE;
    goto _Exit;
  }

  Status = FALSE;

  //
  // Retrieve and check EVP_PKEY data from X509 Certificate.
  //
  Pkey = X509_get_pubkey (X509Cert);
  if ((Pkey == NULL) || (EVP_PKEY_id (Pkey) != EVP_PKEY_RSA)) {
    goto _Exit;
  }

  //
  // Duplicate RSA Context from the retrieved EVP_PKEY.
  //
  if ((*RsaContext = RSAPublicKey_dup (EVP_PKEY_get0_RSA (Pkey))) != NULL) {
    Status = TRUE;
  }

_Exit:
  //
  // Release Resources.
  //
  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  if (Pkey != NULL) {
    EVP_PKEY_free (Pkey);
  }

  return Status;
}

/**
  Verify one X509 certificate was issued by the trusted CA.

  @param[in]      Cert         Pointer to the DER-encoded X509 certificate to be verified.
  @param[in]      CertSize     Size of the X509 certificate in bytes.
  @param[in]      CACert       Pointer to the DER-encoded trusted CA certificate.
  @param[in]      CACertSize   Size of the CA Certificate in bytes.

  If Cert is NULL, then return FALSE.
  If CACert is NULL, then return FALSE.

  @retval  TRUE   The certificate was issued by the trusted CA.
  @retval  FALSE  Invalid certificate or the certificate was not issued by the given
                  trusted CA.

**/
BOOLEAN
EFIAPI
X509VerifyCert (
  IN  CONST UINT8  *Cert,
  IN  UINTN        CertSize,
  IN  CONST UINT8  *CACert,
  IN  UINTN        CACertSize
  )
{
  BOOLEAN         Status;
  X509            *X509Cert;
  X509            *X509CACert;
  X509_STORE      *CertStore;
  X509_STORE_CTX  *CertCtx;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (CACert == NULL)) {
    return FALSE;
  }

  Status     = FALSE;
  X509Cert   = NULL;
  X509CACert = NULL;
  CertStore  = NULL;
  CertCtx    = NULL;

  //
  // Register & Initialize necessary digest algorithms for certificate verification.
  //
  if (EVP_add_digest (EVP_md5 ()) == 0) {
    goto _Exit;
  }

  if (EVP_add_digest (EVP_sha1 ()) == 0) {
    goto _Exit;
  }

  if (EVP_add_digest (EVP_sha256 ()) == 0) {
    goto _Exit;
  }

  //
  // Read DER-encoded certificate to be verified and Construct X509 object.
  //
  Status = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    Status = FALSE;
    goto _Exit;
  }

  //
  // Read DER-encoded root certificate and Construct X509 object.
  //
  Status = X509ConstructCertificate (CACert, CACertSize, (UINT8 **)&X509CACert);
  if ((X509CACert == NULL) || (!Status)) {
    Status = FALSE;
    goto _Exit;
  }

  Status = FALSE;

  //
  // Set up X509 Store for trusted certificate.
  //
  CertStore = X509_STORE_new ();
  if (CertStore == NULL) {
    goto _Exit;
  }

  if (!(X509_STORE_add_cert (CertStore, X509CACert))) {
    goto _Exit;
  }

  //
  // Allow partial certificate chains, terminated by a non-self-signed but
  // still trusted intermediate certificate. Also disable time checks.
  //
  X509_STORE_set_flags (
    CertStore,
    X509_V_FLAG_PARTIAL_CHAIN | X509_V_FLAG_NO_CHECK_TIME
    );

  //
  // Set up X509_STORE_CTX for the subsequent verification operation.
  //
  CertCtx = X509_STORE_CTX_new ();
  if (CertCtx == NULL) {
    goto _Exit;
  }

  if (!X509_STORE_CTX_init (CertCtx, CertStore, X509Cert, NULL)) {
    goto _Exit;
  }

  //
  // X509 Certificate Verification.
  //
  Status = (BOOLEAN)X509_verify_cert (CertCtx);
  X509_STORE_CTX_cleanup (CertCtx);

_Exit:
  //
  // Release Resources.
  //
  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  if (X509CACert != NULL) {
    X509_free (X509CACert);
  }

  if (CertStore != NULL) {
    X509_STORE_free (CertStore);
  }

  X509_STORE_CTX_free (CertCtx);

  return Status;
}

/**
  Retrieve the TBSCertificate from one given X.509 certificate.

  @param[in]      Cert         Pointer to the given DER-encoded X509 certificate.
  @param[in]      CertSize     Size of the X509 certificate in bytes.
  @param[out]     TBSCert      DER-Encoded To-Be-Signed certificate.
  @param[out]     TBSCertSize  Size of the TBS certificate in bytes.

  If Cert is NULL, then return FALSE.
  If TBSCert is NULL, then return FALSE.
  If TBSCertSize is NULL, then return FALSE.

  @retval  TRUE   The TBSCertificate was retrieved successfully.
  @retval  FALSE  Invalid X.509 certificate.

**/
BOOLEAN
EFIAPI
X509GetTBSCert (
  IN  CONST UINT8  *Cert,
  IN  UINTN        CertSize,
  OUT UINT8        **TBSCert,
  OUT UINTN        *TBSCertSize
  )
{
  CONST UINT8  *Temp;
  UINT32       Asn1Tag;
  UINT32       ObjClass;
  UINTN        Length;
  UINTN        Inf;

  Asn1Tag = (UINT32)V_ASN1_UNDEF;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (TBSCert == NULL) ||
      (TBSCertSize == NULL) || (CertSize > INT_MAX))
  {
    return FALSE;
  }

  //
  // An X.509 Certificate is: (defined in RFC3280)
  //   Certificate  ::=  SEQUENCE  {
  //     tbsCertificate       TBSCertificate,
  //     signatureAlgorithm   AlgorithmIdentifier,
  //     signature            BIT STRING }
  //
  // and
  //
  //  TBSCertificate  ::=  SEQUENCE  {
  //    version         [0]  Version DEFAULT v1,
  //    ...
  //    }
  //
  // So we can just ASN1-parse the x.509 DER-encoded data. If we strip
  // the first SEQUENCE, the second SEQUENCE is the TBSCertificate.
  //
  Temp   = Cert;
  Length = 0;
  Inf    = ASN1_get_object (&Temp, (long *)&Length, (int *)&Asn1Tag, (int *)&ObjClass, (long)CertSize);

  if (((Inf & 0x80) == 0x80) || (Asn1Tag != V_ASN1_SEQUENCE)) {
    return FALSE;
  }

  *TBSCert = (UINT8 *)Temp;

  Inf = ASN1_get_object (&Temp, (long *)&Length, (int *)&Asn1Tag, (int *)&ObjClass, (long)Length);
  //
  // Verify the parsed TBSCertificate is one correct SEQUENCE data.
  //
  if (((Inf & 0x80) == 0x80) || (Asn1Tag != V_ASN1_SEQUENCE)) {
    return FALSE;
  }

  *TBSCertSize = Length + (Temp - *TBSCert);

  return TRUE;
}

/**
  Retrieve the EC Public Key from one DER-encoded X509 certificate.

  @param[in]  Cert         Pointer to the DER-encoded X509 certificate.
  @param[in]  CertSize     Size of the X509 certificate in bytes.
  @param[out] EcContext    Pointer to new-generated EC DSA context which contain the retrieved
                           EC public key component. Use EcFree() function to free the
                           resource.

  If Cert is NULL, then return FALSE.
  If EcContext is NULL, then return FALSE.

  @retval  TRUE   EC Public Key was retrieved successfully.
  @retval  FALSE  Fail to retrieve EC public key from X509 certificate.

**/
BOOLEAN
EFIAPI
EcGetPublicKeyFromX509 (
  IN   CONST UINT8  *Cert,
  IN   UINTN        CertSize,
  OUT  VOID         **EcContext
  )
{
  BOOLEAN   Status;
  EVP_PKEY  *Pkey;
  X509      *X509Cert;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (EcContext == NULL)) {
    return FALSE;
  }

  Pkey     = NULL;
  X509Cert = NULL;

  //
  // Read DER-encoded X509 Certificate and Construct X509 object.
  //
  Status = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    Status = FALSE;
    goto _Exit;
  }

  Status = FALSE;

  //
  // Retrieve and check EVP_PKEY data from X509 Certificate.
  //
  Pkey = X509_get_pubkey (X509Cert);
  if ((Pkey == NULL) || (EVP_PKEY_id (Pkey) != EVP_PKEY_EC)) {
    goto _Exit;
  }

  //
  // Duplicate EC Context from the retrieved EVP_PKEY.
  //
  if ((*EcContext = EC_KEY_dup (EVP_PKEY_get0_EC_KEY (Pkey))) != NULL) {
    Status = TRUE;
  }

_Exit:
  //
  // Release Resources.
  //
  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  if (Pkey != NULL) {
    EVP_PKEY_free (Pkey);
  }

  return Status;
}

/**
  Retrieve the version from one X.509 certificate.

  If Cert is NULL, then return FALSE.
  If CertSize is 0, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in]      Cert         Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize     Size of the X509 certificate in bytes.
  @param[out]     Version      Pointer to the retrieved version integer.

  @retval TRUE           The certificate version retrieved successfully.
  @retval FALSE          If  Cert is NULL or CertSize is Zero.
  @retval FALSE          The operation is not supported.

**/
BOOLEAN
EFIAPI
X509GetVersion (
  IN      CONST UINT8  *Cert,
  IN      UINTN        CertSize,
  OUT     UINTN        *Version
  )
{
  BOOLEAN  Status;
  X509     *X509Cert;

  X509Cert = NULL;
  Status   = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    //
    // Invalid X.509 Certificate
    //
    Status = FALSE;
  }

  if (Status) {
    *Version = X509_get_version (X509Cert);
  }

  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  return Status;
}

/**
  Retrieve the serialNumber from one X.509 certificate.

  If Cert is NULL, then return FALSE.
  If CertSize is 0, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in]      Cert         Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize     Size of the X509 certificate in bytes.
  @param[out]     SerialNumber  Pointer to the retrieved certificate SerialNumber bytes.
  @param[in, out] SerialNumberSize  The size in bytes of the SerialNumber buffer on input,
                               and the size of buffer returned SerialNumber on output.

  @retval TRUE                     The certificate serialNumber retrieved successfully.
  @retval FALSE                    If Cert is NULL or CertSize is Zero.
                                   If SerialNumberSize is NULL.
                                   If Certificate is invalid.
  @retval FALSE                    If no SerialNumber exists.
  @retval FALSE                    If the SerialNumber is NULL. The required buffer size
                                   (including the final null) is returned in the
                                   SerialNumberSize parameter.
  @retval FALSE                    The operation is not supported.
**/
BOOLEAN
EFIAPI
X509GetSerialNumber (
  IN      CONST UINT8 *Cert,
  IN      UINTN CertSize,
  OUT     UINT8 *SerialNumber, OPTIONAL
  IN OUT  UINTN         *SerialNumberSize
  )
{
  BOOLEAN       Status;
  X509          *X509Cert;
  ASN1_INTEGER  *Asn1Integer;

  Status = FALSE;
  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (SerialNumberSize == NULL)) {
    return Status;
  }

  X509Cert = NULL;

  //
  // Read DER-encoded X509 Certificate and Construct X509 object.
  //
  Status = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    *SerialNumberSize = 0;
    Status            = FALSE;
    goto _Exit;
  }

  //
  // Retrieve subject name from certificate object.
  //
  Asn1Integer = X509_get_serialNumber (X509Cert);
  if (Asn1Integer == NULL) {
    *SerialNumberSize = 0;
    Status            = FALSE;
    goto _Exit;
  }

  if (*SerialNumberSize < (UINTN)Asn1Integer->length) {
    *SerialNumberSize = (UINTN)Asn1Integer->length;
    Status            = FALSE;
    goto _Exit;
  }

  if (SerialNumber != NULL) {
    CopyMem (SerialNumber, Asn1Integer->data, (UINTN)Asn1Integer->length);
    Status = TRUE;
  }

  *SerialNumberSize = (UINTN)Asn1Integer->length;

_Exit:
  //
  // Release Resources.
  //
  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  return Status;
}

/**
  Retrieve the issuer bytes from one X.509 certificate.

  If Cert is NULL, then return FALSE.
  If CertIssuerSize is NULL, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in]      Cert         Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize     Size of the X509 certificate in bytes.
  @param[out]     CertIssuer  Pointer to the retrieved certificate subject bytes.
  @param[in, out] CertIssuerSize  The size in bytes of the CertIssuer buffer on input,
                               and the size of buffer returned CertSubject on output.

  @retval  TRUE   The certificate issuer retrieved successfully.
  @retval  FALSE  Invalid certificate, or the CertIssuerSize is too small for the result.
                  The CertIssuerSize will be updated with the required size.
  @retval  FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
X509GetIssuerName (
  IN      CONST UINT8  *Cert,
  IN      UINTN        CertSize,
  OUT     UINT8        *CertIssuer,
  IN OUT  UINTN        *CertIssuerSize
  )
{
  BOOLEAN    Status;
  X509       *X509Cert;
  X509_NAME  *X509Name;
  UINTN      X509NameSize;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (CertIssuerSize == NULL)) {
    return FALSE;
  }

  X509Cert = NULL;

  //
  // Read DER-encoded X509 Certificate and Construct X509 object.
  //
  Status = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    Status = FALSE;
    goto _Exit;
  }

  Status = FALSE;

  //
  // Retrieve subject name from certificate object.
  //
  X509Name = X509_get_subject_name (X509Cert);
  if (X509Name == NULL) {
    goto _Exit;
  }

  X509NameSize = i2d_X509_NAME (X509Name, NULL);
  if (*CertIssuerSize < X509NameSize) {
    *CertIssuerSize = X509NameSize;
    goto _Exit;
  }

  *CertIssuerSize = X509NameSize;
  if (CertIssuer != NULL) {
    i2d_X509_NAME (X509Name, &CertIssuer);
    Status = TRUE;
  }

_Exit:
  //
  // Release Resources.
  //
  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  return Status;
}

/**
  Retrieve the Signature Algorithm from one X.509 certificate.

  @param[in]      Cert             Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize         Size of the X509 certificate in bytes.
  @param[out]     Oid              Signature Algorithm Object identifier buffer.
  @param[in,out]  OidSize          Signature Algorithm Object identifier buffer size

  @retval TRUE           The certificate Extension data retrieved successfully.
  @retval FALSE                    If Cert is NULL.
                                   If OidSize is NULL.
                                   If Oid is not NULL and *OidSize is 0.
                                   If Certificate is invalid.
  @retval FALSE                    If no SignatureType.
  @retval FALSE                    If the Oid is NULL. The required buffer size
                                   is returned in the OidSize.
  @retval FALSE                    The operation is not supported.
**/
BOOLEAN
EFIAPI
X509GetSignatureAlgorithm (
  IN CONST UINT8 *Cert,
  IN       UINTN CertSize,
  OUT   UINT8 *Oid, OPTIONAL
  IN OUT   UINTN       *OidSize
  )
{
  BOOLEAN      Status;
  X509         *X509Cert;
  int          Nid;
  ASN1_OBJECT  *Asn1Obj;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (OidSize == NULL) || (CertSize == 0)) {
    return FALSE;
  }

  X509Cert = NULL;
  Status   = FALSE;

  //
  // Read DER-encoded X509 Certificate and Construct X509 object.
  //
  Status = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    Status = FALSE;
    goto _Exit;
  }

  //
  // Retrieve subject name from certificate object.
  //
  Nid = X509_get_signature_nid (X509Cert);
  if (Nid == NID_undef) {
    *OidSize = 0;
    Status   = FALSE;
    goto _Exit;
  }

  Asn1Obj = OBJ_nid2obj (Nid);
  if (Asn1Obj == NULL) {
    *OidSize = 0;
    Status   = FALSE;
    goto _Exit;
  }

  if (*OidSize < (UINTN)Asn1Obj->length) {
    *OidSize = Asn1Obj->length;
    Status   = FALSE;
    goto _Exit;
  }

  if (Oid != NULL) {
    CopyMem (Oid, Asn1Obj->data, Asn1Obj->length);
  }

  *OidSize = Asn1Obj->length;
  Status   = TRUE;

_Exit:
  //
  // Release Resources.
  //
  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  return Status;
}

/**
  Retrieve Extension data from one X.509 certificate.

  @param[in]      Cert             Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize         Size of the X509 certificate in bytes.
  @param[in]      Oid              Object identifier buffer
  @param[in]      OidSize          Object identifier buffer size
  @param[out]     ExtensionData    Extension bytes.
  @param[in, out] ExtensionDataSize Extension bytes size.

  @retval TRUE                     The certificate Extension data retrieved successfully.
  @retval FALSE                    If Cert is NULL.
                                   If ExtensionDataSize is NULL.
                                   If ExtensionData is not NULL and *ExtensionDataSize is 0.
                                   If Certificate is invalid.
  @retval FALSE                    If no Extension entry match Oid.
  @retval FALSE                    If the ExtensionData is NULL. The required buffer size
                                   is returned in the ExtensionDataSize parameter.
  @retval FALSE                    The operation is not supported.
**/
BOOLEAN
EFIAPI
X509GetExtensionData (
  IN     CONST UINT8  *Cert,
  IN     UINTN        CertSize,
  IN     CONST UINT8  *Oid,
  IN     UINTN        OidSize,
  OUT UINT8           *ExtensionData,
  IN OUT UINTN        *ExtensionDataSize
  )
{
  BOOLEAN  Status;
  INTN     i;
  X509     *X509Cert;

  CONST STACK_OF (X509_EXTENSION) *Extensions;
  ASN1_OBJECT        *Asn1Obj;
  ASN1_OCTET_STRING  *Asn1Oct;
  X509_EXTENSION     *Ext;
  UINTN              ObjLength;
  UINTN              OctLength;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (CertSize == 0) || (Oid == NULL) || (OidSize == 0) || (ExtensionDataSize == NULL)) {
    return FALSE;
  }

  X509Cert = NULL;
  Status   = FALSE;

  //
  // Read DER-encoded X509 Certificate and Construct X509 object.
  //
  Status = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    *ExtensionDataSize = 0;
    goto Cleanup;
  }

  //
  // Retrieve Extensions from certificate object.
  //
  Extensions = X509_get0_extensions (X509Cert);
  if (sk_X509_EXTENSION_num (Extensions) <= 0) {
    *ExtensionDataSize = 0;
    goto Cleanup;
  }

  //
  // Traverse Extensions
  //
  Status    = FALSE;
  Asn1Oct   = NULL;
  OctLength = 0;
  for (i = 0; i < sk_X509_EXTENSION_num (Extensions); i++) {
    Ext = sk_X509_EXTENSION_value (Extensions, (int)i);
    if (Ext == NULL) {
      continue;
    }

    Asn1Obj = X509_EXTENSION_get_object (Ext);
    if (Asn1Obj == NULL) {
      continue;
    }

    Asn1Oct = X509_EXTENSION_get_data (Ext);
    if (Asn1Oct == NULL) {
      continue;
    }

    ObjLength = OBJ_length (Asn1Obj);
    OctLength = ASN1_STRING_length (Asn1Oct);
    if ((OidSize == ObjLength) && (CompareMem (OBJ_get0_data (Asn1Obj), Oid, OidSize) == 0)) {
      //
      // Extension Found
      //
      Status = TRUE;
      break;
    }

    //
    // reset to 0 if not found
    //
    OctLength = 0;
  }

  if (Status) {
    if (*ExtensionDataSize < OctLength) {
      *ExtensionDataSize = OctLength;
      Status             = FALSE;
      goto Cleanup;
    }

    if (Asn1Oct != NULL) {
      CopyMem (ExtensionData, ASN1_STRING_get0_data (Asn1Oct), OctLength);
    }

    *ExtensionDataSize = OctLength;
  } else {
    *ExtensionDataSize = 0;
  }

Cleanup:
  //
  // Release Resources.
  //
  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  return Status;
}

/**
  Retrieve the Extended Key Usage from one X.509 certificate.

  @param[in]      Cert             Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize         Size of the X509 certificate in bytes.
  @param[out]     Usage            Key Usage bytes.
  @param[in, out] UsageSize        Key Usage buffer size in bytes.

  @retval TRUE                     The Usage bytes retrieve successfully.
  @retval FALSE                    If Cert is NULL.
                                   If CertSize is NULL.
                                   If Usage is not NULL and *UsageSize is 0.
                                   If Cert is invalid.
  @retval FALSE                    If the Usage is NULL. The required buffer size
                                   is returned in the UsageSize parameter.
  @retval FALSE                    The operation is not supported.
**/
BOOLEAN
EFIAPI
X509GetExtendedKeyUsage (
  IN     CONST UINT8  *Cert,
  IN     UINTN        CertSize,
  OUT UINT8           *Usage,
  IN OUT UINTN        *UsageSize
  )
{
  BOOLEAN  Status;

  Status = X509GetExtensionData (Cert, CertSize, mOidExtKeyUsage, sizeof (mOidExtKeyUsage), Usage, UsageSize);
  return Status;
}

/**
  Retrieve the Validity from one X.509 certificate

  If Cert is NULL, then return FALSE.
  If CertIssuerSize is NULL, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in]      Cert         Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize     Size of the X509 certificate in bytes.
  @param[out]     From         notBefore Pointer to DateTime object.
  @param[in,out]  FromSize     notBefore DateTime object size.
  @param[out]     To           notAfter Pointer to DateTime object.
  @param[in,out]  ToSize       notAfter DateTime object size.

  Note: X509CompareDateTime to compare DateTime oject
        x509SetDateTime to get a DateTime object from a DateTimeStr

  @retval  TRUE   The certificate Validity retrieved successfully.
  @retval  FALSE  Invalid certificate, or Validity retrieve failed.
  @retval  FALSE  This interface is not supported.
**/
BOOLEAN
EFIAPI
X509GetValidity  (
  IN     CONST UINT8  *Cert,
  IN     UINTN        CertSize,
  IN     UINT8        *From,
  IN OUT UINTN        *FromSize,
  IN     UINT8        *To,
  IN OUT UINTN        *ToSize
  )
{
  BOOLEAN          Status;
  X509             *X509Cert;
  CONST ASN1_TIME  *F;
  CONST ASN1_TIME  *T;
  UINTN            TSize;
  UINTN            FSize;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (FromSize == NULL) || (ToSize == NULL) || (CertSize == 0)) {
    return FALSE;
  }

  X509Cert = NULL;
  Status   = FALSE;

  //
  // Read DER-encoded X509 Certificate and Construct X509 object.
  //
  Status = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    goto _Exit;
  }

  //
  // Retrieve Validity from/to from certificate object.
  //
  F = X509_get0_notBefore (X509Cert);
  T = X509_get0_notAfter (X509Cert);

  if ((F == NULL) || (T == NULL)) {
    goto _Exit;
  }

  FSize = sizeof (ASN1_TIME) + F->length;
  if (*FromSize < FSize) {
    *FromSize = FSize;
    goto _Exit;
  }

  *FromSize = FSize;
  if (From != NULL) {
    CopyMem (From, F, sizeof (ASN1_TIME));
    ((ASN1_TIME *)From)->data = From + sizeof (ASN1_TIME);
    CopyMem (From + sizeof (ASN1_TIME), F->data, F->length);
  }

  TSize = sizeof (ASN1_TIME) + T->length;
  if (*ToSize < TSize) {
    *ToSize = TSize;
    goto _Exit;
  }

  *ToSize = TSize;
  if (To != NULL) {
    CopyMem (To, T, sizeof (ASN1_TIME));
    ((ASN1_TIME *)To)->data = To + sizeof (ASN1_TIME);
    CopyMem (To + sizeof (ASN1_TIME), T->data, T->length);
  }

  Status = TRUE;

_Exit:
  //
  // Release Resources.
  //
  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  return Status;
}

/**
  Format a DateTimeStr to DataTime object in DataTime Buffer

  If DateTimeStr is NULL, then return FALSE.
  If DateTimeSize is NULL, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in]      DateTimeStr      DateTime string like YYYYMMDDhhmmssZ
                                   Ref: https://www.w3.org/TR/NOTE-datetime
                                   Z stand for UTC time
  @param[out]     DateTime         Pointer to a DateTime object.
  @param[in,out]  DateTimeSize     DateTime object buffer size.

  @retval TRUE                     The DateTime object create successfully.
  @retval FALSE                    If DateTimeStr is NULL.
                                   If DateTimeSize is NULL.
                                   If DateTime is not NULL and *DateTimeSize is 0.
                                   If Year Month Day Hour Minute Second combination is invalid datetime.
  @retval FALSE                    If the DateTime is NULL. The required buffer size
                                   (including the final null) is returned in the
                                   DateTimeSize parameter.
  @retval FALSE                    The operation is not supported.
**/
BOOLEAN
EFIAPI
X509FormatDateTime (
  IN  CONST  CHAR8  *DateTimeStr,
  OUT VOID          *DateTime,
  IN OUT UINTN      *DateTimeSize
  )
{
  BOOLEAN    Status;
  INT32      Ret;
  ASN1_TIME  *Dt;
  UINTN      DSize;

  Dt     = NULL;
  Status = FALSE;

  Dt = ASN1_TIME_new ();
  if (Dt == NULL) {
    Status = FALSE;
    goto Cleanup;
  }

  Ret = ASN1_TIME_set_string_X509 (Dt, DateTimeStr);
  if (Ret != 1) {
    Status = FALSE;
    goto Cleanup;
  }

  DSize = sizeof (ASN1_TIME) + Dt->length;
  if (*DateTimeSize < DSize) {
    *DateTimeSize = DSize;
    Status        = FALSE;
    goto Cleanup;
  }

  *DateTimeSize = DSize;
  if (DateTime != NULL) {
    CopyMem (DateTime, Dt, sizeof (ASN1_TIME));
    ((ASN1_TIME *)DateTime)->data = (UINT8 *)DateTime + sizeof (ASN1_TIME);
    CopyMem ((UINT8 *)DateTime + sizeof (ASN1_TIME), Dt->data, Dt->length);
  }

  Status = TRUE;

Cleanup:
  if (Dt != NULL) {
    ASN1_TIME_free (Dt);
  }

  return Status;
}

/**
  Compare DateTime1 object and DateTime2 object.

  If DateTime1 is NULL, then return -2.
  If DateTime2 is NULL, then return -2.
  If DateTime1 == DateTime2, then return 0
  If DateTime1 > DateTime2, then return 1
  If DateTime1 < DateTime2, then return -1

  @param[in]      DateTime1         Pointer to a DateTime Ojbect
  @param[in]      DateTime2         Pointer to a DateTime Object

  @retval  0      If DateTime1 == DateTime2
  @retval  1      If DateTime1 > DateTime2
  @retval  -1     If DateTime1 < DateTime2
**/
INT32
EFIAPI
X509CompareDateTime (
  IN CONST  VOID  *DateTime1,
  IN CONST  VOID  *DateTime2
  )
{
  return (INT32)ASN1_TIME_compare (DateTime1, DateTime2);
}

/**
  Retrieve the Key Usage from one X.509 certificate.

  @param[in]      Cert             Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize         Size of the X509 certificate in bytes.
  @param[out]     Usage            Key Usage (CRYPTO_X509_KU_*)

  @retval  TRUE   The certificate Key Usage retrieved successfully.
  @retval  FALSE  Invalid certificate, or Usage is NULL
  @retval  FALSE  This interface is not supported.
**/
BOOLEAN
EFIAPI
X509GetKeyUsage (
  IN    CONST UINT8  *Cert,
  IN    UINTN        CertSize,
  OUT   UINTN        *Usage
  )
{
  BOOLEAN  Status;
  X509     *X509Cert;

  //
  // Check input parameters.
  //
  if ((Cert == NULL) || (Usage == NULL)) {
    return FALSE;
  }

  X509Cert = NULL;
  Status   = FALSE;

  //
  // Read DER-encoded X509 Certificate and Construct X509 object.
  //
  Status = X509ConstructCertificate (Cert, CertSize, (UINT8 **)&X509Cert);
  if ((X509Cert == NULL) || (!Status)) {
    goto _Exit;
  }

  //
  // Retrieve subject name from certificate object.
  //
  *Usage = X509_get_key_usage (X509Cert);
  if (*Usage == NID_undef) {
    goto _Exit;
  }

  Status = TRUE;

_Exit:
  //
  // Release Resources.
  //
  if (X509Cert != NULL) {
    X509_free (X509Cert);
  }

  return Status;
}

/**
  Verify one X509 certificate was issued by the trusted CA.
  @param[in]      RootCert          Trusted Root Certificate buffer

  @param[in]      RootCertLength    Trusted Root Certificate buffer length
  @param[in]      CertChain         One or more ASN.1 DER-encoded X.509 certificates
                                    where the first certificate is signed by the Root
                                    Certificate or is the Root Certificate itself. and
                                    subsequent certificate is signed by the preceding
                                    certificate.
  @param[in]      CertChainLength   Total length of the certificate chain, in bytes.

  @retval  TRUE   All certificates was issued by the first certificate in X509Certchain.
  @retval  FALSE  Invalid certificate or the certificate was not issued by the given
                  trusted CA.
**/
BOOLEAN
EFIAPI
X509VerifyCertChain (
  IN CONST UINT8  *RootCert,
  IN UINTN        RootCertLength,
  IN CONST UINT8  *CertChain,
  IN UINTN        CertChainLength
  )
{
  CONST UINT8  *TmpPtr;
  UINTN        Length;
  UINT32       Asn1Tag;
  UINT32       ObjClass;
  CONST UINT8  *CurrentCert;
  UINTN        CurrentCertLen;
  CONST UINT8  *PrecedingCert;
  UINTN        PrecedingCertLen;
  BOOLEAN      VerifyFlag;
  INT32        Ret;

  PrecedingCert    = RootCert;
  PrecedingCertLen = RootCertLength;

  CurrentCert    = CertChain;
  Length         = 0;
  CurrentCertLen = 0;

  VerifyFlag = FALSE;
  while (TRUE) {
    TmpPtr = CurrentCert;
    Ret    = ASN1_get_object (
               (CONST UINT8 **)&TmpPtr,
               (long *)&Length,
               (int *)&Asn1Tag,
               (int *)&ObjClass,
               (long)(CertChainLength + CertChain - TmpPtr)
               );
    if ((Asn1Tag != V_ASN1_SEQUENCE) || (Ret == 0x80)) {
      break;
    }

    //
    // Calculate CurrentCert length;
    //
    CurrentCertLen = TmpPtr - CurrentCert + Length;

    //
    // Verify CurrentCert with preceding cert;
    //
    VerifyFlag = X509VerifyCert (CurrentCert, CurrentCertLen, PrecedingCert, PrecedingCertLen);
    if (VerifyFlag == FALSE) {
      break;
    }

    //
    // move Current cert to Preceding cert
    //
    PrecedingCertLen = CurrentCertLen;
    PrecedingCert    = CurrentCert;

    //
    // Move to next
    //
    CurrentCert = CurrentCert + CurrentCertLen;
  }

  return VerifyFlag;
}

/**
  Get one X509 certificate from CertChain.

  @param[in]      CertChain         One or more ASN.1 DER-encoded X.509 certificates
                                    where the first certificate is signed by the Root
                                    Certificate or is the Root Certificate itself. and
                                    subsequent certificate is signed by the preceding
                                    certificate.
  @param[in]      CertChainLength   Total length of the certificate chain, in bytes.

  @param[in]      CertIndex         Index of certificate.

  @param[out]     Cert              The certificate at the index of CertChain.
  @param[out]     CertLength        The length certificate at the index of CertChain.

  @retval  TRUE   Success.
  @retval  FALSE  Failed to get certificate from certificate chain.
**/
BOOLEAN
EFIAPI
X509GetCertFromCertChain (
  IN CONST UINT8   *CertChain,
  IN UINTN         CertChainLength,
  IN CONST INT32   CertIndex,
  OUT CONST UINT8  **Cert,
  OUT UINTN        *CertLength
  )
{
  UINTN        Asn1Len;
  INT32        CurrentIndex;
  UINTN        CurrentCertLen;
  CONST UINT8  *CurrentCert;
  CONST UINT8  *TmpPtr;
  INT32        Ret;
  UINT32       Asn1Tag;
  UINT32       ObjClass;

  //
  // Check input parameters.
  //
  if ((CertChain == NULL) || (Cert == NULL) ||
      (CertIndex < -1) || (CertLength == NULL))
  {
    return FALSE;
  }

  Asn1Len        = 0;
  CurrentCertLen = 0;
  CurrentCert    = CertChain;
  CurrentIndex   = -1;

  //
  // Traverse the certificate chain
  //
  while (TRUE) {
    TmpPtr = CurrentCert;

    // Get asn1 object and taglen
    Ret = ASN1_get_object (
            (CONST UINT8 **)&TmpPtr,
            (long *)&Asn1Len,
            (int *)&Asn1Tag,
            (int *)&ObjClass,
            (long)(CertChainLength + CertChain - TmpPtr)
            );
    if ((Asn1Tag != V_ASN1_SEQUENCE) || (Ret == 0x80)) {
      break;
    }

    //
    // Calculate CurrentCert length;
    //
    CurrentCertLen = TmpPtr - CurrentCert + Asn1Len;
    CurrentIndex++;

    if (CurrentIndex == CertIndex) {
      *Cert       = CurrentCert;
      *CertLength = CurrentCertLen;
      return TRUE;
    }

    //
    // Move to next
    //
    CurrentCert = CurrentCert + CurrentCertLen;
  }

  //
  // If CertIndex is -1, Return the last certificate
  //
  if ((CertIndex == -1) && (CurrentIndex >= 0)) {
    *Cert       = CurrentCert - CurrentCertLen;
    *CertLength = CurrentCertLen;
    return TRUE;
  }

  return FALSE;
}

/**
  Retrieve the tag and length of the tag.

  @param Ptr      The position in the ASN.1 data
  @param End      End of data
  @param Length   The variable that will receive the length
  @param Tag      The expected tag

  @retval      TRUE   Get tag successful
  @retval      FALSe  Failed to get tag or tag not match
**/
BOOLEAN
EFIAPI
Asn1GetTag (
  IN OUT UINT8    **Ptr,
  IN CONST UINT8  *End,
  OUT UINTN       *Length,
  IN     UINT32   Tag
  )
{
  UINT8   *PtrOld;
  INT32   ObjTag;
  INT32   ObjCls;
  long    ObjLength;
  UINT32  Inf;

  //
  // Save Ptr position
  //
  PtrOld = *Ptr;

  Inf = ASN1_get_object ((CONST UINT8 **)Ptr, &ObjLength, &ObjTag, &ObjCls, (INT32)(End - (*Ptr)));
  if (((Inf & 0x80) == 0x00) &&
      (ObjTag == (INT32)(Tag & CRYPTO_ASN1_TAG_VALUE_MASK)) &&
      (ObjCls == (INT32)(Tag & CRYPTO_ASN1_TAG_CLASS_MASK)))
  {
    *Length = (UINTN)ObjLength;
    return TRUE;
  } else {
    //
    // if doesn't match Tag, restore Ptr to origin Ptr
    //
    *Ptr = PtrOld;
    return FALSE;
  }
}

/**
  Retrieve the basic constraints from one X.509 certificate.

  @param[in]      Cert                     Pointer to the DER-encoded X509 certificate.
  @param[in]      CertSize                 size of the X509 certificate in bytes.
  @param[out]     BasicConstraints         basic constraints bytes.
  @param[in, out] BasicConstraintsSize     basic constraints buffer size in bytes.

  @retval TRUE                     The basic constraints retrieve successfully.
  @retval FALSE                    If cert is NULL.
                                   If cert_size is NULL.
                                   If basic_constraints is not NULL and *basic_constraints_size is 0.
                                   If cert is invalid.
  @retval FALSE                    The required buffer size is small.
                                   The return buffer size is basic_constraints_size parameter.
  @retval FALSE                    If no Extension entry match oid.
  @retval FALSE                    The operation is not supported.
 **/
BOOLEAN
EFIAPI
X509GetExtendedBasicConstraints             (
  CONST UINT8  *Cert,
  UINTN        CertSize,
  UINT8        *BasicConstraints,
  UINTN        *BasicConstraintsSize
  )
{
  BOOLEAN  Status;

  if ((Cert == NULL) || (CertSize == 0) || (BasicConstraintsSize == NULL)) {
    return FALSE;
  }

  Status = X509GetExtensionData (
             (UINT8 *)Cert,
             CertSize,
             mOidBasicConstraints,
             sizeof (mOidBasicConstraints),
             BasicConstraints,
             BasicConstraintsSize
             );

  return Status;
}
