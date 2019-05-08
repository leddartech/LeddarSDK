// *****************************************************************************
// Module..: LeddarTech
//
/// \file    LtKeyboardUtils.h
///
/// \brief   Utilities for keyboards.
///
/// \author  Patrick Boulay
///
/// \since   February 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"

namespace LeddarUtils
{
    namespace LtKeyboardUtils
    {
        int GetKey( void );
        bool KeyPressed( void );
        char WaitKey( void );

#ifndef _WIN32
        void SetNonBlocking( bool aState );
#endif
    }
}