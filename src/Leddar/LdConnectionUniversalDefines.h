// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnectionUniversalDefines.h
///
/// \brief   Base class of LdConnectionUniversalDefines
///
/// \author  Patrick Boulay
///
/// \since   October 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include <stdint.h>

///\ enum eCmd
///\brief List of commands.
typedef enum
{
    REGMAP_READ     = 0x0B,     //< Read data.
    REGMAP_WRITE    = 0x02,     //< Write data.
    REGMAP_RDSR     = 0x05,     //< Read status register.
    REGMAP_WRDIS    = 0x04,     //< Write disable.
    REGMAP_WREN     = 0x06,     //< Write enable.
    REGMAP_SWRST    = 0x99,     //< Software reset.
    REGMAP_CE       = 0xC7     //< Reset configuration to default values.
} eCmd;

///\ enum eTrnInfo
///\brief Command information tag.
typedef enum
{
    REGMAP_NO_ERR = 0,
    REGMAP_ACCESS_RIGHT_VIOLATION = 1 << 0,
    REGMAP_INVALID_ADDR = 1 << 1,
    REGMAP_CMD_NOT_FOUND = 1 << 2,
    REGMAP_WRITE_DISABLE = 1 << 3,
    REGMAP_CRC_FAILED = 1 << 4,
    REGMAP_CMD_EXEC_ERROR = 1 << 5,
    REGMAP_INVALID_PACKET = 1 << 6
} eTrnInfo;

/// \struct sTransactionCfg
/// \brief  Transaction configuration
typedef struct
{
    uint8_t  mSecureTransferEnableFlag; //< Secure transfer enable flag.
    uint8_t  mTransferMode;             //< Transfer mode (0 = free run, 1 = blocking read, 2 = partial blocking read)
    uint16_t mTransactionCrc;           //< crc of the last transaction.
    uint16_t mTransactionInfo;          //< Information about the last transaction.
    uint8_t  mReadyDeassertingData;      //< register that deasserts the ready pin (0 = trace, 1 = detection).
} sTransactionCfg;
