/** @file
  The internal header file includes the common header files, defines
  internal structure and functions used by MmCore module.

  Copyright (c) 2009 - 2014, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2016 - 2021, Arm Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _MM_CORE_H_
#define _MM_CORE_H_

#include <PiMm.h>
#include <StandaloneMm.h>

#include <Protocol/DxeMmReadyToLock.h>
#include <Protocol/MmReadyToLock.h>
#include <Protocol/MmEndOfDxe.h>
#include <Protocol/MmCommunication2.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/MmConfiguration.h>

#include <Guid/Apriori.h>
#include <Guid/EventGroup.h>
#include <Guid/EventLegacyBios.h>
#include <Guid/ZeroGuid.h>
#include <Guid/MemoryProfile.h>
#include <Guid/HobList.h>
#include <Guid/MmFvDispatch.h>
#include <Guid/MmramMemoryReserve.h>
#include <Guid/MmCommBuffer.h>
#include <Guid/PiSmmMemoryAttributesTable.h>

#include <Library/StandaloneMmCoreEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeCoffLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/HobPrintLib.h>
#include <Library/ImagePropertiesRecordLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/StandaloneMmMemLib.h>
#include <Library/HobLib.h>
#include <Library/PerformanceLib.h>

#include "StandaloneMmCorePrivateData.h"

extern EFI_MM_SYSTEM_TABLE  *mMemoryAllocationMmst;

//
// Used to build a table of MMI Handlers that the MM Core registers
//
typedef struct {
  EFI_MM_HANDLER_ENTRY_POINT    Handler;
  EFI_GUID                      *HandlerType;
  EFI_HANDLE                    DispatchHandle;
  BOOLEAN                       UnRegister;
} MM_CORE_MMI_HANDLERS;

//
// Structure for recording the state of an MM Driver
//
#define EFI_MM_DRIVER_ENTRY_SIGNATURE  SIGNATURE_32('s', 'd','r','v')

typedef struct {
  UINTN                         Signature;
  LIST_ENTRY                    Link;               // mDriverList

  LIST_ENTRY                    ScheduledLink;      // mScheduledQueue

  EFI_FIRMWARE_VOLUME_HEADER    *FwVolHeader;
  EFI_GUID                      FileName;
  VOID                          *Pe32Data;
  UINTN                         Pe32DataSize;

  VOID                          *Depex;
  UINTN                         DepexSize;

  BOOLEAN                       Before;
  BOOLEAN                       After;
  EFI_GUID                      BeforeAfterGuid;

  BOOLEAN                       Dependent;
  BOOLEAN                       Scheduled;
  BOOLEAN                       Initialized;
  BOOLEAN                       DepexProtocolError;

  EFI_HANDLE                    ImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL     LoadedImage;
  //
  // Image EntryPoint in MMRAM
  //
  PHYSICAL_ADDRESS              ImageEntryPoint;
  //
  // Image Buffer in MMRAM
  //
  PHYSICAL_ADDRESS              ImageBuffer;
  //
  // Image Page Number
  //
  UINTN                         NumberOfPage;
} EFI_MM_DRIVER_ENTRY;

#define EFI_HANDLE_SIGNATURE  SIGNATURE_32('h','n','d','l')

///
/// IHANDLE - contains a list of protocol handles
///
typedef struct {
  UINTN         Signature;
  /// All handles list of IHANDLE
  LIST_ENTRY    AllHandles;
  /// List of PROTOCOL_INTERFACE's for this handle
  LIST_ENTRY    Protocols;
  UINTN         LocateRequest;
} IHANDLE;

#define ASSERT_IS_HANDLE(a)  ASSERT((a)->Signature == EFI_HANDLE_SIGNATURE)

#define PROTOCOL_ENTRY_SIGNATURE  SIGNATURE_32('p','r','t','e')

///
/// PROTOCOL_ENTRY - each different protocol has 1 entry in the protocol
/// database.  Each handler that supports this protocol is listed, along
/// with a list of registered notifies.
///
typedef struct {
  UINTN         Signature;
  /// Link Entry inserted to mProtocolDatabase
  LIST_ENTRY    AllEntries;
  /// ID of the protocol
  EFI_GUID      ProtocolID;
  /// All protocol interfaces
  LIST_ENTRY    Protocols;
  /// Registered notification handlers
  LIST_ENTRY    Notify;
} PROTOCOL_ENTRY;

#define PROTOCOL_INTERFACE_SIGNATURE  SIGNATURE_32('p','i','f','c')

///
/// PROTOCOL_INTERFACE - each protocol installed on a handle is tracked
/// with a protocol interface structure
///
typedef struct {
  UINTN             Signature;
  /// Link on IHANDLE.Protocols
  LIST_ENTRY        Link;
  /// Back pointer
  IHANDLE           *Handle;
  /// Link on PROTOCOL_ENTRY.Protocols
  LIST_ENTRY        ByProtocol;
  /// The protocol ID
  PROTOCOL_ENTRY    *Protocol;
  /// The interface value
  VOID              *Interface;
} PROTOCOL_INTERFACE;

#define PROTOCOL_NOTIFY_SIGNATURE  SIGNATURE_32('p','r','t','n')

///
/// PROTOCOL_NOTIFY - used for each register notification for a protocol
///
typedef struct {
  UINTN               Signature;
  PROTOCOL_ENTRY      *Protocol;
  /// All notifications for this protocol
  LIST_ENTRY          Link;
  /// Notification function
  EFI_MM_NOTIFY_FN    Function;
  /// Last position notified
  LIST_ENTRY          *Position;
} PROTOCOL_NOTIFY;

//
// MM Core Global Variables
//
extern EFI_MM_SYSTEM_TABLE  gMmCoreMmst;
extern LIST_ENTRY           gHandleList;
extern BOOLEAN              mMmEntryPointRegistered;

/**
  Called to initialize the memory service.

  @param   MmramRangeCount       Number of MMRAM Regions
  @param   MmramRanges           Pointer to MMRAM Descriptors

**/
VOID
MmInitializeMemoryServices (
  IN UINTN                 MmramRangeCount,
  IN EFI_MMRAM_DESCRIPTOR  *MmramRanges
  );

