////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   LeddarPrototypes/LdSensorPixell.cpp
///
/// \brief  Implements the LdSensorPixell class for Pixell sensor
///
/// Copyright (c) 2019 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////
///
#include "LdSensorPixell.h"
#if defined(BUILD_ETHERNET) && defined(BUILD_AUTO)

#include "LdPropertyIds.h"

#include "LtExceptions.h"
#include "LtScope.h"
#include "LtStringUtils.h"
#include "comm/LtComLeddarTechPublic.h"
#include "comm/LtComEthernetPublic.h"
using namespace LeddarCore;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function: LdSensorPixell::LdSensorPixell
///
/// \brief   Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \param  aConnection Connection information
///
/// \author  Patrick Boulay
///
/// \since   May 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDevice::LdSensorPixell::LdSensorPixell( LeddarConnection::LdConnection *aConnection ) :
    LdSensorLeddarAuto( aConnection )
{
    InitProperties();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function: LdSensorPixell::~LdSensorPixell
///
/// \brief   Destructor
///
/// \author  Patrick Boulay
///
/// \since   May 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDevice::LdSensorPixell::~LdSensorPixell( void )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorPixell::InitProperties( void )
///
/// \brief   Create properties for this specific sensor.
///
/// \author  Patrick Boulay
///
/// \since   May 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorPixell::InitProperties( void )
{
    using namespace LeddarCore;

    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_FIRMWARE_VERSION_STRUCT,
                              LtComLeddarTechPublic::LT_COMM_ID_FIRMWARE_VERSION_V3, sizeof( LtComLeddarTechPublic::sFirmwareVersion ), "Firmware version" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_SUB_HFOV,
                              LtComLeddarTechPublic::LT_COMM_ID_AUTO_SUB_HFOV, 4, 0, 2, "Fields of view of the sub-modules" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_SUB_HPOSITION,
                              LtComLeddarTechPublic::LT_COMM_ID_AUTO_SUB_POSITION, 4, 0, 2, "Position of the submodules relative to the center of the sensor" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_SUB_HSEGMENT,
                              LtComLeddarTechPublic::LT_COMM_ID_AUTO_CHANNEL_SUB_NUMBER_HORIZONTAL, 2, "Number of horizontal channels by zones" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_STATUS_ALERT,
                              LtComLeddarTechPublic::LT_COMM_ID_STATUS_ALERT, sizeof( LtComLeddarTechPublic::sLtCommElementAlert ), "Sensor status" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SYSTEM_TIME,
                              LtComLeddarTechPublic::LT_COMM_ID_AUTO_SYSTEM_TIME, sizeof( uint64_t ), "Timestamp in microseconds since 1970/01/01" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SYNCHRONIZATION,
                              LtComLeddarTechPublic::LT_COMM_ID_AUTO_TIME_SYNC_METHOD, sizeof( uint8_t ), true, "Time synchronization method to be used: 0 = None, 1 = PTP, 2 = PPS" ) );

    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_ORIGIN_X,
                              LtComLeddarTechPublic::LT_COMM_ID_SENSOR_POSITION_X, 4, 0, 3, "X position" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_ORIGIN_Y,
                              LtComLeddarTechPublic::LT_COMM_ID_SENSOR_POSITION_Y, 4, 0, 3, "Y position" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_ORIGIN_Z,
                              LtComLeddarTechPublic::LT_COMM_ID_SENSOR_POSITION_Z, 4, 0, 3, "Z position" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_YAW,
                              LtComLeddarTechPublic::LT_COMM_ID_SENSOR_ORIENTATION_YAW, 4, 0, 3, "Yaw" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_PITCH,
                              LtComLeddarTechPublic::LT_COMM_ID_SENSOR_ORIENTATION_PITCH, 4, 0, 3, "Pitch" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_ROLL,
                              LtComLeddarTechPublic::LT_COMM_ID_SENSOR_ORIENTATION_ROLL, 4, 0, 3, "Roll" ) );

    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_CHANNEL_ANGLE_AZIMUT,
                              LtComLeddarTechPublic::LT_COMM_ID_AUTO_CHANNEL_ANGLES_AZIMUT, 4, 0, 2, "Azimut of each channels" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_SAVE, LdPropertyIds::ID_CHANNEL_ANGLE_ELEVATION,
                              LtComLeddarTechPublic::LT_COMM_ID_AUTO_CHANNEL_ANGLES_ELEVATION, 4, 0, 2, "Elevation of each channels" ) );



    GetResultStates()->GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_STATE_CPU_TEMP,
            LtComLeddarTechPublic::LT_COMM_ID_CPU_TEMP, 4, 0, 2, "CPU temp" ) );
    GetResultStates()->GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_PMIC_TEMP,
            LtComLeddarTechPublic::LT_COMM_ID_AUTO_PMIC_TEMP, 4, 0, 2, "PMIC Temp" ) );

    mProperties->GetEnumProperty( LdPropertyIds::ID_SYNCHRONIZATION )->AddEnumPair( 0, "None" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_SYNCHRONIZATION )->AddEnumPair( 1, "PTP" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_SYNCHRONIZATION )->AddEnumPair( 2, "PPS" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorPixell::GetConstants( void )
///
/// \brief  Fetch constant from sensor and update data
///
/// \author Patrick Boulay
/// \date   June 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorPixell::GetConstants( void )
{

    LdSensorLeddarAuto::GetConstants();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorPixell::UpdateConstants( void )
///
/// \brief  Updates the constants
///
/// \author User
/// \date   December 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorPixell::UpdateConstants( void )
{
    LdSensorLeddarAuto::UpdateConstants();

    GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_STATE_CPU_TEMP )->SetScale( GetProperties()->GetIntegerProperty(
                LeddarCore::LdPropertyIds::ID_TEMPERATURE_SCALE )->ValueT<uint32_t>() );
    GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_PMIC_TEMP )->SetScale( GetProperties()->GetIntegerProperty(
                LeddarCore::LdPropertyIds::ID_TEMPERATURE_SCALE )->ValueT<uint32_t>() );

    uint32_t lDistanceScale = mProperties->GetIntegerProperty( LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>( 0 );
    mProperties->GetFloatProperty( LdPropertyIds::ID_TIMEBASE_DELAY )->SetScale( lDistanceScale );
    mProperties->GetFloatProperty( LdPropertyIds::ID_INTENSITY_COMPENSATIONS )->SetScale( lDistanceScale );


}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorPixell::Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions, uint32_t aSubOptions )
///
/// \brief   Reset the device
///
/// \param   aType Reset type
///
/// \author  Patrick Boulay
///
/// \since   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorPixell::Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions, uint32_t aSubOptions )
{
    if( aType == LeddarDefines::RT_CONFIG_RESET )
    {
        mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_RESET_CONFIG );
        mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COM_ID_PARAM_GROUP_CATEGORY, 1, sizeof( aSubOptions ), &aSubOptions, sizeof( aSubOptions ) );
        mProtocolConfig->SendRequest();
        mProtocolConfig->ReadAnswer();
    }
    else
    {
        LdSensorLeddarAuto::Reset( aType, aOptions, aSubOptions );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorPixell::GetCalib( void )
///
/// \brief  Gets the calib from the sensor and re-order the data
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorPixell::GetCalib( void )
{
    LdSensorLeddarAuto::GetCalib();

    //Re-orders the time base delays so they are in channel index order
    LeddarCore::LdFloatProperty *lTimeBaseDelays = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_TIMEBASE_DELAY );
    std::vector<float> lSensorOrderTimeBaseDelays( lTimeBaseDelays->Count(), 0.0f );

    for( size_t i = 0; i < lTimeBaseDelays->Count(); ++i )
    {
        lSensorOrderTimeBaseDelays[i] = lTimeBaseDelays->Value( i );
    }

    for( uint32_t i = 0; i < lTimeBaseDelays->Count(); ++i )
    {
        lTimeBaseDelays->SetValue( SensorChannelIndexToEchoChannelIndex( i ), lSensorOrderTimeBaseDelays[i] );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorPixell::GetStatus( void )
///
/// \brief   Get sensor status - Should replace ping
///
/// \author  Patrick Boulay
///
/// \since   July 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorPixell::GetStatus( void )
{
    if( mPingEnabled )
    {
        LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );
        mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_STATUS );
        mProtocolConfig->SendRequest();
        mProtocolConfig->ReadAnswer();

        if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
        {
            throw LeddarException::LtComException( "Get status error, request code: " + LeddarUtils::LtStringUtils::IntToString( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_STATUS )
                                                   + " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ), LeddarException::ERROR_COM_READ );
        }

        mProtocolConfig->ReadElementToProperties( GetProperties() );

        std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LdProperty::CAT_INFO );

        for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
        {
            if( ( *lIter )->Modified() )
            {
                ( *lIter )->SetClean();
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t LeddarDevice::LdSensorPixell::SensorChannelIndexToEchoChannelIndex( uint32_t aSensorChannelIndex )
///
/// \brief  Convert the channel index from sensor internal behaviour to the standard echo channel index
///
/// \author David Levy
/// \date   October 2019
///
/// \param  aSensorChannelIndex Zero-based index of the internal sensor channel.
///
/// \returns    Standard echo channel index
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t
LeddarDevice::LdSensorPixell::SensorChannelIndexToEchoChannelIndex( uint32_t aSensorChannelIndex )
{
    uint8_t lNumberOfGain = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->EnumSize();
    uint8_t lSubModuleCount = static_cast<uint8_t>( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_SUB_HSEGMENT )->Count() );
    uint32_t lHChannelCount = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_HSEGMENT )->ValueT<uint32_t>( 0 );
    uint32_t lSubMHChannelCount = lHChannelCount / lSubModuleCount;

    uint32_t lSubmoduleAndGain = aSensorChannelIndex / ( lSubMHChannelCount * GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_VSEGMENT )->ValueT<uint32_t>( 0 ) );
    uint32_t lSubModuleChannelIndex = aSensorChannelIndex %
                                      ( lSubMHChannelCount * GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_VSEGMENT )->ValueT<uint32_t>( 0 ) );
    uint32_t lHChannelIndex = lSubModuleChannelIndex % lSubMHChannelCount; ///< In a submodule
    uint32_t lVChannelIndex = lSubModuleChannelIndex / lSubMHChannelCount;
    uint32_t lSDKSubmoduleHChannelIndex = lSubMHChannelCount - lHChannelIndex - 1;

    uint8_t lSubModule = lSubmoduleAndGain / lNumberOfGain;
    uint8_t lGain = lSubmoduleAndGain % lNumberOfGain;
    uint32_t lSDKSubModule = lSubModuleCount - lSubModule - 1;

    uint32_t lSDKChannelIndex = lGain * lHChannelCount * GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_VSEGMENT )->ValueT<uint32_t>( 0 );
    lSDKChannelIndex += lVChannelIndex * lHChannelCount;
    lSDKChannelIndex += lSDKSubModule * lSubMHChannelCount;
    lSDKChannelIndex += lSDKSubmoduleHChannelIndex;
    return lSDKChannelIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t LeddarDevice::LdSensorPixell::EchoChannelIndexToSensorChannelIndex( uint32_t aEchoChannelIndex, uint8_t aGainIndex )
