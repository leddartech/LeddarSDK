////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdLjrRecordReader.h
///
/// \brief  Declares the LdLjrRecordReader class
///         An implementation of a record reader for the json lines format (ljr = Leddar Json Record)
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LdRecordReader.h"

#include <fstream>
#include <string>

namespace LeddarRecord
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdLjrRecordReader
    ///
    /// \brief  An implementation of a record reader for the json lines format (ljr = Leddar Json Record)
    ///
    /// \author David Levy
    /// \date   October 2018
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdLjrRecordReader : public LdRecordReader
    {
      public:
        explicit LdLjrRecordReader( const std::string &aFile );
        ~LdLjrRecordReader();
        static uint32_t DeviceTypeFromHeader( const std::string &aFile );

        virtual void ReadNext() override;
        virtual void ReadPrevious() override;
        virtual void MoveTo( uint32_t aFrame ) override;
        virtual LeddarDevice::LdSensor *CreateSensor() override;
        uint32_t GetCurrentPosition() const override;

      protected:
        void InitProperties();

      private:
        std::ifstream mFile; /// File handle
        uint32_t mCurrentLine = 0;

        enum ePropContainer
        {
            PC_Invalid = 0,
            PC_Sensor  = 1,
            PC_States  = 2,
            PC_Echoes  = 3
        };

        void ReadHeader( const std::string &aLine );
        void ReadProperties( const std::string &aLine, ePropContainer aContainer );
        void ReadEchoProperties( const std::string &aLine );
        void ReadFrame( const std::string &aLine );
    };
} // namespace LeddarRecord