/**
  The MmInstallConfigurationTable() function is used to maintain the list
  of configuration tables that are stored in the System Management System
  Table.  The list is stored as an array of (GUID, Pointer) pairs.  The list
  must be allocated from pool memory with PoolType set to EfiRuntimeServicesData.

  @param  SystemTable      A pointer to the MM System Table (SMST).
  @param  Guid             A pointer to the GUID for the entry to add, update, or remove.
  @param  Table            A pointer to the buffer of the table to add.
  @param  TableSize        The size of the table to install.

  @retval EFI_SUCCESS           The (Guid, Table) pair was added, updated, or removed.
  @retval EFI_INVALID_PARAMETER Guid is not valid.
  @retval EFI_NOT_FOUND         An attempt was made to delete a non-existent entry.
  @retval EFI_OUT_OF_RESOURCES  There is not enough memory available to complete the operation.

**/
EFI_STATUS
EFIAPI
MmInstallConfigurationTable (
  IN  CONST EFI_MM_SYSTEM_TABLE  *SystemTable,
  IN  CONST EFI_GUID             *Guid,
  IN  VOID                       *Table,
  IN  UINTN                      TableSize
  );

/**
  Wrapper function to MmInstallProtocolInterfaceNotify.  This is the public API which
  Calls the private one which contains a BOOLEAN parameter for notifications

  @param  UserHandle             The handle to install the protocol handler on,
                                 or NULL if a new handle is to be allocated
  @param  Protocol               The protocol to add to the handle
  @param  InterfaceType          Indicates whether Interface is supplied in
                                 native form.
  @param  Interface              The interface for the protocol being added

  @return Status code

**/
EFI_STATUS
EFIAPI
MmInstallProtocolInterface (
  IN OUT EFI_HANDLE      *UserHandle,
  IN EFI_GUID            *Protocol,
  IN EFI_INTERFACE_TYPE  InterfaceType,
  IN VOID                *Interface
  );

