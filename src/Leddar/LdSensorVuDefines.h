// *****************************************************************************
// Module..: Leddar
//
/// \file    LdSensorVuDefines.h
///
/// \brief   Define M7 sensors
///
/// \author  Patrick Boulay
///
/// \since   August 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#if defined(BUILD_VU)

#include <vector>

namespace LeddarDefines
{
    namespace LdSensorVuDefines
    {

        enum eUpdateType
        {
            UT_ASIC,
            UT_FIRMWARE,
            UT_FPGA
        } ;

        enum eFirmwareUpdateProgress
        {
            FIUP_JUMP_IN_BOOTLOADER = 1,
            FIUP_ENABLE_WRITE,
            FIUP_CHIP_ERASE,
            FIUP_COPY_DATA,
            FIUP_DISABLE_WRITE,
            FIUP_CHECK_DATA_INTEGRITY,
            FIUP_SOFTWARE_RESET
        } ;

        enum eFpgaUpdateProgress
        {
            FPUP_JUMP_IN_BOOTLOADER = 1,
            FPUP_ENABLE_WRITE,
            FPUP_COPY_DATA,
            FPUP_UPDATE_FPGA,
            FPUP_DISABLE_WRITE,
            FPUP_RESET
        } ;

        enum eFpgaUpdateStatus
        {
            BL_FPGA_UPDATE_STATUS_NONE = 0,                 ///< FPGA update status nothing
            BL_FPGA_UPDATE_WRITING = 1,                     ///< FPGA update in writing process
            BL_FPGA_UPDATE_CRC_ERROR = 2,                   ///< FPGA update error: bad CRC-16
            BL_FPGA_UPDATE_TYPE_UNSUPPORTED = 3,            ///< FPGA update error: fpga update type unsupported
            BL_FPGA_UPDATE_SESSION_OPENNED = 4,             ///< FPGA update error: session already open
            BL_FPGA_UPDATE_SUCCESS = 10,                    ///< FPGA update successful
            BL_FPGA_UPDATE_ERR_VERIFY_FAIL = 11,            ///< FPGA update error: verify failed
            BL_FPGA_UPDATE_ERR_FIND_ALGO_FILE = 12,         ///< FPGA update error: algo file not found
            BL_FPGA_UPDATE_ERR_FIND_DATA_FILE = 13,         ///< FPGA update error: data file not found
            BL_FPGA_UPDATE_ERR_WRONG_VERSION = 14,          ///< FPGA update error: wrong version
            BL_FPGA_UPDATE_ERR_ALGO_FILE_ERROR = 15,        ///< FPGA update error: algo file error
            BL_FPGA_UPDATE_ERR_DATA_FILE_ERROR = 16,        ///< FPGA update error: data file error
            BL_FPGA_UPDATE_ERR_OUT_OF_MEMORY = 17           ///< FPGA update error: out of memory
        } ;


        enum eFirmwareUpdateStatus
        {
            BL_APP_UPDATE_STATUS_NONE           = 0,    ///< Application firmware update status nothing
            BL_APP_UPDATE_WRITING               = 1,    ///< Application firmware update in writing process
            BL_APP_UPDATE_CRC_ERROR             = 2,    ///< Application firmware update error: bad CRC-16
            BL_APP_UPDATE_SESSION_OPENNED       = 3,    ///< Application firmware update error: application firmware session already open
            BL_APP_OTHER_UPDATE_SESSION_OPENNED = 4,    ///< Application firmware update error: other update session already open

            BL_APP_UPDATE_SUCCESS               = 10,   ///< Application firmware update successful
            BL_APP_UPDATE_ERROR                 = 11,   ///< Application firmware update error
            BL_APP_UPDATE_ERR_OUT_OF_MEMORY     = 12,   ///< Application firmware update error: out of memory
            BL_APP_UPDATE_ERR_OVERSIZE          = 13    ///< Application firmware update error: out of block size
        };
        enum eUpdateAddr
        {
            SPECIAL_BOOT_COMMANDS   = 0x00FFFFFF,
            RAM_UPDATE_LOGICAL_ADDR = 0x00800000,
            MAIN_APP_BASE_ADDR      = 0x00000000,
            ASIC_PACTH_BASE_ADDR    = 0x00A80000,
            ASIC_PACTH_DATA_ADDR    = ( ASIC_PACTH_BASE_ADDR + 4 )
        } ;
    }
}

#endif
