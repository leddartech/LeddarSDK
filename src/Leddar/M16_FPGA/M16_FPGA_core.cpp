/********************************************************************
* Lattice Semiconductor Corp. Copyright 210
*
* core.c / .cpp
*
* Version: 4.0.0
* 2008/05/22    Add error code to initialization.
*
* 2008/01/25    Beta complete
*
**********************************************************************/

#include "M16_FPGA_core.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

#include "M16_FPGA_hardware.h"
#include "M16_FPGA_intrface.h"
#include "M16_FPGA_opcode.h"
#include "M16_FPGA_debug.h"
#include "M16_FPGA_util.h"

/************************************************************************
*
* Definition of System properties
*
* This section defines properties of the processing system.  This
* part need to be configured when generating algorithm byte stream.
*
* MAXBUF        - maximum buffer allowed.
* MAXTRANSBUF   - maximum transmission buffer allowed.
*   HOLDAF      - time (millisecond) hold after fail, must be positive
*                 0:        not continue, exit.
*                 Other:    milliseconds
* MAXSTACK  - maximum stack allowed, indicating maximum nested loop
*                 allowed in a loop / repeat.
* MAX_MASKSIZE- maximum mask size allowed in bytes.  4 or more is required
*
**************************************************************************/

#define MAXBUF          200
#define MAXTRANSBUF     500
#define HOLDAF          0
#define MAXSTACK        3
#define MAX_MASKSIZE    32
#define MAX_DEBUGSTR    80
#define HEADERCRCSIZE   2

/************************************************************************
*
* Global variable & access functions
*
**************************************************************************/

const unsigned char version[] = {4, 0, 0};
unsigned char algoBuffer[MAXBUF];
unsigned char stack[MAXSTACK];

/*************************************************************************
*
* External variables declared in hardware.c module.
*
**************************************************************************/
extern unsigned int a_uiCheckFailedRow ;
extern unsigned int a_uiRowCount;

/*************************************************************************
* Global variable for channel
**************************************************************************/
unsigned char currentChannel;

unsigned char getCurrentChannel()
{
    return currentChannel;
}


/**************************************************************************
*
* Processing functions
*
**************************************************************************/

/**************************************************************************
* Function SSPIEm_init()
* Start initialization
**************************************************************************/

