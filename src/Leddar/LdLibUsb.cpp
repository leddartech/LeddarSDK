/// *****************************************************************************
/// Module..: LeddarConnection
///
/// \file    LdLibUsb.cpp
///
/// \brief   Class definition of ldLibUsb
///
/// \author  David Levy
///
/// \since   January 2017
///
/// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
/// *****************************************************************************

#include "LdLibUsb.h"
#ifdef BUILD_USB

#include "LdConnectionInfoUsb.h"
#include "LtDefines.h"
#include "LtExceptions.h"
#include "LtStringUtils.h"

#include "comm/LtComUSBPublic.h"

#include <cstring>
#include <sstream>

#ifdef _WIN32
#include "libusb.h"
#else
#include "libusb-1.0/libusb.h"
#endif

using namespace LeddarConnection;

libusb_context *LdLibUsb::mContext = nullptr;

/// *****************************************************************************
/// Function: LdLibUsb::LdLibUsb
///
/// \brief   Constructor.
///
/// \param   aConnectionInfo Connection information.
/// \param   aInterface      Interface for this interface (optional).
///
/// \author  David Levy
///
/// \since   January 2017
// *****************************************************************************
LdLibUsb::LdLibUsb( const LdConnectionInfoUsb *aConnectionInfo, LdConnection *aInterface ) :
    LdInterfaceUsb( aConnectionInfo, aInterface ),
    mHandle( nullptr ),
    mReadTimeout( 1000 ),
    mWriteTimeout( 1000 )
{

}

/// *****************************************************************************
/// Function: LdLibUsb::~LdLibUsb
///
/// \brief   Destructor.
///
/// \author  David Levy
///
/// \since   January 2017
/// *****************************************************************************
LdLibUsb::~LdLibUsb()
{
}


/// *****************************************************************************
/// Function: LdLibUsb::GetDeviceList
///
/// \brief   Return list of connected device.
///          The function release the ownership of the returned objects.
///
/// \param   aVendorId Filter by VendorId (0 if no filter)
/// \param   aProductId Filter by ProductId (0 if no filter)
/// \param   aSerialNumber Filter by Serial number (empty if no filter)
///
/// \author  David Levy
///
/// \since   January 2017
/// *****************************************************************************
std::vector<LdConnectionInfo *>
LdLibUsb::GetDeviceList( uint32_t aVendorId, uint32_t aProductId, const std::string &aSerialNumber )
{
    std::vector<LdConnectionInfo *> lResultList;


    libusb_device **lDevs; //pointer to pointer of device, used to retrieve a list of devices
    libusb_context *lCtx = nullptr; //a libusb session
    int lStatus; //for return values
    ssize_t lCnt; //holding number of devices in list
    lStatus = libusb_init( &lCtx ); //initialize a library session

    if( lStatus < 0 )
    {
        throw LeddarException::LtComException( "USB Init Error" );
    }

    lCnt = libusb_get_device_list( lCtx, &lDevs ); //get the list of devices

    if( lCnt < 0 )
    {
        throw LeddarException::LtComException( "Get Device Error" );
    }

    int lAlreadyOpen = 0;
    ssize_t i; //for iterating through the list

    for( i = 0; i < lCnt; i++ )
    {
        bool isAlreadyOpen = false;
        libusb_device_descriptor lDesc;
        lStatus = libusb_get_device_descriptor( lDevs[ i ], &lDesc );

        if( lStatus < 0 )
        {
            throw LeddarException::LtComException( "Failed to get device descriptor" );
        }

        std::string lSerialNumber = "";
        LtComUSBPublic::LtComUsbIdtAnswerIdentify lIdentity;
        bool lAddSensor = false;

        if( aVendorId != 0 && lDesc.idVendor == aVendorId && aProductId != 0 && lDesc.idProduct == aProductId )
        {
            libusb_device_handle *lHandle = nullptr;

            try
            {
                if( libusb_open( lDevs[ i ], &lHandle ) )
                {
                    lSerialNumber = std::string( "????" ) + LeddarUtils::LtStringUtils::IntToString( lAlreadyOpen );
                    ++lAlreadyOpen;
                    isAlreadyOpen = true;
                }
                else
                {
                    uint8_t lBuffer[ 500 ];
                    // Query the device to get Indentify info
                    VerifyError( libusb_control_transfer( lHandle, 0xC0, LtComUSBPublic::LT_COM_USB_SETUP_REQ_CMD_IDENTIFY, 0, 0, lBuffer, LT_ALEN( lBuffer ), 1000 ) );

                    memcpy( &lIdentity, lBuffer, sizeof( lIdentity ) );
                    lSerialNumber = lIdentity.mSerialNumber;

                    libusb_close( lHandle );
                    lHandle = nullptr;
                }
            }
            catch( ... )
            {
                if( lHandle != nullptr )
                {
                    libusb_close( lHandle );
                    lHandle = nullptr;
                }

                continue;
            }


            lAddSensor = true;
        }
        else if( aVendorId == 0 && aProductId == 0 )
        {
            lAddSensor = true;
        }

        // Filter the by serial number
        if( aSerialNumber != "" && lSerialNumber != aSerialNumber )
        {
            lAddSensor = false;
        }

        if( lAddSensor )
        {
            std::string lDescription = "USB vendor ID: ";
            lDescription += LeddarUtils::LtStringUtils::IntToString( lDesc.idVendor, 16 );
            lDescription += " product ID: " + LeddarUtils::LtStringUtils::IntToString( lDesc.idProduct, 16 );

            if( lSerialNumber != "" )
            {
                lDescription += " Serial Number: " + lSerialNumber;
            }

            LdConnectionInfoUsb *lUsbInfo = new LdConnectionInfoUsb( LdConnectionInfoUsb::CT_USB, lDescription, lDesc.idVendor, lDesc.idProduct, libusb_get_bus_number( lDevs[ i ] ),
                    libusb_get_device_address( lDevs[ i ] ), lSerialNumber, lIdentity, isAlreadyOpen );
            lResultList.push_back( lUsbInfo );
        }



    }

    libusb_free_device_list( lDevs, 1 ); //free the list, unref the devices in it
    libusb_exit( lCtx ); //close the session

    return lResultList;
}

