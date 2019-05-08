// *****************************************************************************
// LeddarExample.cpp
// Example to connect to all LeddarTech sensors supported in this SDK.
//
// Important on Visual Studio:
// For LeddarVu 8 SPI connection, add these lines in LeddarExample properties->Debugging->Environment:
// 32 bits: PATH=%PATH%;$(ProjectDir)\..\..\libs\MPSSE\windows\x86;$(ProjectDir)\..\..\libs\FTDI\windows\x86
// 64 bits: PATH=%PATH%;$(ProjectDir)\..\..\libs\MPSSE\windows\x64;$(ProjectDir)\..\..\libs\FTDI\windows\x64
// or copy libMPSSE.dll and ftd2xx.dll in the executable folder.
//
// *****************************************************************************


#include <iostream>
#include <iomanip>

#include "LdConnectionFactory.h"
#include "LdDeviceFactory.h"
#include "LdPropertyIds.h"
#include "LdResultEchoes.h"
#include "LdSensorVu8.h"
#include "LdSpiFTDI.h"
#include "LdLjrRecorder.h"
#include "LdRecordPlayer.h"

//Modbus
#include "LdConnectionInfoModbus.h"
#include "LdLibModbusSerial.h"
#include "LdSensorOneModbus.h"
#include "LdSensorM16Modbus.h"
#include "LdSensorVu8Modbus.h"

//USB
#include "LdLibUsb.h"
#include "LdProtocolLeddartechUSB.h"

//CANBus
#include "LdCanKomodo.h"
#include "LdProtocolCan.h"
#include "LdConnectionUniversalCan.h"
#include "LdSensorM16Can.h"
#include "LdSensorVu8Can.h"

//Utils
#include "LtExceptions.h"
#include "LtKeyboardUtils.h"
#include "LtStringUtils.h"
#include "LtTimeUtils.h"

/// Example class that show how to use callback
/// Class must inherit from LdObject and override the
/// Callback( LdObject *aSender, const SIGNALS aSignal, void * ) function
class LdDisplayer : public LeddarCore::LdObject
{
public:
    explicit LdDisplayer( LeddarDevice::LdSensor *aSensor ): mSensor( aSensor ) {
        mStates = mSensor->GetResultStates();
        mEchoes = mSensor->GetResultEchoes();

        mDistanceScale = mEchoes->GetDistanceScale();
        mAmplitudeScale = mEchoes->GetAmplitudeScale();

        //And connect to callback
        mStates->ConnectSignal( this, LeddarCore::LdObject::NEW_DATA );
        mEchoes->ConnectSignal( this, LeddarCore::LdObject::NEW_DATA );
    };

private:
    LeddarDevice::LdSensor *mSensor;
    LeddarConnection::LdResultStates *mStates;
    LeddarConnection::LdResultEchoes *mEchoes;

    uint32_t mDistanceScale, mAmplitudeScale;

    virtual void Callback( LdObject *aSender, const SIGNALS aSignal, void * ) override {

        if( aSignal != LeddarCore::LdObject::NEW_DATA )
            return;

        if( aSender == mStates ) {
            static uint32_t lLastTimeStamp = 0;
            LeddarConnection::LdResultStates *lResultStates = mSensor->GetResultStates();
            std::cout << "Cpuload: " << lResultStates->GetProperties()->FindProperty( LeddarCore::LdPropertyIds::ID_RS_CPU_LOAD )->GetStringValue();

            if( lResultStates->GetProperties()->FindProperty( LeddarCore::LdPropertyIds::ID_RS_SYSTEM_TEMP ) ) {
                std::cout << "Temp = " << lResultStates->GetProperties()->FindProperty( LeddarCore::LdPropertyIds::ID_RS_SYSTEM_TEMP )->GetStringValue() << " C";

            }

            std::cout << " @ " << lResultStates->GetTimestamp() << std::endl;
            std::cout << " Refresh Rate = " << 1000.0 / ( lResultStates->GetTimestamp() - lLastTimeStamp ) << std::endl << std::endl;
            lLastTimeStamp = lResultStates->GetTimestamp();
        }
        else if( aSender == mEchoes ) {
            mEchoes->Lock( LeddarConnection::B_GET );
            std::cout << "Channel\tDistance\tAmplitude - Count = " << mEchoes->GetEchoCount() << " @ " << mEchoes->GetTimestamp() << std::endl;
            std::vector<LeddarConnection::LdEcho> &lEchoes = *( mEchoes->GetEchoes() );

            uint32_t lIncrement = 1;

            if( mEchoes->GetEchoCount() > 1000 )
                lIncrement = 1000;

            else if( mEchoes->GetEchoCount() > 100 )
                lIncrement = 100;

            for( uint32_t i = 0; i < mEchoes->GetEchoCount(); i = i + lIncrement ) {
                std::cout << lEchoes[i].mChannelIndex;
                std::cout << "\t" << std::setprecision( 3 ) << ( ( float )lEchoes[i].mDistance / ( float )mDistanceScale );
                std::cout << "\t\t" << std::setprecision( 3 ) << ( ( float )lEchoes[i].mAmplitude / ( float )mAmplitudeScale ) << std::endl;
            }

            mEchoes->UnLock( LeddarConnection::B_GET );
        }
    }
};