int SSPIEm_init( unsigned int algoID )
{
    unsigned char currentByte = 0;
    int i                     = 0;
    unsigned int mask         = 0;
    CSU headerCS;
    /* initialize header check sum unit */
    init_CS( &headerCS, HEADERCRCSIZE * 8, 8 );
    /* initialize debug */
#ifdef  DEBUG_LEVEL_1
    dbgu_init();
#endif

    /* initialize SPI */
    if( !SPI_init() )
    {
#ifdef  DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, INIT_SPI_FAIL );
#endif
        return ERROR_INIT_SPI;
    }

#ifdef  DEBUG_LEVEL_2
    dbgu_putint( DBGU_L2_INIT, INIT_BEGIN ); //"Initialization begin"
#endif

    /* initialize algorithm utility */
    if( !algoInit() )
    {
#ifdef  DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, INIT_ALGO_FAIL );
#endif
        return ERROR_INIT_ALGO;
    }

    /* discard comments, if available */
    do
    {
        if( !VME_getByte( &currentByte, 0, 0, 0 ) )
        {
#ifdef  DEBUG_LEVEL_1
            dbgu_putint( DBGU_L1_ALGO_INIT, NO_ALGOID );
#endif
            return ERROR_INIT;
        }

        putChunk( &headerCS, ( unsigned int ) currentByte );

        if( currentByte == HCOMMENT )
        {
            if( proc_HCOMMENT( 0, 0, 0, &headerCS ) == PROC_FAIL )
            {
#ifdef  DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_ALGO_INIT, COMMENT_END_UNEXPECTED );
#endif
                return ERROR_INIT;
            }
        }
    }
    while( currentByte == HCOMMENT );

    /* check ALGOID */
    if( currentByte != ALGOID )
    {
#ifdef  DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, NO_ALGOID );
#endif
        return ERROR_INIT;
    }
    else
    {
        for( i = 0; i < 4; i++ )
        {
            if( VME_getByte( &currentByte, 0, 0, 0 ) )
            {
                putChunk( &headerCS, ( unsigned int ) currentByte );

                if( currentByte != ( unsigned char )( algoID >> ( ( 3 - i ) * 8 ) ) )
                {
                    if( algoID != 0xFFFFFFFF )
                    {
#ifdef  DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_MISMATCH, NO_ALGOID );
#endif
                        return ERROR_INIT;
                    }
                }
            }
            /* no algorithm ID byte available */
            else
            {
#ifdef  DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_ALGO_INIT, NO_ALGOID );
#endif
                return ERROR_INIT;
            }
        }
    }

    /* check VERSION */
    if( !VME_getByte( &currentByte, 0, 0, 0 ) ||
            currentByte != VERSION )
    {
#ifdef  DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, NO_VERSION );
#endif
        return ERROR_INIT;
    }
    else
    {
        putChunk( &headerCS, ( unsigned int ) currentByte );

        for( i = 0; i < 3; i++ )
        {
            if( VME_getByte( &currentByte, 0, 0, 0 ) )
            {
                putChunk( &headerCS, ( unsigned int ) currentByte );

                if( currentByte > version[i] )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_MISMATCH, NO_VERSION );
#endif
                    return ERROR_INIT_VERSION;
                }
            }
            /* no version byte available */
            else
            {
#ifdef  DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_ALGO_INIT, NO_VERSION );
#endif
                return ERROR_INIT;
            }
        }
    }

    /* check BUFFERREQ */
    if( !VME_getByte( &currentByte, 0, 0, 0 ) ||
            currentByte != BUFFERREQ )
    {
#ifdef  DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, NO_BUFFERREQ );
#endif
        return ERROR_INIT;
    }
    else
    {
        putChunk( &headerCS, ( unsigned int ) currentByte );

        if( !VME_getByte( &currentByte, 0, 0, 0 ) ||
                currentByte > MAXBUF )
        {
#ifdef  DEBUG_LEVEL_1
            dbgu_putint( DBGU_L1_MISMATCH, NO_BUFFERREQ );
#endif
            return ERROR_INIT;
        }
        else
            putChunk( &headerCS, ( unsigned int ) currentByte );
    }

    /* check STACKREQ */
    if( !VME_getByte( &currentByte, 0, 0, 0 ) ||
            currentByte != STACKREQ )
    {
#ifdef  DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, NO_STACKREQ );
#endif
        return ERROR_INIT;
    }
    else
    {
        putChunk( &headerCS, ( unsigned int ) currentByte );

        /* check STACKREQ */
        if( !VME_getByte( &currentByte, 0, 0, 0 ) ||
                currentByte > MAXSTACK )
        {
#ifdef  DEBUG_LEVEL_1
            dbgu_putint( DBGU_L1_MISMATCH, NO_STACKREQ );
#endif
            return ERROR_INIT;
        }
        /* no STACKREQ byte available */
        else
        {
            putChunk( &headerCS, ( unsigned int ) currentByte );
        }
    }

    /* check MASKBUFREQ */
    if( !VME_getByte( &currentByte, 0, 0, 0 ) ||
            currentByte != MASKBUFREQ )
    {
#ifdef DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, NO_MASKBUFREQ );
#endif
        return ERROR_INIT;
    }
    else
    {
        putChunk( &headerCS, ( unsigned int ) currentByte );

        if( !VME_getByte( &currentByte, 0, 0, 0 ) ||
                currentByte > MAX_MASKSIZE )
        {
#ifdef DEBUG_LEVEL_1
            dbgu_putint( DBGU_L1_MISMATCH, NO_MASKBUFREQ );
#endif
            return ERROR_INIT;
        }
        else
            putChunk( &headerCS, ( unsigned int ) currentByte );
    }

    /* store Channel */
    if( !VME_getByte( &currentByte, 0, 0, 0 ) ||
            currentByte != HCHANNEL )
    {
#ifdef  DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, NO_CHANNEL );
#endif
        return ERROR_INIT;
    }
    else
    {
        putChunk( &headerCS, ( unsigned int ) currentByte );

        if( !VME_getByte( &currentByte, 0, 0, 0 ) )
        {
#ifdef  DEBUG_LEVEL_1
            dbgu_putint( DBGU_L1_ALGO_INIT, NO_CHANNEL );
#endif
            return ERROR_INIT;
        }
        else
        {
            putChunk( &headerCS, ( unsigned int ) currentByte );
            currentChannel = currentByte;
        }
    }

    /* check COMPRESSION */
    if( !dataInit() )
    {
        return ERROR_INIT_DATA;
    }

    if( !VME_getByte( &currentByte, 0, 0, 0 ) )
    {
#ifdef  DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, NO_COMPRESSION );
#endif
        return ERROR_INIT;
    }
    else
    {
        putChunk( &headerCS, ( unsigned int ) currentByte );

        if( currentByte == COMPRESSION || currentByte == HCOMMENT )
        {
            if( !VME_getByte( &currentByte, 0, 0, 0 ) )
            {
#ifdef  DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_ALGO_INIT, NO_COMPRESSION );
#endif
                return ERROR_INIT;
            }

            putChunk( &headerCS, currentByte );
        }
        else
        {
#ifdef  DEBUG_LEVEL_1
            dbgu_putint( DBGU_L1_ALGO_INIT, NO_COMPRESSION );
#endif
            return ERROR_INIT;
        }
    }

    /* check HEADERCS (done) */
    if( !VME_getByte( &currentByte, 0, 0, 0 ) ||
            currentByte != HEADERCRC )
    {
#ifdef DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, NO_HEADERCS );
#endif
        return ERROR_INIT;
    }
    else
    {
        for( i = 0; i < HEADERCRCSIZE; i++ )
        {
            if( !VME_getByte( &currentByte, 0, 0, 0 ) )
            {
#ifdef DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_ALGO_INIT, NO_HEADERCS );
#endif
                return ERROR_INIT;
            }

            mask = 0xFF << ( 8 * ( HEADERCRCSIZE - i - 1 ) );

            if( currentByte !=
                    ( ( headerCS.csValue & mask ) >> ( 8 * ( HEADERCRCSIZE - i - 1 ) ) ) &&
                    currentByte != 0xFF )
            {
#ifdef DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_MISMATCH, NO_HEADERCS );
#endif
                return ERROR_INIT_CHECKSUM;
            }
        }
    }

    /* get STARTOFALGO byte */
    if( !VME_getByte( &currentByte, 0, 0, 0 ) ||
            currentByte != STARTOFALGO )
    {
#ifdef  DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, NO_STARTOFALGO );
#endif
        return ERROR_INIT;
    }
    else
    {
#ifdef  DEBUG_LEVEL_2
        dbgu_putint( DBGU_L2_INIT, INIT_COMPLETE );
#endif
    }

    a_uiCheckFailedRow = 0;
    a_uiRowCount       = 0;
    return PROC_COMPLETE;
}

/**************************************************************************
* Processing functions
* These functions are processing functions SSPI_processVME will call
* during operation.  They are more internal and it is recommended
* not to call these functions outside SSPI_processVME.
**************************************************************************/

