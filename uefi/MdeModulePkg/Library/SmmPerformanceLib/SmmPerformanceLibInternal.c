/** @file
  Performance Library used in the MM phase.

  This library instance provides infrastructure for MM drivers to log performance
  data. It consumes MM PerformanceEx or Performance Protocol published by SmmCorePerformanceLib
  to log performance data. If both MM PerformanceEx and Performance Protocol are not available,
  it does not log any performance information.

  Copyright (c) 2011 - 2023, Intel Corporation. All rights reserved.<BR>
  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Guid/PerformanceMeasurement.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MmServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/PerformanceLib.h>

#include "SmmPerformanceLibInternal.h"

//
// The cached MM Performance Protocol and MM PerformanceEx Protocol interface.
EDKII_PERFORMANCE_MEASUREMENT_PROTOCOL  *mPerformanceMeasurement = NULL;
BOOLEAN                                 mPerformanceMeasurementEnabled;
VOID                                    *mPerformanceLibExitBootServicesRegistration;

/**
  Registers a callback to perform library actions needed at exit boot services.

  @param[in] ExitBootServicesProtocolGuid  The protocol GUID to register the callback for.

  @retval EFI_SUCCESS The callback was registered successfully.
  @retval Others      An error occurred registering the callback.
 **/
EFI_STATUS
RegisterExitBootServicesCallback (
  IN  CONST EFI_GUID  *ExitBootServicesProtocolGuid
  )
{
  EFI_STATUS  Status;

  mPerformanceMeasurementEnabled =  (BOOLEAN)((PcdGet8 (PcdPerformanceLibraryPropertyMask) & PERFORMANCE_LIBRARY_PROPERTY_MEASUREMENT_ENABLED) != 0);

  Status = gMmst->MmRegisterProtocolNotify (
                    ExitBootServicesProtocolGuid,
                    SmmPerformanceLibExitBootServicesCallback,
                    &mPerformanceLibExitBootServicesRegistration
                    );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Unregisters a callback to perform library actions needed at exit boot services.

  @param[in] ExitBootServicesProtocolGuid  The protocol GUID to unregister the callback for.

  @retval EFI_SUCCESS The callback was unregistered successfully.
  @retval Others      An error occurred unregistering the callback.
 **/
EFI_STATUS
UnregisterExitBootServicesCallback (
  IN  CONST EFI_GUID  *ExitBootServicesProtocolGuid
  )
{
  EFI_STATUS  Status;

  //
  // Unregister SmmExitBootServices notification.
  //
  Status = gMmst->MmRegisterProtocolNotify (
                    ExitBootServicesProtocolGuid,
                    NULL,
                    &mPerformanceLibExitBootServicesRegistration
                    );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  This is the Event call back function is triggered in MM to notify the Library
  the system is entering runtime phase.

  @param[in] Protocol   Points to the protocol's unique identifier
  @param[in] Interface  Points to the interface instance
  @param[in] Handle     The handle on which the interface was installed

  @retval EFI_SUCCESS SmmAtRuntimeCallBack runs successfully
 **/
EFI_STATUS
EFIAPI
SmmPerformanceLibExitBootServicesCallback (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  //
  // Disable performance measurement after ExitBootServices because
  // 1. Performance measurement might impact SMI latency at runtime;
  // 2. Performance log is copied to non SMRAM at ReadyToBoot so runtime performance
  //    log is not useful.
  //
  mPerformanceMeasurementEnabled = FALSE;

  return EFI_SUCCESS;
}

/**
  The function caches the pointers to MM PerformanceEx protocol and Performance Protocol.

  The function locates MM PerformanceEx protocol and Performance Protocol from protocol database.

  @retval EFI_SUCCESS     MM PerformanceEx protocol or Performance Protocol is successfully located.
  @retval EFI_NOT_FOUND   Both MM PerformanceEx protocol and Performance Protocol are not located to log performance.

**/
EFI_STATUS
GetPerformanceMeasurementProtocol (
  VOID
  )
{
  EFI_STATUS                              Status;
  EDKII_PERFORMANCE_MEASUREMENT_PROTOCOL  *PerformanceMeasurement;

  if (mPerformanceMeasurement != NULL) {
    return EFI_SUCCESS;
  }

  Status = gMmst->MmLocateProtocol (&gEdkiiSmmPerformanceMeasurementProtocolGuid, NULL, (VOID **)&PerformanceMeasurement);
  if (!EFI_ERROR (Status)) {
    ASSERT (PerformanceMeasurement != NULL);

    //
    // Cache PerformanceMeasurement Protocol.
    //
    mPerformanceMeasurement = PerformanceMeasurement;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

/**
  Creates a record for the beginning of a performance measurement.

  Creates a record that contains the Handle, Token, Module and Identifier.
  If TimeStamp is not zero, then TimeStamp is added to the record as the start time.
  If TimeStamp is zero, then this function reads the current time stamp
  and adds that time stamp value to the record as the start time.

  @param  Handle                  Pointer to environment specific context used
                                  to identify the component being measured.
  @param  Token                   Pointer to a Null-terminated ASCII string
                                  that identifies the component being measured.
  @param  Module                  Pointer to a Null-terminated ASCII string
                                  that identifies the module being measured.
  @param  TimeStamp               64-bit time stamp.
  @param  Identifier              32-bit identifier. If the value is 0, the created record
                                  is same as the one created by StartPerformanceMeasurement.

  @retval RETURN_SUCCESS          The start of the measurement was recorded.
  @retval RETURN_OUT_OF_RESOURCES There are not enough resources to record the measurement.

**/
RETURN_STATUS
EFIAPI
StartPerformanceMeasurementEx (
  IN CONST VOID   *Handle   OPTIONAL,
  IN CONST CHAR8  *Token    OPTIONAL,
  IN CONST CHAR8  *Module   OPTIONAL,
  IN UINT64       TimeStamp,
  IN UINT32       Identifier
  )
{
  EFI_STATUS   Status;
  CONST CHAR8  *String;

  Status = GetPerformanceMeasurementProtocol ();
  if (EFI_ERROR (Status)) {
    return RETURN_NOT_FOUND;
  }

  if (Token != NULL) {
    String = Token;
  } else if (Module != NULL) {
    String = Module;
  } else {
    String = NULL;
  }

  if (mPerformanceMeasurement != NULL) {
    Status = mPerformanceMeasurement->CreatePerformanceMeasurement (Handle, NULL, String, TimeStamp, 0, Identifier, PerfStartEntry);
  } else {
    ASSERT (FALSE);
  }

  return (RETURN_STATUS)Status;
}

/**
  Fills in the end time of a performance measurement.

  Looks up the record that matches Handle, Token and Module.
  If the record can not be found then return RETURN_NOT_FOUND.
  If the record is found and TimeStamp is not zero,
  then TimeStamp is added to the record as the end time.
  If the record is found and TimeStamp is zero, then this function reads
  the current time stamp and adds that time stamp value to the record as the end time.

  @param  Handle                  Pointer to environment specific context used
                                  to identify the component being measured.
  @param  Token                   Pointer to a Null-terminated ASCII string
                                  that identifies the component being measured.
  @param  Module                  Pointer to a Null-terminated ASCII string
                                  that identifies the module being measured.
  @param  TimeStamp               64-bit time stamp.
  @param  Identifier              32-bit identifier. If the value is 0, the found record
                                  is same as the one found by EndPerformanceMeasurement.

  @retval RETURN_SUCCESS          The end of  the measurement was recorded.
  @retval RETURN_NOT_FOUND        The specified measurement record could not be found.

**/
RETURN_STATUS
EFIAPI
EndPerformanceMeasurementEx (
  IN CONST VOID   *Handle   OPTIONAL,
  IN CONST CHAR8  *Token    OPTIONAL,
  IN CONST CHAR8  *Module   OPTIONAL,
  IN UINT64       TimeStamp,
  IN UINT32       Identifier
  )
{
  EFI_STATUS   Status;
  CONST CHAR8  *String;

  Status = GetPerformanceMeasurementProtocol ();
  if (EFI_ERROR (Status)) {
    return RETURN_NOT_FOUND;
  }

  if (Token != NULL) {
    String = Token;
  } else if (Module != NULL) {
    String = Module;
  } else {
    String = NULL;
  }

  if (mPerformanceMeasurement != NULL) {
    Status = mPerformanceMeasurement->CreatePerformanceMeasurement (Handle, NULL, String, TimeStamp, 0, Identifier, PerfEndEntry);
  } else {
    ASSERT (FALSE);
  }

  return (RETURN_STATUS)Status;
}

/**
  Attempts to retrieve a performance measurement log entry from the performance measurement log.
  It can also retrieve the log created by StartPerformanceMeasurement and EndPerformanceMeasurement,
  and then assign the Identifier with 0.

  Attempts to retrieve the performance log entry specified by LogEntryKey.  If LogEntryKey is
  zero on entry, then an attempt is made to retrieve the first entry from the performance log,
  and the key for the second entry in the log is returned.  If the performance log is empty,
  then no entry is retrieved and zero is returned.  If LogEntryKey is not zero, then the performance
  log entry associated with LogEntryKey is retrieved, and the key for the next entry in the log is
  returned.  If LogEntryKey is the key for the last entry in the log, then the last log entry is
  retrieved and an implementation specific non-zero key value that specifies the end of the performance
  log is returned.  If LogEntryKey is equal this implementation specific non-zero key value, then no entry
  is retrieved and zero is returned.  In the cases where a performance log entry can be returned,
  the log entry is returned in Handle, Token, Module, StartTimeStamp, EndTimeStamp and Identifier.
  If LogEntryKey is not a valid log entry key for the performance measurement log, then ASSERT().
  If Handle is NULL, then ASSERT().
  If Token is NULL, then ASSERT().
  If Module is NULL, then ASSERT().
  If StartTimeStamp is NULL, then ASSERT().
  If EndTimeStamp is NULL, then ASSERT().
  If Identifier is NULL, then ASSERT().

  @param  LogEntryKey             On entry, the key of the performance measurement log entry to retrieve.
                                  0, then the first performance measurement log entry is retrieved.
                                  On exit, the key of the next performance log entry.
  @param  Handle                  Pointer to environment specific context used to identify the component
                                  being measured.
  @param  Token                   Pointer to a Null-terminated ASCII string that identifies the component
                                  being measured.
  @param  Module                  Pointer to a Null-terminated ASCII string that identifies the module
                                  being measured.
  @param  StartTimeStamp          Pointer to the 64-bit time stamp that was recorded when the measurement
                                  was started.
  @param  EndTimeStamp            Pointer to the 64-bit time stamp that was recorded when the measurement
                                  was ended.
  @param  Identifier              Pointer to the 32-bit identifier that was recorded.

  @return The key for the next performance log entry (in general case).

**/
UINTN
EFIAPI
GetPerformanceMeasurementEx (
  IN  UINTN        LogEntryKey,
  OUT CONST VOID   **Handle,
  OUT CONST CHAR8  **Token,
  OUT CONST CHAR8  **Module,
  OUT UINT64       *StartTimeStamp,
  OUT UINT64       *EndTimeStamp,
  OUT UINT32       *Identifier
  )
{
  return 0;
}

/**
  Creates a record for the beginning of a performance measurement.

  Creates a record that contains the Handle, Token, and Module.
  If TimeStamp is not zero, then TimeStamp is added to the record as the start time.
  If TimeStamp is zero, then this function reads the current time stamp
  and adds that time stamp value to the record as the start time.

  @param  Handle                  Pointer to environment specific context used
                                  to identify the component being measured.
  @param  Token                   Pointer to a Null-terminated ASCII string
                                  that identifies the component being measured.
  @param  Module                  Pointer to a Null-terminated ASCII string
                                  that identifies the module being measured.
  @param  TimeStamp               64-bit time stamp.

  @retval RETURN_SUCCESS          The start of the measurement was recorded.
  @retval RETURN_OUT_OF_RESOURCES There are not enough resources to record the measurement.

**/
RETURN_STATUS
EFIAPI
StartPerformanceMeasurement (
  IN CONST VOID   *Handle   OPTIONAL,
  IN CONST CHAR8  *Token    OPTIONAL,
  IN CONST CHAR8  *Module   OPTIONAL,
  IN UINT64       TimeStamp
  )
{
  return StartPerformanceMeasurementEx (Handle, Token, Module, TimeStamp, 0);
}

/**
  Fills in the end time of a performance measurement.

  Looks up the record that matches Handle, Token, and Module.
  If the record can not be found then return RETURN_NOT_FOUND.
  If the record is found and TimeStamp is not zero,
  then TimeStamp is added to the record as the end time.
  If the record is found and TimeStamp is zero, then this function reads
  the current time stamp and adds that time stamp value to the record as the end time.

  @param  Handle                  Pointer to environment specific context used
                                  to identify the component being measured.
  @param  Token                   Pointer to a Null-terminated ASCII string
                                  that identifies the component being measured.
  @param  Module                  Pointer to a Null-terminated ASCII string
                                  that identifies the module being measured.
  @param  TimeStamp               64-bit time stamp.

  @retval RETURN_SUCCESS          The end of  the measurement was recorded.
  @retval RETURN_NOT_FOUND        The specified measurement record could not be found.

**/
RETURN_STATUS
EFIAPI
EndPerformanceMeasurement (
  IN CONST VOID   *Handle   OPTIONAL,
  IN CONST CHAR8  *Token    OPTIONAL,
  IN CONST CHAR8  *Module   OPTIONAL,
  IN UINT64       TimeStamp
  )
{
  return EndPerformanceMeasurementEx (Handle, Token, Module, TimeStamp, 0);
}

/**
  Attempts to retrieve a performance measurement log entry from the performance measurement log.
  It can also retrieve the log created by StartPerformanceMeasurementEx and EndPerformanceMeasurementEx,
  and then eliminate the Identifier.

  Attempts to retrieve the performance log entry specified by LogEntryKey.  If LogEntryKey is
  zero on entry, then an attempt is made to retrieve the first entry from the performance log,
  and the key for the second entry in the log is returned.  If the performance log is empty,
  then no entry is retrieved and zero is returned.  If LogEntryKey is not zero, then the performance
  log entry associated with LogEntryKey is retrieved, and the key for the next entry in the log is
  returned.  If LogEntryKey is the key for the last entry in the log, then the last log entry is
  retrieved and an implementation specific non-zero key value that specifies the end of the performance
  log is returned.  If LogEntryKey is equal this implementation specific non-zero key value, then no entry
  is retrieved and zero is returned.  In the cases where a performance log entry can be returned,
  the log entry is returned in Handle, Token, Module, StartTimeStamp, and EndTimeStamp.
  If LogEntryKey is not a valid log entry key for the performance measurement log, then ASSERT().
  If Handle is NULL, then ASSERT().
  If Token is NULL, then ASSERT().
  If Module is NULL, then ASSERT().
  If StartTimeStamp is NULL, then ASSERT().
  If EndTimeStamp is NULL, then ASSERT().

  @param  LogEntryKey             On entry, the key of the performance measurement log entry to retrieve.
                                  0, then the first performance measurement log entry is retrieved.
                                  On exit, the key of the next performance log entry.
  @param  Handle                  Pointer to environment specific context used to identify the component
                                  being measured.
  @param  Token                   Pointer to a Null-terminated ASCII string that identifies the component
                                  being measured.
  @param  Module                  Pointer to a Null-terminated ASCII string that identifies the module
                                  being measured.
  @param  StartTimeStamp          Pointer to the 64-bit time stamp that was recorded when the measurement
                                  was started.
  @param  EndTimeStamp            Pointer to the 64-bit time stamp that was recorded when the measurement
                                  was ended.

  @return The key for the next performance log entry (in general case).

**/
UINTN
EFIAPI
GetPerformanceMeasurement (
  IN  UINTN        LogEntryKey,
  OUT CONST VOID   **Handle,
  OUT CONST CHAR8  **Token,
  OUT CONST CHAR8  **Module,
  OUT UINT64       *StartTimeStamp,
  OUT UINT64       *EndTimeStamp
  )
{
  return 0;
}

/**
  Returns TRUE if the performance measurement macros are enabled.

  This function returns TRUE if the PERFORMANCE_LIBRARY_PROPERTY_MEASUREMENT_ENABLED bit of
  PcdPerformanceLibraryPropertyMask is set.  Otherwise FALSE is returned.

  @retval TRUE                    The PERFORMANCE_LIBRARY_PROPERTY_MEASUREMENT_ENABLED bit of
                                  PcdPerformanceLibraryPropertyMask is set.
  @retval FALSE                   The PERFORMANCE_LIBRARY_PROPERTY_MEASUREMENT_ENABLED bit of
                                  PcdPerformanceLibraryPropertyMask is clear.

**/
BOOLEAN
EFIAPI
PerformanceMeasurementEnabled (
  VOID
  )
{
  return mPerformanceMeasurementEnabled;
}

/**
  Create performance record with event description and a timestamp.

  @param CallerIdentifier  - Image handle or pointer to caller ID GUID
  @param Guid              - Pointer to a GUID
  @param String            - Pointer to a string describing the measurement
  @param Address           - Pointer to a location in memory relevant to the measurement
  @param Identifier        - Performance identifier describing the type of measurement

  @retval RETURN_SUCCESS           - Successfully created performance record
  @retval RETURN_OUT_OF_RESOURCES  - Ran out of space to store the records
  @retval RETURN_INVALID_PARAMETER - Invalid parameter passed to function - NULL
                                     pointer or invalid PerfId

**/
RETURN_STATUS
EFIAPI
LogPerformanceMeasurement (
  IN CONST VOID   *CallerIdentifier,
  IN CONST VOID   *Guid     OPTIONAL,
  IN CONST CHAR8  *String   OPTIONAL,
  IN UINT64       Address  OPTIONAL,
  IN UINT32       Identifier
  )
{
  EFI_STATUS  Status;

  Status = GetPerformanceMeasurementProtocol ();
  if (EFI_ERROR (Status)) {
    return RETURN_OUT_OF_RESOURCES;
  }

  if (mPerformanceMeasurement != NULL) {
    Status = mPerformanceMeasurement->CreatePerformanceMeasurement (CallerIdentifier, Guid, String, 0, Address, Identifier, PerfEntry);
  } else {
    ASSERT (FALSE);
  }

  return (RETURN_STATUS)Status;
}

/**
  Check whether the specified performance measurement can be logged.

  This function returns TRUE when the PERFORMANCE_LIBRARY_PROPERTY_MEASUREMENT_ENABLED bit of PcdPerformanceLibraryPropertyMask is set
  and the Type disable bit in PcdPerformanceLibraryPropertyMask is not set.

  @param Type        - Type of the performance measurement entry.

  @retval TRUE         The performance measurement can be logged.
  @retval FALSE        The performance measurement can NOT be logged.

**/
BOOLEAN
EFIAPI
LogPerformanceMeasurementEnabled (
  IN  CONST UINTN  Type
  )
{
  //
  // When Performance measurement is enabled and the type is not filtered, the performance can be logged.
  //
  if (PerformanceMeasurementEnabled () && ((PcdGet8 (PcdPerformanceLibraryPropertyMask) & Type) == 0)) {
    return TRUE;
  }

  return FALSE;
}
