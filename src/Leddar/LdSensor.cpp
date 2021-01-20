// *****************************************************************************
// Module..: Leddar
//
/// \file    LdSensor.cpp
///
/// \brief   Definition of all sensors.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdSensor.h"

#include "LdConnection.h"
#include "LdIntegerProperty.h"
#include "LtFileUtils.h"
#include "LtStringUtils.h"
#include "LtMathUtils.h"

#include "LdPropertyIds.h"
#include "comm/LtComLeddarTechPublic.h"

#include <cstring>

using namespace LeddarDevice;

// *****************************************************************************
// Function: LdSensor::LdSensor
//
/// \brief   Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \param   aConnection Connection object.
/// \param   aProperties Properties container for this sensor.
///                      If null, the sensor will declare a new instance of LdPropertiesContainer.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
LdSensor::LdSensor( LeddarConnection::LdConnection *aConnection, LeddarCore::LdPropertiesContainer *aProperties ) :
    LdDevice( aConnection, aProperties ),
    mEchoes(),
    mStates(),
    mDataMask( 0 )
{
    InitProperties();
}

// *****************************************************************************
// Function: LdSensorVuLdSensor::~LdSensor
//
/// \brief   Destructor.
///
/// \author  David Levy
///
/// \since   June 2017
// *****************************************************************************
LdSensor::~LdSensor()
{
}

// *****************************************************************************
// Function: LdSensor::GetData
//
/// \brief   Get the data from the sensor.
///          The function SetDataMask must be call first to set the data level.
///
/// \return  True is new data was processed, otherwise false.
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
bool
LdSensor::GetData( void )
{
    bool lDataReceived = false;

    if( mDataMask == DM_NONE )
    {
        SetDataMask( DM_ALL ); //echoes | states
    }

    if( ( mDataMask & DM_ECHOES ) == DM_ECHOES )
    {
        lDataReceived = GetEchoes();
    }

    if( ( mDataMask & DM_STATES ) == DM_STATES )
    {
        GetStates();
        lDataReceived = true;
    }

    return lDataReceived;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensor::ComputeCartesianCoordinates()
///
/// \brief  Updates echo vector with cartesian coordinates.
///
/// \author David Lévy
/// \date   November 2018
///
/// \exception  std::invalid_argument   Thrown when an invalid argument error condition occurs.
/// \exception  std::out_of_range       Thrown when the input argument are out of range (from SphericalToCartesian)
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensor::ComputeCartesianCoordinates()
{
    //This is a generic conversion
    // A better one can be provided by overridding this function in the corresponding sensor class

    GetResultEchoes()->Lock( LeddarConnection::B_SET );
    double lHFoV = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_HFOV )->Value();
    double lVFoV = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_VFOV )->Value();
    int32_t lHChanNumber = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_HSEGMENT )->Value();
    double lDistanceScale = static_cast<double>( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->Value() );

    if( lHFoV <= 0 || lVFoV <= 0 || lHChanNumber == 0 )
    {
        throw std::invalid_argument( "Argument out of allowed values" );
    }

    std::vector<LeddarConnection::LdEcho> &lEchoes = *GetResultEchoes()->GetEchoes( LeddarConnection::B_SET );

    for( size_t i = 0; i < GetResultEchoes()->GetEchoCount( LeddarConnection::B_SET ); ++i )
    {
        uint16_t lHIndex = lEchoes[i].mChannelIndex % lHChanNumber;
        uint16_t lVIndex = lEchoes[i].mChannelIndex / lHChanNumber;

        //angle taken from this page : https://upload.wikimedia.org/wikipedia/commons/8/8c/Spherical_Coordinates_%28Latitude%2C_Longitude%29.svg but rotate axis so z is the sensor axis
        //angle from sensor axis on horizontal plane
        double theta = LeddarUtils::LtMathUtils::DegreeToRadian( lHIndex * lHFoV / lHChanNumber + lHFoV / ( 2.0 * lHChanNumber ) - lHFoV / 2 );
        //angle from the point to the horizontal plane
        double delta = LeddarUtils::LtMathUtils::DegreeToRadian( lVIndex * lVFoV / lHChanNumber + lVFoV / ( 2.0 * lHChanNumber ) - lVFoV / 2 );

        LeddarUtils::LtMathUtils::LtPointXYZ lPoint = LeddarUtils::LtMathUtils::SphericalToCartesian( double( lEchoes[i].mDistance ) / lDistanceScale, theta, delta );
        lEchoes[i].mX = lPoint.x;
        lEchoes[i].mY = lPoint.y;
        lEchoes[i].mZ = lPoint.z;
    }

    GetResultEchoes()->UnLock( LeddarConnection::B_SET );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDefines::sLicense LeddarDevice::LdSensor::GetVolatileLicense()
