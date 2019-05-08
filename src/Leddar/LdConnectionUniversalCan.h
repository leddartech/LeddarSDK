////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdConnectionUniversalCan.h
///
/// \brief  Declares the LdConnectionUniversalCan class. Protocol for the Vu8 using CANbus interface
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"

#ifdef BUILD_CANBUS

#include "LdConnectionUniversal.h"

namespace LeddarConnection
{
    class LdInterfaceCan;

    class LdConnectionUniversalCan : public LdConnectionUniversal
    {
    public:
        LdConnectionUniversalCan( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface );
        virtual ~LdConnectionUniversalCan();

        virtual void RawConnect( void ) override;
        virtual void Connect( void ) override;
        virtual void Disconnect( void ) override;
        virtual void Read( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t aCRCTry = 0, const int16_t &aIsReadyTimeout = 0 ) override;
        virtual void Write( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t aCRCTry = 0, const int16_t &aPostIsReadyTimeout = 10000,
                            const int16_t   &aPreIsReadyTimeout = 0, const uint16_t &aWaitAfterOpCode = 0 ) override;
        virtual void Reset( LeddarDefines::eResetType aType, bool aEnterBootloader ) override;
        virtual uint16_t InternalBuffers( uint8_t *( &aInputBuffer ), uint8_t *( &aOutputBuffer ) ) override;

    private:
        LdInterfaceCan *mInterfaceCan;
        uint32_t mCurrentBaseAddress;

        void Callback( LdObject *aSender, const SIGNALS aSignal, void *aCanData ) override;
        void ResetBuffers( void );
        uint16_t SetBaseAddress( uint32_t aFullAddress );
    };
}

#endif