/**************************************************************************
* Function SSPI_process
* The main function of the processing engine.  During regular time,
* it automatically gets byte from external storage.  However, this
* function requires an array of buffered algorithm during
* loop / repeat operations.  Input bufferedAlgo must be 0 to indicate
* regular operation.
*
* To call the VME, simply call SSPI_processVME(int debug, 0, 0, 0);
* Having input for this function is better suited for internal use.
* We highly recommend not to use any input in external environment.
*
* Input:
* debug             - 0 for normal mode, 1 for debug mode
* Internal Input:
* *bufferedAlgo     - array of algorithm during loop / repeat operation
* bufferedAlgoSize  - max size of buffer
* repeatVar         - special input for repeat only
*
*
* Output (procReturn value):
* 0 - Process fail
* 1 - Process complete
* 2 - Process successfully over
**************************************************************************/
int SSPIEm_process( unsigned char *bufAlgo, unsigned int bufAlgoSize )
{
    unsigned int    bufAlgoIndex = 0;
    short int       procReturn   = PROC_COMPLETE;
    unsigned char   currentByte  = 0;
    unsigned int    temp         = 0;
#ifdef  DEBUG_LEVEL_2

    if( bufAlgo == 0 )
        dbgu_putint( DBGU_L2_PROC, START_PROC );
    else
        dbgu_putint( DBGU_L2_PROC, START_PROC_BUFFER );

#endif

    while( procReturn == PROC_COMPLETE )
    {
        /************************************************************************
        *   Under STANDBY state, it allows opcode STARTTRAN, WAIT, LOOP, REPEAT
        *   If it is in LOOP or REPEAT, it also allows CONDITION
        ************************************************************************/

        if( !VME_getByte( &currentByte, bufAlgo, bufAlgoSize, &bufAlgoIndex ) )
        {
            if( bufAlgo != 0 )
            {
#ifdef  DEBUG_LEVEL_2
                dbgu_putint( DBGU_L2_PROC, END_PROC_BUFFER );
#endif
                return PROC_OVER;
            }
            else
            {
#ifdef  DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_ALGO_PROC, UNABLE_TO_GET_BYTE );
#endif
                return ERROR_PROC_ALGO;
            }
        }

        switch( currentByte )
        {
            case HCOMMENT:
                if( proc_HCOMMENT( bufAlgo, bufAlgoSize, &bufAlgoIndex, 0 ) == PROC_FAIL )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_ALGO_PROC, COMMENT_END_UNEXPECTED );
#endif
                    return ERROR_PROC_ALGO;
                }

                break;

            case STARTTRAN: /* starts transmission */
#ifdef  DEBUG_LEVEL_2
                dbgu_putint( DBGU_L2_PROC, ENTER_STARTTRAN );
#endif

                if( TRANS_starttranx( getCurrentChannel() ) == PROC_FAIL )
                {
#ifdef DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_TRANX_PROC, STARTTRAN_FAIL );
#endif
                    return ERROR_PROC_HARDWARE;
                }

                break;

            case TRANSIN:
            case TRANSOUT:

                //************************************************************************
                //* Under STARTTRAN, opcode TRANSOUT, TRANSIN are allowed.  Since the
                //* SSPI Embedded system operates under Master SPI mode, it always does
                //* TRANSOUT first.
                //************************************************************************
                if( bufAlgo == 0 )
                    procReturn = proc_TRANS( bufAlgo, bufAlgoSize, &bufAlgoIndex, getCurrentChannel(), currentByte );
                else
                    procReturn = proc_TRANS( &( bufAlgo[bufAlgoIndex] ), bufAlgoSize, &bufAlgoIndex, getCurrentChannel(), currentByte );

                if( procReturn <= 0 )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_PROCESS, TRANX_FAIL ); //"Transmission fail",
#endif
                }

                break;

            case RUNCLOCK:
#ifdef  DEBUG_LEVEL_2
                dbgu_putint( DBGU_L2_PROC, ENTER_RUNCLOCK );
#endif

                if( !TRANS_runClk() )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_PROCESS, RUNCLOCK_FAIL );
#endif
                    procReturn = ERROR_PROC_HARDWARE;
                }

                break;

            case REPEAT:
#ifdef  DEBUG_LEVEL_2
                dbgu_putint( DBGU_L2_PROC, ENTER_REPEAT );
#endif

                /************************************************************************
                * REPEAT opcode is followed by the number of repeats.
                * Then it start processing the transmission by calling proc_REPEAT().
                ************************************************************************/
                temp = VME_getNumber( bufAlgo, bufAlgoSize, &bufAlgoIndex, 0 );

                if( temp == PROC_FAIL )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_ALGO_PROC, NO_NUMBER_OF_REPEAT );
#endif
                    procReturn = ERROR_PROC_ALGO;
                }
                else
                {
                    a_uiCheckFailedRow = 1;
                    a_uiRowCount      = 1;

                    if( bufAlgo == 0 )
                        procReturn = proc_REPEAT( bufAlgo, bufAlgoSize,
                                                  &bufAlgoIndex, temp );
                    else
                        procReturn = proc_REPEAT( &( bufAlgo[bufAlgoIndex] ),
                                                  bufAlgoSize, &bufAlgoIndex, temp );

                    a_uiCheckFailedRow = 0;

                    if( procReturn <= 0 )
                    {
#ifdef  DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_PROCESS, REPEAT_FAIL );
#endif
                    }
                }

                break;

            case LOOP:
#ifdef  DEBUG_LEVEL_2
                dbgu_putint( DBGU_L2_PROC, ENTER_LOOP );
#endif

                /************************************************************************
                * LOOP opcode is followed by the max number of looping, then it process
                * the transmission by calling proc_LOOP().
                *************************************************************************/

                temp = VME_getNumber( bufAlgo, bufAlgoSize, &bufAlgoIndex, 0 );

                if( temp == PROC_FAIL )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_ALGO_PROC, NO_NUMBER_OF_LOOP );
