////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	/Leddar/LdProtocolLeddarEngineRTP.h
///
/// \brief	Declares the ldprotocolleddarenginertp class
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "LdConnection.h"
#include "LdPropertiesContainer.h"

#include <atomic>
#include <functional>
#include <thread>

namespace LeddarConnection
{
    class LdInterfaceEthernet;
    class LdConnectionInfoEthernet;
    class LdRtpPacketReceiver;

    class LdProtocolLeddarEngineRTP : public LdConnection
    {

      public:
        LdProtocolLeddarEngineRTP( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface );
        ~LdProtocolLeddarEngineRTP();

        void Connect( void ) override;
        void Disconnect( void ) override final;
        bool IsConnected( void ) const override { return mIsConnected; }
        void SetConnected( bool aIsConnected ) { mIsConnected = aIsConnected; }

        void SetRtpPacketCallback( std::function<void( const LdRtpPacketReceiver & )> aProcessRtpPacket );
        void SetExceptionCallback( std::function<void( const std::exception_ptr )> aHandleException );

        uint32_t GetPort();

        void StartAcquisition();
        void StopAcquisition();
        bool IsAcquisitionning() { return mAcquisitionning.load(); }

        uint32_t GetLostPacketCount() const;
        uint64_t GetPacketReceivedQty() const { return mReceived; }

        void InitSequence( uint16_t aSequence );
        bool UpdateSequence( uint16_t aSequence );

        void SetMaxDropOut( uint16_t aMaxDropOut ) { mMaxDropOut = aMaxDropOut; }
        uint16_t GetMaxDropOut() const { return mMaxDropOut; }

        void SetMaxDisorder( uint16_t aMaxDisorder ) { mMaxDisorder = aMaxDisorder; }
        uint16_t GetMaxDisorder() const { return mMaxDisorder; }

        void SetMinSequential( uint8_t aMinSequential ) { mMinSequential = aMinSequential; }
        uint8_t GetMinSequential() const { return mMinSequential; }

        void ResetStats();

      private:
        std::function<void( const LdRtpPacketReceiver & )> mProcessRtpPacket;
        std::function<void( const std::exception_ptr )> mHandleException;

        void GetDataLoop();
        void ResetSequence( uint16_t aSequence );

        uint8_t mTransferBuffer[19000]                          = {};
        bool mIsConnected                                       = false;
        LdInterfaceEthernet *mInterfaceEthernet                 = nullptr;
        const LdConnectionInfoEthernet *mConnectionInfoEthernet = nullptr;
        uint16_t mPortFrom                                      = 0;
        std::string mAddressFrom;
        std::atomic<bool> mAcquisitionning = ATOMIC_VAR_INIT( false );
        std::thread mDataThread;

        static constexpr uint32_t RTP_SEQ_MOD = ( 1 << 16 );
        uint16_t mMaxDropOut                  = 3000;
        uint16_t mMaxDisorder                 = 100;
        uint8_t mMinSequential                = 1;

        uint16_t mMaxSeq{};    /* highest seq. number seen */
        uint64_t mCycles{};    /* shifted count of seq. number cycles */
        uint64_t mBaseSeq{};   /* base seq number */
        uint32_t mBadSeq{};    /* last 'bad' seq number + 1 */
        uint32_t mProbation{}; /* sequ. packets till source is valid */
        uint64_t mReceived{};  /* packets received */

        std::atomic_bool mResetStatsRequest = ATOMIC_VAR_INIT( false );
    };
} // namespace LeddarConnection