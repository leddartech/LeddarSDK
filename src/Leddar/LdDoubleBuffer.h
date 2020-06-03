////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdDoubleBuffer.h
///
/// \brief   LdDoubleBuffer class definition
///     Class to manage dual buffering.
///     The class that instantiate it needs to handle the (de)initialization of mGetBuffer->mBuffer and mSetBuffer->mBuffer
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#include "LdIntegerProperty.h"

namespace LeddarConnection
{
    typedef struct DataBuffer
    {
        DataBuffer() : mBuffer( nullptr ), mBusy( false ) {}
        void        *mBuffer; //Actual data buffer. The class doesnt know the type, so the class that uses it does all actual manipulation (other than the swap)
        bool        mBusy;
    } DataBuffer;

    enum eBuffer
    {
        B_SET,
        B_GET
    };

    class LdDoubleBuffer
    {
    public:
        LdDoubleBuffer();
        ~LdDoubleBuffer();
        void Init( void *aGetBuffer, void *aSetBuffer, LeddarCore::LdIntegerProperty *aTimestamp, LeddarCore::LdIntegerProperty *aTimestamp64 = nullptr );

        void Swap();
        void Lock( eBuffer aBuffer ) { aBuffer == B_GET ? mGetBuffer->mBusy = true : mSetBuffer->mBusy = true; }
        void UnLock( eBuffer aBuffer ) { aBuffer == B_GET ? mGetBuffer->mBusy = false : mSetBuffer->mBusy = false; }

        uint32_t    GetTimestamp( eBuffer aBuffer = B_GET ) const;
        void        SetTimestamp( uint32_t aTimestamp );
        uint64_t    GetTimestamp64( eBuffer aBuffer = B_GET ) const;
        void        SetTimestamp64( uint64_t aTimestamp );
        uint64_t    GetFrameId( eBuffer aBuffer = B_GET ) const;
        void        SetFrameId( uint64_t aFrameId );

        DataBuffer *GetBuffer( eBuffer aBuffer ) {return ( aBuffer == B_GET ? mGetBuffer : mSetBuffer ); }
        const DataBuffer *GetConstBuffer( eBuffer aBuffer ) const {return ( aBuffer == B_GET ? mGetBuffer : mSetBuffer ); }

    private:
        LeddarCore::LdIntegerProperty *mTimestamp, *mTimestamp64; //Set buffer is value 1, Get is value 0
        LeddarCore::LdIntegerProperty mFrameId;
        DataBuffer *mGetBuffer;
        DataBuffer *mSetBuffer;

        LdDoubleBuffer( const LdDoubleBuffer &aBuffer ); //Disable copy constructor
        LdDoubleBuffer &operator=( const LdDoubleBuffer &aBuffer ); //Disable equal operator
    };
}

