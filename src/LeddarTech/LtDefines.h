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
#define LT_SDK_MINOR_VERSION 2
#define LT_SDK_TYPE_VERSION  0
#define LT_SDK_BUILD_VERSION 85

#define LT_SDK_VERSION_STRING_BUILDER( a, b, c, d ) #a "." #b "." #c "." #d
#define LT_SDK_VERSION_MACRO( a, b, c, d ) LT_SDK_VERSION_STRING_BUILDER( a, b, c, d )
#define LT_SDK_VERSION_STRING LT_SDK_VERSION_MACRO( LT_SDK_MAJOR_VERSION, LT_SDK_MINOR_VERSION, LT_SDK_TYPE_VERSION, LT_SDK_BUILD_VERSION )


// Various define if you don't need / want to build the entire SDK
// Sensor type
#define BUILD_ONE               /// LeddarOne sensors
#define BUILD_VU                /// LeddarVu sensors
#define BUILD_M16               /// M16 / IS16 / Evalkit
#define BUILD_AUTO              /// LeddarAuto sensors
#define BUILD_DTEC              /// DTec sensors
// Physical Protocol
#define BUILD_MODBUS            /// Modbus (includes Vu8 serial)
#define BUILD_SPI               /// Generic SPI (for hardware independent SPI)
#define BUILD_SPI_FTDI          /// SPI using FTDI hardware
//#define BUILD_SPI_BCM2835       /// SPI using BCM2835 (raspberry pi)
#define BUILD_CANBUS            /// Generic CANBus (for hardware independent CAN)
#define BUILD_CANBUS_KOMODO     /// CANBus using Komodo hardware
#define BUILD_USB               /// Usb
#define BUILD_ETHERNET          /// Ethernet