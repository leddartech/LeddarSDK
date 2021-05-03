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

#include "LdPropertiesContainer.h"

#include <memory>
#include <mutex>

namespace LeddarConnection
{
    template <class T> class LdDoubleBuffer; // forward declaration for friend

    template <class T> struct DataBuffer
    {
        T *Buffer() { return mBuffer.get(); }
        const T *Buffer() const { return mBuffer.get(); }
        const LeddarCore::LdPropertiesContainer *GetProperties() const { return &mProperties; }
        void AddProperty( LeddarCore::LdProperty *aProperty )
        {
            mProperties.AddProperty( aProperty );
            aProperty->EnableCallbacks( false );
        }
        void SetPropertyCount( uint32_t aId, size_t aCount ) { mProperties.GetProperty( aId )->SetCount( aCount ); }
        void SetPropertyValue( uint32_t aId, int32_t aIndex, boost::any aValue ) { mProperties.GetProperty( aId )->ForceAnyValue( aIndex, aValue ); }
        void ForceRawStorage( uint32_t aId, uint8_t *aBuffer, size_t aCount, uint32_t aSize )
        {
            auto *lProp = mProperties.GetProperty( aId );
            lProp->ForceRawStorage( aBuffer, aCount, aSize );
            lProp->SetClean();
        }

      private:
        friend class LdDoubleBuffer<T>;
        mutable std::mutex mMutex;
        std::unique_ptr<T> mBuffer = std::unique_ptr<T>( new T() );
        LeddarCore::LdPropertiesContainer mProperties;
    };

    enum eBuffer
    {
        B_SET,
        B_GET
    };

    template <class T> class LdDoubleBuffer
    {
      public:
        LdDoubleBuffer()  = default;
        ~LdDoubleBuffer() = default;

        void Swap()
        {
            std::lock( mSetBuffer->mMutex, mGetBuffer->mMutex );
            std::lock_guard<std::mutex> lockSet( mSetBuffer->mMutex, std::adopt_lock );
            std::lock_guard<std::mutex> lockGet( mGetBuffer->mMutex, std::adopt_lock );

            std::swap( mSetBuffer, mGetBuffer );
        };

        std::unique_lock<std::mutex> GetUniqueLock( eBuffer aBuffer, bool aDefer = false ) const
        {
            if( aDefer )
            {
                std::unique_lock<std::mutex> lock( aBuffer == B_GET ? mGetBuffer->mMutex : mSetBuffer->mMutex, std::defer_lock );
                return lock;
            }
            else
            {
                std::unique_lock<std::mutex> lock( aBuffer == B_GET ? mGetBuffer->mMutex : mSetBuffer->mMutex );
                return lock;
            }
        }

        DataBuffer<T> *GetBuffer( eBuffer aBuffer ) { return ( aBuffer == B_GET ? mGetBuffer.get() : mSetBuffer.get() ); }
        const DataBuffer<T> *GetConstBuffer( eBuffer aBuffer ) const { return ( aBuffer == B_GET ? mGetBuffer.get() : mSetBuffer.get() ); }
        
        void SetPropertyCount( uint32_t aId, size_t aCount )
        {
            mGetBuffer->SetPropertyCount( aId, aCount );
            mSetBuffer->SetPropertyCount( aId, aCount );
        }

        void SetPropertyValue( uint32_t aId, int32_t aIndex, boost::any aValue ) { mSetBuffer->SetPropertyValue( aId, aIndex, aValue ); }
        void ForceRawStorage( uint32_t aId, uint8_t *aBuffer, size_t aCount, uint32_t aSize )
        {
            mSetBuffer->ForceRawStorage( aId, aBuffer, aCount, aSize );
        }
        const LeddarCore::LdPropertiesContainer *GetProperties( eBuffer aBuffer = B_GET ) const
        {
            const LeddarCore::LdPropertiesContainer *lPropContainer;
            if( aBuffer == B_GET )
            {
                lPropContainer = mGetBuffer->GetProperties();
            }
            else
            {
                lPropContainer = mSetBuffer->GetProperties();
            }
            return lPropContainer;
        }
        void AddProperty( LeddarCore::LdProperty *aProperty )
        {
            mGetBuffer->AddProperty( aProperty );
            auto lClone = aProperty->Clone();
            mSetBuffer->AddProperty( lClone );
        }

      private:
        std::unique_ptr<DataBuffer<T>> mGetBuffer = std::unique_ptr<DataBuffer<T>>( new DataBuffer<T>() );
        std::unique_ptr<DataBuffer<T>> mSetBuffer = std::unique_ptr<DataBuffer<T>>( new DataBuffer<T>() );

        LdDoubleBuffer( const LdDoubleBuffer &aBuffer ) = delete;            // Disable copy constructor
        LdDoubleBuffer &operator=( const LdDoubleBuffer &aBuffer ) = delete; // Disable equal operator
    };
} // namespace LeddarConnection
