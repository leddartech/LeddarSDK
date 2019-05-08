// *****************************************************************************
// Module..: LeddarTech
//
/// \file    LtSystemUtils.cpp
///
/// \brief   System utilities
///
/// \author  Patrick Boulay
///
/// \since   June 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LtSystemUtils.h"
#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#include "LtStringUtils.h"
#else
#include <dirent.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#endif


// *****************************************************************************
// Function: LtSystemUtils::GetEnvVariable
//
/// \brief   Get enviroment variable
///
/// \param   aVariableName Environment variable name.
///
/// \author  Patrick Boulay
///
/// \since   June 2016
// *****************************************************************************

std::string
LeddarUtils::LtSystemUtils::GetEnvVariable( const std::string &aVariableName )
{
#ifdef _MSC_VER
    std::string lResult;
    char *lValue;
    size_t lLength;
    errno_t lErr = _dupenv_s( &lValue, &lLength, aVariableName.c_str() );

    if( lErr || lValue == nullptr )
    {
        return "";
    }

    lResult = lValue;
    free( lValue );

    return lResult;

#else
    char *lGetTraceByChannelPtr = std::getenv( aVariableName.c_str() );

    if( lGetTraceByChannelPtr == nullptr )
    {
        return "";
    }
    else
    {
        return lGetTraceByChannelPtr;
    }

#endif


}

// *****************************************************************************
// Function: LtSystemUtils::IsEnvVariableExist
//
/// \brief   Returns if the environment variable exist.
///
/// \param   aVariableName Environment variable name.
///
/// \return  True if the variable exist.
///
/// \author  Patrick Boulay
///
/// \since   June 2016
// *****************************************************************************

bool
LeddarUtils::LtSystemUtils::IsEnvVariableExist( const std::string &aVariableName )
{
#ifdef _MSC_VER
    char *lValue;
    size_t lLength;
    errno_t lErr = _dupenv_s( &lValue, &lLength, aVariableName.c_str() );

    if( lErr || lValue == nullptr )
    {
        return false;
    }
    else
    {
        return true;
    }

#else
    return ( std::getenv( aVariableName.c_str() ) == nullptr ? false : true );
#endif
}


#ifndef _WIN32
// *****************************************************************************
// Function: LtSystemUtils::DirectoryExists
//
/// \brief   Check if the directory exist
///
/// \param  aPath   Path of the directory
///
/// \return True if the directory exist.
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

bool
LeddarUtils::LtSystemUtils::DirectoryExists( const std::string &aPath )
{
    struct stat lInfo;

    if( stat( aPath.c_str(), &lInfo ) != 0 )
    {
        return false;
    }
    else if( lInfo.st_mode & S_IFDIR )
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif

// *****************************************************************************
// Function: LtSystemUtils::GetSerialPorts
//
/// \brief   Get the serial port list
///
/// \return  Return the list of serial ports available in the OS.
///          ex: Windows: \\\\.\\COM1, Linux: /dev/ttyS1, /dev/ttyUSB0
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

std::vector<std::string>
LeddarUtils::LtSystemUtils::GetSerialPorts( void )
{
    std::vector<std::string> lOutputList;
#ifdef _WIN32
    size_t lSize = 5000;
    std::vector<TCHAR> lpTargetPath( lSize ); // buffer to store the path of the COMPORTS
    std::string lCOMStr( "COM" );

    for( int i = 0; i < 255; i++ ) // checking ports from COM0 to COM255
    {
        std::string lCOMName = lCOMStr + LeddarUtils::LtStringUtils::IntToString( i );

        DWORD lResult = QueryDosDevice( lCOMName.c_str(), ( LPSTR )&lpTargetPath[0], 5000 );

        // Test the return value and error if any
        if( lResult != 0 ) //QueryDosDevice returns zero if it didn't find an object
        {
            lOutputList.push_back( std::string( "\\\\.\\" ) + lCOMName );
        }

        if( ::GetLastError() == ERROR_INSUFFICIENT_BUFFER )
        {
            lSize *= 2;

            if( lSize > 20000 )
            {
                throw std::logic_error( "Vector to query COM device is out of space." );
            }

            lpTargetPath.resize( lSize ); // in case the buffer got filled, increase size of the buffer.
            continue;
        }

    }

#else

    std::string lBaseDir( "/sys/class/tty/" );
    DIR *dp;
    struct dirent *dirp;

    if( ( dp = opendir( lBaseDir.c_str() ) ) == nullptr )
    {
        return lOutputList;
    }

    while( ( dirp = readdir( dp ) ) != nullptr )
    {
        std::string lDirName( dirp->d_name );
        std::string lPathToDevice = lBaseDir + lDirName + std::string( "/device" );

        if( DirectoryExists( lPathToDevice ) )
        {
            lOutputList.push_back( std::string( "/dev/" ) + lDirName );
        }
    }

    closedir( dp );

#endif
    return lOutputList;
}
