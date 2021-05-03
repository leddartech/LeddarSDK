////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdProtocolLeddarTechHelper.cpp
///
/// \brief	Helper class related to Leddartech protocol
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdProtocolLeddarTechHelper.h"
#include "LdProperty.h"
#include "LdProtocolLeddarTech.h"
#include "LdSensor.h"
#include "comm/LtComLeddarTechPublic.h"

#include <LtExceptions.h>
#include <LtStringUtils.h>

using namespace LeddarConnection;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LdProtocolLeddarTechHelper::LdProtocolLeddarTechHelper( LeddarDevice::LdSensor *aSensor, std::shared_ptr<LdProtocolLeddarTech> aProtocolConfig )
///
/// \brief	Constructor
///
/// \param [in,out]	aSensor		   	If non-null, the sensor.
/// \param 		   	aProtocolConfig	The protocol for configuration.
///
/// \author	Alain Ferron
/// \date	October 2020
////////////////////////////////////////////////////////////////////////////////////////////////////
LdProtocolLeddarTechHelper::LdProtocolLeddarTechHelper( LeddarDevice::LdSensor *aSensor, LdProtocolLeddarTech* aProtocolConfig )
    : mSensor( aSensor )
    , mProtocolConfig( aProtocolConfig )

{
    mProperties = mSensor->GetProperties();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarConnection::LdProtocolLeddarTechHelper::~LdProtocolLeddarTechHelper()
///
/// \brief	Destructor
///
/// \author	Alain Ferron
/// \date	October 2020
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdProtocolLeddarTechHelper::~LdProtocolLeddarTechHelper() {}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	const LdConnectionInfo *LeddarConnection::LdProtocolLeddarTechHelper::GetConnectionInfo( void ) const
///
/// \brief	Gets connection information
///
/// \author	Alain Ferron
/// \date	February 2021
///
/// \returns	Null if it fails, else the connection information.
////////////////////////////////////////////////////////////////////////////////////////////////////
const LdConnectionInfo *LeddarConnection::LdProtocolLeddarTechHelper::GetConnectionInfo( void ) const { return mProtocolConfig->GetConnectionInfo(); }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LdConnection *LeddarConnection::LdProtocolLeddarTechHelper::GetInterface( void ) const
///
/// \brief	Gets the interface
///
/// \author	Alain Ferron
/// \date	February 2021
///
/// \returns	Null if it fails, else the interface.
////////////////////////////////////////////////////////////////////////////////////////////////////
LdConnection *LeddarConnection::LdProtocolLeddarTechHelper::GetInterface( void ) const { return mProtocolConfig->GetInterface(); }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LdProtocolLeddarTechHelper::SetDataMask(uint32_t aDataMask)
///
/// \brief	Sets data mask to receive only needed data
///
/// \author	Alain Ferron
/// \date	October 2020
///
/// \exception	LeddarException::LtComException	Thrown when a Lt Com error condition occurs.
///
/// \param	aDataMask	The data mask.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LdProtocolLeddarTechHelper::SetDataMask( uint32_t aDataMask )
{
    std::lock_guard<std::mutex> lock( mMutex );
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_DATA_LEVEL_V2, 1, sizeof( aDataMask ), &aDataMask, sizeof( aDataMask ) );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException(
            "Set data mask error, request code: " + LeddarUtils::LtStringUtils::IntToString( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET ) +
                " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
            LeddarException::ERROR_COM_WRITE );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LdProtocolLeddarTechHelper::SendCommand(uint16_t aRequestCode, unsigned int aRetryNbr)
///
/// \brief	Sends a command
///
/// \author	Alain Ferron
/// \date	October 2020
///
/// \exception	LeddarException::LtComException	Thrown when a Lt Com error condition occurs.
///
/// \param	aRequestCode	The request code.
/// \param	aRetryNbr   	The retry number.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LdProtocolLeddarTechHelper::SendCommand( uint16_t aRequestCode, unsigned int aRetryNbr )
{
    std::lock_guard<std::mutex> lock( mMutex );
    mProtocolConfig->StartRequest( aRequestCode );
    mProtocolConfig->SendRequest();

    unsigned int lCount = aRetryNbr;
    bool lRetry         = false;

    do
    {
        try
        {
            lRetry = false;
            mProtocolConfig->ReadAnswer();

            if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
            {
                throw LeddarException::LtComException( "Send command, request code: " + LeddarUtils::LtStringUtils::IntToString( aRequestCode ) +
                                                           " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
                                                       LeddarException::ERROR_COM_UNKNOWN );
            }
        }
        catch( LeddarException::LtComException &e )
        {
            if( e.GetDisconnect() == true )
                throw;

            ( lCount-- != 0 ) ? lRetry = true : throw;
        }
    } while( lRetry );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LdProtocolLeddarTechHelper::GetCategoryPropertiesFromDevice( LeddarCore::LdProperty::eCategories aCategory, uint16_t aRequestCode )
///
/// \brief	Gets category properties from device
///
/// \author	Alain Ferron
/// \date	November 2020
///
/// \exception	LeddarException::LtComException	Thrown when a Lt Com error condition occurs.
///
/// \param	aCategory   	The category.
/// \param	aRequestCode	The request code.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LdProtocolLeddarTechHelper::GetCategoryPropertiesFromDevice( LeddarCore::LdProperty::eCategories aCategory, uint16_t aRequestCode )
{
    std::unique_lock<std::mutex> lock( mMutex );
    mProtocolConfig->StartRequest( aRequestCode );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException( "Get category properties error, request code: " + LeddarUtils::LtStringUtils::IntToString( aRequestCode ) +
                                                   " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
                                               LeddarException::ERROR_COM_READ );
    }

    mProtocolConfig->ReadElementToProperties( mProperties );

    lock.unlock();
    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( aCategory );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LdProtocolLeddarTechHelper::SetCategoryPropertiesOnDevice( LeddarCore::LdProperty::eCategories aCategory, uint16_t aRequestCode )
///
/// \brief	Sets category properties on device
///
/// \author	Alain Ferron
/// \date	November 2020
///
/// \exception	LeddarException::LtComException	Thrown when a Lt Com error condition occurs.
///
/// \param	aCategory   	The category.
/// \param	aRequestCode	The request code.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LdProtocolLeddarTechHelper::SetCategoryPropertiesOnDevice( LeddarCore::LdProperty::eCategories aCategory, uint16_t aRequestCode )
{
    std::unique_lock<std::mutex> lock( mMutex );
    mProtocolConfig->StartRequest( aRequestCode );

    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( aCategory );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            auto lStorage = ( *lIter )->GetStorage();
            mProtocolConfig->AddElement( ( *lIter )->GetDeviceId(), static_cast<uint16_t>( ( *lIter )->Count() ), ( *lIter )->UnitSize(), lStorage.data(),
                                         static_cast<uint32_t>( ( *lIter )->Stride() ) );
        }
    }

    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException( "Get category properties, request code: " + LeddarUtils::LtStringUtils::IntToString( aRequestCode ) +
                                                   " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
                                               LeddarException::ERROR_COM_WRITE );
    }

    lock.unlock();

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }
}