#endif
                    procReturn = ERROR_PROC_ALGO;
                }
                else
                {
                    if( bufAlgo == 0 )
                    {
                        procReturn = proc_LOOP( bufAlgo, bufAlgoSize, &bufAlgoIndex, temp );
                    }
                    else
                    {
                        procReturn = proc_LOOP( &( bufAlgo[bufAlgoIndex] ), bufAlgoSize, &bufAlgoIndex, temp );
                    }

                    if( procReturn <= 0 )
                    {
#ifdef  DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_PROCESS, LOOP_FAIL );
#endif
                        procReturn = ERROR_LOOP_COND;
                    }
                }

                break;

            case WAIT:      /* process WAIT */
#ifdef  DEBUG_LEVEL_2
                dbgu_putint( DBGU_L2_PROC, ENTER_WAIT ); "Enter WAIT"
                ,
#endif

                /************************************************************************
                * WAIT opcode is followed by wait time in millisecond, then it process
                * the wait by calling proc_WAIT().
                ************************************************************************/

                temp = VME_getNumber( bufAlgo, bufAlgoSize, &bufAlgoIndex, 0 );

                if( temp == PROC_FAIL )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_ALGO_PROC, NO_NUMBER_OF_WAIT );
#endif
                    procReturn = ERROR_PROC_ALGO;
                }
                else
                {
                    procReturn = wait( temp );
                }

                break;

            case RESETDATA:
                if( !dataReset( 1 ) )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_PROCESS, RESETDATA_FAIL );
#endif
                    procReturn = ERROR_PROC_DATA;
                }

                break;

            case ENDTRAN:
                if( !TRANS_endtranx() )
                {
#ifdef DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_TRANX_PROC, ENDTRAN_FAIL );
#endif
                    procReturn = ERROR_PROC_HARDWARE;
                }

                break;

            case ENDOFALGO:
#ifdef  DEBUG_LEVEL_2
                if( bufAlgo != 0 )
                    dbgu_putint( DBGU_L2_PROC, END_PROC_BUFFER );
                else
                    dbgu_putint( DBGU_L2_PROC, END_PROC );

#endif
                procReturn = PROC_OVER;
                break;

            default:
#ifdef  DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_ALGO_PROC, UNRECOGNIZED_OPCODE ); /*"Unrecognized opcode" */
#endif
                return ERROR_PROC_ALGO;
                break;
        }
    }

    if( bufAlgo == 0 )
    {
        if( !algoFinal() )
            procReturn = ERROR_PROC_ALGO;

        if( !dataFinal() )
            procReturn = ERROR_PROC_DATA;

        if( !SPI_final() )
            procReturn = ERROR_PROC_HARDWARE;
    }

    return procReturn;
}

/**************************************************************************
* Function proc_TRANS
* Start processing transmissions
*
* Input:
* bufAlgo           - the pointer to buffered algorithm if available
* bufAlgoSize       - the size of buffered algorithm
*                     this field is discarded if bufAlgo is 0
*   absBufAlgoIndex - the pointer to the absolute Algorithm Index from
*                     the caller so proc_TRANS accumulates the absolute
*                     index while processing the transmission
*
* Return:
* PROC_FAIL     - Transmission fail or mismatch appears
* PROC_COMPLETE - Transmission complete
**************************************************************************/

#define SS_NO_DATA  0
#define BUFFER_TX   1
#define BUFFER_RX   2
#define DATA_TX     3
#define DATA_RX     4

int proc_TRANS( unsigned char *bufAlgo, unsigned int bufAlgoSize,
                unsigned int *absbufAlgoIndex, unsigned char, unsigned char currentByte )
{
    unsigned char trBuffer[MAXTRANSBUF];
    unsigned char maskBuffer[MAX_MASKSIZE / 8];
    int trCount = 0;
    unsigned int bufAlgoIndex = 0;
    short int flag_mask = 0;
    short int flag_transin = 0;
    int byteNum = 0;
    short int retVal = 0;
    unsigned int mismatch = 0;
    int temp;
    int i;

    while( retVal != PROC_OVER )
    {
        switch( currentByte )
        {
            case HCOMMENT:
                if( proc_HCOMMENT( bufAlgo, bufAlgoSize, &bufAlgoIndex, 0 ) == PROC_FAIL )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_ALGO_PROC, COMMENT_END_UNEXPECTED );
#endif
                    return ERROR_PROC_ALGO;
                }

                break;

            case WAIT:
                //************************************************************************
                //* WAIT opcode is followed by wait time in millisecond, then it process
                //* the wait by calling proc_WAIT().
                //************************************************************************

                temp = VME_getNumber( bufAlgo, bufAlgoSize, &bufAlgoIndex, 0 );

                if( temp == PROC_FAIL )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_ALGO_PROC, NO_NUMBER_OF_WAIT ); //"No millisecond after WAIT",
#endif
                    return ERROR_PROC_ALGO;
                }
                else
                {
                    wait( temp );
                }

                break;

            // since the proc system is Master SPI, it always transmit data out first
            case TRANSOUT:
#ifdef DEBUG_LEVEL_2
                dbgu_putint( 8, 2 ); //"Enter TRANSOUT",
