/** @file
Decompressor. Algorithm Ported from OPSD code (Decomp.asm) for Efi and Tiano
compress algorithm.

Copyright (c) 2004 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

--*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Decompress.h"

//
// Decompression algorithm begins here
//
#ifndef UINT8_MAX
#define UINT8_MAX 0xff
#endif
#define BITBUFSIZ 32
#define MAXMATCH  256
#define THRESHOLD 3
#define CODE_BIT  16
#define BAD_TABLE - 1

//
// C: Char&Len Set; P: Position Set; T: exTra Set
//
#define NC      (0xff + MAXMATCH + 2 - THRESHOLD)
#define CBIT    9
#define EFIPBIT 4
#define MAXPBIT 5
#define TBIT    5
#define MAXNP ((1U << MAXPBIT) - 1)
#define NT    (CODE_BIT + 3)
#if NT > MAXNP
#define NPT NT
#else
#define NPT MAXNP
#endif

typedef struct {
  UINT8   *mSrcBase;  // Starting address of compressed data
  UINT8   *mDstBase;  // Starting address of decompressed data
  UINT32  mOutBuf;
  UINT32  mInBuf;

  UINT16  mBitCount;
  UINT32  mBitBuf;
  UINT32  mSubBitBuf;
  UINT16  mBlockSize;
  UINT32  mCompSize;
  UINT32  mOrigSize;

  UINT16  mBadTableFlag;

  UINT16  mLeft[2 * NC - 1];
  UINT16  mRight[2 * NC - 1];
  UINT8   mCLen[NC];
  UINT8   mPTLen[NPT];
  UINT16  mCTable[4096];
  UINT16  mPTTable[256];
} SCRATCH_DATA;

STATIC UINT16 mPbit = EFIPBIT;

/**
  Shift mBitBuf NumOfBits left. Read in NumOfBits of bits from source.

  @param Sd        The global scratch data
  @param NumOfBit  The number of bits to shift and read.
**/
STATIC
VOID
FillBuf (
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16        NumOfBits
  )
{
  Sd->mBitBuf = (UINT32) (((UINT64)Sd->mBitBuf) << NumOfBits);

  while (NumOfBits > Sd->mBitCount) {

    Sd->mBitBuf |= (UINT32) (((UINT64)Sd->mSubBitBuf) << (NumOfBits = (UINT16) (NumOfBits - Sd->mBitCount)));

    if (Sd->mCompSize > 0) {
      //
      // Get 1 byte into SubBitBuf
      //
      Sd->mCompSize--;
      Sd->mSubBitBuf  = 0;
      Sd->mSubBitBuf  = Sd->mSrcBase[Sd->mInBuf++];
      Sd->mBitCount   = 8;

    } else {
      //
      // No more bits from the source, just pad zero bit.
      //
      Sd->mSubBitBuf  = 0;
      Sd->mBitCount   = 8;

    }
  }

  Sd->mBitCount = (UINT16) (Sd->mBitCount - NumOfBits);
  Sd->mBitBuf |= Sd->mSubBitBuf >> Sd->mBitCount;
}

/**
  Get NumOfBits of bits out from mBitBuf. Fill mBitBuf with subsequent
  NumOfBits of bits from source. Returns NumOfBits of bits that are
  popped out.

  @param Sd            The global scratch data.
  @param NumOfBits     The number of bits to pop and read.

  @return The bits that are popped out.
**/
STATIC
UINT32
GetBits (
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16        NumOfBits
  )
{
  UINT32  OutBits;

  OutBits = (UINT32) (Sd->mBitBuf >> (BITBUFSIZ - NumOfBits));

  FillBuf (Sd, NumOfBits);

  return OutBits;
}

