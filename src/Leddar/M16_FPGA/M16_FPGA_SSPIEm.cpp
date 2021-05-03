/************************************************************************
* Lattice Semiconductor Corp. Copyright 2011
*
* SSPI Embedded System
*
* Version: 4.0.0
*       Add SSPIEm_preset() function prototype
*
*
* Main processing engine
*
*************************************************************************/
#include "M16_FPGA_SSPIEm.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

#include "M16_FPGA_intrface.h"
#include "M16_FPGA_core.h"

/*********************************************************************
*
* SSPIEm_preset()
*
* This function calls algoPreset and dataPreset to set which algorithm
* and data files are going to be processed.  Input(s) to the function
* may depend on configuration.
*
*********************************************************************/

int SSPIEm_preset( const unsigned char *aAlgo, unsigned int aAlgoSize,
                   const unsigned char *aData, unsigned int aDataSize )
{
    int retVal = algoPreset( aAlgo, aAlgoSize );

    if( retVal )
        retVal = dataPreset( aData, aDataSize );

    return retVal;
}
/************************************************************************
* Function SSPIEm
* The main function of the processing engine.  During regular time,
* it automatically gets byte from external storage.  However, this
* function requires an array of buffered algorithm during
* loop / repeat operations.  Input bufAlgo must be 0 to indicate
* regular operation.
*
* To call the VME, simply call SSPIEm(int debug);
*************************************************************************/
int SSPIEm( unsigned int algoID )
{
    int retVal = 0;
    retVal = SSPIEm_init( algoID );

    if( retVal <= 0 )
        return retVal;

    retVal = SSPIEm_process( 0, 0 );
    return retVal;
}

#endif