#endif
                // get transmit size in bits, whether the data is compressed or not
                trCount = VME_getNumber( bufAlgo, bufAlgoSize, &bufAlgoIndex, 0 );

                if( trCount == PROC_FAIL )
                {
#ifdef DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_ALGO_TRANX, NO_TRANSOUT_SIZE ); //"No byte available at Size",
#endif
                    return ERROR_PROC_ALGO;     // no byte available at Size
                }

                byteNum = trCount / 8;

                if( trCount % 8 != 0 )
                    byteNum ++;

                // check if the next Byte is DATA or DATAM
                if( !VME_getByte( &currentByte, bufAlgo, bufAlgoSize, &bufAlgoIndex ) )
                {
#ifdef DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_ALGO_TRANX, NO_TRANSOUT_TYPE ); //"Algo error: no byte available at DATA type",
#endif
                    return ERROR_PROC_ALGO;     // no byte available at DATA type
                }

                if( currentByte == ALGODATA )
                {

                    // buffer transmit bytes
                    for( i = 0; i < byteNum; i++ )
                    {
                        if( !VME_getByte( &( trBuffer[i] ), bufAlgo, bufAlgoSize, &bufAlgoIndex ) )
                        {
#ifdef DEBUG_LEVEL_1
                            dbgu_putint( DBGU_L1_ALGO_TRANX, NO_TRANSOUT_DATA ); //"Algo error: no byte available for transmit",
#endif
                            return ERROR_PROC_ALGO;
                        }
                    }

                    retVal = TRANS_transceive_stream( trCount, trBuffer, 0, SS_NO_DATA, 0, flag_mask, maskBuffer );

                    if( retVal <= 0 && retVal != ERROR_VERIFICATION )
                    {
#ifdef DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_TRANX_PROC, TRANX_OPCODE_FAIL );
#endif
                        return retVal;
                    }
                }
                else if( currentByte == PROGDATAEH )
                {
                    retVal = TRANS_transceive_stream( 0, trBuffer, trCount, DATA_TX, &currentByte, flag_mask, maskBuffer );

                    if( retVal <= 0 )
                    {
#ifdef DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_TRANX_PROC, TRANX_OUT_PROG_FAIL );
#endif
                        return retVal;
                    }

                }
                else
                {
#ifdef DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_ALGO_TRANX, NO_TRANSOUT_TYPE ); //"Algo error: no byte available at DATA type",
#endif
                    return ERROR_PROC_ALGO;     // no byte available at DATA type
                }

                flag_transin = 0;
                break;

            case ALGODATA:
                if( !flag_transin )
                {
                    retVal = TRANS_transceive_stream( 0, 0, trCount, BUFFER_TX, trBuffer, flag_mask, maskBuffer );

                    if( retVal <= 0 )
                    {
#ifdef DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_TRANX_PROC, TRANX_OUT_ALGO_FAIL );
#endif
                        return retVal;
                    }
                }
                else
                {
                    retVal = TRANS_transceive_stream( 0, 0, trCount, BUFFER_RX, trBuffer, flag_mask, maskBuffer );

                    if( retVal <= 0 && retVal != ERROR_VERIFICATION )
                    {
#ifdef DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_TRANX_PROC, TRANX_IN_ALGO_FAIL ); //"Transmit error: unable to transmit",
#endif
                        return retVal;
                    }

                    for( i = 0; i < byteNum; i++ )
                    {
                        if( !VME_getByte( &currentByte, bufAlgo, bufAlgoSize, &bufAlgoIndex ) )
                        {
#ifdef DEBUG_LEVEL_1
                            dbgu_putint( DBGU_L1_ALGO_TRANX, NO_TRANSIN_DATA ); //"Algo error: no byte available for transmit",
#endif
                            return ERROR_PROC_ALGO;
                        }

                        if( flag_mask )
                        {
                            trBuffer[i] = trBuffer[i] & maskBuffer[i];
                            currentByte  = currentByte & maskBuffer[i];
                        }
                        else if( i == byteNum - 1 && trCount % 8 != 0 )
                            trBuffer[i] = trBuffer[i] & ( ~( ( unsigned char )( 0xFF >> ( trCount % 8 ) ) ) );

                        if( trBuffer[i] != currentByte )
                            mismatch ++;
                    }
                }

                break;

            case PROGDATA:
                if( !flag_transin )
                {
                    retVal = TRANS_transceive_stream( 0, trBuffer, trCount, DATA_TX, 0, flag_mask, maskBuffer );

                    if( retVal <= 0 )
                    {
#ifdef DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_TRANX_PROC, TRANX_OUT_PROG_FAIL );
#endif
                        return retVal;
                    }
                }
                else
                {
                    retVal = TRANS_transceive_stream( 0, trBuffer, trCount, DATA_RX, 0, flag_mask, maskBuffer );

                    if( retVal <= 0 && retVal != ERROR_VERIFICATION )
                    {
#ifdef DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_TRANX_PROC, TRANX_IN_PROG_FAIL ); //"Transmit error: unable to transmit",
#endif
                        return retVal;
                    }
                }

                break;

            case PROGDATAEH:
                if( !flag_transin )
                {
                    retVal = TRANS_transceive_stream( 0, trBuffer, trCount, DATA_TX, &currentByte, flag_mask, maskBuffer );

                    if( retVal <= 0 )
                    {
#ifdef DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_TRANX_PROC, TRANX_OUT_PROG_FAIL );
#endif
                        return retVal;
                    }
                }
                else
                {
                    retVal = TRANS_transceive_stream( 0, trBuffer, trCount, DATA_RX, &currentByte, flag_mask, maskBuffer );

                    if( retVal <= 0 && retVal != ERROR_VERIFICATION )
                    {
#ifdef DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_TRANX_PROC, TRANX_IN_PROG_FAIL ); //"Transmit error: unable to transmit",
#endif
                        return retVal;
                    }
                }

                break;

            case TRANSIN:
                trCount = VME_getNumber( bufAlgo, bufAlgoSize, &bufAlgoIndex, 0 );

                if( trCount == PROC_FAIL )
                {
#ifdef DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_ALGO_TRANX, NO_TRANSIN_SIZE ); //"No byte available at Size",
#endif
                    return ERROR_PROC_ALGO;     // no byte available at Size
                }

                byteNum = trCount / 8;

                if( trCount % 8 != 0 )
                    byteNum ++;

                flag_transin = 1;
                break;

            case MASK:
                if( trCount <= MAX_MASKSIZE )
                {
                    for( i = 0; i < byteNum; i++ )
                    {
                        if( !VME_getByte( &( maskBuffer[i] ), bufAlgo, bufAlgoSize, &bufAlgoIndex ) )
                        {
#ifdef DEBUG_LEVEL_1
                            dbgu_putint( DBGU_L1_ALGO_TRANX, NO_TRANSIN_MASK ); //"No byte available at Mask",
#endif
                            return ERROR_PROC_ALGO;     // no byte available at Mask
                        }
                    }

                    flag_mask = 1;
                }

                break;

            case ENDTRAN:
                if( !TRANS_endtranx() )
                {
#ifdef DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_TRANX_PROC, ENDTRAN_FAIL );
#endif
                    return ERROR_PROC_HARDWARE;
                }

                if( bufAlgo != 0 )
                    ( *absbufAlgoIndex ) += bufAlgoIndex;

                if( mismatch )
                {
                    return ERROR_VERIFICATION;
                }
                else
                    return PROC_COMPLETE;

                break;

            case REPEAT:
                // starts repeat. Buffer whatever in the repeat loop.
                //************************************************************************
                //* REPEAT opcode is followed by the number of repeats.
                //* Then it start processing the transmission by calling proc_REPEAT().
                //************************************************************************
                temp = VME_getNumber( bufAlgo, bufAlgoSize, &bufAlgoIndex, 0 );

                if( temp == PROC_FAIL )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_ALGO_PROC, NO_NUMBER_OF_REPEAT );