/**
  Creates Huffman Code mapping table according to code length array.

  @param Sd        The global scratch data
  @param NumOfChar Number of symbols in the symbol set
  @param BitLen    Code length array
  @param TableBits The width of the mapping table
  @param Table     The table

  @retval 0         - OK.
  @retval BAD_TABLE - The table is corrupted.
**/
STATIC
UINT16
MakeTable (
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16        NumOfChar,
  IN  UINT8         *BitLen,
  IN  UINT16        TableBits,
  OUT UINT16        *Table
  )
{
  UINT16  Count[17];
  UINT16  Weight[17];
  UINT16  Start[18];
  UINT16  *Pointer;
  UINT16  Index3;
  UINT16  Index;
  UINT16  Len;
  UINT16  Char;
  UINT16  JuBits;
  UINT16  Avail;
  UINT16  NextCode;
  UINT16  Mask;
  UINT16  MaxTableLength;

  for (Index = 1; Index <= 16; Index++) {
    Count[Index] = 0;
  }

  for (Index = 0; Index < NumOfChar; Index++) {
    if (BitLen[Index] > 16) {
      return (UINT16) BAD_TABLE;
    }
    Count[BitLen[Index]]++;
  }

  Start[1] = 0;

  for (Index = 1; Index <= 16; Index++) {
    Start[Index + 1] = (UINT16) (Start[Index] + (Count[Index] << (16 - Index)));
  }

  if (Start[17] != 0) {
    /*(1U << 16)*/
    return (UINT16) BAD_TABLE;
  }

  JuBits = (UINT16) (16 - TableBits);

  for (Index = 1; Index <= TableBits; Index++) {
    Start[Index] >>= JuBits;
    Weight[Index] = (UINT16) (1U << (TableBits - Index));
  }

  while (Index <= 16) {
    Weight[Index] = (UINT16) (1U << (16 - Index));
    Index++;
  }

  Index = (UINT16) (Start[TableBits + 1] >> JuBits);

  if (Index != 0) {
    Index3 = (UINT16) (1U << TableBits);
    while (Index != Index3) {
      Table[Index++] = 0;
    }
  }

  Avail = NumOfChar;
  Mask  = (UINT16) (1U << (15 - TableBits));
  MaxTableLength = (UINT16) (1U << TableBits);

  for (Char = 0; Char < NumOfChar; Char++) {

    Len = BitLen[Char];
    if (Len == 0 || Len >= 17) {
      continue;
    }

    NextCode = (UINT16) (Start[Len] + Weight[Len]);

    if (Len <= TableBits) {

      if (Start[Len] >= NextCode || NextCode > MaxTableLength){
        return (UINT16) BAD_TABLE;
      }

      for (Index = Start[Len]; Index < NextCode; Index++) {
        Table[Index] = Char;
      }

    } else {

      Index3  = Start[Len];
      Pointer = &Table[Index3 >> JuBits];
      Index   = (UINT16) (Len - TableBits);

      while (Index != 0) {
        if (*Pointer == 0) {
          Sd->mRight[Avail]                     = Sd->mLeft[Avail] = 0;
          *Pointer = Avail++;
        }

        if (Index3 & Mask) {
          Pointer = &Sd->mRight[*Pointer];
        } else {
          Pointer = &Sd->mLeft[*Pointer];
        }

        Index3 <<= 1;
        Index--;
      }

      *Pointer = Char;

    }

    Start[Len] = NextCode;
  }
  //
  // Succeeds
  //
  return 0;
}

/**
  Decodes a position value.

  @param Sd      the global scratch data

  @return The position value decoded.
**/
STATIC
UINT32
DecodeP (
  IN  SCRATCH_DATA  *Sd
  )
{
  UINT16  Val;
  UINT32  Mask;
  UINT32  Pos;

  Val = Sd->mPTTable[Sd->mBitBuf >> (BITBUFSIZ - 8)];

  if (Val >= MAXNP) {
    Mask = 1U << (BITBUFSIZ - 1 - 8);

    do {

      if (Sd->mBitBuf & Mask) {
        Val = Sd->mRight[Val];
      } else {
        Val = Sd->mLeft[Val];
      }

      Mask >>= 1;
    } while (Val >= MAXNP);
  }
  //
  // Advance what we have read
  //
  FillBuf (Sd, Sd->mPTLen[Val]);

  Pos = Val;
  if (Val > 1) {
    Pos = (UINT32) ((1U << (Val - 1)) + GetBits (Sd, (UINT16) (Val - 1)));
  }

  return Pos;
}

