#ifndef _M16_FPGA_CORE_H_
#define _M16_FPGA_CORE_H_

#include "LtDefines.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

#include "M16_FPGA_util.h"

#define PROC_FAIL       0
#define PROC_COMPLETE   1
#define PROC_OVER       2

/************************************************************************
*
* Function / struct definition
*
*************************************************************************/

/************************************************************************
* Processing functions
*************************************************************************/

int SSPIEm_process( unsigned char *bufAlgo, unsigned int bufAlgoSize );
int SSPIEm_init( unsigned int algoID );

int VME_getByte( unsigned char *byteOut,
                 unsigned char *bufferedAlgo, unsigned int bufferedAlgoSize,
                 unsigned int *bufferedAlgoIndex );
unsigned int VME_getNumber( unsigned char *bufAlgo, unsigned int bufAlgoSize,
                            unsigned int *bufAlgoIndex, unsigned int *byteCount );

/************************************************************************
* Function / struct definition
*************************************************************************/

int proc_TRANS( unsigned char *bufferedAlgo, unsigned int bufferedAlgoSize,
                unsigned int *absBufferedAlgoIndex, unsigned char channel, unsigned char currentByte );
int proc_REPEAT( unsigned char *bufferedAlgo, unsigned int bufferedAlgoSize,
                 unsigned int *absBufferedAlgoIndex, unsigned int LoopMax );
int proc_LOOP( unsigned char *bufferedAlgo, unsigned int bufferedAlgoSize,
               unsigned int *absBufferedAlgoIndex, unsigned int LoopMax );
int proc_HCOMMENT( unsigned char *bufferedAlgo, unsigned int bufferedAlgoSize,
                   unsigned int *absBufferedAlgoIndex, CSU *headerCS );

#endif
#endif