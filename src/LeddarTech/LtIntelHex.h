// ****************************************************************************
/*!
Module:     Melexis SPI, Intel HEX manager

Platform:   Win32 - Win64

\file       LtIntelHex.h

\brief      Manages Intel HEX files.

\author     Jean-F. Bernier

\since      2015-12-11
*/
// ***************************************************************************

#pragma once
#include <stdint.h>
#include <string.h>
#include <istream>

//*****************************************************************************
//*************** Data Type Definitions ***************************************
//*****************************************************************************

namespace IntelHEX
{

    /// \struct IntelHexMem
    /// \brief  Holds data of an Intel HEX file memory.
    struct IntelHexMem
    {
        uint16_t start;       // Start address of the written memory block
        uint16_t end;         // Last address of the written memory block
        uint16_t nByte;       // Number of bytes written between start and end
        uint8_t  mem[ 65536 ];  // The memory block
        uint8_t  cnt[ 65536 ];  // Write counts in each memory cell

        IntelHexMem()
        {
            memset( mem, 0, sizeof( mem ) );
            memset( cnt, 0, sizeof( cnt ) );
            start = 0;
            end = 0;
            nByte = 0;
        }
    };

    /// \struct IntelHex
    /// \brief  Holds data of an Intel HEX record.
    struct IntelHex
    {
        uint8_t  count;       // Number of data bytes
        uint16_t addr;        // Address
        uint8_t  type;        // Record type, can be a IHexType
        uint8_t  data[256];   // Raw data bytes
        uint8_t cksum;       // Checksum value
    };

    /// \enum   IHexType
    /// \brief  Intel HEX codes
    enum IHexType
    {
        IHEX_DATA = 0,  // Data field
        IHEX_EOF,       // End-of-file
        IHEX_ESA,       // Extended segment address
        IHEX_SSA,       // Start segment address
        IHEX_ELA,       // Extended linear address
        IHEX_SLA        // Start linear address
    };


    //*****************************************************************************
    //*************** Public Functions ********************************************
    //*****************************************************************************

    int IHEX_Swap( IntelHexMem &mem );
    int IHEX_Load( const char *aPath, IntelHexMem &aMem );
    int IHEX_Load( std::istream& aStream, IntelHexMem &aMem );
    int IHEX_LoadFromBuffer( const uint8_t *aBuffer, uint32_t aSize, IntelHEX::IntelHexMem &aMem );
    int IHEX_Parse(const char *line, IntelHEX::IntelHex &hex);

}