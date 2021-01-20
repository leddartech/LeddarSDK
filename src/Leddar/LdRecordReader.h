////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdRecordReader.h
///
/// \brief  Declares the LdRecordReader class
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LdSensor.h"
#include <stdint.h>

namespace LeddarRecord
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdRecordReader.
    ///
    /// \brief  Interface class to read recording data from a file.
    ///
    /// \author David Levy
    /// \date   May 2018
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdRecordReader
    {
    public:
        virtual ~LdRecordReader() {
            if( mSensor ) {
                delete mSensor;
                mSensor = nullptr;
            }
        }

        virtual void ReadNext() = 0;
        virtual void ReadPrevious() = 0;
        virtual void MoveTo( uint32_t aFrame ) = 0;
        virtual LeddarDevice::LdSensor *CreateSensor() = 0;
        uint32_t GetRecordSize() const { return mRecordSize; }  //Returns the number of frames in the record

        uint32_t    GetDeviceType() const { return mDeviceType; }
        void        SetDeviceType( uint32_t aDeviceType ) { mDeviceType = aDeviceType; }

    protected:
        LdRecordReader(): mSensor( nullptr ), mDeviceType( 0 ), mRecordSize( 0 ), mCommProtocol( LeddarDevice::LdSensor::P_NONE ) {}

        LeddarDevice::LdSensor *mSensor;

        void SetRecordSize( uint32_t aSize ) { mRecordSize = aSize; }

        const LeddarDevice::LdSensor::eProtocol GetCommProtocol( void ) const { return mCommProtocol;}
        void SetCommProtocol( LeddarDevice::LdSensor::eProtocol aCommProtocol )  { mCommProtocol = aCommProtocol; }

    private:
        uint32_t mDeviceType;
        uint32_t mRecordSize;
        LeddarDevice::LdSensor::eProtocol mCommProtocol;
    };
}