/**
  Reads code lengths for the Extra Set or the Position Set

  @param Sd        The global scratch data
  @param nn        Number of symbols
  @param nbit      Number of bits needed to represent nn
  @param Special   The special symbol that needs to be taken care of

  @retval 0         - OK.
  @retval BAD_TABLE - Table is corrupted.
**/
STATIC
UINT16
ReadPTLen (
  IN  SCRATCH_DATA  *Sd,
  IN  UINT16        nn,
  IN  UINT16        nbit,
  IN  UINT16        Special
  )
{
  UINT16  Number;
  UINT16  CharC;
  UINT16  Index;
  UINT32  Mask;

  assert (nn <= NPT);

  Number = (UINT16) GetBits (Sd, nbit);

  if (Number == 0) {
    CharC = (UINT16) GetBits (Sd, nbit);

    for (Index = 0; Index < 256; Index++) {
      Sd->mPTTable[Index] = CharC;
    }

    for (Index = 0; Index < nn; Index++) {
      Sd->mPTLen[Index] = 0;
    }

    return 0;
  }

  Index = 0;

  while (Index < Number && Index < NPT) {

    CharC = (UINT16) (Sd->mBitBuf >> (BITBUFSIZ - 3));

    if (CharC == 7) {
      Mask = 1U << (BITBUFSIZ - 1 - 3);
      while (Mask & Sd->mBitBuf) {
        Mask >>= 1;
        CharC += 1;
      }
    }

    FillBuf (Sd, (UINT16) ((CharC < 7) ? 3 : CharC - 3));

    Sd->mPTLen[Index++] = (UINT8) CharC;

    if (Index == Special) {
      CharC = (UINT16) GetBits (Sd, 2);
      CharC--;
      while ((INT16) (CharC) >= 0 && Index < NPT) {
        Sd->mPTLen[Index++] = 0;
        CharC--;
      }
    }
  }

  while (Index < nn && Index < NPT) {
    Sd->mPTLen[Index++] = 0;
  }

  return MakeTable (Sd, nn, Sd->mPTLen, 8, Sd->mPTTable);
}

/**
  Reads code lengths for Char&Len Set.

  @param Sd    the global scratch data
**/
STATIC
VOID
ReadCLen (
  SCRATCH_DATA  *Sd
  )
{
  UINT16  Number;
  UINT16  CharC;
  UINT16  Index;
  UINT32  Mask;

  Number = (UINT16) GetBits (Sd, CBIT);

  if (Number == 0) {
    CharC = (UINT16) GetBits (Sd, CBIT);

    for (Index = 0; Index < NC; Index++) {
      Sd->mCLen[Index] = 0;
    }

    for (Index = 0; Index < 4096; Index++) {
      Sd->mCTable[Index] = CharC;
    }

    return ;
  }

  Index = 0;
  while (Index < Number) {

    CharC = Sd->mPTTable[Sd->mBitBuf >> (BITBUFSIZ - 8)];
    if (CharC >= NT) {
      Mask = 1U << (BITBUFSIZ - 1 - 8);

      do {

        if (Mask & Sd->mBitBuf) {
          CharC = Sd->mRight[CharC];
        } else {
          CharC = Sd->mLeft[CharC];
        }

        Mask >>= 1;

      } while (CharC >= NT);
    }
    //
    // Advance what we have read
    //
    FillBuf (Sd, Sd->mPTLen[CharC]);

    if (CharC <= 2) {

      if (CharC == 0) {
        CharC = 1;
      } else if (CharC == 1) {
        CharC = (UINT16) (GetBits (Sd, 4) + 3);
      } else if (CharC == 2) {
        CharC = (UINT16) (GetBits (Sd, CBIT) + 20);
      }

      CharC--;
      while ((INT16) (CharC) >= 0) {
        Sd->mCLen[Index++] = 0;
        CharC--;
      }

    } else {

      Sd->mCLen[Index++] = (UINT8) (CharC - 2);

    }
  }

  while (Index < NC) {
    Sd->mCLen[Index++] = 0;
  }

  MakeTable (Sd, NC, Sd->mCLen, 12, Sd->mCTable);

  return ;
}

