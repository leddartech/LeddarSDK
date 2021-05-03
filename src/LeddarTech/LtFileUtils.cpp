// *****************************************************************************
// Module..: LeddarTech
//
/// \file    LtFileUtils.cpp
///
/// \brief   Utilities for file manipulation
///
/// \author  Patrick Boulay
///
/// \since   March 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LtFileUtils.h"
#include "LtExceptions.h"
#include "LtStringUtils.h"

#include <fstream>
#include <cerrno>

/// *****************************************************************************
/// Function: ReadFileToBuffer
///
/// \brief   Read a file and store it in an array of bytes.
///
/// \param   aFilename File name to read.
///
/// \return  Content of the file in an array of bytes.
///
/// \author  Patrick Boulay
///
/// \since   March 2017
/// *****************************************************************************
std::vector<uint8_t>
LeddarUtils::LtFileUtils::ReadFileToBuffer( const std::string &aFilename )
{
    std::ifstream lFile;
    lFile.exceptions( std::ios_base::failbit );

    try
    {
        lFile.open( aFilename.c_str(), std::ios::binary | std::ios::ate );
    }
    catch( ... )
    {
        throw LeddarException::LtException( "File " + aFilename + " not found." );
    }

    if( lFile.is_open() )
    {
        std::streamsize lSize = lFile.tellg();

        if( lSize < 0 )
        {
            throw LeddarException::LtException( "Unable to read file " + aFilename );
        }

        lFile.seekg( 0, std::ios::beg );
        std::vector<uint8_t> lResultBuffer( static_cast<uint32_t>( lSize ) );

        if( !lFile.read( ( char * )&lResultBuffer[ 0 ], lSize ) )
        {
            throw LeddarException::LtException( "Unable to read file " + aFilename );
        }

        return lResultBuffer;
    }

    return std::vector<uint8_t>();
}

/// *****************************************************************************
/// Function: FileExtension
///
/// \brief   Return the extension of a file (everything after the last dot of the file name)
///
/// \param   aFilename File you need the extension from
///
/// \return  File extensio.
///
/// \author  David Levy
///
/// \since   May 2018
/// *****************************************************************************
std::string
LeddarUtils::LtFileUtils::FileExtension( const std::string &aFilename )
{
    if( aFilename.find_last_of( "." ) == std::string::npos )
        return "";
    else
        return aFilename.substr( aFilename.find_last_of( "." ) + 1 );
}



// *****************************************************************************
// Function: LtFileUtils::LoadHex
//
/// \brief   Load a LTB file.
///
/// \param  aFilename   Filename
///
/// \return An IntelHexMem struct.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************

IntelHEX::IntelHexMem *
LeddarUtils::LtFileUtils::LoadHex( const std::string &aFilename )
{
    // Open intel hex file
    IntelHEX::IntelHexMem *lMem = new IntelHEX::IntelHexMem;

    if( IHEX_Load( aFilename.c_str(), *lMem ) < 0 )
    {
        throw std::ios_base::failure( "File " + aFilename + " not found." );
    }


    return lMem;
}

// *****************************************************************************
// Function: LtFileUtils::::LoadHexFromBuffer
//
/// \brief   Load a Hex file from memory buffer.
///
/// \param  aBuffer   Memory buffer
/// \param  aSize     Buffer size
///
/// \return An IntelHexMem struct.
///
/// \author  Patrick Boulay
///
/// \since   Mai 2016
// *****************************************************************************

IntelHEX::IntelHexMem *
LeddarUtils::LtFileUtils::LoadHexFromBuffer( const uint8_t *aBuffer, uint32_t aSize )
{
    // Open intel hex file
    IntelHEX::IntelHexMem *lMem = new IntelHEX::IntelHexMem;

    if( IHEX_LoadFromBuffer( aBuffer, aSize, *lMem ) < 0 )
    {
        throw std::ios_base::failure( "Buffer not valid." );
    }


    return lMem;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarUtils::LtFileUtils::LtLtbReader::LtLtbReader( const std::string& aFileName )
///
/// \brief  Constructor. Open the file and check that it has the right signature
///
/// \param  aFileName   Filename of the file.
///
/// \author David Levy
/// \date   July 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarUtils::LtFileUtils::LtLtbReader::LtLtbReader( const std::string &aFileName ) :
    mFile(),
    mDeviceType( 0 )
{
    std::string lExtension = LeddarUtils::LtStringUtils::ToLower( LeddarUtils::LtFileUtils::FileExtension( aFileName ) );

    if( lExtension != "ltb" )
    {
        throw std::logic_error( "Firmware upgrade file must be a LeddarTech Binary file (ltb)" );
    }

    mFile.exceptions( mFile.failbit );
    mFile.open( aFileName.c_str(), std::ios_base::in | std::ios_base::binary );

    if( !mFile.is_open() )
    {
        throw std::logic_error( "Could not open file - Error code: " + LeddarUtils::LtStringUtils::IntToString( errno ) );
    }

    //File headers
    uint32_t lValue;
    mFile.read( reinterpret_cast<char *>( &lValue ), sizeof( lValue ) );

    if( lValue != LTB_SIGNATURE )
    {
        throw std::logic_error( "Wrong signature file." );
    }

    mFile.read( reinterpret_cast<char *>( &lValue ), sizeof( lValue ) );

    if( lValue != LT_DOCUMENT_VERSION && lValue != LT_DOCUMENT_VERSION_SDK && lValue != LT_DOCUMENT_VERSION_SDK_POST_DOUBLE_BUFFER_REWORK )
    {
        throw std::logic_error( "Wrong document version." );
    }

    //First section
    LtElementHeader lHeader;
    mFile.read( reinterpret_cast<char *>( &lHeader ), sizeof( lHeader ) );

    // The header just read must be that of a section.
    if( ( ( lHeader.mFlags & LTDF_SECTION ) == 0 ) || ( lHeader.mCount != 1 ) )
    {
        throw std::logic_error( "Error reading main section from file" );
    }

    if( lHeader.mId != ID_LTB_FIRMWARE_SECTION )
    {
        throw std::logic_error( "File does not contain firmware data." );
    }

    int64_t lSizeToRead = lHeader.mUnitSize;

    lSizeToRead -= Read( reinterpret_cast<char *>( &lHeader ), sizeof( lHeader ) );

    if( lHeader.mId != ID_LTB_DEVICE_TYPE || sizeof( mDeviceType ) != lHeader.mUnitSize )
    {
        throw std::logic_error( "File does not contain firmware data." );
    }

    lSizeToRead -= Read( reinterpret_cast<char *>( &mDeviceType ), sizeof( mDeviceType ) );

    //Loop through all the firmware data
    while( lSizeToRead > 0 )
    {
        lSizeToRead -= Read( reinterpret_cast<char *>( &lHeader ), sizeof( lHeader ) );
        mFirmwares.push_back( std::make_pair( lHeader.mId, std::vector<uint8_t>( lHeader.mCount ) ) );
        lSizeToRead -= Read( reinterpret_cast<char *>( mFirmwares.back().second.data() ), lHeader.mCount );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarUtils::LtFileUtils::LtLtbReader::~LtLtbReader()
///
/// \brief  Destructor.
///
/// \author David Levy
/// \date   July 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarUtils::LtFileUtils::LtLtbReader::~LtLtbReader()
{
    //mFile.close();
}
