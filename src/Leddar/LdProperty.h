////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdProperty.h
///
/// \brief  Declares the LdProperty class
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LdObject.h"

#include <boost/any.hpp>

#include <assert.h>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>

namespace LeddarCore
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdProperty.
    ///
    /// \brief  Base class of all properties.
    ///
    /// \author Patrick Boulay
    /// \date   January 2016
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdProperty : public LdObject
    {
      public:
        /// \brief  Defines categories of properties.
        enum eCategories
        {
            CAT_OTHER         = 1,
            CAT_INFO          = 2,
            CAT_CALIBRATION   = 4,
            CAT_CONFIGURATION = 8,
            CAT_CONSTANT      = 16
        };
        /// \brief  Defines feature bits that describe caracteristics of properties.
        ///
        enum eFeatures
        {
            F_NONE                = 0,
            F_EDITABLE            = 1 << 1,
            F_SAVE                = 1 << 2,
            F_NO_MODIFIED_WARNING = 1 << 3
        };
        /// \brief  Defines property types.
        ///
        enum ePropertyType
        {
            TYPE_BITFIELD = 0,
            TYPE_BOOL     = 1,
            TYPE_ENUM     = 2,
            TYPE_FLOAT    = 3,
            TYPE_INTEGER  = 4,
            TYPE_TEXT     = 5,
            TYPE_BUFFER   = 6
        };
        
        void EmitSignal( const SIGNALS aSignal, void *aExtraData = nullptr ) override;
        bool Modified( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformModified();
        }
        void Restore( void )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformRestore();
        }
        void SetClean( void )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetClean();
        }
        void SetCount( size_t aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetCount( aValue );
        }
        size_t Count( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformCount();
        }
        uint32_t UnitSize( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformUnitSize();
        }

        ePropertyType GetType( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetType();
        }

        uint32_t GetFeatures( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetFeatures();
        }

        bool Signed( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformSigned();
        }

        size_t Stride( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformStride();
        }

        // Interfaces for child class
        std::string GetStringValue( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetStringValue( aIndex );
        };
        void SetStringValue( size_t aIndex, const std::string &aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetStringValue( aIndex, aValue );
        }
        void ForceStringValue( size_t aIndex, const std::string &aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceStringValue( aIndex, aValue );
        }

        uint32_t GetId( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetId();
        }

        uint32_t GetDeviceId( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetDeviceId();
        }

        void SetDeviceId( uint16_t aDeviceId )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetDeviceId( aDeviceId );
        }

        eCategories GetCategory( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetCategory();
        }

        std::string GetDescription( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetDescription();
        }

        void SetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetRawStorage( aBuffer, aCount, aSize );
        }
        void ForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceRawStorage( aBuffer, aCount, aSize );
        }

        int32_t RawValue( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformRawValue( aIndex );
        }
        void SetRawValue( size_t aIndex, int32_t aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetRawValue( aIndex, aValue );
        }
        std::vector<uint8_t> GetStorage() const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetStorage();
        }

        void EnableCallbacks( bool aEnable ) { mEnableCallbacks = aEnable; }

        void SetAnyValue( size_t aIndex, const boost::any &aNewValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetAnyValue( aIndex, aNewValue );
        }

        void ForceAnyValue( size_t aIndex, const boost::any &aNewValue );

        LdProperty *Clone() {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformClone();
        }
      protected:
        LdProperty( const LdProperty &aProperty );

        LdProperty( ePropertyType aPropertyType, eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint32_t aDeviceId, uint32_t aUnitSize, size_t aStride,
                    const std::string &aDescription = "" );

        const uint8_t *CStorage( void ) const { return &mStorage[0]; }
        uint8_t *Storage( void ) { return &mStorage[0]; }
        const uint8_t *BackupStorage( void ) const { return &mBackupStorage[0]; }
        bool IsInitialized( void ) const { return mInitialized; }
        void SetInitialized( bool aStatus ) { mInitialized = aStatus; }
        void VerifyInitialization( void ) const;
        void CanEdit( void );

        virtual std::string PerformGetStringValue( size_t aIndex ) const                 = 0;
        virtual void PerformSetStringValue( size_t aIndex, const std::string &aValue )   = 0;
        virtual void PerformForceStringValue( size_t aIndex, const std::string &aValue ) = 0;

        virtual bool PerformSigned( void ) const { return false; }
        void PerformSetClean();
        void PerformRestore();
        virtual void PerformSetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize );
        virtual void PerformForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize );
        virtual size_t PerformStride( void ) const { return mStride; }
        virtual void PerformSetRawValue( size_t aIndex, int32_t aValue );

        std::vector<uint8_t> PerformGetStorage() const { return mStorage; }
        int32_t PerformRawValue( size_t aIndex ) const { return reinterpret_cast<const int32_t *>( CStorage() )[aIndex]; }

        void PerformSetCount( size_t aValue );
        size_t PerformCount( void ) const { return ( mStride == 0 ? 0 : mStorage.size() / mStride ); }
        bool PerformModified() const;
        uint32_t PerformGetId( void ) const { return mId; }
        uint32_t PerformUnitSize( void ) const { return mUnitSize; }
        ePropertyType PerformGetType( void ) const { return mPropertyType; }
        uint32_t PerformGetFeatures( void ) const { return mFeatures; }
        uint32_t PerformGetDeviceId( void ) const { return mDeviceId; }
        void PerformSetDeviceId( uint16_t aDeviceId ) { mDeviceId = aDeviceId; }
        eCategories PerformGetCategory( void ) const { return mCategory; }
        std::string PerformGetDescription( void ) const { return mDescription; }
        virtual void PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue ) = 0;

        mutable std::recursive_mutex mPropertyMutex;
        bool mCheckEditable; ///< Check if the property is editable before modifying it - true except when using ForceValue()
        size_t mStride;
        uint32_t mUnitSize;

      private:
        virtual LdProperty *PerformClone() = 0;
        
        LdProperty();

        eCategories mCategory; ///< The id used by the device (which we do not control). 0 means that this property is not used in communication with the device.
        uint32_t mFeatures;    ///< Features of the property.
        uint32_t mId;          ///< The id in files and also the generic id we control. See \ref LeddarCore::LdPropertyIds::eLdPropertyIds
        ePropertyType mPropertyType;

        std::string mDescription;
        uint32_t mDeviceId;
        bool mInitialized;
        bool mEnableCallbacks = true;

        std::vector<uint8_t> mStorage, mBackupStorage;
    };

} // namespace LeddarCore
