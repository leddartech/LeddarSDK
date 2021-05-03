/*********************************************************************
* Lattice Semiconductor Corp. Copyright 2011
* hardware.cpp
* Version 4.0
*
* This file contains interface between SSPI Embedded and the hardware.
* Depending on hardware availability, implementation may be different.
* Therefore the user is responsible to modify functions here in order
*   to use SSPI Embedded engine to drive the hardware.
***********************************************************************/

#include "M16_FPGA_hardware.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

#define WIN32_LEAN_AND_MEAN
//#ifdef WIN32
//#include <Windows.h>
//#endif
#include <stdlib.h>
#include <assert.h>
#include <thread>
#include <vector>
#include <mutex>
#include <cstring>
#undef max
#undef min

#include "LdProtocolLeddarTech.h"
#include "LdSensorM16.h"
#include "comm/LtComLeddarTechPublic.h"

#include "M16_FPGA_intrface.h"
#include "M16_FPGA_opcode.h"
#include "M16_FPGA_debug.h"

/***********************************************************************
*
* Global variables.
*
***********************************************************************/
unsigned int a_uiCheckFailedRow = 0;
unsigned int a_uiRowCount       = 0;

//LeddarConnection::LdProtocolLeddarTech *gConnection = NULL;
// Max size seen with the bitstream used during development
// was 36, so play it safe and use a little more.
static uint8_t gTransmitBuffer[60];
static int gTransmitIndex;

/***********************************************************************
*
* Debug utility functions
*
***********************************************************************/

/**********************************************************************
* There are 2 functions here:
*
* dbgu_init() - this function is responsible to initialize debug
*                   functionality.
*
* dbgu_putint() -   this function take 2 integers from SSPI Embedded
*                   processing system.  It is up to the user to take
*                   advantage of these integers.
***********************************************************************/

/***********************************************************************
* Function dbgu_init()
* Purpose: Initialize debugging unit.
*
* Return:       1 - succeed
*               0 - fail
*
* If you don't want to use the debug option, you don't need to
* implement this function.
*
************************************************************************/

/*********************************************************************
* here you may implement debug initializing function.
**********************************************************************/
int dbgu_init()
{
    return 1;
}

/***********************************************************************
* Function dbgu_putint(int debugCode, int debugCode2)
* Purpose: Return 2 integers from the core for user to run debug.
*
* If you don't want to use the debug option, you don't need to
* implement this function.
*
* 0x[debugCode][debugCode2] forms a char number that will map to
* a constant string.  You may use these strings to implement flexible
* debugging option
************************************************************************/

/***********************************************************************
* here you may implement debug function.
************************************************************************/
void dbgu_putint( int, int )
{
}

/***********************************************************************
*
* Debug utility functions
*
************************************************************************/

/***********************************************************************
* The following functions may require user modification:
*
* SPI_init() -  this function is responsible to initialize SSPI
*                   port.
*
* SPI_final() - this function is responsible to turn off SSPI port.
*
* wait() -      this function take the number of millisecond and
*                   wait for the time specified.
*
* TRANS_transmitBytes() -
*                   this function is responsible to transmit data over
*                   SSPI port.
*
* TRANS_receiveBytes() -
*                   this function is responsible to receive data
*                   through SSPI port.
*
* TRANS_starttranx() -
*                   this function initiates a transmission by pulling
*                   chip-select low.
*
* TRANS_endtranx() -
*                   this function terminates the transmission by pulling
*                   chip-select high.
*
* TRANS_cstoggle() -
*                   this function pulls chip-select low, then pulls it
*                   high.
*
* TRANS_runClk() -
*                   this function is responsible to drive at least
*                   3 extra clocks after chip-select is pulled high.
*
*
* In order to use stream transmission, dataBuffer[] is required to
* buffer the data.  Please refer to devices' specification for the
* number of bytes required.  For XP2-40, minimum is 423 bytes.
* Declare a little bit more than the minimum, just to be safe.
*
*
************************************************************************/
unsigned char dataBuffer[1024];
/************************************************************************
* Function SPI_init()
* Purpose: Initialize SPI port
*
* Return:       1 - succeed
*               0 - fail
*
* If you already initialize the SPI port in your embedded system,
* simply make the function to return 1.
************************************************************************/

