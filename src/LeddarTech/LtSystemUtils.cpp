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

#include "LtStringUtils.h"

#include <cstdlib>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dirent.h>
#include <string>
#include <vector>
#endif

#include <sys/stat.h>


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
    std::string lPath = aPath;
#ifdef _WIN32

    //On windows the path must not end with a \ except if its the root of the drive
    if( lPath.length() > 0 && lPath.back() == '\\' && std::count( lPath.begin(), lPath.end(), '\\' ) > 1 )
    {
        lPath = lPath.substr( 0, lPath.size() - 1 );
    }
    else if( lPath.length() > 0 && lPath.back() != '\\' && std::count( lPath.begin(), lPath.end(), '\\' ) == 0 )
    {
        lPath.push_back( '\\' );
    }

#endif
    struct stat lInfo;

    if( stat( lPath.c_str(), &lInfo ) != 0 )
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::string LeddarUtils::LtSystemUtils::ErrnoToString( int aErrno )
///
/// \brief  Convert error into a readable string
///
/// \param  aErrno  The error number.
///
/// \returns    A std::string.
///
/// \author David Lévy
/// \date   January 2020
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string LeddarUtils::LtSystemUtils::ErrnoToString( int aErrno )
{
    std::string lError = "";

#ifdef _WIN32
    char msgbuf [256] = {0};
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, ///< flags
                   NULL,                                                       ///< lpsource
                   aErrno,                                                 ///< message id
                   MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),                ///< languageid
                   msgbuf,                                                     ///< output buffer
                   sizeof( msgbuf ),                                           ///< size of msgbuf, bytes
                   NULL );

    if( msgbuf[0] != 0 )
        lError += ": " + std::string( msgbuf );

#else
    locale_t locale = newlocale( LC_CTYPE_MASK | LC_NUMERIC_MASK | LC_TIME_MASK | LC_COLLATE_MASK |
                                 LC_MONETARY_MASK | LC_MESSAGES_MASK, "", ( locale_t )0 );
    std::string lTemp = std::string( strerror_l( aErrno, locale ) );

    if( lTemp.length() != 0 )
        lError += ": " + lTemp;

#endif // _WIN32

    return LeddarUtils::LtStringUtils::IntToString( aErrno ) + lError;
}