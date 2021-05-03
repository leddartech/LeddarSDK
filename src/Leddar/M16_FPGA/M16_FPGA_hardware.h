#ifndef _M16_FPGA_HARDWARE_H_
#define _M16_FPGA_HARDWARE_H_

#include "LtDefines.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

/************************************************************************
*
* Function Definition
*
*************************************************************************/
//#define DISPLAY           1
//#define LOG_DISPLAY       1
/************************************************************************
* Hardware functions
*************************************************************************/
int SPI_init();
int SPI_final();
int wait( int ms );

/************************************************************************
* SPI transmission functions
*************************************************************************/
int TRANS_starttranx( unsigned char channel );
int TRANS_endtranx();
int TRANS_cstoggle( unsigned char channel );
int TRANS_runClk();
int TRANS_transmitBytes( unsigned char *trBuffer, int trCount );
int TRANS_receiveBytes( unsigned char *rcBuffer, int rcCount );

int TRANS_transceive_stream( int trCount, unsigned char *trBuffer,
                             int trCount2, int flag, unsigned char *trBuffer2,
                             int flag_mask, unsigned char *mask_buffer );


/************************************************************************
* debug utility functions
*
* If you wish to enable debugging functionalities, uncomment definition
* DEBUG_LEVEL_1
*************************************************************************/

#define DEBUG_LEVEL_1   1
#ifdef  DEBUG_LEVEL_1
//#define   DEBUG_LEVEL_2   1
#endif
#ifdef  DEBUG_LEVEL_2
//#define   DEBUG_LEVEL_3   1
#endif

/************************************************************************
* debugging level
* for debug level 1, uncomment definition of DEBUG_LEVEL_1
* for debug level 2, uncomment definition of DEBUG_LEVEL_1. DEBUG_LEVEL_2
* for debug level 3, uncomment definition of DEBUG_LEVEL_1, DEBUG_LEVEL_2, DEBUG_LEVEL_3
*************************************************************************/

#ifdef  DEBUG_LEVEL_1
#include "M16_FPGA_debug.h"
int dbgu_init();
void dbgu_putint( int debugCode, int debugCode2 );
//#define   DEBUG_LEVEL_2   1
#endif

#ifdef  DEBUG_LEVEL_2
//#define   DEBUG_LEVEL_3   1
#endif

#endif
#endif