/************************************************************************
* here you may implement SSPI initialization functions.
************************************************************************/
int SPI_init()
{

    return 1;
}
/************************************************************************
* Function SPI_final()
* Purpose: Finalize SPI port
*
* Return:       1 - succeed
*               0 - fail
*
* If you plan to leave the SPI port on, or it is managed in your
* embedded system, simply make the function return 1.
************************************************************************/

/************************************************************************
* here you may implement SSPI disable functions.
************************************************************************/
int SPI_final()
{
    return 1;
}

/************************************************************************
* Function wait(int ms)
* Purpose: Hold the process for some time (unit millisecond)
* Users must implement a delay to observe a_usTimeDelay, where
* bit 15 of the a_usTimeDelay defines the unit.
*      1 = milliseconds
*      0 = microseconds
* Example:
*      a_usTimeDelay = 0x0001 = 1 microsecond delay.
*      a_usTimeDelay = 0x8001 = 1 millisecond delay.
*
* This subroutine is called upon to provide a delay from 1 millisecond to a few
* hundreds milliseconds each time.
* It is understood that due to a_usTimeDelay is defined as unsigned short, a 16 bits
* integer, this function is restricted to produce a delay to 64000 micro-seconds
* or 32000 milli-second maximum. The VME file will never pass on to this function
* a delay time > those maximum number. If it needs more than those maximum, the VME
* file will launch the delay function several times to realize a larger delay time
* cummulatively.
* It is perfectly alright to provide a longer delay than required. It is not
* acceptable if the delay is shorter.
*
* Delay function example--using the machine clock signal of the native CPU------
* When porting ispVME to a native CPU environment, the speed of CPU or
* the system clock that drives the CPU is usually known.
* The speed or the time it takes for the native CPU to execute one for loop
* then can be calculated as follows:
*       The for loop usually is compiled into the ASSEMBLY code as shown below:
*       LOOP: DEC RA;
*             JNZ LOOP;
*       If each line of assembly code needs 4 machine cycles to execute,
*       the total number of machine cycles to execute the loop is 2 x 4 = 8.
*       Usually system clock = machine clock (the internal CPU clock).
*       Note: Some CPU has a clock multiplier to double the system clock for
*              the machine clock.
*
*       Let the machine clock frequency of the CPU be F, or 1 machine cycle = 1/F.
*       The time it takes to execute one for loop = (1/F ) x 8.
*       Or one micro-second = F(MHz)/8;
*
* Example: The CPU internal clock is set to 100Mhz, then one micro-second = 100/8 = 12
*
* The C code shown below can be used to create the millisecond accuracy.
* Users only need to enter the speed of the cpu.
*
************************************************************************/
int wait( int a_msTimeDelay )
{
    // Replace complex sample code with a simple Sleep.
    std::this_thread::sleep_for( std::chrono::milliseconds( a_msTimeDelay ) );

    return 1;
}

/************************************************************************
* Function TRANS_transmitBytes(unsigned char *trBuffer, int trCount)
* Purpose: To transmit certain number of bits, indicating by trCount,
* over SPI port.
*
* Data for transmission is stored in trBuffer.
* trCount indicates number of bits to be transmitted.  It should be
* divisible by 8.  If it is not divisible by 8, pad the data with 1's.
* The function returns 1 if success, or 0 if fail.
************************************************************************/

/************************************************************************
* here you may implement transmitByte function
************************************************************************/
int TRANS_transmitBytes( unsigned char *trBuffer, int trCount )
{
    // Never seen not a multiple of 8 during developpment, but just in case.
    if( trCount & 7 )
    {
        trCount += 8;
    }

    trCount >>= 3;

    if( static_cast<size_t>( gTransmitIndex + trCount ) < sizeof( gTransmitBuffer ) )
    {
        memcpy( gTransmitBuffer + gTransmitIndex, trBuffer, trCount );
        gTransmitIndex += trCount;

        return 1;
    }

    return 0;
}