// *****************************************************************************
// Function: displayListConnections
//
/// \brief   Display the name of the available connections
///
/// \arg[in]  vector of the connections
// *****************************************************************************
void displayListConnections( std::vector<LeddarConnection::LdConnectionInfo *> &aConnections )
{
    if( aConnections.size() == 0 )
    {
        std::cout << "No devices available." << std::endl;
        return;
    }

    std::cout << "Connection list:" << std::endl;

    for( size_t i = 0; i < aConnections.size(); ++i )
    {
        std::cout << i + 1 << " - " << aConnections[ i ]->GetDisplayName() << std::endl;
    }

    std::cout << std::endl;
}

// *****************************************************************************
// Function: deleteAllButOneConnections
//
/// \brief   Delete all (but one) pointers stored in vector after a GetDeviceList()
///
/// param[in] Vector returned by GetDeviceList
/// param[in] Index of the pointer NOT to delete
///
// *****************************************************************************
void deleteAllButOneConnections( std::vector<LeddarConnection::LdConnectionInfo *> &aConnections, int aButOne = -1 )
{
    for( size_t i = 0; i < aConnections.size(); ++i )
    {
        if( i != aButOne )
            delete aConnections[i];
    }
}

// *****************************************************************************
// Function: ValidInput
//
/// \brief   Validate input after a cin (to be used after using cin into an integer)
///
// *****************************************************************************
bool ValidInput( void )
{
    if( std::cin.fail() )
    {
        std::cin.clear();
        std::cin.ignore( 256, '\n' ); //256 is arbitrarily chosen, should be std::numeric_limits<std::streamsize>::max() for c++11
        std::cout << "Bad value." << std::endl;
        return false;
    }

    return true;
}

