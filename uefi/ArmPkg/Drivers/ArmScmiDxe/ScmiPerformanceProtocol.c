/** @file

  Copyright (c) 2017-2023, Arm Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  System Control and Management Interface V3.2, latest version at:
  - https://developer.arm.com/documentation/den0056/latest/

**/

#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/ArmScmiPerformanceProtocol.h>

#include "ArmScmiPerformanceProtocolPrivate.h"
#include "ScmiPrivate.h"

/** Return version of the performance management protocol supported by SCP.
   firmware.

  @param[in]  This      A Pointer to SCMI_PERFORMANCE_PROTOCOL Instance.

  @param[out] Version   Version of the supported SCMI performance management
                        protocol.

  @retval EFI_SUCCESS       The version is returned.
  @retval EFI_DEVICE_ERROR  SCP returns an SCMI error.
  @retval !(EFI_SUCCESS)    Other errors.
**/
STATIC
EFI_STATUS
PerformanceGetVersion (
  IN  SCMI_PERFORMANCE_PROTOCOL  *This,
  OUT UINT32                     *Version
  )
{
  return ScmiGetProtocolVersion (ScmiProtocolIdPerformance, Version);
}

/** Return protocol attributes of the performance management protocol.

  @param[in] This         A Pointer to SCMI_PERFORMANCE_PROTOCOL Instance.

  @param[out] Attributes  Protocol attributes.

  @retval EFI_SUCCESS       Protocol attributes are returned.
  @retval EFI_DEVICE_ERROR  SCP returns an SCMI error.
  @retval !(EFI_SUCCESS)    Other errors.
**/
STATIC
EFI_STATUS
PerformanceGetAttributes (
  IN  SCMI_PERFORMANCE_PROTOCOL             *This,
  OUT SCMI_PERFORMANCE_PROTOCOL_ATTRIBUTES  *Attributes
  )
{
  EFI_STATUS  Status;
  UINT32      *ReturnValues;

  Status = ScmiGetProtocolAttributes (
             ScmiProtocolIdPerformance,
             &ReturnValues
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CopyMem (
    Attributes,
    ReturnValues,
    sizeof (SCMI_PERFORMANCE_PROTOCOL_ATTRIBUTES)
    );

  return EFI_SUCCESS;
}

/** Return performance domain attributes.

  @param[in]  This        A Pointer to SCMI_PERFORMANCE_PROTOCOL Instance.
  @param[in]  DomainId    Identifier for the performance domain.

  @param[out] Attributes  Performance domain attributes.

  @retval EFI_SUCCESS       Domain attributes are returned.
  @retval EFI_DEVICE_ERROR  SCP returns an SCMI error.
  @retval !(EFI_SUCCESS)    Other errors.
**/
STATIC
EFI_STATUS
PerformanceDomainAttributes (
  IN  SCMI_PERFORMANCE_PROTOCOL           *This,
  IN  UINT32                              DomainId,
  OUT SCMI_PERFORMANCE_DOMAIN_ATTRIBUTES  *DomainAttributes
  )
{
  EFI_STATUS    Status;
  UINT32        *MessageParams;
  UINT32        *ReturnValues;
  UINT32        PayloadLength;
  SCMI_COMMAND  Cmd;

  Status = ScmiCommandGetPayload (&MessageParams);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *MessageParams = DomainId;

  Cmd.ProtocolId = ScmiProtocolIdPerformance;
  Cmd.MessageId  = ScmiMessageIdPerformanceDomainAttributes;

  PayloadLength = sizeof (DomainId);

  Status = ScmiCommandExecute (
             &Cmd,
             &PayloadLength,
             &ReturnValues
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CopyMem (
    DomainAttributes,
    ReturnValues,
    sizeof (SCMI_PERFORMANCE_DOMAIN_ATTRIBUTES)
    );

  return EFI_SUCCESS;
}

/** Return list of performance domain levels of a given domain.

  @param[in] This        A Pointer to SCMI_PERFORMANCE_PROTOCOL Instance.
  @param[in] DomainId    Identifier for the performance domain.

  @param[out] NumLevels   Total number of levels a domain can support.

  @param[in,out]  LevelArraySize Size of the performance level array.

  @param[out] LevelArray   Array of the performance levels.

  @retval EFI_SUCCESS          Domain levels are returned.
  @retval EFI_DEVICE_ERROR     SCP returns an SCMI error.
  @retval EFI_BUFFER_TOO_SMALL LevelArraySize is too small for the result.
                               It has been updated to the size needed.
  @retval !(EFI_SUCCESS)       Other errors.
**/
STATIC
EFI_STATUS
PerformanceDescribeLevels (
  IN     SCMI_PERFORMANCE_PROTOCOL  *This,
  IN     UINT32                     DomainId,
  OUT    UINT32                     *NumLevels,
  IN OUT UINT32                     *LevelArraySize,
  OUT    SCMI_PERFORMANCE_LEVEL     *LevelArray
  )
{
  EFI_STATUS    Status;
  UINT32        PayloadLength;
  SCMI_COMMAND  Cmd;
  UINT32        *MessageParams1;
  UINT32        *MessageParams2;
  UINT32        LevelIndex;
  UINT32        RequiredSize;
  UINT32        LevelNo;
  UINT32        ReturnNumLevels;
  UINT32        ReturnRemainNumLevels;

  PERF_DESCRIBE_LEVELS  *Levels;

  Status = ScmiCommandGetPayload (&MessageParams1);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  LevelIndex   = 0;
  RequiredSize = 0;

  MessageParams2 = MessageParams1 + 1;

  Cmd.ProtocolId = ScmiProtocolIdPerformance;
  Cmd.MessageId  = ScmiMessageIdPerformanceDescribeLevels;

  do {
    *MessageParams1 = DomainId;
    *MessageParams2 = LevelIndex;

    // Note, PayloadLength is an IN/OUT parameter.
    PayloadLength = sizeof (DomainId) + sizeof (LevelIndex);

    Status = ScmiCommandExecute (
               &Cmd,
               &PayloadLength,
               (UINT32 **)&Levels
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    ReturnNumLevels       = NUM_PERF_LEVELS (Levels->NumLevels);
    ReturnRemainNumLevels = NUM_REMAIN_PERF_LEVELS (Levels->NumLevels);

    if (RequiredSize == 0) {
      *NumLevels = ReturnNumLevels + ReturnRemainNumLevels;

      RequiredSize =  (*NumLevels) * sizeof (SCMI_PERFORMANCE_LEVEL);
      if (RequiredSize > (*LevelArraySize)) {
        // Update LevelArraySize with required size.
        *LevelArraySize = RequiredSize;
        return EFI_BUFFER_TOO_SMALL;
      }
    }

    for (LevelNo = 0; LevelNo < ReturnNumLevels; LevelNo++) {
      CopyMem (
        &LevelArray[LevelIndex++],
        &Levels->PerfLevel[LevelNo],
        sizeof (SCMI_PERFORMANCE_LEVEL)
        );
    }
  } while (ReturnRemainNumLevels != 0);

  *LevelArraySize = RequiredSize;

  return EFI_SUCCESS;
}

/** Set performance limits of a domain.

  @param[in] This        A Pointer to SCMI_PERFORMANCE_PROTOCOL Instance.
  @param[in] DomainId    Identifier for the performance domain.
  @param[in] Limit       Performance limit to set.

  @retval EFI_SUCCESS          Performance limits set successfully.
  @retval EFI_DEVICE_ERROR     SCP returns an SCMI error.
  @retval !(EFI_SUCCESS)       Other errors.
**/
EFI_STATUS
PerformanceLimitsSet (
  IN SCMI_PERFORMANCE_PROTOCOL  *This,
  IN UINT32                     DomainId,
  IN SCMI_PERFORMANCE_LIMITS    *Limits
  )
{
  EFI_STATUS    Status;
  UINT32        PayloadLength;
  SCMI_COMMAND  Cmd;
  UINT32        *MessageParams;

  Status = ScmiCommandGetPayload (&MessageParams);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *MessageParams++ = DomainId;
  *MessageParams++ = Limits->RangeMax;
  *MessageParams   = Limits->RangeMin;

  Cmd.ProtocolId = ScmiProtocolIdPerformance;
  Cmd.MessageId  = ScmiMessageIdPerformanceLimitsSet;

  PayloadLength = sizeof (DomainId) + sizeof (SCMI_PERFORMANCE_LIMITS);

  Status = ScmiCommandExecute (
             &Cmd,
             &PayloadLength,
             NULL
             );

  return Status;
}

/** Get performance limits of a domain.

  @param[in]  This        A Pointer to SCMI_PERFORMANCE_PROTOCOL Instance.
  @param[in]  DomainId    Identifier for the performance domain.

  @param[out] Limit       Performance Limits of the domain.

  @retval EFI_SUCCESS          Performance limits are returned.
  @retval EFI_DEVICE_ERROR     SCP returns an SCMI error.
  @retval !(EFI_SUCCESS)       Other errors.
**/
EFI_STATUS
PerformanceLimitsGet (
  SCMI_PERFORMANCE_PROTOCOL  *This,
  UINT32                     DomainId,
  SCMI_PERFORMANCE_LIMITS    *Limits
  )
{
  EFI_STATUS    Status;
  UINT32        PayloadLength;
  SCMI_COMMAND  Cmd;
  UINT32        *MessageParams;

  SCMI_PERFORMANCE_LIMITS  *ReturnValues;

  Status = ScmiCommandGetPayload (&MessageParams);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *MessageParams = DomainId;

  Cmd.ProtocolId = ScmiProtocolIdPerformance;
  Cmd.MessageId  = ScmiMessageIdPerformanceLimitsGet;

  PayloadLength = sizeof (DomainId);

  Status = ScmiCommandExecute (
             &Cmd,
             &PayloadLength,
             (UINT32 **)&ReturnValues
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Limits->RangeMax = ReturnValues->RangeMax;
  Limits->RangeMin = ReturnValues->RangeMin;

  return EFI_SUCCESS;
}

/** Set performance level of a domain.

  @param[in]  This        A Pointer to SCMI_PERFORMANCE_PROTOCOL Instance.
  @param[in]  DomainId    Identifier for the performance domain.
  @param[in]  Level       Performance level of the domain.

  @retval EFI_SUCCESS          Performance level set successfully.
  @retval EFI_DEVICE_ERROR     SCP returns an SCMI error.
  @retval !(EFI_SUCCESS)       Other errors.
**/
EFI_STATUS
PerformanceLevelSet (
  IN SCMI_PERFORMANCE_PROTOCOL  *This,
  IN UINT32                     DomainId,
  IN UINT32                     Level
  )
{
  EFI_STATUS    Status;
  UINT32        PayloadLength;
  SCMI_COMMAND  Cmd;
  UINT32        *MessageParams;

  Status = ScmiCommandGetPayload (&MessageParams);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *MessageParams++ = DomainId;
  *MessageParams   = Level;

  Cmd.ProtocolId = ScmiProtocolIdPerformance;
  Cmd.MessageId  = ScmiMessageIdPerformanceLevelSet;

  PayloadLength = sizeof (DomainId) + sizeof (Level);

  Status = ScmiCommandExecute (
             &Cmd,
             &PayloadLength,
             NULL
             );

  return Status;
}

/** Get performance level of a domain.

  @param[in]  This        A Pointer to SCMI_PERFORMANCE_PROTOCOL Instance.
  @param[in]  DomainId    Identifier for the performance domain.

  @param[out] Level       Performance level of the domain.

  @retval EFI_SUCCESS          Performance level got successfully.
  @retval EFI_DEVICE_ERROR     SCP returns an SCMI error.
  @retval !(EFI_SUCCESS)       Other errors.
**/
EFI_STATUS
PerformanceLevelGet (
  IN  SCMI_PERFORMANCE_PROTOCOL  *This,
  IN  UINT32                     DomainId,
  OUT UINT32                     *Level
  )
{
  EFI_STATUS    Status;
  UINT32        PayloadLength;
  SCMI_COMMAND  Cmd;
  UINT32        *ReturnValues;
  UINT32        *MessageParams;

  Status = ScmiCommandGetPayload (&MessageParams);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *MessageParams = DomainId;

  Cmd.ProtocolId = ScmiProtocolIdPerformance;
  Cmd.MessageId  = ScmiMessageIdPerformanceLevelGet;

  PayloadLength = sizeof (DomainId);

  Status = ScmiCommandExecute (
             &Cmd,
             &PayloadLength,
             &ReturnValues
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *Level = *ReturnValues;

  return EFI_SUCCESS;
}

/** Discover the attributes of the FastChannel for the specified
    performance domain and the specified message.

  @param[in]  This        A Pointer to SCMI_PERFORMANCE_PROTOCOL Instance.
  @param[in]  DomainId    Identifier for the performance domain.
  @param[in]  MessageId   Message Id of the FastChannel to discover.
                          Must be one of:
                           - PERFORMANCE_LIMITS_SET
                           - PERFORMANCE_LIMITS_GET
                           - PERFORMANCE_LEVEL_SET
                           - PERFORMANCE_LEVEL_GET
  @param[out] FastChannel If success, contains the FastChannel description.

  @retval EFI_SUCCESS             Performance level got successfully.
  @retval EFI_DEVICE_ERROR        SCP returns an SCMI error.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_TIMEOUT             Time out.
  @retval EFI_UNSUPPORTED         Unsupported.
**/
EFI_STATUS
DescribeFastchannel (
  IN  SCMI_PERFORMANCE_PROTOCOL     *This,
  IN  UINT32                        DomainId,
  IN  SCMI_MESSAGE_ID_PERFORMANCE   MessageId,
  OUT SCMI_PERFORMANCE_FASTCHANNEL  *FastChannel
  )
{
  EFI_STATUS    Status;
  SCMI_COMMAND  Cmd;
  UINT32        PayloadLength;
  UINT32        *ReturnValues;
  UINT32        *MessageParams;

  if ((This == NULL)  ||
      (FastChannel == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Status = ScmiCommandGetPayload (&MessageParams);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *MessageParams++ = DomainId;
  *MessageParams   = MessageId;

  Cmd.ProtocolId = ScmiProtocolIdPerformance;
  Cmd.MessageId  = ScmiMessageIdPerformanceDescribeFastchannel;
  PayloadLength  = sizeof (DomainId) + sizeof (MessageId);

  Status = ScmiCommandExecute (
             &Cmd,
             &PayloadLength,
             &ReturnValues
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CopyMem (
    FastChannel,
    ReturnValues,
    sizeof (SCMI_PERFORMANCE_FASTCHANNEL)
    );

  return Status;
}

// Instance of the SCMI performance management protocol.
STATIC CONST SCMI_PERFORMANCE_PROTOCOL  PerformanceProtocol = {
  PerformanceGetVersion,
  PerformanceGetAttributes,
  PerformanceDomainAttributes,
  PerformanceDescribeLevels,
  PerformanceLimitsSet,
  PerformanceLimitsGet,
  PerformanceLevelSet,
  PerformanceLevelGet,
  DescribeFastchannel,
};

/** Initialize performance management protocol and install on a given Handle.

  @param[in] Handle              Handle to install performance management
                                 protocol.

  @retval EFI_SUCCESS            Performance protocol installed successfully.
**/
EFI_STATUS
ScmiPerformanceProtocolInit (
  IN EFI_HANDLE  *Handle
  )
{
  return gBS->InstallMultipleProtocolInterfaces (
                Handle,
                &gArmScmiPerformanceProtocolGuid,
                &PerformanceProtocol,
                NULL
                );
}