// *****************************************************************************
// Function: LdLibUsb::Connect
//
/// \brief   Connect to the device using the information in LdConnectionInfo provided in the constructor.
///
/// \exception LtConnectionFailed If the connection failed.
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

void
LdLibUsb::Connect( void )
{
    libusb_device **lList = nullptr;

    ssize_t lCount = libusb_get_device_list( Context(), &lList );

    if( lCount == 0 )
    {
        throw LeddarException::LtComException( "No USB device found." );
    }

    try
    {
        for( int i = 0; i < lCount; ++i )
        {
            if( libusb_get_bus_number( lList[ i ] ) == mConnectionInfoUsb->GetBusNumber() && libusb_get_device_address( lList[ i ] ) == mConnectionInfoUsb->GetDeviceAddress() )
            {
                VerifyError( libusb_open( lList[ i ], &mHandle ) );
                VerifyError( libusb_claim_interface( mHandle, 0 ) );
                break;
            }
        }

        // Device not found, on a reset ad reconnect, the device address has changed, search the new
        if( mHandle == nullptr )
        {
            std::vector<LdConnectionInfo *> lConnectionInfos =
                LdLibUsb::GetDeviceList( mConnectionInfoUsb->GetVendorID(), mConnectionInfoUsb->GetProductID(), mConnectionInfoUsb->GetSerialNumber() );

            if( lConnectionInfos.size() == 1 )
            {
                LeddarConnection::LdConnectionInfoUsb *lNewConnectionInfoUsb = dynamic_cast<LeddarConnection::LdConnectionInfoUsb *>( lConnectionInfos[0] );

                for( int i = 0; i < lCount; ++i )
                {
                    if( libusb_get_bus_number( lList[i] ) == lNewConnectionInfoUsb->GetBusNumber() && libusb_get_device_address( lList[i] ) == lNewConnectionInfoUsb->GetDeviceAddress() )
                    {
                        VerifyError( libusb_open( lList[i], &mHandle ) );
                        VerifyError( libusb_claim_interface( mHandle, 0 ) );
                        break;
                    }
                }

                delete lNewConnectionInfoUsb;
                lNewConnectionInfoUsb = nullptr;

                if( mHandle == nullptr )
                {
                    throw LeddarException::LtComException( "Reconnection failed, no device found with vendor ID: " +
                                                           LeddarUtils::LtStringUtils::IntToString( mConnectionInfoUsb->GetVendorID() ) + " product ID: " + LeddarUtils::LtStringUtils::IntToString( mConnectionInfoUsb->GetVendorID() )
                                                           + ", serial number: " + mConnectionInfoUsb->GetSerialNumber()
                                                           + ", bus number: " + LeddarUtils::LtStringUtils::IntToString( mConnectionInfoUsb->GetBusNumber() )
                                                           + " and device address: " + LeddarUtils::LtStringUtils::IntToString( mConnectionInfoUsb->GetDeviceAddress() ) );
                }
            }
            else if( lConnectionInfos.size() == 0 )
            {
                throw LeddarException::LtComException( "Reconnection failed, no device found with vendor ID: " +
                                                       LeddarUtils::LtStringUtils::IntToString( mConnectionInfoUsb->GetVendorID() ) + ", product ID: " + LeddarUtils::LtStringUtils::IntToString( mConnectionInfoUsb->GetVendorID() )
                                                       + " and serial number: " + mConnectionInfoUsb->GetSerialNumber() );
            }
            else
            {
                for( size_t i = 0; i < lConnectionInfos.size(); ++i )
                {
                    delete lConnectionInfos[i];
                }

                throw LeddarException::LtComException( "Reconnection failed, more than one device found with vendor ID: " +
                                                       LeddarUtils::LtStringUtils::IntToString( mConnectionInfoUsb->GetVendorID() ) + ", product ID: " + LeddarUtils::LtStringUtils::IntToString( mConnectionInfoUsb->GetVendorID() )
                                                       + " and serial number: " + mConnectionInfoUsb->GetSerialNumber() );
            }
        }

        libusb_free_device_list( lList, true );

    }
    catch( ... )
    {
        if( mHandle != nullptr )
        {
            libusb_release_interface( mHandle, 0 );
            libusb_close( mHandle );
            mHandle = nullptr;
        }

        if( lList != nullptr )
        {
            libusb_free_device_list( lList, true );
        }

        throw LeddarException::LtComException( "Error to connect USB device, Vendor ID: " + LeddarUtils::LtStringUtils::IntToString( mConnectionInfoUsb->GetVendorID(),
                                               16 ) + " Product ID: " + LeddarUtils::LtStringUtils::IntToString( mConnectionInfoUsb->GetProductID(), 16 ) );
    }

}

