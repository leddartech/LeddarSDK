// *****************************************************************************
// Module..: LeddarPrivate
//
/// \file    LdProtocolLeddarTech.h
///
/// \brief   Class definition of LdProtocolLeddarTech
///
/// \author  David Levy
///
/// \since   March 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdConnection.h"
#include "LdPropertiesContainer.h"

namespace LeddarConnection
{
    class LdProtocolLeddarTech : public LdConnection
    {
    public:
        struct LdEcho
        {
            int32_t  mDistance;
            uint32_t mAmplitude;
            uint32_t mBase;
            uint16_t mMaxIndex;
            uint32_t mChannelIndex;
            uint8_t  mValid;
            uint32_t mAmplitudeLowScale;
            uint32_t mSaturationWidth;
        };

        struct sIdentifyInfo
        {
            uint16_t mDeviceType;
            std::string mDeviceName;
            std::string mDeviceSerialNumber;
            uint32_t    mServerState;
            uint16_t    mBusyProgress;
        };


        LdProtocolLeddarTech( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface );
        virtual ~LdProtocolLeddarTech( void );

        virtual void            Connect( void ) override;
        virtual void            Disconnect( void ) override;
        virtual bool            IsConnected( void ) const override { return mIsConnected; }
        virtual void            SetConnected( bool aIsConnected ) { mIsConnected = aIsConnected; }

        virtual void            QueryDeviceInfo( void ) {};
        virtual void            StartRequest( uint16_t aCode );
        virtual void            SendRequest( void );
        virtual void            ReadAnswer( void ) = 0;
        virtual void            ReadRequest( void );
        virtual uint16_t        GetRequestCode( void ) { return mRequestCode; }
        virtual sIdentifyInfo   GetInfo( void ) { return mIdentityInfo; }
        virtual uint32_t        GetMessageSize( void ) { return static_cast<uint32_t>( mMessageSize ); }
        virtual void            AddElement( uint16_t aId, uint16_t aCount, uint32_t aSize, const void *aData, uint32_t aStride );
        virtual bool            ReadElement( void );
        virtual void            ReadElementToProperties( LeddarCore::LdPropertiesContainer *aProperties );
        virtual void            PushElementDataToBuffer( void *aDest, uint16_t aCount, uint32_t aSize, size_t aStride );
        virtual void           *GetElementData( void ) const;
        virtual uint16_t        GetElementId( void ) const { return mElementId; }
        virtual uint16_t        GetElementCount( void ) const { return mElementCount; }
        virtual uint32_t        GetElementSize( void ) const { return mElementSize; }
        uint16_t                GetAnswerCode( void ) const { return mAnswerCode; }
        void                    SetDataServer( bool aIsDataServer ) { mIsDataServer = aIsDataServer; }

    protected:
        virtual void     Write( uint32_t /*aSize*/ ) {};
        virtual void     Read( uint32_t aSize ) = 0;

        void             VerifyConnection( void ) const;
        virtual bool     ReadElementToProperty( LeddarCore::LdPropertiesContainer *aProperties );

        bool        mIsConnected;
        bool        mIsDataServer;


        sIdentifyInfo mIdentityInfo;

        uint8_t     mProtocolVersion;
        uint16_t    mAnswerCode;
        uint16_t    mRequestCode;
        int64_t     mMessageSize;
        uint32_t   *mTotalMessageSize;

        uint32_t    mElementOffset;
        uint16_t    mElementId;
        uint16_t    mElementCount;
        uint32_t    mElementSize;
        uint32_t    mElementValueOffset;
    };
}