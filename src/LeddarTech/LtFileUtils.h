// *****************************************************************************
// Module..: LeddarTech
//
/// \file    LtFileUtils.h
///
/// \brief   Utilities for file manipulation
///
/// \author  Patrick Boulay
///
/// \since   March 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include <stdint.h>
#include <vector>
#include <string>

namespace LeddarUtils
{
    namespace LtFileUtils
    {
        std::vector<uint8_t> ReadFileToBuffer( const std::string &aFilename );
        std::string FileExtension( const std::string &aFilename );
    }
}