// *****************************************************************************
// Function: LdLibUsb::Disconnect
//
/// \brief   Disconnect the device.
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

void
LdLibUsb::Disconnect( void )
{
    if( mHandle != nullptr )
    {
        libusb_release_interface( mHandle, 0 );
        libusb_close( mHandle );
        mHandle = nullptr;
    }
}

// *****************************************************************************
// Function: LdLibUsb::VerifyError
//
/// \brief   Throw an error if an error with libusb occurs
///
/// \param   aCode Error code
///
/// \thow    LtComException
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

void
LdLibUsb::VerifyError( int aCode )
{
    if( aCode == LIBUSB_ERROR_TIMEOUT )
    {
        throw LeddarException::LtTimeoutException( std::string( "LibUsb error: " ) + libusb_error_name( aCode ) + "(" + LeddarUtils::LtStringUtils::IntToString( aCode ) + ")" );
    }
    else if( aCode < 0 )
    {
        throw LeddarException::LtComException( std::string( "LibUsb error: " ) + libusb_error_name( aCode ) + "(" + LeddarUtils::LtStringUtils::IntToString( aCode ) + ")", aCode );
    }
}

// *****************************************************************************
// Function: LdLibUsb::Context
//
/// \brief   Returns the libusb context. The context need to be initialized only once.
///
/// \return  Context
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

libusb_context *
LdLibUsb::Context( void )
{
    if( mContext == nullptr )
    {
        VerifyError( libusb_init( &mContext ) );
    }

    return mContext;
}

// *****************************************************************************
// Function: LdLibUsb::Read
//
/// \brief   Read data
///
/// \param   aEndPoint Endpoint of the usb transfert
/// \param   aData   Buffer to store device reading
/// \param   aSize   Size of the buffer to read
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

void
LdLibUsb::Read( uint8_t aEndPoint, uint8_t *aData, uint32_t aSize )
{
    int lLen = 0;
    // Add the direction bit to the endpoint, bit 7: 0 = Write, 1 = Read
    // the 0x4000 size is a workaround a bug on the raspberry pi, it shouldnt be necessary
    libusb_bulk_transfer( mHandle, aEndPoint | LIBUSB_ENDPOINT_IN, aData, aSize > 0x4000 ? 0x4000 : aSize, &lLen, mReadTimeout );

    if( aSize > 0x4000 && lLen == 0x4000 )
    {
        libusb_bulk_transfer( mHandle, aEndPoint | LIBUSB_ENDPOINT_IN, aData + 0x4000, aSize - 0x4000, &lLen, mReadTimeout );
    }
    else if( lLen == aSize )
    {
        throw std::runtime_error( "Receive buffer is too small" );
    }
}

// *****************************************************************************
// Function: LdLibUsb::Write
//
/// \brief   Write data
///
/// \param   aEndPoint Endpoint of the usb transfert
/// \param   aData     Buffer to write
/// \param   aSize     Size of the buffer to write
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

void
LdLibUsb::Write( uint8_t aEndPoint, uint8_t *aData, uint32_t aSize )
{
    int lLen = 0;

    VerifyError( libusb_bulk_transfer( mHandle, aEndPoint, static_cast<unsigned char *>( aData ), aSize, &lLen, mWriteTimeout ) );
}

// *****************************************************************************
// Function: LdLibUsb::ControlTransfert
//
/// \brief   Control transfert
///
/// \param   aRequestType Request type
/// \param   aRequest     Request
/// \param   aData        Buffer for the transfert
/// \param   aSize        Size of the buffer to write
/// \param   aTimeout     Timeout of the transfert
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

void
LdLibUsb::ControlTransfert( uint8_t aRequestType, uint8_t aRequest, uint8_t *aData, uint32_t aSize, uint16_t aTimeout )
{
    uint8_t lBuffer[ 500 ];

    VerifyError( libusb_control_transfer( mHandle, aRequestType, aRequest, 0, 0, lBuffer, 500, aTimeout ) );

    memcpy( aData, lBuffer, aSize );
}

#endif
