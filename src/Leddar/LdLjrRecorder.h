////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdLjrRecorder.h
///
/// \brief  Declares LdLjrRecorder class
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "LdRecorder.h"

#include <fstream>

//Forward declaration
#include "rapidjson/fwd.h"

namespace LeddarRecord
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdLjrRecorder
    ///
    /// \brief  Class to record using json lines format (ljr = Leddar Json Record)
    ///
    /// \author David Levy
    /// \date   October 2018
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdLjrRecorder : public LdRecorder
    {
    public:
        explicit LdLjrRecorder( LeddarDevice::LdSensor *aSensor );
        virtual ~LdLjrRecorder();

        virtual std::string StartRecording( const std::string &aPath = "" ) override;
        virtual void StopRecording() override;

    private:
        //disable copy constructor and equal operator
        explicit LdLjrRecorder( const LdLjrRecorder & );
        LdLjrRecorder &operator=( const LdLjrRecorder & );

        void AddFileHeader();
        void AddAllProperties();
        void AddProperty( const LeddarCore::LdProperty *aProperty );
        void AddPropertyValues( const LeddarCore::LdProperty *aProperty );

        virtual void Callback( LdObject *aSender, const SIGNALS aSignal, void * ) override;
        void StartFrame( LeddarConnection::LdResultProvider *aResults );
        void EndFrame();
        void StatesCallback();
        void EchoesCallback();
        void PropertyCallback( LeddarCore::LdProperty *aProperty );

        std::ofstream mFile;
        rapidjson::StringBuffer *mStringBuffer;
        rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<char>, rapidjson::UTF8<char>, rapidjson::CrtAllocator, 0> *mWriter;
        uint64_t mLastTimestamp;
    };
}