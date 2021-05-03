#include "LtIntelHex.h"
#include "LtDefines.h"

#include <errno.h>
#include <stdio.h>
#include <fstream>
#include <sstream>

#ifndef _MSC_VER
// Fix for portable scanf_s on Linux
#define sscanf_s sscanf
const char *scanf_validate( const char *fmt, const char *file, long line )
{
    const char *p = fmt;

    while( 1 )
    {
        p = strstr( p, "%s" );

        if( p == nullptr )
            break;

        if( ( p == fmt ) || ( *( p - 1 ) != '%' ) )
        {
            fprintf( stderr, "Hey, you used \"%%s\" in %s: line %ld!\n", file, line );
            return fmt;
        }
    }

    return fmt;
}

#endif

//*****************************************************************************
//*************** Data Type Definitions ***************************************
//*****************************************************************************



/// ****************************************************************************
/// \fn     IHEX_Swap
///
/// \brief  Swaps Intel HEX memory map by groups of 2 bytes.
///
/// \param[in/out] mem: The map to swap.
///
/// \return     Error code. 0 on success, -1 on error.
///
/// \authors    Jean-François Bernier
/// \since      2016-01-07
/// ****************************************************************************
int IntelHEX::IHEX_Swap( IntelHEX::IntelHexMem &mem )
{
    if( ( mem.end - mem.start + 1 ) % 2 )
    {
        return -1;
    }

    for( uint16_t a = mem.start; a < mem.end; a += 2 )
    {
        uint8_t tmp = mem.mem[ a ];
        mem.mem[ a ] = mem.mem[ a + 1 ];
        mem.mem[ a + 1 ] = tmp;
    }

    return 0;
}


/// ****************************************************************************
/// \fn     IHEX_Parse
///
/// \brief  Parses a line of text from an Intel HEX file.
///
/// \param[in] *line: Text line to be parsed into structure
/// \param[out]  hex: Filled Intel HEX structure
///
/// \return     Error code. 0 on success, -1 on error.
///
/// \authors    Jean-François Bernier
/// \since      2015-12-11
/// ****************************************************************************
int IntelHEX::IHEX_Parse( const char *line, IntelHex &hex )
{
    uint32_t sum;
    uint32_t tmp;
    size_t len;
    uint32_t n = 0;
    int err;
    memset( hex.data, 0, sizeof( hex.data ) );

    if( line[ 0 ] != ':' )
    {
        return -1;
    }

    len = strlen( line );

    // Minimum record length is 11 if no data is present
    if( len < 11 )
    {
        return -1;
    }

    ++line;

    err = sscanf_s( line, "%02x", &tmp );

    //err = sscanf( line, "%02x", &tmp );
    if( err == 0 || err == EOF )
    {
        return -1;
    }

    hex.count = tmp;
    line += 2;

    // We accept longer lines. Extra characters can be commments.
    if( ( int )len < ( 11 + hex.count * 2 ) )
    {
        return -1;
    }

    err = sscanf_s( line, "%04x", &tmp );

    //err = sscanf( line, "%04x", &tmp );
    if( err == 0 || err == EOF )
    {
        return -1;
    }

    hex.addr = tmp;
    line += 4;

    err = sscanf_s( line, "%02x", &tmp );

    //err = sscanf( line, "%02x", &tmp );
    if( err == 0 || err == EOF )
    {
        return -1;
    }

    hex.type = tmp;
    line += 2;

    // Start computation of the checksum. Add all bytes together.
    sum = hex.count + ( ( hex.addr >> 8 ) & 0xFF ) + ( hex.addr & 0xFF ) + hex.type;

    while( n < hex.count )
    {
        err = sscanf_s( line, "%02x", &tmp );

        //err = sscanf( line, "%02x", &tmp );
        if( err == 0 || err == EOF )
        {
            return -1;
        }

        hex.data[ n ] = tmp;
        line += 2;
        sum += hex.data[ n ];

        ++n;
    }

    err = sscanf_s( line, "%02x", &tmp );

    //err = sscanf( line, "%02x", &tmp );
    if( err == 0 || err == EOF )
    {
        return -1;
    }

    hex.cksum = tmp;

    // Validate checksum. Sum of the 2 must be zero,
    // because cksum is the 2-complement of the byte sum.
    if( ( ( sum & 0xFF ) + hex.cksum ) & 0xFF )
    {
        return -1;
    }

    return 0;
}


