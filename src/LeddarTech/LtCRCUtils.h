// ****************************************************************************
/*!
Module:     Utilities module.

\file       LtCRCUtils.h

\brief      Interface of crc utilities module.

\author     Frédéric Parent
\since      Dec 18, 2012

\copyright (c) 2012-2016 LeddarTech Inc. All rights reserved.
*/
// ***************************************************************************

#pragma once


//*****************************************************************************
//*************** Header Includes *********************************************
//*****************************************************************************

// Standard C libraries include files
#include <stdint.h>
#include <cstring>

#include "LtDefines.h"

//*****************************************************************************
//*************** Constants and Macros ****************************************
//*****************************************************************************

#ifndef CRC_CODE_SIZE_OPTIMIZED
#define CRC_CODE_SIZE_OPTIMIZED                         0   // Set to 1 for code size optimization or to 0 for execution optimization.
#endif

/// \def    CRCUTILS_CRC16_INIT_VALUE
/// \brief  Start value to use with \ref LeddarUtils::LtCRCUtils::Crc16 function.
#define CRCUTILS_CRC16_INIT_VALUE        0xFFFFU

/// \def    CRCUTILS_CRC16CCIT_INIT_VALUE
/// \brief  Start value to use with \ref LeddarUtils::LtCRCUtils::Crc16Ccitt function.
#define CRCUTILS_CRC16CCIT_INIT_VALUE    0x0000U

/// \def    CRCUTILS_CRC32_INIT_VALUE
/// \brief  Start value to use with \ref LeddarUtils::LtCRCUtils::Crc32 function.
#define CRCUTILS_CRC32_INIT_VALUE          0x00000000L

namespace LeddarUtils
{
    namespace LtCRCUtils
    {
        //*****************************************************************************
        //*************** Public Function Declarations ********************************
        //*****************************************************************************

        uint32_t Crc32( uint32_t aInitialCrc32Value, const void *aPtr, size_t aPtrSize );
        uint16_t Crc16( uint16_t aInitialCrc16Value, const void *aPtr, size_t aPtrSize );
        uint16_t Crc16Ccitt( uint16_t aInitialCrc16Value, const void *aData, size_t aPtrSize, uint8_t aByteSwapFlag );
        uint16_t Chksum16( const void *pPtr, size_t ptrSize );
        uint16_t TwoComplChksum16( const void *pPtr, size_t ptrSize );
        uint16_t ComputeCRC16( const uint8_t *aData, size_t aLength );
    }
}

