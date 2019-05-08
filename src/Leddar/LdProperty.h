////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdProperty.h
///
/// \brief  Declares the LdProperty class
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LdObject.h"

#include <string>
#include <vector>
#include <stdint.h>

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
            CAT_OTHER = 1,
            CAT_INFO = 2,
            CAT_CALIBRATION = 4,
            CAT_CONFIGURATION = 8,
            CAT_CONSTANT = 16
        };
        /// \brief  Defines feature bits that describe caracteristics of properties.
        ///
        enum eFeatures
        {
            F_NONE      = 0,
            F_EDITABLE  = 1,
            F_SAVE      = 2
        };
        /// \brief  Defines property types.
        ///
        enum ePropertyType
        {
            TYPE_BITFIELD   = 0,
            TYPE_BOOL       = 1,
            TYPE_ENUM       = 2,
            TYPE_FLOAT      = 3,
            TYPE_INTEGER    = 4,
            TYPE_TEXT       = 5,
            TYPE_BUFFER     = 6
        };

        bool Modified( void ) const;
        void Restore( void );
        void SetClean( void );
        void SetCount( size_t aValue );
        size_t Count( void ) const { return mStorage.size() / mStride; }
        uint32_t UnitSize( void ) const { return mUnitSize; }
        ePropertyType GetType( void ) const { return mPropertyType; }
        uint32_t GetFeatures( void ) const { return mFeatures; }

        virtual bool Signed( void ) const { return false; }
        virtual size_t Stride( void ) const { return mStride; }

        // Interfaces for child class
        virtual std::string GetStringValue( size_t aIndex = 0 ) const = 0;
        virtual void SetStringValue( size_t aIndex, const std::string &aValue ) = 0;
        virtual void ForceStringValue( size_t aIndex, const std::string &aValue ) = 0;

        uint32_t GetId( void ) const { return mId;  }
        uint32_t GetDeviceId( void ) const { return mDeviceId; }
        void     SetDeviceId( uint16_t aDeviceId ) { mDeviceId = aDeviceId; }
        eCategories GetCategory( void ) const { return mCategory; }
        const std::string &GetDescription( void ) const { return mDescription; }

        virtual void SetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize );
        virtual void ForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize );
        const uint8_t *CStorage( void ) const { return &mStorage[ 0 ]; }
        int32_t RawValue( size_t aIndex = 0 ) const { return reinterpret_cast<const int32_t *>( CStorage() )[aIndex]; }
        virtual void SetRawValue( size_t aIndex, int32_t aValue );

    protected:
        LdProperty( ePropertyType aPropertyType, eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint32_t aDeviceId, uint32_t aUnitSize, size_t aStride,
                    const std::string &aDescription = "" );

        uint8_t *Storage( void ) { return &mStorage[ 0 ]; }
        const uint8_t *BackupStorage( void ) const { return &mBackupStorage[0]; }
        bool IsInitialized( void ) const { return mInitialized; }
        void SetInitialized( bool aStatus ) { mInitialized = aStatus; }
        void VerifyInitialization( void ) const;
        void CanEdit( void ) const;

        bool    mCheckEditable; ///< Check if the property is editable before modifying it - true except when using ForceValue()

    private:
        LdProperty();

        const eCategories   mCategory; ///< The id used by the device (which we do not control). 0 means that this property is not used in communication with the device.
        const uint32_t      mFeatures; ///< Features of the property.
        const uint32_t      mId;       ///< The id in files and also the generic id we control. See \ref LeddarCore::LdPropertyIds::eLdPropertyIds
        const ePropertyType mPropertyType;

        std::string         mDescription;
        uint32_t            mDeviceId;
        bool                mInitialized;
        size_t              mStride;
        uint32_t            mUnitSize;

        std::vector<uint8_t> mStorage, mBackupStorage;

    };

}
