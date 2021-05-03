////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdLjrRecorder.h
///
/// \brief  Declares LdLjrRecorder class
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "LdRecorder.h"

// Forward declaration
#include "rapidjson/fwd.h"

#include <chrono>
#include <fstream>
#include <mutex>

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
        explicit LdLjrRecorder( const LdLjrRecorder & ) = delete;
        LdLjrRecorder &operator=( const LdLjrRecorder & ) = delete;
        explicit LdLjrRecorder( LeddarDevice::LdSensor *aSensor );
        virtual ~LdLjrRecorder();

        virtual std::string StartRecording( const std::string &aPath = "" ) override;
        virtual void StopRecording() override;
        virtual uint64_t GetCurrentRecordingSize() const override;
        virtual uint64_t GetElapsedTimeMs() const override;

      private:
        void AddFileHeader();
        void AddAllProperties();
        void AddProperty( const LeddarCore::LdProperty *aProperty );
        void AddPropertyValues( const LeddarCore::LdProperty *aProperty );

        virtual void Callback( LdObject *aSender, const SIGNALS aSignal, void * ) override;
        void StartFrame();
        void EndFrame();
        void StatesCallback();
        void EchoesCallback();
        void PropertyCallback( LeddarCore::LdProperty *aProperty );

        std::ostream *mOutStream;
        std::ofstream *mFile;
        rapidjson::StringBuffer *mStringBuffer;
        rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<char>, rapidjson::UTF8<char>, rapidjson::CrtAllocator, 0> *mWriter;
        uint64_t mLastTimestamp;
        std::chrono::steady_clock::time_point mStartingTime;
        std::mutex mMutex;
    };
} // namespace LeddarRecord