// *****************************************************************************
// Module..: LeddarTech
//
/// \file    LtSystemUtils.h
///
/// \brief   Utilities for time.
///
/// \author  Patrick Boulay
///
/// \since   June 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"

#include <string>
#include <vector>

namespace LeddarUtils
{
    namespace LtSystemUtils
    {
        std::string GetEnvVariable( const std::string &aVariableName );
        bool IsEnvVariableExist( const std::string &aVariableName );
        std::vector<std::string> GetSerialPorts( void );

#ifndef _WIN32
        bool DirectoryExists( const std::string &aPath );
#endif
    }
}