///
/// \brief  Gets volatile license
///
/// \returns    The volatile license.
///
/// \author David Levy
/// \date   August 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDefines::sLicense LeddarDevice::LdSensor::GetVolatileLicense()
{
    GetLicenses();

    LeddarCore::LdIntegerProperty *lLicenseInfo = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_VOLATILE_LICENSE_INFO );
    LeddarCore::LdBufferProperty *lLicenseProp = GetProperties()->GetBufferProperty( LeddarCore::LdPropertyIds::ID_VOLATILE_LICENSE );

    LeddarDefines::sLicense lLicense = LeddarDefines::sLicense();

    if( lLicenseInfo != nullptr && lLicenseInfo->Count() > 0 && lLicenseProp != nullptr && lLicenseProp->Count() > 0 )
    {
        lLicense.mLicense = lLicenseProp->GetStringValue();
        lLicense.mType = lLicenseInfo->Value() & 0xFFFF;
        lLicense.mSubType = static_cast<uint8_t>( lLicenseInfo->ValueT<uint32_t>() >> 16 );
    }

    return lLicense;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensor::RemoveVolatileLicense( void )
///
/// \brief  Removes the volatile license
///
/// \author David Levy
/// \date   August 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensor::RemoveVolatileLicense( void )
{
    try
    {
        SendLicense( "", true );
    }
    catch( std::runtime_error &e )
    {
        //Invalid license sent on purpose to remove the real license
        if( strcmp( e.what(), "Invalid license." ) != 0 )
            throw;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensor::UpdateFirmware( const std::string& aFileName, LeddarCore::LdIntegerProperty* aProcessPercentage, LeddarCore::LdBoolProperty* aCancel )
///
/// \brief  Updates the firmware/fpga/driver using the provided ltb file
///
/// \param          aFileName           Path to the ltb file.
/// \param [in,out] aProcessPercentage  If non-null, the process percentage property.
/// \param [in,out] aCancel             If non-null, the cancel property.
///
/// \author David Levy
/// \date   July 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensor::UpdateFirmware( const std::string &aFileName, LeddarCore::LdIntegerProperty *aProcessPercentage, LeddarCore::LdBoolProperty *aCancel )
{
    LeddarUtils::LtFileUtils::LtLtbReader lLtbReader( aFileName );

    if( lLtbReader.GetDeviceType() != GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE )->Value() )
    {
        throw std::logic_error( "Provided file is not for this device" );
    }

    const std::list< std::pair <uint32_t, std::vector<uint8_t> > > &lFirmwares = lLtbReader.GetFirmwares();

    for( std::list<std::pair <uint32_t, std::vector<uint8_t> > >::const_iterator it = lFirmwares.begin(); it != lFirmwares.end(); ++it )
    {
        UpdateFirmware( LtbTypeToFirmwareType( it->first ), it->second, aProcessPercentage, aCancel );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t LeddarDevice::LdSensor::ConvertDataMaskToLTDataMask( uint32_t aMask )
///
/// \brief  Convert SDK data mask to LeddarTech internal data mask
///
/// \param  aMask   The mask.
///
/// \return The data converted mask to LeddarTech data mask.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t LeddarDevice::LdSensor::ConvertDataMaskToLTDataMask( uint32_t aMask )
{
    uint32_t lLTDataMask = 0;

    if( ( aMask & DM_ECHOES ) == DM_ECHOES )
        lLTDataMask |= LtComLeddarTechPublic::LT_DATA_LEVEL_ECHOES;

    if( ( aMask & DM_STATES ) == DM_STATES )
        lLTDataMask |= LtComLeddarTechPublic::LT_DATA_LEVEL_STATE;

    return lLTDataMask;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensor::InitProperties( void )
///
/// \brief   Create properties for all sensors.
///          Do not call the function from Ldsensor contructor,
///          some LdSensor children (ex: LdCarrierEnhancedModbus) will call InitProperties twice
///          on the same properties container.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensor::InitProperties( void )
{
    using namespace LeddarCore;

    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_DEVICE_TYPE, LtComLeddarTechPublic::LT_COMM_ID_DEVICE_TYPE, 2,
                              "Device type" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_HSEGMENT, 0, 2, "Number of horizontal segments" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_VSEGMENT, 0, 2, "Number of vertical segments" ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->ForceValue( 0, 1 );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->SetClean();
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_CONNECTION_TYPE, 0, 2, "Connection type" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_HFOV, LtComLeddarTechPublic::LT_COMM_ID_HFOV, 4, 0, 3,
                              "Horizontal field of view." ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_VFOV, LtComLeddarTechPublic::LT_COMM_ID_VFOV, 4, 0, 3,
                              "Vertical field of view. Default value is 3 for module but actual value is between 0.3 and 7.5" ) );
    mProperties->GetFloatProperty( LdPropertyIds::ID_VFOV )->ForceValue( 0, 3 );
    mProperties->GetFloatProperty( LdPropertyIds::ID_VFOV )->SetClean();
    mProperties->GetFloatProperty( LdPropertyIds::ID_HFOV )->ForceValue( 0,
            45 ); //Default value for Vu8 can bus and M16 modbus/canbus sensor. It is not the correct value, but we need one for ROS
    mProperties->GetFloatProperty( LdPropertyIds::ID_HFOV )->SetClean();
}