/**
  Allocates pages from the memory map.

  @param  Type                   The type of allocation to perform
  @param  MemoryType             The type of memory to turn the allocated pages
                                 into
  @param  NumberOfPages          The number of pages to allocate
  @param  Memory                 A pointer to receive the base allocated memory
                                 address

  @retval EFI_INVALID_PARAMETER  Parameters violate checking rules defined in spec.
  @retval EFI_NOT_FOUND          Could not allocate pages match the requirement.
  @retval EFI_OUT_OF_RESOURCES   No enough pages to allocate.
  @retval EFI_SUCCESS            Pages successfully allocated.

**/
EFI_STATUS
EFIAPI
MmAllocatePages (
  IN      EFI_ALLOCATE_TYPE     Type,
  IN      EFI_MEMORY_TYPE       MemoryType,
  IN      UINTN                 NumberOfPages,
  OUT     EFI_PHYSICAL_ADDRESS  *Memory
  );

/**
  Allocates pages from the memory map.

  @param  Type                   The type of allocation to perform
  @param  MemoryType             The type of memory to turn the allocated pages
                                 into
  @param  NumberOfPages          The number of pages to allocate
  @param  Memory                 A pointer to receive the base allocated memory
                                 address

  @retval EFI_INVALID_PARAMETER  Parameters violate checking rules defined in spec.
  @retval EFI_NOT_FOUND          Could not allocate pages match the requirement.
  @retval EFI_OUT_OF_RESOURCES   No enough pages to allocate.
  @retval EFI_SUCCESS            Pages successfully allocated.

**/
EFI_STATUS
EFIAPI
MmInternalAllocatePages (
  IN      EFI_ALLOCATE_TYPE     Type,
  IN      EFI_MEMORY_TYPE       MemoryType,
  IN      UINTN                 NumberOfPages,
  OUT     EFI_PHYSICAL_ADDRESS  *Memory
  );

/**
  Frees previous allocated pages.

  @param  Memory                 Base address of memory being freed
  @param  NumberOfPages          The number of pages to free

  @retval EFI_NOT_FOUND          Could not find the entry that covers the range
  @retval EFI_INVALID_PARAMETER  Address not aligned, Address is zero or NumberOfPages is zero.
  @return EFI_SUCCESS            Pages successfully freed.

**/
EFI_STATUS
EFIAPI
MmFreePages (
  IN      EFI_PHYSICAL_ADDRESS  Memory,
  IN      UINTN                 NumberOfPages
  );

/**
  Frees previous allocated pages.

  @param  Memory                 Base address of memory being freed
  @param  NumberOfPages          The number of pages to free

  @retval EFI_NOT_FOUND          Could not find the entry that covers the range
  @retval EFI_INVALID_PARAMETER  Address not aligned, Address is zero or NumberOfPages is zero.
  @return EFI_SUCCESS            Pages successfully freed.

**/
EFI_STATUS
EFIAPI
MmInternalFreePages (
  IN      EFI_PHYSICAL_ADDRESS  Memory,
  IN      UINTN                 NumberOfPages
  );

/**
  Allocate pool of a particular type.

  @param  PoolType               Type of pool to allocate
  @param  Size                   The amount of pool to allocate
  @param  Buffer                 The address to return a pointer to the allocated
                                 pool

  @retval EFI_INVALID_PARAMETER  PoolType not valid
  @retval EFI_OUT_OF_RESOURCES   Size exceeds max pool size or allocation failed.
  @retval EFI_SUCCESS            Pool successfully allocated.

**/
EFI_STATUS
EFIAPI
MmAllocatePool (
  IN      EFI_MEMORY_TYPE  PoolType,
  IN      UINTN            Size,
  OUT     VOID             **Buffer
  );

/**
  Allocate pool of a particular type.

  @param  PoolType               Type of pool to allocate
  @param  Size                   The amount of pool to allocate
  @param  Buffer                 The address to return a pointer to the allocated
                                 pool

  @retval EFI_INVALID_PARAMETER  PoolType not valid
  @retval EFI_OUT_OF_RESOURCES   Size exceeds max pool size or allocation failed.
  @retval EFI_SUCCESS            Pool successfully allocated.

**/
EFI_STATUS
EFIAPI
MmInternalAllocatePool (
  IN      EFI_MEMORY_TYPE  PoolType,
  IN      UINTN            Size,
  OUT     VOID             **Buffer
  );

/**
  Frees pool.

  @param  Buffer                 The allocated pool entry to free

  @retval EFI_INVALID_PARAMETER  Buffer is not a valid value.
  @retval EFI_SUCCESS            Pool successfully freed.

**/
EFI_STATUS
EFIAPI
MmFreePool (
  IN      VOID  *Buffer
  );