#endif
                    return ERROR_PROC_ALGO;
                }
                else
                {
                    if( bufAlgo == 0 )
                        retVal = proc_REPEAT( bufAlgo, bufAlgoSize, &bufAlgoIndex, temp );
                    else
                        retVal = proc_REPEAT( &( bufAlgo[bufAlgoIndex] ), bufAlgoSize, &bufAlgoIndex, temp );

                    if( retVal <= 0 )
                    {
#ifdef  DEBUG_LEVEL_1
                        dbgu_putint( DBGU_L1_PROCESS, REPEAT_FAIL );
#endif
                        return ERROR_PROC_ALGO;
                    }
                }

                break;

            case RESETDATA:
                if( !dataReset( 1 ) )
                {
#ifdef  DEBUG_LEVEL_1
                    dbgu_putint( DBGU_L1_PROCESS, RESETDATA_FAIL ); // fail to reset data
#endif
                    retVal = ERROR_PROC_DATA;
                }

                break;

            default:
                if( bufAlgo != 0 )
                {
                    if( bufAlgo != 0 )
                        ( *absbufAlgoIndex ) += bufAlgoIndex;

                    if( mismatch )
                    {
                        return ERROR_VERIFICATION;
                    }
                    else
                        return PROC_COMPLETE;
                }

#ifdef DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_ALGO_TRANX, UNRECOGNIZED_OPCODE );
#endif
                return ERROR_PROC_ALGO;
                break;
        }

        if( !VME_getByte( &currentByte, bufAlgo, bufAlgoSize, &bufAlgoIndex ) )
        {
#ifdef DEBUG_LEVEL_1
            dbgu_putint( DBGU_L1_ALGO_TRANX, NO_TRANX_OPCODE );
#endif
            return ERROR_PROC_ALGO;
        }
    }

    if( bufAlgo != 0 )
        ( *absbufAlgoIndex ) += bufAlgoIndex;

    if( retVal <= 0 )
        return retVal;

    if( mismatch )
    {
#ifdef DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_TRANX_PROC, COMPARE_FAIL );
        // internal debugging
        dbgu_putint( 8, mismatch );
#ifdef DEBUG_LEVEL_3

        for( i = 0; i < 4; i++ )
        {
            dbgu_putint( 2, ( unsigned int )( trBuffer[i] >> 4 ) ); //"Mismatch occurs",
            dbgu_putint( 2, ( unsigned int )( trBuffer[i] & 0x0f ) ); //"Mismatch occurs",
        }

#endif
#endif
        return ERROR_VERIFICATION;
    }
    else
        return PROC_COMPLETE;

}

