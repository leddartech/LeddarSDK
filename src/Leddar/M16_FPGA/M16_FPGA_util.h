#ifndef _M16_FPGA_UTIL_H_
#define _M16_FPGA_UTIL_H_

#include "LtDefines.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

/************************************************************************
* Utility functions
************************************************************************/

typedef struct checksum
{
    unsigned int csValue;
    short int csWidth;
    short int csChunkSize;
} CSU;

void init_CS( CSU *cs, short int width, short int chunkSize );
unsigned int getCheckSum( CSU *cs );
void putChunk( CSU *cs, unsigned int chunk );

#endif
#endif