/************************************************************************
* Function TRANS_receiveBytes(unsigned char *rcBuffer, int rcCount)
* Purpose: To receive certain number of bits, indicating by rcCount,
* over SPI port.
*
* Data received can be stored in rcBuffer.
* rcCount indicates number of bits to receive.  It should be
* divisible by 8.  If it is not divisible by 8, pad the data with 1's.
* The function returns 1 if success, or 0 if fail.
*************************************************************************/

/*********************************************************************
* here you may implement transmitByte function
*********************************************************************/
int TRANS_receiveBytes( unsigned char *rcBuffer, int rcCount )
{
    // Never seen not a multiple of 8 during developpment, but just in case.
    if( rcCount & 7 )
    {
        rcCount += 8;
    }

    rcCount >>= 3;

    if( static_cast<size_t>( gTransmitIndex + rcCount ) <= sizeof( gTransmitBuffer ) )
    {
        LeddarDevice::LdSensorM16::gConnection->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_UPDATE );
        uint8_t lFPGAType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_FPGA;
        LeddarDevice::LdSensorM16::gConnection->AddElement( LtComLeddarTechPublic::LT_COMM_ID_PROCESSOR, 1, sizeof( uint8_t ), &lFPGAType, sizeof( uint8_t ) );
        LeddarDevice::LdSensorM16::gConnection->AddElement( LtComLeddarTechPublic::LT_COMM_ID_RAW_DATA, gTransmitIndex + rcCount, sizeof( uint8_t ), gTransmitBuffer, sizeof( uint8_t ) );
        LeddarDevice::LdSensorM16::gConnection->SendRequest();
        LeddarDevice::LdSensorM16::gConnection->ReadAnswer();

        while( LeddarDevice::LdSensorM16::gConnection->ReadElement() )
        {
            switch( LeddarDevice::LdSensorM16::gConnection->GetElementId() )
            {
                case LtComLeddarTechPublic::LT_COMM_ID_RAW_DATA:
                    uint8_t *lTempBuffer;
                    lTempBuffer = static_cast<uint8_t *>( LeddarDevice::LdSensorM16::gConnection->GetElementData() );
                    memcpy( gTransmitBuffer, lTempBuffer, LeddarDevice::LdSensorM16::gConnection->GetElementCount() );
                    break;
            }
        }

        // What SSPIEmbeded wants is just the return data for the
        // command but an SPI link is bi-directionnal all the time
        // so we do not include the data received while transmitting.
        memcpy( rcBuffer, gTransmitBuffer + gTransmitIndex, rcCount );
        gTransmitIndex = 0;

        return 1;
    }

    return 0;
}

/************************************************************************
* Function TRANS_starttranx(unsigned char channel)
* Purpose: To start an SPI transmission
*
* Return:       1 - succeed
*               0 - fail
*
* This function is responsible to pull chip select low.
* If your embedded system has a dedicated SPI port that does not require
* manually pulling chip select low, simply return 1.
*************************************************************************/
/*********************************************************************
* here you should implement starting SPI transmission.
**********************************************************************/
int TRANS_starttranx( unsigned char )
{
    gTransmitIndex = 0;
    return 1;
}
/************************************************************************
* Function TRANS_endtranx()
* Purpose: To end an SPI transmission
*
* Return:       1 - succeed
*               0 - fail
*
* If your embedded system has a dedicated SPI port that does not require
* implementing this function, simply return 1.
*************************************************************************/