///
/// \brief  Convert the standard echo channel index to the channel index from sensor internal behaviour
///
/// \exception  std::invalid_argument   Thrown when an invalid argument error condition occurs.
///
/// \param  aEchoChannelIndex   Zero-based index of the channel. Echo order.
///
/// \returns    An uint32_t.
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t
LeddarDevice::LdSensorPixell::EchoChannelIndexToSensorChannelIndex( uint32_t aEchoChannelIndex )
{
    //Sensor order is [Submodule][Gain][LocalChannelIndex]
    // And submodule order is inversed compared to echoes order
    uint8_t lNumberOfGain = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->EnumSize();
    uint8_t lSubModuleCount = static_cast<uint8_t>( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_SUB_HSEGMENT )->Count() );
    uint32_t lHChannelCount = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_HSEGMENT )->ValueT<uint32_t>( 0 );
    uint32_t lSubMHChannelCount = lHChannelCount / lSubModuleCount;
    uint32_t lChannelIndex = aEchoChannelIndex % ( lHChannelCount * GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_VSEGMENT )->ValueT<uint32_t>( 0 ) );
    uint8_t lGain = aEchoChannelIndex / ( lHChannelCount * GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_VSEGMENT )->ValueT<uint32_t>( 0 ) );
    uint32_t lHChannelIndex = lChannelIndex % lHChannelCount;
    uint32_t lVChannelIndex = lChannelIndex / lHChannelCount;
    uint32_t lSubmodule = lHChannelIndex / GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_SUB_HSEGMENT )->ValueT<uint32_t>( 0 );
    uint32_t lSubmoduleHChannelIndex = lHChannelIndex % GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_SUB_HSEGMENT )->ValueT<uint32_t>( 0 );

    if( aEchoChannelIndex > lHChannelCount * GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_VSEGMENT )->ValueT<uint32_t>( 0 )*lNumberOfGain )
    {
        throw std::invalid_argument( "Channel index is superior to channel count" );
    }

    uint32_t lSensorSubmodule = lSubModuleCount - lSubmodule - 1;
    uint32_t lSensorSubmoduleHChannelIndex = lSubMHChannelCount - lSubmoduleHChannelIndex - 1;

    uint32_t lSensorChannelIndex = lSubMHChannelCount * GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_VSEGMENT )->ValueT<uint32_t>( 0 ) *
                                   lSensorSubmodule * lNumberOfGain;
    lSensorChannelIndex += lSubMHChannelCount * GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_VSEGMENT )->ValueT<uint32_t>( 0 ) * lGain;
    lSensorChannelIndex += lVChannelIndex * lSubMHChannelCount;
    lSensorChannelIndex += lSensorSubmoduleHChannelIndex;
    return lSensorChannelIndex;

}

#endif
