// *****************************************************************************
// Module..: Leddar
//
/// \file    LdDefines.h
///
/// \brief   Defines and enums
///
/// \author  Patrick Boulay
///
/// \since   April 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include <stdint.h>
#include <string>

namespace LeddarDefines
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eResetType
    ///
    /// \brief  Values that represent reset types
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum eResetType
    {
        RT_SOFT_RESET,  ///< Software reset
        RT_HARD_RESET,  ///< Hardware reset
        RT_CONFIG_RESET ///< Reset to factory default configuration
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eResetOptions
    ///
    /// \brief  Values that represent reset options
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum eResetOptions
    {
        RO_NO_OPTION = 0,
        RO_MAIN      = 1 << 1, ///< Reset the sensor to the standard firmware
        RO_FACTORY   = 1 << 2, ///< Reset the sensor to the backup factory firmware
        RO_SAFEMODE  = 1 << 3  ///< Reset safe mode state
    };

    struct sLicense
    {
        uint16_t    mType;
        uint8_t     mSubType;
        std::string mLicense;
    };

    /// \def eLicenseType
    /// \brief License type available
    enum eLicenseType
    {
        LT_NO = 0,
        LT_ADMIN = 1,
        LT_INTEGRATOR = 2,
        LT_PUBLIC = 4,
        LT_TRACE = 8,
        LT_COUNT
    };
}