/**
  Decode a character/length value.

  @param Sd    The global scratch data.

  @return The value decoded.
**/
STATIC
UINT16
DecodeC (
  SCRATCH_DATA  *Sd
  )
{
  UINT16  Index2;
  UINT32  Mask;

  if (Sd->mBlockSize == 0) {
    //
    // Starting a new block
    //
    Sd->mBlockSize    = (UINT16) GetBits (Sd, 16);
    Sd->mBadTableFlag = ReadPTLen (Sd, NT, TBIT, 3);
    if (Sd->mBadTableFlag != 0) {
      return 0;
    }

    ReadCLen (Sd);

    Sd->mBadTableFlag = ReadPTLen (Sd, MAXNP, mPbit, (UINT16) (-1));
    if (Sd->mBadTableFlag != 0) {
      return 0;
    }
  }

  Sd->mBlockSize--;
  Index2 = Sd->mCTable[Sd->mBitBuf >> (BITBUFSIZ - 12)];

  if (Index2 >= NC) {
    Mask = 1U << (BITBUFSIZ - 1 - 12);

    do {
      if (Sd->mBitBuf & Mask) {
        Index2 = Sd->mRight[Index2];
      } else {
        Index2 = Sd->mLeft[Index2];
      }

      Mask >>= 1;
    } while (Index2 >= NC);
  }
  //
  // Advance what we have read
  //
  FillBuf (Sd, Sd->mCLen[Index2]);

  return Index2;
}

/**
  Decode the source data and put the resulting data into the destination buffer.

  @param Sd            The global scratch data
 **/
STATIC
VOID
Decode (
  SCRATCH_DATA  *Sd
  )
{
  UINT16  BytesRemain;
  UINT32  DataIdx;
  UINT16  CharC;

  BytesRemain = (UINT16) (-1);

  DataIdx     = 0;

  for (;;) {
    CharC = DecodeC (Sd);
    if (Sd->mBadTableFlag != 0) {
      return ;
    }

    if (CharC < 256) {
      //
      // Process an Original character
      //
      Sd->mDstBase[Sd->mOutBuf++] = (UINT8) CharC;
      if (Sd->mOutBuf >= Sd->mOrigSize) {
        return ;
      }

    } else {
      //
      // Process a Pointer
      //
      CharC       = (UINT16) (CharC - (UINT8_MAX + 1 - THRESHOLD));

      BytesRemain = CharC;

      DataIdx     = Sd->mOutBuf - DecodeP (Sd) - 1;

      BytesRemain--;
      while ((INT16) (BytesRemain) >= 0) {
        if (Sd->mOutBuf >= Sd->mOrigSize) {
          return ;
        }
        if (DataIdx >= Sd->mOrigSize) {
          Sd->mBadTableFlag = (UINT16) BAD_TABLE;
          return ;
        }
        Sd->mDstBase[Sd->mOutBuf++] = Sd->mDstBase[DataIdx++];

        BytesRemain--;
      }
      //
      // Once mOutBuf is fully filled, directly return
      //
      if (Sd->mOutBuf >= Sd->mOrigSize) {
        return ;
      }
    }
  }

  return ;
}