/************************************************************************
* Function proc_REPEAT
* Process Repeat block
*
* Input:
* bufAlgo       - the pointer to buffered algorithm if available
* bufAlgoSize   - the size of buffered algorithm
*                 this field is discarded if bufAlgo is 0
*   absBufAlgoIndex - the pointer to the absolute Algorithm Index from the caller
*                     so proc_REPEAT accumulates the absolute index while
*                     processing the transmission
* LoopMax       - Max number of repeat
*
* Return:
* PROC_FAIL     - loop condition not met
* PROC_COMPLETE - loop condition met
************************************************************************/
int proc_REPEAT( unsigned char *bufAlgo, unsigned int bufAlgoSize,
                 unsigned int *absBufAlgoIndex, unsigned int LoopMax )
{
    unsigned char       *bufferPtr   = 0;   /* loop / repeat buffer */
    unsigned int        bufferSize   = 0;   /* size of algorithm within loop / repeat */
    unsigned int        loopCount    = 0;
    unsigned int        stackIndex   = 0;
    unsigned char       currentByte  = 0;
    unsigned int        bufAlgoIndex = 0;
    int                 flag         = 1;   /* initialize to 1 for counting / buffering algo */
    /* when processing loop, flag becomes the condition */
    /* whether the loop succeed or fail. */


    int i = 0;

    // initialize bufferPtr.  If processed is not buffered,
    // it points to the beginning of buffer.  Else it points to where bufAlgo points to
    if( bufAlgo == 0 )
    {
        for( i = 0; i < MAXBUF; i++ )
        {
            algoBuffer[i] = 0;
        }

        bufferPtr = algoBuffer; // points to the global buffer
    }
    else
        bufferPtr = bufAlgo;

#ifdef DEBUG_LEVEL_2
    dbgu_putint( DBGU_L2_REPEAT, PREPARE_BUFFER );
#endif

    do
    {
        if( !VME_getByte( &currentByte, bufAlgo, bufAlgoSize, &bufAlgoIndex ) )
        {
#ifdef DEBUG_LEVEL_1
            dbgu_putint( DBGU_L1_REPEAT, BUFFER_FAIL );
#endif
            return ERROR_PROC_ALGO;
        }

        if( currentByte == HCOMMENT )
        {
            if( proc_HCOMMENT( bufAlgo, bufAlgoSize, &bufAlgoIndex, 0 ) == PROC_FAIL )
            {
#ifdef DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_REPEAT, REPEAT_COMMENT_FAIL );
#endif
                return ERROR_PROC_ALGO;
            }
        }
        /* nested LOOP / REPEAT check
           if currentByte is LOOP or REPEAT, nested loop, put in stack */
        else if( currentByte == LOOP || currentByte == REPEAT )
        {
            stack[stackIndex] = currentByte;
            stackIndex++;
        }
        /* if currentByte is ENDREPEAT, pop from stack */
        else if( currentByte == ENDREPEAT )
        {
            if( stackIndex == 0 )
                flag = 0;       /* end of loop */
            else if( stack[stackIndex - 1] != REPEAT )
            {
#ifdef DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_REPEAT, STACK_MISMATCH ); /* Stack mismatch when buffering REPEAT */
#endif
                return ERROR_PROC_ALGO;
            }
            else
                stackIndex--;
        }
        /* if currentByte is ENDLOOP, check if its end of loop,
           or pop from stack if its end of nested loop */
        else if( currentByte == ENDLOOP )
        {
            if( stackIndex == 0 || stack[stackIndex - 1] != LOOP )
            {
#ifdef DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_REPEAT, STACK_MISMATCH );
#endif
                return ERROR_PROC_ALGO;
            }
            else
                stackIndex--;
        }

        /* discard comment for buffering */
        if( flag == 1 && currentByte != HCOMMENT )
        {
            if( bufferSize > MAXBUF )
            {
#ifdef DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_REPEAT, REPEAT_SIZE_EXCEED );
#endif
                return ERROR_PROC_ALGO;
            }
            else
            {
                if( bufAlgo == 0 )
                    algoBuffer[bufferSize] = currentByte;

                bufferSize++;
            }
        }
    }
    while( flag == 1 );

#ifdef DEBUG_LEVEL_2
    dbgu_putint( DBGU_L2_REPEAT, FINISH_BUFFER ); /* End calculate REPEAT size, and buffer REPEAT */
#endif
    bufAlgoIndex = 0;
    /* process REPEAT */
    flag = 0;
    loopCount = 0;
#ifdef DEBUG_LEVEL_2
    dbgu_putint( DBGU_L2_REPEAT, START_PROC_REPEAT );
#endif

    do
    {
        flag = SSPIEm_process( bufferPtr, bufferSize );
        loopCount ++;
    }
    while( flag == PROC_OVER && loopCount < LoopMax );

    if( flag <= 0 )
    {
#ifdef DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_REPEAT, REPEAT_COND_FAIL ); /* REPEAT condition fails */
#endif
        ( *absBufAlgoIndex ) += bufferSize + 1;
        return flag;
    }
    else
    {
#ifdef DEBUG_LEVEL_2
        dbgu_putint( DBGU_L2_REPEAT, END_PROC_REPEAT ); /* End processing REPEAT */
#endif
        ( *absBufAlgoIndex ) += bufferSize + 1;
        return PROC_COMPLETE;
    }
}
/**************************************************************************
* Function proc_LOOP
* Process Loop block
*
* The function will see if the processes is being buffered.  If the
* processes is not buffered, it buffers the processes.  Else it will
* calculate the maximum of the buffer between LOOP - ENDLOOP block.
*
* Note that the format of the loop requires the condition check to be
* the end of the loop block.  Once the last process succeed, the
* loop is completed.
*
* The loop will break if all the processes within the loop succeed.
* It will continue if any step in the loop fails.  Therefore it is
* better to put the condition process, such as TRANS with TRANSIN, as
* the last process in a loop so it will go through all the processes
* before deciding whether the loop will continue or break.
*
* Input:
* bufAlgo           - the pointer to buffered algorithm if available
* bufAlgoSize       - the size of buffered algorithm
*                     this field is discarded if bufAlgo is 0
*   absBufAlgoIndex - the pointer to the absolute Algorithm Index from the caller
*                     so proc_LOOP accumulates the absolute index while
*                     processing the transmission
* LoopMax           - Max number of loop allowed
*
* Return:
* PROC_FAIL     - loop condition not met
* PROC_COMPLETE - loop condition met
**************************************************************************/
int proc_LOOP( unsigned char *bufAlgo, unsigned int bufAlgoSize,
               unsigned int *absBufAlgoIndex, unsigned int LoopMax )
{
    unsigned char       *bufferPtr     = 0; /* loop buffer */
    unsigned int        bufferSize     = 0; /* size of algo within loop */
    unsigned int        loopCount      = 0;
    unsigned int        stackIndex     = 0;
    unsigned char       currentByte    = 0;
    unsigned int        bufAlgoIndex   = 0;
    int                 flag           = 1; /* initialize to 1 for counting / buffering algo */
    /*when processing loop, flag becomes the condition */
    /*whether the loop succeed or fail. */


    int i = 0;

    // initialize bufferPtr.  If processed is not buffered,
    // it points to the beginning of buffer.  Else it points to where bufAlgo points to
    if( bufAlgo == 0 )
    {
        for( i = 0; i < MAXBUF; i++ )
        {
            algoBuffer[i] = 0;
        }

        bufferPtr = algoBuffer; // points to the global buffer
    }
    else
        bufferPtr = bufAlgo;

#ifdef DEBUG_LEVEL_2
    dbgu_putint( DBGU_L2_LOOP, PREPARE_BUFFER );
#endif

    do
    {
        if( !VME_getByte( &currentByte, bufAlgo, bufAlgoSize, &bufAlgoIndex ) )
        {
#ifdef DEBUG_LEVEL_1
            dbgu_putint( DBGU_L1_LOOP, BUFFER_FAIL );
#endif
            return ERROR_PROC_ALGO;
        }

        if( currentByte == HCOMMENT )
        {
            if( proc_HCOMMENT( bufAlgo, bufAlgoSize, &bufAlgoIndex, 0 ) == PROC_FAIL )
            {
#ifdef DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_REPEAT, LOOP_COMMENT_FAIL );
#endif
                return ERROR_PROC_ALGO;
            }
        }
        else if( currentByte == LOOP || currentByte == REPEAT )
        {
            stack[stackIndex] = currentByte;
            stackIndex++;
        }
        else if( currentByte == ENDREPEAT )
        {
            if( stackIndex == 0 || stack[stackIndex - 1] != REPEAT )
            {
#ifdef DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_LOOP, STACK_MISMATCH );
#endif
                return ERROR_PROC_ALGO;
            }
            else
                stackIndex--;
        }
        /* if currentByte is ENDLOOP, check if its end of loop,
           or pop from stack if its end of nested loop */
        else if( currentByte == ENDLOOP )
        {
            if( stackIndex == 0 )
                flag = 0;
            else if( stack[stackIndex - 1] != LOOP )
            {
#ifdef DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_LOOP, STACK_MISMATCH );
#endif
                return ERROR_PROC_ALGO;
            }
            else
                stackIndex--;
        }

        /* discard comment for buffering */
        if( flag == 1 && currentByte != HCOMMENT )
        {
            if( bufferSize > MAXBUF )
            {
#ifdef DEBUG_LEVEL_1
                dbgu_putint( DBGU_L1_LOOP, LOOP_SIZE_EXCEED ); /* Loop size exceed Maximum Buffer size */
#endif
                return ERROR_PROC_ALGO;
            }
            else
            {
                if( bufAlgo == 0 )
                    algoBuffer[bufferSize] = currentByte;

                bufferSize++;
            }
        }
    }
    while( flag == 1 );

#ifdef DEBUG_LEVEL_2
    dbgu_putint( DBGU_L2_LOOP, FINISH_BUFFER ); /* End calculate LOOP size, and buffer LOOP */
#endif
    bufAlgoIndex = 0;
    /* process loop */
    flag = 0;
    loopCount = 0;
#ifdef DEBUG_LEVEL_2
    dbgu_putint( DBGU_L2_LOOP, START_PROC_LOOP );
#endif

    do
    {
        flag = SSPIEm_process( bufferPtr, bufferSize );
        loopCount ++;
    }
    while( flag <= 0 && loopCount < LoopMax );

    if( flag <= 0 )
    {
#ifdef DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_LOOP, LOOP_COND_FAIL ); /*LOOP condition not met */
#endif
        ( *absBufAlgoIndex ) += bufferSize + 1;
        return flag;
    }
    else
    {
#ifdef DEBUG_LEVEL_2
        dbgu_putint( DBGU_L2_LOOP, END_PROC_LOOP ); /*End processing LOOP */
#endif
        ( *absBufAlgoIndex ) += bufferSize + 1;
        return PROC_COMPLETE;
    }
}
/**************************************************************************
* Function proc_HCOMMENT
* Process comment block
*
***************************************************************************/
int proc_HCOMMENT( unsigned char *bufferedAlgo, unsigned int bufferedAlgoSize,
                   unsigned int *absBufferedAlgoIndex, CSU *headerCS )
{
    unsigned char currentByte  = 0;

    do
    {
        if( !VME_getByte( &currentByte, bufferedAlgo, bufferedAlgoSize, absBufferedAlgoIndex ) )
        {

            return PROC_FAIL;
        }

        if( headerCS != 0 )
            putChunk( headerCS, ( unsigned int ) currentByte );
    }
    while( currentByte != HENDCOMMENT );

    return PROC_COMPLETE;
}
/**************************************************************************
*
* VME internal functions
* Internal functions for VME to use
*
**************************************************************************/

