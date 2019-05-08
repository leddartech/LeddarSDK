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

#include "LdSensor.h"
#include "LdObject.h" // For callback

namespace LeddarRecord
{
    class LdRecorder : public LeddarCore::LdObject
    {
    public:
        virtual ~LdRecorder() {};

        virtual std::string StartRecording( const std::string &aPath = "" ) = 0;
        virtual void StopRecording() = 0;

    protected:
        //disable copy constructor and equal operator
        explicit LdRecorder( const LdRecorder & );
        LdRecorder &operator=( const LdRecorder & );

        explicit LdRecorder( LeddarDevice::LdSensor *aSensor ): mSensor( aSensor ) {
            if( !aSensor ) {
                throw std::invalid_argument( "Sensor must be a valid pointer" );
            }

            mStates = mSensor->GetResultStates();
            mEchoes = mSensor->GetResultEchoes();

            mStates->ConnectSignal( this, LeddarCore::LdObject::NEW_DATA );
            mEchoes->ConnectSignal( this, LeddarCore::LdObject::NEW_DATA );

            std::vector<LeddarCore::LdProperty *> lProperties =  mSensor->GetProperties()->FindPropertiesByFeature( LeddarCore::LdProperty::F_SAVE );

            for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter ) {
                ( *lIter )->ConnectSignal( this, LeddarCore::LdObject::VALUE_CHANGED );
            }
        };

        LeddarDevice::LdSensor *mSensor;    /// Pointer to the sensor we are recording from - Should be const, but we dont have all the requried functions

        LeddarConnection::LdResultStates *mStates;
        LeddarConnection::LdResultEchoes *mEchoes;

    private:
        virtual void Callback( LdObject *aSender, const SIGNALS aSignal, void * ) override = 0; // Implement this function to manage callbacks
    };
}