/**
  The implementation of EFI_DECOMPRESS_PROTOCOL.GetInfo().

  @param Source      The source buffer containing the compressed data.
  @param SrcSize     The size of source buffer
  @param DstSize     The size of destination buffer.
  @param ScratchSize The size of scratch buffer.

  @retval EFI_SUCCESS           - The size of destination buffer and the size of scratch buffer are successfully retrieved.
  @retval EFI_INVALID_PARAMETER - The source data is corrupted
**/
EFI_STATUS
GetInfo (
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  OUT     UINT32  *DstSize,
  OUT     UINT32  *ScratchSize
  )
{
  UINT8 *Src;
  UINT32 CompSize;

  *ScratchSize  = sizeof (SCRATCH_DATA);

  Src           = Source;
  if (SrcSize < 8) {
    return EFI_INVALID_PARAMETER;
  }

  CompSize = Src[0] + (Src[1] << 8) + (Src[2] << 16) + (Src[3] << 24);
  *DstSize = Src[4] + (Src[5] << 8) + (Src[6] << 16) + (Src[7] << 24);

  if (SrcSize < CompSize + 8 || (CompSize + 8) < 8) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

/**
  The implementation Efi and Tiano Decompress().

  @param Source      - The source buffer containing the compressed data.
  @param SrcSize     - The size of source buffer
  @param Destination - The destination buffer to store the decompressed data
  @param DstSize     - The size of destination buffer.
  @param Scratch     - The buffer used internally by the decompress routine. This  buffer is needed to store intermediate data.
  @param ScratchSize - The size of scratch buffer.

  @retval EFI_SUCCESS           - Decompression is successful
  @retval EFI_INVALID_PARAMETER - The source data is corrupted
**/
EFI_STATUS
Decompress (
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  IN OUT  VOID    *Destination,
  IN      UINT32  DstSize,
  IN OUT  VOID    *Scratch,
  IN      UINT32  ScratchSize
  )
{
  UINT32        Index;
  UINT32        CompSize;
  UINT32        OrigSize;
  EFI_STATUS    Status;
  SCRATCH_DATA  *Sd;
  UINT8         *Src;
  UINT8         *Dst;

  Status  = EFI_SUCCESS;
  Src     = Source;
  Dst     = Destination;

  if (ScratchSize < sizeof (SCRATCH_DATA)) {
    return EFI_INVALID_PARAMETER;
  }

  Sd = (SCRATCH_DATA *) Scratch;

  if (SrcSize < 8) {
    return EFI_INVALID_PARAMETER;
  }

  CompSize  = Src[0] + (Src[1] << 8) + (Src[2] << 16) + (Src[3] << 24);
  OrigSize  = Src[4] + (Src[5] << 8) + (Src[6] << 16) + (Src[7] << 24);

  if (SrcSize < CompSize + 8 || (CompSize + 8) < 8) {
    return EFI_INVALID_PARAMETER;
  }

  if (DstSize != OrigSize) {
    return EFI_INVALID_PARAMETER;
  }

  Src = Src + 8;

  for (Index = 0; Index < sizeof (SCRATCH_DATA); Index++) {
    ((UINT8 *) Sd)[Index] = 0;
  }

  Sd->mSrcBase  = Src;
  Sd->mDstBase  = Dst;
  Sd->mCompSize = CompSize;
  Sd->mOrigSize = OrigSize;

  //
  // Fill the first BITBUFSIZ bits
  //
  FillBuf (Sd, BITBUFSIZ);

  //
  // Decompress it
  //
  Decode (Sd);

  if (Sd->mBadTableFlag != 0) {
    //
    // Something wrong with the source
    //
    Status = EFI_INVALID_PARAMETER;
  }

  return Status;
}

/**
  The implementation Efi Decompress GetInfo().

  @param Source      The source buffer containing the compressed data.
  @param SrcSize     The size of source buffer
  @param DstSize     The size of destination buffer.
  @param ScratchSize The size of scratch buffer.

  @retval EFI_SUCCESS           The size of destination buffer and the size of scratch buffer are successfully retrieved.
  @retval EFI_INVALID_PARAMETER The source data is corrupted
**/
EFI_STATUS
EfiGetInfo (
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  OUT     UINT32  *DstSize,
  OUT     UINT32  *ScratchSize
  )
{
  return GetInfo (Source, SrcSize, DstSize, ScratchSize);
}

/**
  The implementation Tiano Decompress GetInfo().

  @param Source      The source buffer containing the compressed data.
  @param SrcSize     The size of source buffer
  @param DstSize     The size of destination buffer.
  @param ScratchSize The size of scratch buffer.

  @retval EFI_SUCCESS           The size of destination buffer and the size of scratch buffer are successfully retrieved.
  @retval EFI_INVALID_PARAMETER The source data is corrupted
**/
EFI_STATUS
TianoGetInfo (
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  OUT     UINT32  *DstSize,
  OUT     UINT32  *ScratchSize
  )
{
  return GetInfo (Source, SrcSize, DstSize, ScratchSize);
}

/**
  The implementation of Efi Decompress().

  @param Source      The source buffer containing the compressed data.
  @param SrcSize     The size of source buffer
  @param Destination The destination buffer to store the decompressed data
  @param DstSize     The size of destination buffer.
  @param Scratch     The buffer used internally by the decompress routine. This  buffer is needed to store intermediate data.
  @param ScratchSize The size of scratch buffer.

  @retval EFI_SUCCESS           Decompression is successful
  @retval EFI_INVALID_PARAMETER The source data is corrupted
**/
EFI_STATUS
EfiDecompress (
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  IN OUT  VOID    *Destination,
  IN      UINT32  DstSize,
  IN OUT  VOID    *Scratch,
  IN      UINT32  ScratchSize
  )
{
  mPbit = EFIPBIT;
  return Decompress (Source, SrcSize, Destination, DstSize, Scratch, ScratchSize);
}

/**
  The implementation of Tiano Decompress().

  @param Source      The source buffer containing the compressed data.
  @param SrcSize     The size of source buffer
  @param Destination The destination buffer to store the decompressed data
  @param DstSize     The size of destination buffer.
  @param Scratch     The buffer used internally by the decompress routine. This  buffer is needed to store intermediate data.
  @param ScratchSize The size of scratch buffer.

  @retval EFI_SUCCESS           Decompression is successful
  @retval EFI_INVALID_PARAMETER The source data is corrupted
**/
EFI_STATUS
TianoDecompress (
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  IN OUT  VOID    *Destination,
  IN      UINT32  DstSize,
  IN OUT  VOID    *Scratch,
  IN      UINT32  ScratchSize
  )
{
  mPbit = MAXPBIT;
  return Decompress (Source, SrcSize, Destination, DstSize, Scratch, ScratchSize);
}

EFI_STATUS
Extract (
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
     OUT  VOID    **Destination,
     OUT  UINT32  *DstSize,
  IN      UINTN   Algorithm
  )
{
  VOID          *Scratch;
  UINT32        ScratchSize;
  EFI_STATUS    Status;

  Scratch = NULL;
  Status  = EFI_SUCCESS;

  switch (Algorithm) {
  case 0:
    *Destination = (VOID *)malloc(SrcSize);
    if (*Destination != NULL) {
      memcpy(*Destination, Source, SrcSize);
    } else {
      Status = EFI_OUT_OF_RESOURCES;
    }
    break;
  case 1:
    Status = EfiGetInfo(Source, SrcSize, DstSize, &ScratchSize);
    if (Status == EFI_SUCCESS) {
      Scratch = (VOID *)malloc(ScratchSize);
      if (Scratch == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      *Destination = (VOID *)malloc(*DstSize);
      if (*Destination == NULL) {
        free (Scratch);
        return EFI_OUT_OF_RESOURCES;
      }

      Status = EfiDecompress(Source, SrcSize, *Destination, *DstSize, Scratch, ScratchSize);
    }
    break;
  case 2:
    Status = TianoGetInfo(Source, SrcSize, DstSize, &ScratchSize);
    if (Status == EFI_SUCCESS) {
      Scratch = (VOID *)malloc(ScratchSize);
      if (Scratch == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      *Destination = (VOID *)malloc(*DstSize);
      if (*Destination == NULL) {
        free (Scratch);
        return EFI_OUT_OF_RESOURCES;
      }

      Status = TianoDecompress(Source, SrcSize, *Destination, *DstSize, Scratch, ScratchSize);
    }
    break;
  default:
    Status = EFI_INVALID_PARAMETER;
  }

  if (Scratch != NULL) {
    free (Scratch);
  }

  return Status;
}


