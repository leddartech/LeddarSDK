#ifndef _M16_FPGA_SSPIEM_H_
#define _M16_FPGA_SSPIEM_H_

#include "LtDefines.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

int SSPIEm_preset( const unsigned char *aAlgo, unsigned int aAlgoSize,
                   const unsigned char *aData, unsigned int aDataSize );
int SSPIEm( unsigned int algoID );

#endif
#endif