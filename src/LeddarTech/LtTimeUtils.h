// *****************************************************************************
// Module..: LeddarTech
//
/// \file    LtTimeUtils.h
///
/// \brief   Utilities for time.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include <stdint.h>
#include <string>

namespace LeddarUtils
{
    namespace LtTimeUtils
    {
        void Wait( uint32_t aMilliseconds );
        void WaitBlockingMicro( uint32_t aMicroseconds );
    }
}