/*********************************************************************
* here you should implement ending SPI transmission.
**********************************************************************/
int TRANS_endtranx()
{
    if( gTransmitIndex > 0 )
    {
        LeddarDevice::LdSensorM16::gConnection->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_UPDATE );
        uint8_t lSoftwareType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_FPGA;
        LeddarDevice::LdSensorM16::gConnection->AddElement( LtComLeddarTechPublic::LT_COMM_ID_PROCESSOR, 1, sizeof( lSoftwareType ), &lSoftwareType, sizeof( lSoftwareType ) );
        LeddarDevice::LdSensorM16::gConnection->AddElement( LtComLeddarTechPublic::LT_COMM_ID_RAW_DATA, gTransmitIndex, 1, gTransmitBuffer, 1 );
        LeddarDevice::LdSensorM16::gConnection->SendRequest();
        LeddarDevice::LdSensorM16::gConnection->ReadAnswer();
        gTransmitIndex = 0;
    }

    return 1;
}

/************************************************************************
* Function TRANS_cstoggle(unsigned char channel)
* Purpose: To toggle chip select (CS) of specific channel
*
* Return:       1 - succeed
*               0 - fail
*
* If your embedded system has a dedicated SPI port that does not
* allow bit banging, simply transmit a byte of 0xFF to the device,
* and the device will ignore that.
*************************************************************************/
int TRANS_cstoggle( unsigned char channel )
{
    if( channel != 0x00 )
        return 0;
    else
    {
        /*********************************************************************
        * here you should implement toggling CS.
        *
        * Currently it prints message on screen and in log file.
        **********************************************************************/

        return 1;
    }
}

/************************************************************************
* Function TRANS_runClk()
* Purpose: To drive extra clock.
*
* Return:       1 - succeed
*               0 - fail
*
* If your embedded system has a dedicated SPI port that does not
* allow bit banging, simply transmit a byte of 0xFF on another channel
* that is not being used, so the device will only see the clock.
*************************************************************************/

/*********************************************************************
* here you should implement running free clock
**********************************************************************/
int TRANS_runClk()
{
    return 1;
}
/************************************************************************
* Function TRANS_transceive_stream(int trCount, unsigned char *trBuffer,
*                   int trCount2, int flag, unsigned char *trBuffer2
* Purpose: Transmits opcode and transceive data
*
* It will have the following operations depending on the flag:
*
*       SS_NO_DATA: end of transmission.  trCount2 and trBuffer2 are discarded
*       BUFFER_TX: transmit data from trBuffer2
*       BUFFER_RX: receive data and compare it with trBuffer2
*       DATA_TX: transmit data from external source
*       DATA_RX: receive data and compare it with data from external source
*
* If the data is not byte bounded and your SPI port only transmit/ receive
* byte bounded data, you need to have padding to make it byte-bounded.
* If you are transmit non-byte-bounded data, put the padding at the beginning
* of the data.  If you are receiving data, do not compare the padding,
* which is at the end of the transfer.
*************************************************************************/

#define SS_NO_DATA  0
#define BUFFER_TX   1
#define BUFFER_RX   2
#define DATA_TX     3
#define DATA_RX     4

