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
        RT_SOFT_RESET,
        RT_HARD_RESET,
        RT_CONFIG_RESET
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eResetOptions
    ///
    /// \brief  Values that represent reset options
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum eResetOptions
    {
        RO_NO_OPTION,
        RO_MAIN,
        RO_FACTORY
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