/**************************************************************************
* VME_getByte
* Get a byte for algorithm
*
* Input:
* byteOut       - the address of output byte
* bufAlgo       - pointer to the buffered algorithm, if needed
* bufAlgoSize   - the size of buffered algorithm.
*                 this field is ignored if bufAlgo is 0
* bufAlgoIndex- the address of current index of the buffered algorithm
*             this field is ignored if bufAlgo is 0
**************************************************************************/

int VME_getByte( unsigned char *byteOut, unsigned char *bufAlgo,
                 unsigned int bufAlgoSize, unsigned int *bufAlgoIndex )
{
    if( bufAlgo == 0 )
    {
#ifdef DEBUG_LEVEL_3
        dbgu_putint( 12, 1 );
#endif
        algoGetByte( byteOut );
        return 1;
    }
    else
    {
#ifdef DEBUG_LEVEL_3
        dbgu_putint( 12, 2 );
#endif

        if( *bufAlgoIndex < bufAlgoSize )
        {
            *byteOut = bufAlgo[( *bufAlgoIndex )];
            ( *bufAlgoIndex )++;
            return 1;
        }
        else
            return 0;
    }
}
/**************************************************************************
* VME_getByte
* Get a number for algorithm
*
***************************************************************************/
unsigned int VME_getNumber( unsigned char *bufAlgo, unsigned int bufAlgoSize,
                            unsigned int *bufAlgoIndex, unsigned int *byteCount )
{
    unsigned char byteIn = 0x80;
    unsigned int output  = 0;
    short int i          = 0;

    do
    {
        if( !VME_getByte( &byteIn, bufAlgo, bufAlgoSize, bufAlgoIndex ) )
        {
            return PROC_FAIL;
        }
        else
        {
            output += ( unsigned long )( ( byteIn & 0x7F ) << ( 7 * i ) );
            i++;
        }
    }
    while( byteIn & 0x80 );

    if( byteCount )
        *byteCount += i;

    return output;
}

#endif