int TRANS_transceive_stream( int trCount, unsigned char *trBuffer,
                             int trCount2, int flag, unsigned char *trBuffer2,
                             int mask_flag, unsigned char *maskBuffer )
{
    int i                         = 0;
    unsigned short int tranxByte  = 0;
    unsigned char trByte          = 0;
    unsigned char dataByte        = 0;
    int mismatch                  = 0;
    unsigned char dataID          = 0;

    if( trCount > 0 )
    {
        /* calculate # of bytes being transmitted */
        tranxByte = ( unsigned short )( trCount / 8 );

        if( trCount % 8 != 0 )
        {
            tranxByte ++;
            trCount += ( 8 - ( trCount % 8 ) );
        }

        if( !TRANS_transmitBytes( trBuffer, trCount ) )
            return ERROR_PROC_HARDWARE;
    }

    switch( flag )
    {
        case SS_NO_DATA:
            return 1;
            break;

        case BUFFER_TX:
            tranxByte = ( unsigned short )( trCount2 / 8 );

            if( trCount2 % 8 != 0 )
            {
                tranxByte ++;
                trCount2 += ( 8 - ( trCount2 % 8 ) );
            }

            if( !TRANS_transmitBytes( trBuffer2, trCount2 ) )
                return ERROR_PROC_HARDWARE;

            return 1;
            break;

        case BUFFER_RX:
            tranxByte = ( unsigned short )( trCount2 / 8 );

            if( trCount2 % 8 != 0 )
            {
                tranxByte ++;
                trCount2 += ( 8 - ( trCount2 % 8 ) );
            }

            if( !TRANS_receiveBytes( trBuffer2, trCount2 ) )
                return ERROR_PROC_HARDWARE;

            return 1;
            break;

        case DATA_TX:
            tranxByte = ( unsigned short )( ( trCount2 + 7 ) / 8 );

            if( trCount2 % 8 != 0 )
            {
                trByte = ( unsigned char )( 0xFF << ( trCount2 % 8 ) );
            }
            else
                trByte = 0;

            if( trBuffer2 != 0 )
                dataID = *trBuffer2;
            else
                dataID = 0x04;

            for( i = 0; i < tranxByte; i++ )
            {
                if( i == 0 )
                {
                    if( !HLDataGetByte( dataID, &dataByte, trCount2 ) )
                        return ERROR_INIT_DATA;
                }
                else
                {
                    if( !HLDataGetByte( dataID, &dataByte, 0 ) )
                        return ERROR_INIT_DATA;
                }

                if( trCount2 % 8 != 0 )
                    trByte += ( unsigned char )( dataByte >> ( 8 - ( trCount2 % 8 ) ) );
                else
                    trByte = dataByte;

                dataBuffer[i] = trByte;

                /* do not remove the line below!  It handles the padding for
                   non-byte-bounded data */
                if( trCount2 % 8 != 0 )
                    trByte = ( unsigned char )( dataByte << ( trCount2 % 8 ) );
            }

            if( trCount2 % 8 != 0 )
            {
                trCount2 += ( 8 - ( trCount2 % 8 ) );
            }

            if( !TRANS_transmitBytes( dataBuffer, trCount2 ) )
                return ERROR_PROC_HARDWARE;

            return 1;
            break;

        case DATA_RX:
            tranxByte = ( unsigned short )( trCount2 / 8 );

            if( trCount2 % 8 != 0 )
            {
                tranxByte ++;
            }

            if( trBuffer2 != 0 )
                dataID = *trBuffer2;
            else
                dataID = 0x04;

            if( !TRANS_receiveBytes( dataBuffer, ( tranxByte * 8 ) ) )
                return ERROR_PROC_HARDWARE;

            for( i = 0; i < tranxByte; i++ )
            {
                if( i == 0 )
                {
                    if( !HLDataGetByte( dataID, &dataByte, trCount2 ) )
                        return ERROR_INIT_DATA;
                }
                else
                {
                    if( !HLDataGetByte( dataID, &dataByte, 0 ) )
                        return ERROR_INIT_DATA;
                }

                trByte = dataBuffer[i];

                if( mask_flag )
                {
                    trByte = trByte & maskBuffer[i];
                    dataByte = dataByte & maskBuffer[i];
                }

                if( i == tranxByte - 1 )
                {
                    trByte = ( unsigned char )( trByte ^ dataByte ) &
                             ( unsigned char )( 0xFF << ( 8 - ( trCount2 % 8 ) ) );
                }
                else
                    trByte = ( unsigned char )( trByte ^ dataByte );

                if( trByte )
                    mismatch ++;
            }

            if( mismatch == 0 )
            {
                if( a_uiCheckFailedRow )
                {
                    a_uiRowCount++;
                }

                return 1;
            }
            else
            {
                if( dataID == 0x01 && a_uiRowCount == 0 )
                {
                    return ERROR_IDCODE;
                }
                else if( dataID == 0x05 )
                {
                    return ERROR_USERCODE;
                }
                else if( dataID == 0x06 )
                {
                    return ERROR_SED;
                }
                else if( dataID == 0x07 )
                {
                    return ERROR_TAG;
                }

                return ERROR_VERIFICATION;
            }

            break;

        default:
            return ERROR_INIT_ALGO;
    }
}

#endif