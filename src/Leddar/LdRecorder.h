////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   LdRecorder.h
///
/// \class  LdRecorder
///
/// \brief  Interface class for recorder
///
/// \author David Levy
/// \date   September 2018
/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LdObject.h" // For callback
#include "LdSensor.h"
#include <mutex>

namespace LeddarRecord
{
    class LdRecorder : public LeddarCore::LdObject
    {
      public:
        explicit LdRecorder( const LdRecorder & ) = delete;
        LdRecorder &operator=( const LdRecorder & ) = delete;
        virtual ~LdRecorder()                       = default;

        virtual std::string StartRecording( const std::string &aPath = "" ) = 0;
        virtual void StopRecording()                                        = 0;
        virtual uint64_t GetCurrentRecordingSize() const                    = 0;
        virtual uint64_t GetElapsedTimeMs() const                           = 0;

      protected:
        explicit LdRecorder( LeddarDevice::LdSensor *aSensor )
            : mSensor( aSensor )
        {
            if( !aSensor )
            {
                throw std::invalid_argument( "Sensor must be a valid pointer" );
            }

            mStates = mSensor->GetResultStates();
            mEchoes = mSensor->GetResultEchoes();

            mStates->ConnectSignal( this, LeddarCore::LdObject::NEW_DATA );
            mEchoes->ConnectSignal( this, LeddarCore::LdObject::NEW_DATA );

            std::vector<LeddarCore::LdProperty *> lProperties = mSensor->GetProperties()->FindPropertiesByFeature( LeddarCore::LdProperty::F_SAVE );

            for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
            {
                ( *lIter )->ConnectSignal( this, LeddarCore::LdObject::VALUE_CHANGED );
            }
        };

        LeddarDevice::LdSensor *mSensor; /// Pointer to the sensor we are recording from - Should be const, but we dont have all the requried functions

        LeddarConnection::LdResultStates *mStates;
        LeddarConnection::LdResultEchoes *mEchoes;
        std::mutex mWriterMutex;

      private:
        virtual void Callback( LdObject *aSender, const SIGNALS aSignal, void * ) override = 0; // Implement this function to manage callbacks
    };
} // namespace LeddarRecord