/**
  Frees pool.

  @param  Buffer                 The allocated pool entry to free

  @retval EFI_INVALID_PARAMETER  Buffer is not a valid value.
  @retval EFI_SUCCESS            Pool successfully freed.

**/
EFI_STATUS
EFIAPI
MmInternalFreePool (
  IN      VOID  *Buffer
  );

/**
  Installs a protocol interface into the boot services environment.

  @param  UserHandle             The handle to install the protocol handler on,
                                 or NULL if a new handle is to be allocated
  @param  Protocol               The protocol to add to the handle
  @param  InterfaceType          Indicates whether Interface is supplied in
                                 native form.
  @param  Interface              The interface for the protocol being added
  @param  Notify                 indicates whether notify the notification list
                                 for this protocol

  @retval EFI_INVALID_PARAMETER  Invalid parameter
  @retval EFI_OUT_OF_RESOURCES   No enough buffer to allocate
  @retval EFI_SUCCESS            Protocol interface successfully installed

**/
EFI_STATUS
MmInstallProtocolInterfaceNotify (
  IN OUT EFI_HANDLE      *UserHandle,
  IN EFI_GUID            *Protocol,
  IN EFI_INTERFACE_TYPE  InterfaceType,
  IN VOID                *Interface,
  IN BOOLEAN             Notify
  );

/**
  Uninstalls all instances of a protocol:interfacer from a handle.
  If the last protocol interface is remove from the handle, the
  handle is freed.

  @param  UserHandle             The handle to remove the protocol handler from
  @param  Protocol               The protocol, of protocol:interface, to remove
  @param  Interface              The interface, of protocol:interface, to remove

  @retval EFI_INVALID_PARAMETER  Protocol is NULL.
  @retval EFI_SUCCESS            Protocol interface successfully uninstalled.

**/
EFI_STATUS
EFIAPI
MmUninstallProtocolInterface (
  IN EFI_HANDLE  UserHandle,
  IN EFI_GUID    *Protocol,
  IN VOID        *Interface
  );

/**
  Queries a handle to determine if it supports a specified protocol.

  @param  UserHandle             The handle being queried.
  @param  Protocol               The published unique identifier of the protocol.
  @param  Interface              Supplies the address where a pointer to the
                                 corresponding Protocol Interface is returned.

  @return The requested protocol interface for the handle

**/
EFI_STATUS
EFIAPI
MmHandleProtocol (
  IN EFI_HANDLE  UserHandle,
  IN EFI_GUID    *Protocol,
  OUT VOID       **Interface
  );

/**
  Add a new protocol notification record for the request protocol.

  @param  Protocol               The requested protocol to add the notify
                                 registration
  @param  Function               Points to the notification function
  @param  Registration           Returns the registration record

  @retval EFI_INVALID_PARAMETER  Invalid parameter
  @retval EFI_SUCCESS            Successfully returned the registration record
                                 that has been added

**/
EFI_STATUS
EFIAPI
MmRegisterProtocolNotify (
  IN  CONST EFI_GUID    *Protocol,
  IN  EFI_MM_NOTIFY_FN  Function,
  OUT VOID              **Registration
  );

/**
  Locates the requested handle(s) and returns them in Buffer.

  @param  SearchType             The type of search to perform to locate the
                                 handles
  @param  Protocol               The protocol to search for
  @param  SearchKey              Dependant on SearchType
  @param  BufferSize             On input the size of Buffer.  On output the
                                 size of data returned.
  @param  Buffer                 The buffer to return the results in

  @retval EFI_BUFFER_TOO_SMALL   Buffer too small, required buffer size is
                                 returned in BufferSize.
  @retval EFI_INVALID_PARAMETER  Invalid parameter
  @retval EFI_SUCCESS            Successfully found the requested handle(s) and
                                 returns them in Buffer.

**/
EFI_STATUS
EFIAPI
MmLocateHandle (
  IN EFI_LOCATE_SEARCH_TYPE  SearchType,
  IN EFI_GUID                *Protocol   OPTIONAL,
  IN VOID                    *SearchKey  OPTIONAL,
  IN OUT UINTN               *BufferSize,
  OUT EFI_HANDLE             *Buffer
  );

