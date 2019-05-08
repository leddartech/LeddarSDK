// *****************************************************************************
// Module..: LeddarTech
//
/// \file    LtKeyboardUtils.cpp
///
/// \brief   Utilities for keyboards.
///
/// \author  Patrick Boulay
///
/// \since   February 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LtKeyboardUtils.h"
#include "LtTimeUtils.h"

#ifdef _WIN32
#include <conio.h>
#else
#include <cstdio>
#include <dlfcn.h>
#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <limits>
#include <link.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#endif

#ifndef _WIN32
// *****************************************************************************
// Function: SetNonBlocking
//
/// \brief   Special helper function to implement the equivalent of _kbhit and
///          _getch in Linux.
///
/// \param   aState  If 0, the terminal will be put in the state in
///                  which it was at the start of the program which is
///                  normally CANONICAL with ECHO. Otherwise the canonical and
///                  echo mode will be disabled.
// *****************************************************************************

void
LeddarUtils::LtKeyboardUtils::SetNonBlocking( bool aState )
{
    static int sInitDone = 0;
    static struct termios sOriginalParams;

    if( !sInitDone )
    {
        tcgetattr( STDIN_FILENO, &sOriginalParams );
        sInitDone = -1;
    }

    if( aState )
    {
        struct termios lNewParams = sOriginalParams;

        lNewParams.c_lflag = ~( ICANON | ECHO );
        tcsetattr( STDIN_FILENO, TCSANOW, &lNewParams );
    }
    else
    {
        tcsetattr( STDIN_FILENO, TCSANOW, &sOriginalParams );
    }
}
#endif

// *****************************************************************************
// Function: GetKey
//
/// \brief   Returns the next character in the keyboard buffer without a need
///          to press enter (used to navigate the menu).
///
/// \return  The character (may have some special values for non printable
///          keys).
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

int
LeddarUtils::LtKeyboardUtils::GetKey( void )
{
#ifdef _WIN32
    return _getch();
#else
    int lResult;

    SetNonBlocking( -1 );
    lResult = getchar();
    SetNonBlocking( 0 );

    return lResult;
#endif
}

// *****************************************************************************
// Function: KeyPressed
//
/// \brief   Verify if a key was pressed and is waiting in the buffer.
///
/// \return  Non zero if a key available, 0 if no key available.
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

bool
LeddarUtils::LtKeyboardUtils::KeyPressed( void )
{
#ifdef _WIN32
    return _kbhit() != 0;
#else
    struct timeval lTimeout;
    fd_set lFds;

    lTimeout.tv_sec = 0;
    lTimeout.tv_usec = 0;

    FD_ZERO( &lFds );
    FD_SET( STDIN_FILENO, &lFds );

    SetNonBlocking( -1 );
    select( STDIN_FILENO + 1, &lFds, NULL, NULL, &lTimeout );
    SetNonBlocking( 0 );
    return FD_ISSET( STDIN_FILENO, &lFds );
#endif
}

// *****************************************************************************
// Function: WaitKey
//
/// \brief   Wait for a key to be pressed on the keyboard, pinging the sensor
///          to keep the connection alive while waiting.
///
/// \return  The character corresponding to the key pressed (converted to
///          uppercase for letters).
// *****************************************************************************

char
LeddarUtils::LtKeyboardUtils::WaitKey( void )
{
#ifdef _WIN32

    // LeddarGetKey is blocking so we need to wait for a key to be pressed
    // before calling it.
    while( !KeyPressed() )
    {
        LtTimeUtils::Wait( 500 );
    }

    return toupper( GetKey() );
#else

    int ch;
    struct termios oldt, newt;

    do
    {
        long oldf, newf;
        tcgetattr( STDIN_FILENO, &oldt );
        newt = oldt;
        newt.c_lflag &= ~( ICANON | ECHO );
        tcsetattr( STDIN_FILENO, TCSANOW, &newt );
        oldf = fcntl( STDIN_FILENO, F_GETFL, 0 );
        newf = oldf | O_NONBLOCK;
        fcntl( STDIN_FILENO, F_SETFL, newf );
        ch = getchar();
        fcntl( STDIN_FILENO, F_SETFL, oldf );
        tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
    }
    while( ch == EOF );

    return static_cast<char>( ch );
#endif
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
