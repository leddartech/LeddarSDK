////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdLjrRecordReader.h
///
/// \brief  Declares the LdLjrRecordReader class
///         An implementation of a record reader for the json lines format (ljr = Leddar Json Record)
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LdRecordReader.h"

#include <string>
#include <fstream>

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

        virtual void ReadNext() override;
        virtual void ReadPrevious() override;
        virtual void MoveTo( uint32_t aFrame ) override;
        virtual LeddarDevice::LdSensor *CreateSensor() override;

    protected:
        void InitProperties();

    private:
        std::ifstream       mFile;         /// File handle
        uint32_t            mCurrentLine;

        void ReadHeader( const std::string &aLine );
        void ReadProperties( const std::string &aLine, bool aFromStates = false );
        void ReadFrame( const std::string &aLine );

    };
}