// *****************************************************************************
// Function: displayEchoes
//
/// \brief   Display echoes
///
/// param[in] Pointers to created sensor
///
// *****************************************************************************
void displayEchoes( const std::string &aSensorName, LeddarDevice::LdSensor *aSensor )
{
    // Retrieves echoes from the device
    bool lNewData = aSensor->GetData();

    if( lNewData )
    {
        LeddarConnection::LdResultEchoes *lResultEchoes = aSensor->GetResultEchoes();
        uint32_t lDistanceScale = lResultEchoes->GetDistanceScale();
        uint32_t lAmplitudeScale = lResultEchoes->GetAmplitudeScale();
        lResultEchoes->Lock( LeddarConnection::B_GET );

        std::cout << aSensorName << std::endl;
        std::cout << "Channel\tDistance\tAmplitude" << std::endl;
        std::vector<LeddarConnection::LdEcho> &lEchoes = *( lResultEchoes->GetEchoes() );

        for( uint32_t i = 0; i < lResultEchoes->GetEchoCount(); ++i )
        {
            std::cout << lEchoes[i].mChannelIndex;
            std::cout << "\t" << std::setprecision( 3 ) << ( ( float )lEchoes[i].mDistance / ( float )lDistanceScale );
            std::cout << "\t\t" << std::setprecision( 3 ) << ( ( float )lEchoes[i].mAmplitude / ( float )lAmplitudeScale ) << std::endl;
        }

        lResultEchoes->UnLock( LeddarConnection::B_GET );
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void readConfiguration( LeddarDevice::LdSensor *aSensor )
///
/// \brief  Reads all properties related to sensor configuration
///
/// \param [in,out] aSensor sensor.
///
/// \author David Levy
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void readConfiguration( LeddarDevice::LdSensor *aSensor )
{
    std::vector<LeddarCore::LdProperty *> lProperties = aSensor->GetProperties()->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CONFIGURATION );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Count() > 0 )
        {
            std::cout << "Id: 0x" << std::hex << ( *lIter )->GetId() << std::dec;
            std::cout << " Desc: \"" << ( *lIter )->GetDescription() << "\" Val: " << ( *lIter )->GetStringValue( 0 ) << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void changeConfiguration( LeddarDevice::LdSensor *aSensor )
///
/// \brief  Change a configuration property value
///
/// \param [in,out] aSensor The sensor.
///
/// \author David Levy
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void changeConfiguration( LeddarDevice::LdSensor *aSensor )
{
    readConfiguration( aSensor );

    uint32_t lChoice = -1;
    std::cout << "Enter id you want to change (in hex, without 0x)" << std::endl;
    std::cin >> std::hex >> lChoice >> std::dec;

    if( !ValidInput() || lChoice == -1 )
    {
        std::cout << "Invalid value try again." << std::endl;
        return;
    }

    auto *lProp = aSensor->GetProperties()->GetProperty( lChoice );

    std::cout << "Current value: " << std::endl;
    std::cout << "Id: " << std::hex << lProp->GetId() << std::dec << " Desc: " << lProp->GetDescription() << " Val: " << lProp->GetStringValue( 0 ) << std::endl;
    std::cout << "New value:" << std::endl;

    std::string lNewValue = "";
    std::cin.ignore();
    std::getline( std::cin, lNewValue );

    if( !ValidInput() )
    {
        std::cout << "Invalid value try again." << std::endl;
        return;
    }

    lProp->SetStringValue( 0, lNewValue ); //We use SetStringValue for simplicity but there is a specialized "SetValue" for each property type

    aSensor->SetConfig();
    aSensor->WriteConfig(); // Not needed for Vu8
}

// *****************************************************************************
// Function: connectedMenu
//
/// \brief   Let the user control the sensor he connected to
///
/// param[in] Pointers to created sensor
///
// *****************************************************************************
void connectedMenu( LeddarDevice::LdSensor *aSensor, LeddarDevice::LdSensor *aSensor2 )
{
    try
    {
        std::string lValidKey = "12345D";

        char lPressedKey = 0;
        LeddarRecord::LdRecorder *lRecorder = nullptr;

        while( 1 )
        {

            std::cout <<  std::endl;
            std::cout << "1 - Read Data" << std::endl;
            std::cout << "2 - Read Data (using callback)" << std::endl;
            std::cout << "3 - Read Configuration" << std::endl;
            std::cout << "4 - Change Configuration" << std::endl;
            std::cout << "5 - Start / Stop recording" << std::endl;

            std::cout << std::endl << "D - Disconnect" << std::endl << std::endl;
            std::cout << std::endl << "Select: ";

            while( 1 )
            {
                lPressedKey = toupper( LeddarUtils::LtKeyboardUtils::GetKey() );

                if( lValidKey.find( lPressedKey ) == std::string::npos )
                {
                    continue;
                }

                break;
            }

            // Exit the while and disconnect
            if( lPressedKey == 'D' )
            {
                break;
            }

            std::string lChoiceBuffer( 1, lPressedKey );
            uint32_t lChoice = LeddarUtils::LtStringUtils::StringToInt( lChoiceBuffer, 10 );

            std::cout << std::endl;

            if( lChoice == 1 )
            {
                std::cout << "Press a key to start reading data and press a key again to stop." << std::endl;
                LeddarUtils::LtKeyboardUtils::WaitKey();
                aSensor->SetDataMask( LeddarDevice::LdSensor::DM_ALL );

                if( aSensor2 != nullptr )
                    aSensor2->SetDataMask( LeddarDevice::LdSensor::DM_ALL );

                while( 1 )
                {
                    displayEchoes( "Sensor 1", aSensor );

                    if( aSensor2 != nullptr )
                    {
                        displayEchoes( "Sensor 2", aSensor2 );
                    }

                    LeddarUtils::LtTimeUtils::Wait( 10 );

                    if( LeddarUtils::LtKeyboardUtils::KeyPressed() )
                    {
                        break;
                    }
                }

                aSensor->SetDataMask( LeddarDevice::LdSensor::DM_NONE );

                if( aSensor2 != nullptr )
                    aSensor2->SetDataMask( LeddarDevice::LdSensor::DM_NONE );
            }
            else if( lChoice == 2 )
            {
                LdDisplayer lDisplayer( aSensor );

                std::cout << "Press a key to start reading data and press a key again to stop." << std::endl;
                LeddarUtils::LtKeyboardUtils::WaitKey();
                aSensor->SetDataMask( LeddarDevice::LdSensor::DM_ALL );

                while( true )
                {
                    aSensor->GetData();
                    LeddarUtils::LtTimeUtils::Wait( 1 );

                    if( LeddarUtils::LtKeyboardUtils::KeyPressed() )
                    {
                        break;
                    }
                }

                aSensor->SetDataMask( LeddarDevice::LdSensor::DM_NONE );
            }
            else if( lChoice == 3 )
            {
                std::cout << "Sensor" << ( aSensor2 != nullptr ? " 1" : "" ) << std::endl;
                readConfiguration( aSensor );

                if( aSensor2 != nullptr )
                {
                    std::cout << "Sensor 2" << std::endl;
                    readConfiguration( aSensor2 );
                }

            }
            else if( lChoice == 4 )
            {
                if( aSensor2 != nullptr )
                {
                    int lSensorNbr = 0;
                    std::cout << "Sensor number : (1-2) ?";
                    std::cin >> lSensorNbr;

                    if( lSensorNbr == 1 )
                        changeConfiguration( aSensor );
                    else if( lSensorNbr == 2 )
                        changeConfiguration( aSensor2 );
                }
                else
                    changeConfiguration( aSensor );
            }
            else if( 5 == lChoice )
            {
                if( lRecorder != nullptr )
                {
                    delete lRecorder;
                    lRecorder = nullptr;
                    std::cout << "Stopping recorder." << std::endl;
                }
                else
                {
                    lRecorder = new LeddarRecord::LdLjrRecorder( aSensor );
                    lRecorder->StartRecording();
                    std::cout << "Starting recorder." << std::endl;
                }
            }

        }
    }

    catch( std::exception &e )
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    aSensor->Disconnect();

    if( aSensor2 != nullptr )
    {
        aSensor2->Disconnect();
    }
}

void testConnection( void )
{
    LeddarConnection::LdConnectionInfo         *lConnectionInfo = nullptr;
    LeddarConnection::LdConnectionInfo         *lConnectionInfo2 = nullptr;
    LeddarConnection::LdConnection             *lConnection = nullptr;
    LeddarConnection::LdConnection             *lConnection2 = nullptr;
    LeddarDevice::LdSensor                     *lSensor = nullptr;
    LeddarDevice::LdSensor                     *lSensor2 = nullptr;
    LeddarRecord::LdRecordPlayer               *lPlayer = nullptr;

    try
    {
        uint32_t lChoice = -1;

        while( true )
        {

            std::cout << "******************** LeddarExample ********************" << std::endl << std::endl;
            std::cout << "1 - Connect to LeddarVu USB/Serial" << std::endl;
            std::cout << "2 - Connect to LeddarVu USB/Serial (Modbus protocol)" << std::endl;
            std::cout << "3 - Connect to LeddarVu SPI" << std::endl;
            std::cout << "4 - Connect to LeddarVu CANBus (protocol SPI)" << std::endl;
            std::cout << "5 - Connect to LeddarVu CANBus (protocol CAN)" << std::endl;
            std::cout << "6 - Connect to M16 USB" << std::endl;
            std::cout << "7 - Connect to M16 Modbus" << std::endl;
            std::cout << "8 - Connect to M16 CANBus" << std::endl;
            std::cout << "9 - Connect to LeddarOne Modbus" << std::endl;
            std::cout << "10 - Read recording" << std::endl;
            std::cout << std::endl << "0 - Exit" << std::endl << std::endl;
            std::cout << std::endl << "Select: ";

            std::cin >> lChoice;

            if( !ValidInput() || lChoice > 12 ) //Its a uint so negative value are big
                continue;

            if( lChoice == 0 )
            {
                return;
            }
            else if( lChoice == 1 ) // LeddarVu USB/Serial
            {
                std::cout << "Connection to Leddar Vu USB/Serial" << std::endl;

                std::vector<LeddarConnection::LdConnectionInfo *> lConnectionsList = LeddarConnection::LdLibModbusSerial::GetDeviceList();

                if( lConnectionsList.size() != 0 )
                {
                    displayListConnections( lConnectionsList );

                    std::cout << "Select connection ( 1 to " << lConnectionsList.size() << " ): " << std::endl;
                    uint32_t lIndexSelected = 0;
                    std::cin >> lIndexSelected;

                    if( lIndexSelected == 0 || lIndexSelected > lConnectionsList.size() )
                    {
                        deleteAllButOneConnections( lConnectionsList );
                        std::cout << "Invalid index, please retry." << std::endl;
                        continue;
                    }

                    uint16_t lModbusAddr;
                    std::cout << "Enter modbus address: " << std::endl;
                    std::cin >> lModbusAddr;

                    deleteAllButOneConnections( lConnectionsList, lIndexSelected - 1 );

                    lConnectionInfo = lConnectionsList[ lIndexSelected - 1 ];
                    dynamic_cast< LeddarConnection::LdConnectionInfoModbus * >( lConnectionInfo )->SetModbusAddr( ( uint8_t )lModbusAddr );

                    // Create the connection object
                    lConnection = dynamic_cast< LeddarConnection::LdConnectionUniversal * >( LeddarConnection::LdConnectionFactory::CreateConnection( lConnectionInfo ) );
                    lSensor = LeddarDevice::LdDeviceFactory::CreateSensor( lConnection );
                }
                else
                {
                    std::cout << "No Serial connections available." << std::endl;
                    continue;
                }
            }
            else if( lChoice == 2 ) // LeddarVu USB/Serial (Modbus protocol)
            {
                std::cout << "Connection to Leddar Vu USB/Serial (Modbus protocol)" << std::endl;

                std::vector<LeddarConnection::LdConnectionInfo *> lConnectionsList = LeddarConnection::LdLibModbusSerial::GetDeviceList();

                if( lConnectionsList.size() != 0 )
                {
                    displayListConnections( lConnectionsList );

                    std::cout << "Select connection ( 1 to " << lConnectionsList.size() << " ): " << std::endl;
                    uint32_t lIndexSelected = 0;
                    std::cin >> lIndexSelected;

                    if( lIndexSelected == 0 || lIndexSelected > lConnectionsList.size() )
                    {
                        deleteAllButOneConnections( lConnectionsList );
                        std::cout << "Invalid index, please retry." << std::endl;
                        continue;
                    }

                    uint16_t lModbusAddr;
                    std::cout << "Enter modbus address: " << std::endl;
                    std::cin >> lModbusAddr;

                    uint16_t lModbusAddr2;
                    std::cout << "Enter modbus address for an other sensor (0 if only one sensor connected): " << std::endl;
                    std::cin >> lModbusAddr2;

                    deleteAllButOneConnections( lConnectionsList, lIndexSelected - 1 );

                    lConnectionInfo = lConnectionsList[ lIndexSelected - 1 ];
                    LeddarConnection::LdConnectionInfoModbus *lConnectionInfoModbus = dynamic_cast< LeddarConnection::LdConnectionInfoModbus * >( lConnectionInfo );
                    lConnectionInfoModbus->SetModbusAddr( ( uint8_t )lModbusAddr );

                    // Create the connection object
                    lConnection = new LeddarConnection::LdLibModbusSerial( dynamic_cast<LeddarConnection::LdConnectionInfoModbus *>( lConnectionInfo ) );
                    lSensor = new LeddarDevice::LdSensorVu8Modbus( lConnection );
                    lSensor->Connect();

                    if( lModbusAddr2 != 0 )
                    {
                        lConnectionInfo2 = new LeddarConnection::LdConnectionInfoModbus( lConnectionInfoModbus->GetAddress(), "", 115200, LeddarConnection::LdConnectionInfoModbus::MB_PARITY_NONE, 8, 1,
                                ( uint8_t )lModbusAddr2 );
                        lConnection2 = new LeddarConnection::LdLibModbusSerial( dynamic_cast<LeddarConnection::LdConnectionInfoModbus *>( lConnectionInfo2 ), lConnection );
                        lSensor2 = new LeddarDevice::LdSensorVu8Modbus( lConnection2 );
                    }
                }
                else
                {
                    std::cout << "No Serial connections available." << std::endl;
                    continue;
                }
            }
            else if( lChoice == 3 ) //Connect to LeddarVu SPI
            {
                std::cout << "Connection to Leddar Vu SPI" << std::endl;

                std::vector<LeddarConnection::LdConnectionInfo *> lConnectionsList = LeddarConnection::LdSpiFTDI::GetDeviceList();

                if( lConnectionsList.size() != 0 )
                {
                    displayListConnections( lConnectionsList );

                    std::cout << "Select connection ( 1 to " << lConnectionsList.size() << " ): " << std::endl;

                    uint32_t lIndexSelected = 0;
                    std::cin >> lIndexSelected;

                    if( lIndexSelected == 0 || lIndexSelected > lConnectionsList.size() )
                    {
                        deleteAllButOneConnections( lConnectionsList );
                        std::cout << "Invalid index, please retry." << std::endl;
                        continue;
                    }

                    deleteAllButOneConnections( lConnectionsList, lIndexSelected - 1 );
                    lConnectionInfo = lConnectionsList[ lIndexSelected - 1 ];
                }
                else
                {
                    std::cout << "No FTDI connections available." << std::endl;
                    continue;
                }

                // Create the connection object
                lConnection = dynamic_cast< LeddarConnection::LdConnectionUniversal * >( LeddarConnection::LdConnectionFactory::CreateConnection( lConnectionInfo ) );

                lSensor = LeddarDevice::LdDeviceFactory::CreateSensor( lConnection );
            }
            else if( lChoice == 4 ) //LeddarVu CANBus (protocol SPI)
            {
                std::vector<LeddarConnection::LdConnectionInfo *> lList = LeddarConnection::LdCanKomodo::GetDeviceList();

                if( lList.size() == 0 )
                {
                    std::cout << "No CAN Komodo found" << std::endl;
                    continue;
                }

                LeddarConnection::LdCanKomodo *lInterface = new LeddarConnection::LdCanKomodo( dynamic_cast<LeddarConnection::LdConnectionInfoCan *>( lList[0] ) );
                LeddarConnection::LdConnectionUniversalCan *lConnection = new LeddarConnection::LdConnectionUniversalCan( lList[0], lInterface );
                lSensor = new LeddarDevice::LdSensorVu8( lConnection );
                lSensor->Connect();
            }
            else if( lChoice == 5 ) // LeddarVu CANBus (protocol CAN)
            {
                std::vector<LeddarConnection::LdConnectionInfo *> lList = LeddarConnection::LdCanKomodo::GetDeviceList();

                if( lList.size() == 0 )
                {
                    std::cout << "No CAN Komodo found" << std::endl;
                    continue;
                }

                LeddarConnection::LdConnectionInfoCan *lInfo = dynamic_cast<LeddarConnection::LdConnectionInfoCan *>( lList[0] );
                LeddarConnection::LdCanKomodo *lInterface = new LeddarConnection::LdCanKomodo( lInfo );
                LeddarConnection::LdProtocolCan *lConnection = new LeddarConnection::LdProtocolCan( lInfo, lInterface, false );
                lSensor = new LeddarDevice::LdSensorVu8Can( lConnection );

                //Optional second sensor:
                //LeddarConnection::LdConnectionInfoCan *lInfo2 = new LeddarConnection::LdConnectionInfoCan( *dynamic_cast<LeddarConnection::LdConnectionInfoCan *>( lList[0] ) );
                //lInfo2->SetBaseIdRx( 0x640 );
                //lInfo2->SetBaseIdTx( 0x650 );
                //auto *lInterface2 = new LeddarConnection::LdCanKomodo( lInfo2, lInterface );
                //auto *lConnection2 = new LeddarConnection::LdProtocolCan( lInfo2, lInterface2, false );
                //lSensor2 = new LeddarDevice::LdSensorVu8Can( lConnection2 );

                lSensor->Connect();
            }
            else if( 6 == lChoice ) // M16 USB
            {
                std::vector<LeddarConnection::LdConnectionInfo *> lConnectionsList = LeddarConnection::LdLibUsb::GetDeviceList( 0x28F1, 0x0400 );
                displayListConnections( lConnectionsList );

                if( lConnectionsList.size() == 0 )
                    continue;

                std::cout << "Select connection ( 1 to " << lConnectionsList.size() << " ): " << std::endl;
                uint32_t lIndexSelected = 0;
                std::cin >> lIndexSelected;

                if( !ValidInput() || lIndexSelected == 0 || lIndexSelected  > lConnectionsList.size() )
                {
                    deleteAllButOneConnections( lConnectionsList );
                    continue;
                }

                lConnectionInfo = lConnectionsList[lIndexSelected - 1];

                uint32_t lIndexSelected2 = 0;

                if( lConnectionsList.size() > 1 )
                {
                    std::cout << "Do you want to connect to another sensor ? (0 if only one sensor)" << std::endl;
                    std::cin >> lIndexSelected2;
                }

                if( !ValidInput() || lIndexSelected2 == lIndexSelected || lIndexSelected2  > lConnectionsList.size() )
                {
                    deleteAllButOneConnections( lConnectionsList );
                    continue;
                }

                if( lIndexSelected2 > 0 )
                {
                    lConnectionInfo2 = lConnectionsList[lIndexSelected2 - 1];
                }

                for( size_t i = 0; i < lConnectionsList.size(); ++i )
                {
                    if( i != lIndexSelected - 1 && i != lIndexSelected2 - 1 )
                        delete lConnectionsList[i];
                }

                // Create the connection object
                LeddarConnection::LdInterfaceUsb *lUsbInterface = new LeddarConnection::LdLibUsb( dynamic_cast<const LeddarConnection::LdConnectionInfoUsb *>
                        ( lConnectionInfo ) );
                lConnection = new LeddarConnection::LdProtocolLeddartechUSB( lConnectionInfo, lUsbInterface );
                lConnection->Connect(); //We can connect to USB to fetch device type
                lSensor = LeddarDevice::LdDeviceFactory::CreateSensor( lConnection );

                if( lConnectionInfo2 )
                {
                    // Create the second connection object
                    LeddarConnection::LdInterfaceUsb *lUsbInterface2 = new LeddarConnection::LdLibUsb( dynamic_cast<const LeddarConnection::LdConnectionInfoUsb *>
                            ( lConnectionInfo2 ) );
                    lConnection2 = new LeddarConnection::LdProtocolLeddartechUSB( lConnectionInfo2, lUsbInterface2 );
                    lConnection2->Connect();
                    lSensor2 = LeddarDevice::LdDeviceFactory::CreateSensor( lConnection2 );
                }
            }
            else if( 7 == lChoice ) //M16 Modbus
            {
                std::cout << "Connection to M16 modbus" << std::endl;
                std::vector<LeddarConnection::LdConnectionInfo *> lConnectionsList = LeddarConnection::LdLibModbusSerial::GetDeviceList();
                displayListConnections( lConnectionsList );

                if( lConnectionsList.size() == 0 )
                    continue;

                std::cout << "Select connection ( 1 to " << lConnectionsList.size() << " ): " << std::endl;
                uint32_t lIndexSelected = 0;
                std::cin >> lIndexSelected;

                if( !ValidInput() || lIndexSelected == 0 || lIndexSelected  > lConnectionsList.size() )
                {
                    deleteAllButOneConnections( lConnectionsList );
                    continue;
                }

                uint16_t lModbusAddr;
                std::cout << "Enter modbus address: " << std::endl;
                std::cin >> lModbusAddr;

                if( !ValidInput() || 0 == lModbusAddr || lModbusAddr > 247 ) //Its a uint so negative value are big
                {
                    deleteAllButOneConnections( lConnectionsList );
                    continue;
                }

                lConnectionInfo = lConnectionsList[lIndexSelected - 1];
                LeddarConnection::LdConnectionInfoModbus *lConnectionInfoModbus = dynamic_cast< LeddarConnection::LdConnectionInfoModbus * >( lConnectionInfo );
                lConnectionInfoModbus->SetModbusAddr( ( uint8_t )lModbusAddr );

                // Create the connection object
                lConnection = new LeddarConnection::LdLibModbusSerial( lConnectionInfoModbus );

                lSensor = new LeddarDevice::LdSensorM16Modbus( lConnection );
                lSensor->Connect();
            }
            else if( 8 == lChoice ) // M16 CANBus
            {
                std::vector<LeddarConnection::LdConnectionInfo *> lList = LeddarConnection::LdCanKomodo::GetDeviceList();

                if( lList.size() == 0 )
                {
                    std::cout << "No CAN Komodo found" << std::endl;
                    continue;
                }

                LeddarConnection::LdCanKomodo *lInterface = new LeddarConnection::LdCanKomodo( dynamic_cast<LeddarConnection::LdConnectionInfoCan *>( lList[0] ) );
                LeddarConnection::LdProtocolCan *lConnection = new LeddarConnection::LdProtocolCan( lList[0], lInterface, true );
                lSensor = new LeddarDevice::LdSensorM16Can( lConnection );

                //Optional second sensor:
                //LeddarConnection::LdConnectionInfoCan *lInfo2 = new LeddarConnection::LdConnectionInfoCan( *dynamic_cast<LeddarConnection::LdConnectionInfoCan *>( lList[0] ) );
                //lInfo2->SetBaseIdRx( 0x640 );
                //lInfo2->SetBaseIdTx( 0x650 );
                //auto *lInterface2 = new LeddarConnection::LdCanKomodo( lInfo2, lInterface );
                //auto *lConnection2 = new LeddarConnection::LdProtocolCan( lInfo2, lInterface2, true );
                //lSensor2 = new LeddarDevice::LdSensorM16Can( lConnection2 );

                lSensor->Connect();
            }
            else if( 9 == lChoice ) //LeddarOne Modbus
            {
                std::cout << "Connection to LeddarOne modbus" << std::endl;
                std::vector<LeddarConnection::LdConnectionInfo *> lConnectionsList = LeddarConnection::LdLibModbusSerial::GetDeviceList();
                displayListConnections( lConnectionsList );

                if( lConnectionsList.size() == 0 )
                {
                    std::cout << "No Serial connections available." << std::endl;
                    continue;
                }

                std::cout << "Select connection ( 1 to " << lConnectionsList.size() << " ): " << std::endl;
                uint32_t lIndexSelected = 0;
                std::cin >> lIndexSelected;

                if( !ValidInput() || lIndexSelected == 0 || lIndexSelected  > lConnectionsList.size() )
                {
                    deleteAllButOneConnections( lConnectionsList );
                    continue;
                }

                uint16_t lModbusAddr;
                std::cout << "Enter modbus address: " << std::endl;
                std::cin >> lModbusAddr;

                if( !ValidInput() || 0 == lModbusAddr || lModbusAddr > 247 ) //Its a uint so negative value are big
                {
                    deleteAllButOneConnections( lConnectionsList );
                    continue;
                }

                uint16_t lModbusAddr2;
                std::cout << "Enter modbus address for an other sensor (0 if only one sensor connected on this COM port):  " << std::endl;
                std::cin >> lModbusAddr2;

                if( !ValidInput() || lModbusAddr2 > 247 ) //Its a uint so negative value are big
                {
                    deleteAllButOneConnections( lConnectionsList );
                    continue;
                }

                deleteAllButOneConnections( lConnectionsList, lIndexSelected - 1 );

                lConnectionInfo = lConnectionsList[lIndexSelected - 1];
                LeddarConnection::LdConnectionInfoModbus *lConnectionInfoModbus = dynamic_cast< LeddarConnection::LdConnectionInfoModbus * >( lConnectionInfo );
                lConnectionInfoModbus->SetModbusAddr( ( uint8_t )lModbusAddr );

                // Create the connection object
                lConnection = new LeddarConnection::LdLibModbusSerial( dynamic_cast<LeddarConnection::LdConnectionInfoModbus *>( lConnectionInfo ) );
                lSensor = new LeddarDevice::LdSensorOneModbus( lConnection );
                lSensor->Connect();

                if( lModbusAddr2 != 0 )
                {
                    lConnectionInfo2 = new LeddarConnection::LdConnectionInfoModbus( lConnectionInfoModbus->GetAddress(), "", 115200, LeddarConnection::LdConnectionInfoModbus::MB_PARITY_NONE, 8, 1,
                            ( uint8_t )lModbusAddr2 );
                    lConnection2 = new LeddarConnection::LdLibModbusSerial( dynamic_cast<LeddarConnection::LdConnectionInfoModbus *>( lConnectionInfo2 ), lConnection );
                    lSensor2 = new LeddarDevice::LdSensorOneModbus( lConnection2 );
                    //No need to call connect, lSensor is connected to the same interface
                }
            }
            else if( 10 == lChoice )
            {
                std::cout << "Path of the record file: ";
                std::cin.ignore();
                std::string lPath;
                std::getline( std::cin, lPath );

                if( lPath.at( 0 ) == '"' && lPath.at( lPath.size() - 1 ) == '"' )
                {
                    lPath.erase( 0, 1 );
                    lPath.erase( lPath.size() - 1, 1 );
                }

                lPlayer = new LeddarRecord::LdRecordPlayer( lPath );
            }

            if( lSensor != nullptr )
            {
                lSensor->GetConstants();
                lSensor->GetConfig();
            }

            if( lSensor2 != nullptr )
            {
                lSensor2->GetConstants();
                lSensor2->GetConfig();
            }

            if( lSensor || lSensor2 )
            {
                connectedMenu( lSensor, lSensor2 );
            }

            if( lPlayer )
            {
                std::cout << "Name: " << lPlayer->GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_DEVICE_NAME )->GetStringValue( 0 ) << std::endl;
                std::cout << "Record size: " << lPlayer->GetRecordSize() << std::endl;
                lPlayer->ReadNext();
                std::cout << "First frame" << std::endl;
                std::cout << "echo count: " << lPlayer->GetResultEchoes()->GetEchoCount() << std::endl;
            }

            if( lSensor )
            {
                lSensor->Disconnect();
                delete lSensor;
            }

            if( lSensor2 )
            {
                lSensor2->Disconnect();
                delete lSensor2;
            }

            if( lPlayer != nullptr )
            {
                delete lPlayer;
            }
        }

    }
    catch( std::exception &e )
    {
        std::cout << "Exception: " << e.what() << std::endl;
#ifdef _WIN32
        system( "pause" );
#endif
    }

    if( lSensor )
    {
        lSensor->Disconnect();
        delete lSensor;
    }

    if( lSensor2 )
    {
        lSensor2->Disconnect();
        delete lSensor2;
    }

    if( lPlayer != nullptr )
    {
        delete lPlayer;
    }
}


int main( int argc, char *argv[] )
{
    testConnection();

    return 0;
}
