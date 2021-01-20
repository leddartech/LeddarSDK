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
#include <fstream>
#include <list>

namespace LeddarUtils
{
    namespace LtFileUtils
    {
        std::vector<uint8_t> ReadFileToBuffer( const std::string &aFilename );
        std::string FileExtension( const std::string &aFilename );

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \class  LtLtbReader.
        ///
        /// \brief  Class used to read and extract data from a ltb (LeddarTech Binary) file.
        ///
        /// \author David Levy
        /// \date   July 2019
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        class LtLtbReader
        {
        public:
            explicit LtLtbReader( const std::string &aFileName );
            ~LtLtbReader();

            uint16_t GetDeviceType() const { return mDeviceType; }

            // Access the Firmwares
            const std::list< std::pair <uint32_t, std::vector<uint8_t> > > &GetFirmwares( void ) const { return mFirmwares; }

            //Enum and struct used in ltb files
            enum eLTB
            {
                LT_DOCUMENT_VERSION = 0x3,
                LT_DOCUMENT_VERSION_SDK = 0x4,


                LTB_SIGNATURE = 0x43218765,

                ID_LTB_FIRMWARE_SECTION = 0x100000,
                ID_LTB_DEVICE_TYPE      = 0x100001,
                ID_LTB_FPGA_ALGO        = 0x100002,
                ID_LTB_FPGA_DATA        = 0x100003,
                ID_LTB_STM_BINARY       = 0x100004,
                ID_LTB_GALAXY_BINARY    = 0x100005,
                ID_LTB_M7_BINARY        = 0x100006,
                ID_LTB_ASIC_HEX         = 0x100007,
                ID_LTB_FPGA_ERASE_ALGO  = 0x100008,
                ID_LTB_FPGA_ERASE_DATA  = 0x100009,
                ID_LTB_LEDDARAUTO_BIN   = 0x10000A,
                ID_LTB_DTEC_BIN         = 0x10000B,
                ID_LTB_DTEC_FPGA        = 0x10000C,
                ID_LTB_LEDDARAUTO_FGPA  = 0x10000D,
                ID_LTB_LEDDARAUTO_OS    = 0x10000E ///< Various data used by the OS, script, driver, etc...
            };

        private:
            //Disable copy constructor. ifstream shouldnt be copied anyway
            LtLtbReader( const LtLtbReader & );

            size_t Read( char *aBuffer, size_t aSizeToRead ) {
                mFile.read( aBuffer, aSizeToRead ); //throw an exception on fail
                return aSizeToRead;
            }


            std::ifstream mFile;    ///< File handle
            uint16_t mDeviceType;   ///< Device type associated with the ltb
            std::list< std::pair <uint32_t, std::vector<uint8_t> > > mFirmwares;

#pragma pack(push,1)
            typedef struct LtElementHeader
            {
                uint32_t mId;
                uint32_t mUnitSize;
                uint32_t mCount;
                uint32_t mFlags;
            } LtElementHeader;
#pragma pack(pop)

            enum { LTDF_SECTION = 1, LTDF_LAST = 2 };
        };
    }
}