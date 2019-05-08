/// *****************************************************************************
/// Module..: Leddar
///
/// \file    LdRecordPlayer.h
///
/// \brief   Definition of class LdRecordPlayer
///             Reads recording data from a file reader and plugs it to usual containers
///
/// \author  David Levy
///
/// \since   May 2018
///
/// Copyright (c) 2018 LeddarTech Inc. All rights reserved.
/// *****************************************************************************

#pragma once

#include <string>

#include "LdResultEchoes.h"
#include "LdResultStates.h"

namespace LeddarDevice
{
    class LdSensor;
}

namespace LeddarRecord
{
    class LdRecordReader;

    class LdRecordPlayer
    {
    public:
        explicit LdRecordPlayer( const std::string &aFile );
        virtual ~LdRecordPlayer();

        void ReadNext();
        void ReadPrevious();
        void MoveTo( uint32_t aFrame );
        uint32_t GetRecordSize() const;

        LeddarConnection::LdResultEchoes *GetResultEchoes( void ) { return mEchoes; }
        LeddarConnection::LdResultStates *GetResultStates( void ) { return mStates; }
        LeddarCore::LdPropertiesContainer *GetProperties( void ) { return mProperties; }

    protected:
        LdRecordPlayer();
        static LdRecordReader *FileToReader( const std::string &aFile );

        LeddarDevice::LdSensor *mSensor; //The reader owns it
        LdRecordReader         *mReader;

        LeddarConnection::LdResultEchoes *mEchoes;
        LeddarConnection::LdResultStates *mStates;

        LeddarCore::LdPropertiesContainer *mProperties;

    private:
        //disable copy constructor and equal operator
        explicit LdRecordPlayer( const LdRecordPlayer & );
        LdRecordPlayer &operator=( const LdRecordPlayer & );
    };
}