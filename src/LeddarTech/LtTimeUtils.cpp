// *****************************************************************************
// Module..: LeddarTech
//
/// \file    LtTimeUtils.cpp
///
/// \brief   Utilities for time
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LtTimeUtils.h"
#include "LtDefines.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <time.h>
#endif

// *****************************************************************************
// Function: LtTimeUtils::Wait
//
/// \brief   Sleep in milliseconds.
///
/// \param   aMilliseconds Time to sleep.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void LeddarUtils::LtTimeUtils::Wait( uint32_t aMilliseconds )
{
#ifdef _WIN32
    Sleep( aMilliseconds );
#else
    usleep( aMilliseconds * 1000 );
#endif
}

// *****************************************************************************
// Function: LtTimeUtils::WaitBlockingMicro
//
/// \brief   Sleep for a given number of microseconds. Keep the cpu busy, so
///          it should be used only for short pause duration.
///
/// \param  aMicroseconds    Number of microseconds to sleep.
///
/// \author  Vincent Simard Bilodeau
///
/// \since   Jully 2016
// *****************************************************************************
void LeddarUtils::LtTimeUtils::WaitBlockingMicro( uint32_t aMicroseconds )
{
#ifdef _WIN32
    unsigned __int64 freq;
    QueryPerformanceFrequency( ( LARGE_INTEGER * )&freq );
    double timerFrequency = ( 1.0 / ( freq / 1e6 ) );
    unsigned __int64 startTime;
    QueryPerformanceCounter( ( LARGE_INTEGER * )&startTime );
    unsigned __int64 endTime;

    do
    {
        QueryPerformanceCounter( ( LARGE_INTEGER * )&endTime );
    }
    while( ( ( endTime - startTime ) * timerFrequency ) < aMicroseconds );

#else
    int lnsec = 1000 * aMicroseconds;
    int lsec = 1000000000; //1 second in nanosecond
    const struct timespec timewait = {lnsec / lsec, lnsec % lsec};
    nanosleep( &timewait, nullptr );
#endif
}
