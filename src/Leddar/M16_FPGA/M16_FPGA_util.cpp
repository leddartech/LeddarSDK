/************************************************************************
* Lattice Semiconductor Corp. Copyright 2008
*
* Utility functions
*
************************************************************************/
#include "M16_FPGA_util.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

void init_CS( CSU *cs, short int width, short int chunkSize )
{
    cs->csWidth = width;
    cs->csChunkSize = chunkSize;
    cs->csValue = 0;
}

unsigned int getCheckSum( CSU *cs )
{
    unsigned int mask = 0xFFFFFFFF;

    mask >>= ( 32 - cs->csWidth );
    return ( cs->csValue & mask );
}

void putChunk( CSU *cs, unsigned int chunk )
{
    unsigned int mask = 0xFFFFFFFF;

    mask >>= ( 32 - cs->csChunkSize );
    cs->csValue += ( chunk & mask );
}
#endif