/// ****************************************************************************
/// \fn     IHEX_Load
///
/// \brief  Loads a Intel HEX file into memory. It is up to the caller to allocate
///         the memory block of 65536 bytes and initialize it correctly.
///
/// \param[in]   *aPath: Path to text file to load
/// \param[out]   *aMem: Where to store the loaded data from file. Must point to
///                     an allocated array of 65536 bytes.
///
/// \return     Error code.
/// \retval     0 on success, file contained an EOF record
/// \retval     1 on success, file didn't contained an EOF record
/// \retval     -1 on file open error
/// \retval     -2 on file parsing error
///
/// \authors    Jean-François Bernier
/// \since      2015-12-11
/// ****************************************************************************
int IntelHEX::IHEX_Load( const char *aPath, IntelHEX::IntelHexMem &aMem )
{
    std::ifstream lFileStream;
    lFileStream.exceptions( std::ios_base::failbit );
    lFileStream.open( aPath, std::ifstream::in );

    return IHEX_Load( lFileStream, aMem );
}

/// ****************************************************************************
/// \fn     IHEX_LoadFromBuffer
///
/// \brief  Loads a Intel HEX buffer into memory. It is up to the caller to allocate
///         the memory block of 65536 bytes and initialize it correctly.
///
/// \param[in]   *aBuffer: Buffer to read.
/// \param[out]   *aMem: Where to store the loaded data from file. Must point to
///                     an allocated array of 65536 bytes.
///
/// \return     Error code.
/// \retval     0 on success, file contained an EOF record
/// \retval     1 on success, file didn't contained an EOF record
/// \retval     -1 on file open error
/// \retval     -2 on file parsing error
///
/// \authors    Patirck Boulay
/// \since      Mai 2016
/// ****************************************************************************
int IntelHEX::IHEX_LoadFromBuffer( const uint8_t *aBuffer, uint32_t aSize, IntelHEX::IntelHexMem &aMem )
{
    std::string lBuffer( ( const char * )aBuffer, aSize );
    std::istringstream lStringStream( lBuffer );
    return IHEX_Load( lStringStream, aMem );
}

/// ****************************************************************************
/// \fn     IHEX_Load
///
/// \brief  Loads a Intel HEX stream into memory. It is up to the caller to allocate
///         the memory block of 65536 bytes and initialize it correctly.
///
/// \param[in]   *aStream: Stream to read.
/// \param[out]  *aMem: Where to store the loaded data from file. Must point to
///                     an allocated array of 65536 bytes.
///
/// \return     Error code.
/// \retval     0 on success, file contained an EOF record
/// \retval     1 on success, file didn't contained an EOF record
/// \retval     -1 on file open error
/// \retval     -2 on file parsing error
///
/// \authors    Jean-François Bernier
/// \since      2015-12-11
/// ****************************************************************************

int IntelHEX::IHEX_Load( std::istream &aStream, IntelHEX::IntelHexMem &aMem )
{
    const int MAXLINE = 1024;
    char line[ MAXLINE ];

    IntelHex hex;
    uint32_t lineIdx = 1;     // Line index
    uint32_t total = 0;       // Nb of data bytes loaded
    int res = -1;
    int a;

    aMem.start = 0xFFFF;
    aMem.end = 0x0000;

    while( true )
    {
        line[ 0 ] = '\0';
        aStream.getline( line, MAXLINE );

        if( aStream.eof() )
        {
            res = 1;
            goto END;
        }

        //strlen( line );

        if( IHEX_Parse( line, hex ) < 0 )
        {
            res = -2;
            goto END;
        }

        ++lineIdx;

        switch( hex.type )
        {
            case IHEX_DATA:
                for( a = 0; a < hex.count; ++a )
                {
                    aMem.mem[ hex.addr ] = hex.data[ a ];
                    ++aMem.cnt[ hex.addr ];

                    if( hex.addr < aMem.start )
                    {
                        aMem.start = hex.addr;
                    }

                    if( hex.addr > aMem.end )
                    {
                        aMem.end = hex.addr;
                    }

                    ++hex.addr;
                    ++total;
                }

                break;

            case IHEX_EOF:
                res = 0;
                goto END;
                break;

            default:
                break;
        }
    }

END:

    aMem.nByte = total;
    return res;
}