/**
  Function returns an array of handles that support the requested protocol
  in a buffer allocated from pool. This is a version of MmLocateHandle()
  that allocates a buffer for the caller.

  @param  SearchType             Specifies which handle(s) are to be returned.
  @param  Protocol               Provides the protocol to search by.    This
                                 parameter is only valid for SearchType
                                 ByProtocol.
  @param  SearchKey              Supplies the search key depending on the
                                 SearchType.
  @param  NumberHandles          The number of handles returned in Buffer.
  @param  Buffer                 A pointer to the buffer to return the requested
                                 array of  handles that support Protocol.

  @retval EFI_SUCCESS            The result array of handles was returned.
  @retval EFI_NOT_FOUND          No handles match the search.
  @retval EFI_OUT_OF_RESOURCES   There is not enough pool memory to store the
                                 matching results.
  @retval EFI_INVALID_PARAMETER  One or more parameters are not valid.

**/
EFI_STATUS
EFIAPI
MmLocateHandleBuffer (
  IN     EFI_LOCATE_SEARCH_TYPE  SearchType,
  IN     EFI_GUID                *Protocol OPTIONAL,
  IN     VOID                    *SearchKey OPTIONAL,
  IN OUT UINTN                   *NumberHandles,
  OUT    EFI_HANDLE              **Buffer
  );

/**
  Return the first Protocol Interface that matches the Protocol GUID. If
  Registration is passed in return a Protocol Instance that was just add
  to the system. If Registration is NULL return the first Protocol Interface
  you find.

  @param  Protocol               The protocol to search for
  @param  Registration           Optional Registration Key returned from
                                 RegisterProtocolNotify()
  @param  Interface              Return the Protocol interface (instance).

  @retval EFI_SUCCESS            If a valid Interface is returned
  @retval EFI_INVALID_PARAMETER  Invalid parameter
  @retval EFI_NOT_FOUND          Protocol interface not found

**/
EFI_STATUS
EFIAPI
MmLocateProtocol (
  IN  EFI_GUID  *Protocol,
  IN  VOID      *Registration OPTIONAL,
  OUT VOID      **Interface
  );

/**
  Manage MMI of a particular type.

  @param  HandlerType    Points to the handler type or NULL for root MMI handlers.
  @param  Context        Points to an optional context buffer.
  @param  CommBuffer     Points to the optional communication buffer.
  @param  CommBufferSize Points to the size of the optional communication buffer.

  @retval EFI_SUCCESS                        Interrupt source was processed successfully but not quiesced.
  @retval EFI_INTERRUPT_PENDING              One or more MMI sources could not be quiesced.
  @retval EFI_WARN_INTERRUPT_SOURCE_PENDING  Interrupt source was not handled or quiesced.
  @retval EFI_WARN_INTERRUPT_SOURCE_QUIESCED Interrupt source was handled and quiesced.

**/
EFI_STATUS
EFIAPI
MmiManage (
  IN     CONST EFI_GUID  *HandlerType,
  IN     CONST VOID      *Context         OPTIONAL,
  IN OUT VOID            *CommBuffer      OPTIONAL,
  IN OUT UINTN           *CommBufferSize  OPTIONAL
  );

/**
  Registers a handler to execute within MM.

  @param  Handler        Handler service function pointer.
  @param  HandlerType    Points to the handler type or NULL for root MMI handlers.
  @param  DispatchHandle On return, contains a unique handle which can be used to later unregister the handler function.

  @retval EFI_SUCCESS           Handler register success.
  @retval EFI_INVALID_PARAMETER Handler or DispatchHandle is NULL.

**/
EFI_STATUS
EFIAPI
MmiHandlerRegister (
  IN   EFI_MM_HANDLER_ENTRY_POINT  Handler,
  IN   CONST EFI_GUID              *HandlerType  OPTIONAL,
  OUT  EFI_HANDLE                  *DispatchHandle
  );

/**
  Unregister a handler in MM.

  @param  DispatchHandle  The handle that was specified when the handler was registered.

  @retval EFI_SUCCESS           Handler function was successfully unregistered.
  @retval EFI_INVALID_PARAMETER DispatchHandle does not refer to a valid handle.

**/
EFI_STATUS
EFIAPI
MmiHandlerUnRegister (
  IN  EFI_HANDLE  DispatchHandle
  );

