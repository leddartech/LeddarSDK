// *****************************************************************************
// Module..: LeddarTech
//
/// \file    LtDefines.h
///
/// \brief   Global definitions for the LeddarTech module.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#define LT_ALEN( a ) (sizeof(a)/sizeof(a[0]))

#define NOMINMAX //Remove windows useless macro min / max

#ifndef _WIN32
#if defined(__cplusplus) && __cplusplus < 201103L
#include <cstddef>
#define nullptr NULL
#define override
#endif
#endif

// Version
#define LT_SDK_MAJOR_VERSION 4
#define LT_SDK_MINOR_VERSION 3
#define LT_SDK_TYPE_VERSION  0
#define LT_SDK_BUILD_VERSION 675

#define LT_SDK_VERSION_STRING_BUILDER( a, b, c, d ) #a "." #b "." #c "." #d
#define LT_SDK_VERSION_MACRO( a, b, c, d ) LT_SDK_VERSION_STRING_BUILDER( a, b, c, d )
#define LT_SDK_VERSION_STRING LT_SDK_VERSION_MACRO( LT_SDK_MAJOR_VERSION, LT_SDK_MINOR_VERSION, LT_SDK_TYPE_VERSION, LT_SDK_BUILD_VERSION )