/**
  This function is the main entry point for an MM handler dispatch
  or communicate-based callback.

  @param  DispatchHandle  The unique handle assigned to this handler by MmiHandlerRegister().
  @param  Context         Points to an optional handler context which was specified when the handler was registered.
  @param  CommBuffer      A pointer to a collection of data in memory that will
                          be conveyed from a non-MM environment into an MM environment.
  @param  CommBufferSize  The size of the CommBuffer.

  @return Status Code

**/
EFI_STATUS
EFIAPI
MmDriverDispatchHandler (
  IN     EFI_HANDLE  DispatchHandle,
  IN     CONST VOID  *Context         OPTIONAL,
  IN OUT VOID        *CommBuffer      OPTIONAL,
  IN OUT UINTN       *CommBufferSize  OPTIONAL
  );

/**
  This function is the main entry point for an MM handler dispatch
  or communicate-based callback.

  @param  DispatchHandle  The unique handle assigned to this handler by MmiHandlerRegister().
  @param  Context         Points to an optional handler context which was specified when the handler was registered.
  @param  CommBuffer      A pointer to a collection of data in memory that will
                          be conveyed from a non-MM environment into an MM environment.
  @param  CommBufferSize  The size of the CommBuffer.

  @return Status Code

**/
EFI_STATUS
EFIAPI
MmExitBootServiceHandler (
  IN     EFI_HANDLE  DispatchHandle,
  IN     CONST VOID  *Context         OPTIONAL,
  IN OUT VOID        *CommBuffer      OPTIONAL,
  IN OUT UINTN       *CommBufferSize  OPTIONAL
  );

/**
  This function is the main entry point for an MM handler dispatch
  or communicate-based callback.

  @param  DispatchHandle  The unique handle assigned to this handler by MmiHandlerRegister().
  @param  Context         Points to an optional handler context which was specified when the handler was registered.
  @param  CommBuffer      A pointer to a collection of data in memory that will
                          be conveyed from a non-MM environment into an MM environment.
  @param  CommBufferSize  The size of the CommBuffer.

  @return Status Code

**/
EFI_STATUS
EFIAPI
MmReadyToBootHandler (
  IN     EFI_HANDLE  DispatchHandle,
  IN     CONST VOID  *Context         OPTIONAL,
  IN OUT VOID        *CommBuffer      OPTIONAL,
  IN OUT UINTN       *CommBufferSize  OPTIONAL
  );

/**
  This function is the main entry point for an MM handler dispatch
  or communicate-based callback.

  @param  DispatchHandle  The unique handle assigned to this handler by MmiHandlerRegister().
  @param  Context         Points to an optional handler context which was specified when the handler was registered.
  @param  CommBuffer      A pointer to a collection of data in memory that will
                          be conveyed from a non-MM environment into an MM environment.
  @param  CommBufferSize  The size of the CommBuffer.

  @return Status Code

**/
EFI_STATUS
EFIAPI
MmReadyToLockHandler (
  IN     EFI_HANDLE  DispatchHandle,
  IN     CONST VOID  *Context         OPTIONAL,
  IN OUT VOID        *CommBuffer      OPTIONAL,
  IN OUT UINTN       *CommBufferSize  OPTIONAL
  );

/**
  Software MMI handler that is called when the EndOfPei event is signaled.
  This function installs the MM EndOfPei Protocol so MM Drivers are informed that
  EndOfPei event is signaled.

  @param  DispatchHandle  The unique handle assigned to this handler by MmiHandlerRegister().
  @param  Context         Points to an optional handler context which was specified when the handler was registered.
  @param  CommBuffer      A pointer to a collection of data in memory that will
                          be conveyed from a non-MM environment into an MM environment.
  @param  CommBufferSize  The size of the CommBuffer.

  @return Status Code

**/
EFI_STATUS
EFIAPI
MmEndOfPeiHandler (
  IN     EFI_HANDLE  DispatchHandle,
  IN     CONST VOID  *Context         OPTIONAL,
  IN OUT VOID        *CommBuffer      OPTIONAL,
  IN OUT UINTN       *CommBufferSize  OPTIONAL
  );

/**
  This function is the main entry point for an MM handler dispatch
  or communicate-based callback.

  @param  DispatchHandle  The unique handle assigned to this handler by MmiHandlerRegister().
  @param  Context         Points to an optional handler context which was specified when the handler was registered.
  @param  CommBuffer      A pointer to a collection of data in memory that will
                          be conveyed from a non-MM environment into an MM environment.
  @param  CommBufferSize  The size of the CommBuffer.

  @return Status Code

**/
EFI_STATUS
EFIAPI
MmEndOfDxeHandler (
  IN     EFI_HANDLE  DispatchHandle,
  IN     CONST VOID  *Context         OPTIONAL,
  IN OUT VOID        *CommBuffer      OPTIONAL,
  IN OUT UINTN       *CommBufferSize  OPTIONAL
  );

/**
  Place holder function until all the MM System Table Service are available.

  @param  Arg1                   Undefined
  @param  Arg2                   Undefined
  @param  Arg3                   Undefined
  @param  Arg4                   Undefined
  @param  Arg5                   Undefined

  @return EFI_NOT_AVAILABLE_YET

**/
EFI_STATUS
EFIAPI
MmEfiNotAvailableYetArg5 (
  UINTN  Arg1,
  UINTN  Arg2,
  UINTN  Arg3,
  UINTN  Arg4,
  UINTN  Arg5
  );

//
// Functions used during debug builds
//

/**
  Traverse the discovered list for any drivers that were discovered but not loaded
  because the dependency expressions evaluated to false.

**/
VOID
MmDisplayDiscoveredNotDispatched (
  VOID
  );

/**
  Add free MMRAM region for use by memory service.

  @param  MemBase                Base address of memory region.
  @param  MemLength              Length of the memory region.
  @param  Type                   Memory type.
  @param  Attributes             Memory region state.

**/
VOID
MmAddMemoryRegion (
  IN      EFI_PHYSICAL_ADDRESS  MemBase,
  IN      UINT64                MemLength,
  IN      EFI_MEMORY_TYPE       Type,
  IN      UINT64                Attributes
  );

/**
  Finds the protocol entry for the requested protocol.

  @param  Protocol               The ID of the protocol
  @param  Create                 Create a new entry if not found

  @return Protocol entry

**/
PROTOCOL_ENTRY  *
MmFindProtocolEntry (
  IN EFI_GUID  *Protocol,
  IN BOOLEAN   Create
  );

/**
  Signal event for every protocol in protocol entry.

  @param  Prot                   Protocol interface

**/
VOID
MmNotifyProtocol (
  IN PROTOCOL_INTERFACE  *Prot
  );

/**
  Finds the protocol instance for the requested handle and protocol.
  Note: This function doesn't do parameters checking, it's caller's responsibility
  to pass in valid parameters.

  @param  Handle                 The handle to search the protocol on
  @param  Protocol               GUID of the protocol
  @param  Interface              The interface for the protocol being searched

  @return Protocol instance (NULL: Not found)

**/
PROTOCOL_INTERFACE *
MmFindProtocolInterface (
  IN IHANDLE   *Handle,
  IN EFI_GUID  *Protocol,
  IN VOID      *Interface
  );

/**
  Removes Protocol from the protocol list (but not the handle list).

  @param  Handle                 The handle to remove protocol on.
  @param  Protocol               GUID of the protocol to be moved
  @param  Interface              The interface of the protocol

  @return Protocol Entry

**/
PROTOCOL_INTERFACE *
MmRemoveInterfaceFromProtocol (
  IN IHANDLE   *Handle,
  IN EFI_GUID  *Protocol,
  IN VOID      *Interface
  );

/**
  This is the POSTFIX version of the dependency evaluator.  This code does
  not need to handle Before or After, as it is not valid to call this
  routine in this case. POSTFIX means all the math is done on top of the stack.

  @param  DriverEntry           DriverEntry element to update.

  @retval TRUE                  If driver is ready to run.
  @retval FALSE                 If driver is not ready to run or some fatal error
                                was found.

**/
BOOLEAN
MmIsSchedulable (
  IN  EFI_MM_DRIVER_ENTRY  *DriverEntry
  );

/**
  Dump MMRAM information.

**/
VOID
DumpMmramInfo (
  VOID
  );

/**
  Given the pointer to the Firmware Volume Header find the
  MM driver and return its PE32 image.

  @param [in] FwVolHeader   Pointer to memory mapped FV
  @param [in] Depth         Nesting depth of encapsulation sections. Callers
                            different from MmCoreFfsFindMmDriver() are
                            responsible for passing in a zero Depth.

  @retval  EFI_SUCCESS            Success.
  @retval  EFI_INVALID_PARAMETER  Invalid parameter.
  @retval  EFI_NOT_FOUND          Could not find section data.
  @retval  EFI_OUT_OF_RESOURCES   Out of resources.
  @retval  EFI_VOLUME_CORRUPTED   Firmware volume is corrupted.
  @retval  EFI_UNSUPPORTED        Operation not supported.
  @retval  EFI_ABORTED            Recursion aborted because Depth has been
                                  greater than or equal to
                                  PcdFwVolMmMaxEncapsulationDepth.

**/
EFI_STATUS
MmCoreFfsFindMmDriver (
  IN  EFI_FIRMWARE_VOLUME_HEADER  *FwVolHeader,
  IN  UINT32                      Depth
  );

#define NEXT_MEMORY_DESCRIPTOR(MemoryDescriptor, Size) \
  ((EFI_MEMORY_DESCRIPTOR *)((UINT8 *)(MemoryDescriptor) + (Size)))

/**
  Initialize MemoryAttributesTable support.
**/
VOID
EFIAPI
MmCoreInitializeMemoryAttributesTable (
  VOID
  );

/**
  This function returns a copy of the current memory map. The map is an array of
  memory descriptors, each of which describes a contiguous block of memory.

  @param[in, out]  MemoryMapSize          A pointer to the size, in bytes, of the
                                          MemoryMap buffer. On input, this is the size of
                                          the buffer allocated by the caller.  On output,
                                          it is the size of the buffer returned by the
                                          firmware  if the buffer was large enough, or the
                                          size of the buffer needed  to contain the map if
                                          the buffer was too small.
  @param[in, out]  MemoryMap              A pointer to the buffer in which firmware places
                                          the current memory map.
  @param[out]      MapKey                 A pointer to the location in which firmware
                                          returns the key for the current memory map.
  @param[out]      DescriptorSize         A pointer to the location in which firmware
                                          returns the size, in bytes, of an individual
                                          EFI_MEMORY_DESCRIPTOR.
  @param[out]      DescriptorVersion      A pointer to the location in which firmware
                                          returns the version number associated with the
                                          EFI_MEMORY_DESCRIPTOR.

  @retval EFI_SUCCESS            The memory map was returned in the MemoryMap
                                 buffer.
  @retval EFI_BUFFER_TOO_SMALL   The MemoryMap buffer was too small. The current
                                 buffer size needed to hold the memory map is
                                 returned in MemoryMapSize.
  @retval EFI_INVALID_PARAMETER  One of the parameters has an invalid value.

**/
EFI_STATUS
EFIAPI
MmCoreGetMemoryMap (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  OUT UINTN                     *MapKey,
  OUT UINTN                     *DescriptorSize,
  OUT UINT32                    *DescriptorVersion
  );

/**
  This is the main Dispatcher for MM and it exits when there are no more
  drivers to run. Drain the mScheduledQueue and load and start a PE
  image for each driver. Search the mDiscoveredList to see if any driver can
  be placed on the mScheduledQueue. If no drivers are placed on the
  mScheduledQueue exit the function.

  @retval EFI_SUCCESS           All of the MM Drivers that could be dispatched
                                have been run and the MM Entry Point has been
                                registered.
  @retval EFI_NOT_READY         The MM Driver that registered the MM Entry Point
                                was just dispatched.
  @retval EFI_NOT_FOUND         There are no MM Drivers available to be dispatched.
  @retval EFI_ALREADY_STARTED   The MM Dispatcher is already running

**/
EFI_STATUS
MmDispatcher (
  VOID
  );

/**
  Dispatch Standalone MM FVs.
  The FVs will be shadowed into MMRAM, caller is responsible for calling
  MmFreeShadowedFvs() to free the shadowed MM FVs.

**/
VOID
MmDispatchFvs (
  VOID
  );

/**
  Free the shadowed MM FVs.

**/
VOID
MmFreeShadowedFvs (
  VOID
  );

#endif
