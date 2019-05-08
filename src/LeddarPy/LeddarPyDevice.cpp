// *****************************************************************************
// Module..: LeddarPy
//
/// \file    LeddarPyDevice.cpp
///
/// \brief   Implementations of functions for Device object used in LeddarPy module
///
/// \author  David Levy
///
/// \since   November 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************


#include "LeddarPyDevice.h"
#include "LeddarPy.h"
#include "PythonHelper.h"
#include "Connecters.h"

#include "LtComLeddarTechPublic.h"
#include "LdPropertyIds.h"
#include "LtStringUtils.h"
#include "LtTimeUtils.h"
#include "LtIntUtilities.h"
#include "LtExceptions.h"

#include "LdLibModbusSerial.h"
#include "LdSpiFTDI.h"
#include "LdLibUsb.h"
#include "LdEthernet.h"

#include "LdSensor.h"


#include "LdLjrRecorder.h"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define NO_IMPORT_ARRAY
#define PY_ARRAY_UNIQUE_SYMBOL leddar_ARRAY_API
#include <numpy/arrayobject.h>

#include <algorithm>


#ifdef _WIN32
#include <Ws2tcpip.h>
#include <windows.h>
void _sleep_ms( int sleepMs )
{
    Sleep( sleepMs );
}

#else
#include <arpa/inet.h>
#include <unistd.h>
void _sleep_ms( int sleepMs )
{
    usleep( sleepMs * 1000 );
}
#endif


class ScopedDataMask
{
public:
    ScopedDataMask( sLeddarDevice *device, uint32_t newDataMask ) : mDevice( device ) {
        mOldDataMask = device->mDataMask;
        mDevice->mSensor->SetDataMask( mOldDataMask | newDataMask );
    }
    ~ScopedDataMask() {
        mDevice->mSensor->SetDataMask( mOldDataMask );
    }
private:
    sLeddarDevice *mDevice;
    uint32_t mOldDataMask;
};

class CallBackManger : public LeddarCore::LdObject
{
public:
    explicit CallBackManger( sLeddarDevice *aSelf ): mSelf( aSelf ) {
        mStates = aSelf->mSensor->GetResultStates();
        mEchoes = aSelf->mSensor->GetResultEchoes();


        //And connect to callback
        mStates->ConnectSignal( this, LeddarCore::LdObject::NEW_DATA );
        mEchoes->ConnectSignal( this, LeddarCore::LdObject::NEW_DATA );

    };

private:
    sLeddarDevice *mSelf;
    LeddarConnection::LdResultStates *mStates;
    LeddarConnection::LdResultEchoes *mEchoes;


    virtual void Callback( LdObject *aSender, const SIGNALS aSignal, void * ) override {

        // if we got a callback, it is necessarily after a call to GetData() from DataThread(),
        // so it both is safe and necessary to remove the lock, or we could deadlock GIL
        if( mSelf->mGetDataLocked ) { //additional safety, it is illegal to unlock a mutex twice, do not use this field outside of DataThread()
            mSelf->mDataThreadMutex.unlock();
            mSelf->mGetDataLocked = false;
        }

        if( aSignal != LeddarCore::LdObject::NEW_DATA )
            return;

        PyGILState_STATE gstate;

        if( aSender == mStates ) {
            if( mSelf->mCallBackState ) {
                //Thread safe python call
                gstate = PyGILState_Ensure();

                if( PyObject *o = PackageStates( mStates ) ) {
                    PyObject_CallFunctionObjArgs( mSelf->mCallBackState, o, NULL );
                    Py_DECREF( o );
                }

                // Release the python thread. No Python API allowed beyond this point.
                PyGILState_Release( gstate );
            }
        }
        else if( aSender == mEchoes ) {
            if( mSelf->mCallBackEcho ) {
                //Thread safe python call
                gstate = PyGILState_Ensure();

                if( PyObject *o = PackageEchoes( mEchoes ) ) {
                    PyObject_CallFunctionObjArgs( mSelf->mCallBackEcho, o, NULL );
                    Py_DECREF( o );
                }

                // Release the python thread. No Python API allowed beyond this point.
                PyGILState_Release( gstate );
            }
        }

    };
};

template <typename F>
PyObject *RetryNTimes( F f, size_t nRetries )
{
    std::string lLastException;

    for( size_t i = 0; i < nRetries; i++ )
    {
        try
        {
            return f();
        }
        catch( const std::exception &e )
        {
            DebugTrace( e.what() );
            lLastException = e.what();
        }
    }

    PyErr_SetString( PyExc_RuntimeError, lLastException.c_str() );
    return nullptr;
}

bool CheckSensor( sLeddarDevice *self )
{
    if( self->mSensor == nullptr )
    {
        PyErr_SetString( PyExc_RuntimeError, "Not connected to a sensor." );
        return false;
    }

    return true;
}
// *****************************************************************************
// Function: Device_new
//
/// \brief   Constructor - No parameter
///
/// \return  New device instance
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
PyObject *Device_new( PyTypeObject *type, PyObject *args, PyObject *kwds )
{
    sLeddarDevice *self;
    self = ( sLeddarDevice * )type->tp_alloc( type, 0 );
    new( self ) sLeddarDevice; //Properly initialize memory

    if( self != nullptr )
    {
        self->mSensor = nullptr;
        self->mRecorder = nullptr;
        self->mStream = false;
        self->mDataMask = LeddarDevice::LdSensor::DM_NONE;
        self->mDataThreadSharedData.mStop = false;
        self->mDataThreadSharedData.mDelay = 5000;
        self->mGetDataLocked = false;
    }

    return ( PyObject * )self;
}

int Device_init( PyObject *self, PyObject *args, PyObject *kwds )
{
    const char *lSensorName;
    int lDeviceType = 0, lAdditionalInfo = 0;

    if( PyArg_ParseTuple( args, "" ) ) //no arguments is ok
        return 0;

    PyErr_Clear();

    if( PyArg_ParseTuple( args, "s|ii", &lSensorName, &lDeviceType, &lAdditionalInfo ) )
    {
        bool connected = Connect( ( sLeddarDevice * )self, args ); //try to connect

        if( !connected )
        {
            DebugTrace( "connection failed!" );
            return -1;
        }
    }

    return 0;
}

// *****************************************************************************
// Function: Device_dealloc
//
/// \brief   Destructor
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
void Device_dealloc( sLeddarDevice *self )
{
    DebugTrace( "Destructing device." );

    Py_XDECREF( self->mCallBackState );
    Py_XDECREF( self->mCallBackEcho );
    Py_XDECREF( self->mCallBackRawTrace );
    Py_XDECREF( self->mCallBackFilteredTrace );

    if( self->mRecorder != nullptr )
    {
        delete self->mRecorder;
        self->mRecorder = nullptr;
    }

    if( self->mSensor != nullptr )
    {
        Disconnect( self, nullptr );
        delete self->mSensor; //FIXME: double free or corruption
        self->mSensor = nullptr;
    }

    DebugTrace( "C++ Device Destructor." );
    self->~sLeddarDevice();
    Py_TYPE( self )->tp_free( ( PyObject * )self );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *Connect( sLeddarDevice *self, PyObject *args )
///
/// \brief  Connects the sensor
///
/// \param [in,out] self    The class instance that this method operates on.
/// \param [in]     args    The arguments.
///                             (string) Sensor name or IP address for Ethernet sensors
///                             (optional but recommended, int) Device type | connection type
///                             (optional, int) Additional information, depending on what you want to connect to
///                                                 Ethernet port or modbus address
///                             (optional, int) Timeout in ms for communication failures (default 2000)
///
/// \return nullptr on error, True on success, False on failure
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *Connect( sLeddarDevice *self, PyObject *args )
{
    DebugTrace( "Connecting" );
    const char *lConnectionString;
    int lDeviceType = 0, lAdditionalInfo = 0, lAdditionalInfo2 = 0;

    if( !PyArg_ParseTuple( args, "s|iii", &lConnectionString, &lDeviceType, &lAdditionalInfo, &lAdditionalInfo2 ) )
        return nullptr;

    int lConnectionType = lDeviceType & 0xFFFF0000;
    lDeviceType = lDeviceType & 0x0000FFFF;

    if( CONNECTION_TYPE_SERIAL == lConnectionType ) // LeddarOne, M16, Vu8
    {
        lAdditionalInfo = lAdditionalInfo == 0 ? 1 : lAdditionalInfo;
        lAdditionalInfo2 = lAdditionalInfo2 == 0 ? 115200 : lAdditionalInfo2;

        if( ConnectSerial( &self->mSensor, lConnectionString, lAdditionalInfo, lAdditionalInfo2 ) )
            Py_RETURN_TRUE;
        else
            Py_RETURN_FALSE;
    }
    else if( CONNECTION_TYPE_USB == lConnectionType ) // M16
    {
        if( ConnectUsb( &self->mSensor, lConnectionString ) )
            Py_RETURN_TRUE;
        else
            Py_RETURN_FALSE;
    }
    else if( CONNECTION_TYPE_SPI_FTDI == lConnectionType ) // Vu8
    {
        if( ConnectSPIFTDI( &self->mSensor, lConnectionString ) )
            Py_RETURN_TRUE;
        else
            Py_RETURN_FALSE;
    }
    else if( CONNECTION_TYPE_CAN_KOMODO == lConnectionType )
    {
        switch( lDeviceType )
        {
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16:
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_LASER:
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_IS16:
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_EVALKIT:
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VU8:
                break;

            case 0:
                DebugTrace( "Please set connection type and device type for CANbus protocol" );
                Py_RETURN_FALSE;
                break;

            default:
                DebugTrace( "Unsupported device type for CAN protocol" );
                Py_RETURN_FALSE;
                break;
        }

        lAdditionalInfo = ( lAdditionalInfo == 0 ? 0x750 : lAdditionalInfo );
        lAdditionalInfo2 = ( lAdditionalInfo2 == 0 ? 0x740 : lAdditionalInfo2 );

        if( ConnectCanKomodo( &self->mSensor, lDeviceType, lAdditionalInfo2, lAdditionalInfo, LeddarUtils::LtStringUtils::StringToUInt( lConnectionString, 10 ) ) )
            Py_RETURN_TRUE;
        else
            Py_RETURN_FALSE;
    }
    else if( CONNECTION_TYPE_ETHERNET == lConnectionType || LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_AUTO_FAMILY == lDeviceType )
    {
        try
        {
            //Check if the first argument is an IPv4 address
            LeddarUtils::LtStringUtils::StringToIp4Addr( lConnectionString ); //will throw if not an IP
        }
        catch( LeddarException::LtInfoException & )
        {
            DebugTrace( "Name is not an IP." );
            Py_RETURN_FALSE;
        }

        lAdditionalInfo = ( lAdditionalInfo == 0 ? 48630 : lAdditionalInfo );
        lAdditionalInfo2 = ( lAdditionalInfo2 == 0 ? 2000 : lAdditionalInfo2 );

        if( ConnectEthernet( &self->mSensor, lConnectionString, lAdditionalInfo, lAdditionalInfo2 ) )
            Py_RETURN_TRUE;
        else
            Py_RETURN_FALSE;
    }
    else if( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16 == lDeviceType || LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_LASER == lDeviceType ||
             LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_IS16 == lDeviceType || LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_EVALKIT == lDeviceType )
    {
        if( !ConnectUsb( &self->mSensor, lConnectionString ) )
        {
            lAdditionalInfo = lAdditionalInfo == 0 ? 1 : lAdditionalInfo;
            lAdditionalInfo2 = lAdditionalInfo2 == 0 ? 115200 : lAdditionalInfo2;

            if( !ConnectSerial( &self->mSensor, lConnectionString, lAdditionalInfo, lAdditionalInfo2 ) )
                Py_RETURN_FALSE;
        }

        if( self->mSensor->GetConnection()->GetDeviceType() != LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16 &&
                self->mSensor->GetConnection()->GetDeviceType() != LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_EVALKIT  &&
                self->mSensor->GetConnection()->GetDeviceType() != LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_IS16 &&
                self->mSensor->GetConnection()->GetDeviceType() != LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_LASER )
        {
            DebugTrace( "Sensor with requested name is not a M16." );
            Py_RETURN_FALSE;
        }

        Py_RETURN_TRUE;
    }
    else if( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SCH_EVALKIT == lDeviceType || LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SCH_LONG_RANGE == lDeviceType )
    {
        lAdditionalInfo = lAdditionalInfo == 0 ? 1 : lAdditionalInfo;
        lAdditionalInfo2 = lAdditionalInfo2 == 0 ? 115200 : lAdditionalInfo2;

        if( !ConnectSerial( &self->mSensor, lConnectionString, lAdditionalInfo, lAdditionalInfo2 ) )
            Py_RETURN_FALSE;

        if( self->mSensor->GetConnection()->GetDeviceType() != LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SCH_EVALKIT &&
                self->mSensor->GetConnection()->GetDeviceType() != LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SCH_LONG_RANGE )
        {
            DebugTrace( "Sensor with requested name is not a LeddarOne." );
            Py_RETURN_FALSE;
        }

        Py_RETURN_TRUE;
    }
    else if( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VU8 == lDeviceType )
    {
        if( !ConnectSPIFTDI( &self->mSensor, lConnectionString ) )
        {
            lAdditionalInfo = lAdditionalInfo == 0 ? 1 : lAdditionalInfo;
            lAdditionalInfo2 = lAdditionalInfo2 == 0 ? 115200 : lAdditionalInfo2;

            if( !ConnectSerial( &self->mSensor, lConnectionString, lAdditionalInfo, lAdditionalInfo2 ) )
                Py_RETURN_FALSE;
        }

        if( self->mSensor->GetConnection()->GetDeviceType() != LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VU8 )
        {
            DebugTrace( "Sensor with requested name is not a Vu8." );
            Py_RETURN_FALSE;
        }

        Py_RETURN_TRUE;
    }
    else
    {
        if( ConnectUsb( &self->mSensor, lConnectionString ) )
            Py_RETURN_TRUE;
        else if( ConnectEthernet( &self->mSensor, lConnectionString, 48630 ) )
            Py_RETURN_TRUE;
        else if( ConnectSerial( &self->mSensor, lConnectionString, 1, 115200 ) )
            Py_RETURN_TRUE;
        else if( ConnectSPIFTDI( &self->mSensor, lConnectionString ) )
            Py_RETURN_TRUE;

        Py_RETURN_FALSE;
    }

    Py_RETURN_FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *Disconnect( sLeddarDevice *self, PyObject *args )
///
/// \brief  Disconnect from sensor
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    No argument.
///
/// \return True on success.
///
/// \author David Levy
/// \date   December 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *Disconnect( sLeddarDevice *self, PyObject *args )
{
    if( self->mSensor != nullptr )
    {
        if( self->mDataThread.joinable() )
        {
            StopDataThread( self, nullptr );
        }

        self->mSensor->Disconnect();
        DebugTrace( "Disconnected" );
    }

    Py_RETURN_TRUE;
}



PyObject *GetPropertiesSnapshot( sLeddarDevice *self )
{
    if( self->mSensor == nullptr )
    {
        PyErr_SetString( PyExc_RuntimeError, "Not connected to a sensor." );
        return nullptr;
    }

    try
    {
        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );

        const auto *all_props = self->mSensor->GetProperties()->GetContent();

        PyObject *lValues = PyDict_New();

        for( auto const &k_v : *all_props )
        {
            LeddarCore::LdProperty *property = k_v.second;
            size_t count = k_v.second->Count();

            if( count > 1 )
            {

                PyObject *listObj = PyList_New( count );

                for( size_t i = 0; i < count; i++ )
                    PyList_SetItem( listObj, i, PyUnicode_FromString( property->GetStringValue( i ).c_str() ) );

                PyDict_SetItemString( lValues, std::to_string( k_v.first ).c_str(), listObj );
            }
        }

        return lValues;
    }
    catch( std::exception &e )
    {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
        return nullptr;
    }
}

int GetPropertyId( char *key )
{
    static PyObject *dict = nullptr;

    if( dict == nullptr )
        dict = GetPropertyIdDict( nullptr, nullptr );

    if( PyObject *value = PyDict_GetItemString( dict, key ) )
    {
        return PyLong_AsLong( value );
    }

    return -1;
}

#define PARSE_PROPERTY_HELPER(args, fmt, ifmt, lPropertyId_address, ...)\
    if( !PyArg_ParseTuple( args, fmt, lPropertyId_address, ##__VA_ARGS__ ) )\
    {\
        PyErr_Clear();\
        char * key = nullptr;\
        if(!PyArg_ParseTuple(args, ifmt, &key, ##__VA_ARGS__))\
        {\
            PyErr_SetString( PyExc_RuntimeError, "Unexpected arguments." );\
            return nullptr;\
        }\
        *lPropertyId_address = GetPropertyId(key);\
        if(*lPropertyId_address == -1)\
            return nullptr;\
    }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *GetPropertyValue( sLeddarDevice *self, PyObject *args )
///
/// \brief  Gets property value
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 int: property id
///                 int: (optional) index
///
/// \return Null if it fails, else the property value.
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *GetPropertyValue( sLeddarDevice *self, PyObject *args )
{
    if( self->mSensor == nullptr )
    {
        PyErr_SetString( PyExc_RuntimeError, "Not connected to a sensor." );
        return nullptr;
    }

    int lPropertyId = 0, lIndex = 0;

    PARSE_PROPERTY_HELPER( args, "i|i", "s|i", &lPropertyId, &lIndex );

    try
    {
        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );
        return PyUnicode_FromString( self->mSensor->GetProperties()->GetProperty( lPropertyId )->GetStringValue( lIndex ).c_str() );
    }
    catch( std::exception &e )
    {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
        return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *GetPropertyCount( sLeddarDevice *self, PyObject *args )
///
/// \brief  Gets property count
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 int: property id
///
/// \return Null if it fails, else the property count.
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *GetPropertyCount( sLeddarDevice *self, PyObject *args )
{
    if( self->mSensor == nullptr )
    {
        PyErr_SetString( PyExc_RuntimeError, "Not connected to a sensor." );
        return nullptr;
    }

    int lPropertyId = 0;

    PARSE_PROPERTY_HELPER( args, "i", "s", &lPropertyId );

    try
    {
        //If we overflow a long with a property count, we have a serious problem...
        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );
        return PyLong_FromLong( static_cast<long>( self->mSensor->GetProperties()->GetProperty( lPropertyId )->Count() ) );
    }
    catch( std::exception &e )
    {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
        return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *GetPropertyAvailableValues( sLeddarDevice *self, PyObject *args )
///
/// \brief  Gets available property values
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 int: property id
///
/// \return Null if it fails, else the property available values.
///
/// \author David Levy
/// \date   December 2017
////////////////////////////////////////////////////////////////////////////////////////////////////


#define SET_CURRENT_VALUES(property, dict, name, TO_PYTHON_TYPE) \
    size_t count = property->Count();\
    if(count > 1)\
    {\
        PyObject *listObj = PyList_New( count );\
        for( size_t i = 0; i < count; i++ )\
            PyList_SetItem( listObj, i, TO_PYTHON_TYPE( property->Value( i ) ) );\
        PyDict_SetItemString( dict, name, listObj);\
    }\
    else\
        PyDict_SetItemString( dict, name, TO_PYTHON_TYPE( property->Value( 0 ) ) );\


PyObject *GetPropertyAvailableValues( sLeddarDevice *self, PyObject *args )
{
    if( !CheckSensor( self ) )
        return nullptr;

    int lPropertyId = 0;

    PARSE_PROPERTY_HELPER( args, "i", "s", &lPropertyId );

    try
    {
        PyObject *lValues = PyDict_New();

        LeddarCore::LdProperty *lProp = nullptr;
        {
            std::lock_guard<std::mutex> lock( self->mDataThreadMutex );
            lProp = self->mSensor->GetProperties()->GetProperty( lPropertyId );
        }

        PyDict_SetItemString( lValues, "features", PyLong_FromLong( lProp->GetFeatures() ) );

        PyDict_SetItemString( lValues, "category", PyLong_FromLong( lProp->GetCategory() ) );

        PyDict_SetItemString( lValues, "const", PyBool_FromLong( lProp->GetCategory() == LeddarCore::LdProperty::CAT_CONSTANT ) );

        switch( lProp->GetType() )
        {
            case LeddarCore::LdProperty::TYPE_ENUM:
                if( LeddarCore::LdEnumProperty *lEnumProp = dynamic_cast< LeddarCore::LdEnumProperty * >( lProp ) )
                {
                    std::vector<std::string> lIntensityValues;

                    for( size_t i = 0; i < lEnumProp->EnumSize(); ++i )
                        lIntensityValues.push_back( lEnumProp->EnumText( i ) );

                    PyDict_SetItemString( lValues, "type", PyUnicode_FromString( "list" ) );
                    PyDict_SetItemString( lValues, "current", PyLong_FromLong( lEnumProp->Value() ) );
                    PyDict_SetItemString( lValues, "data", VectorToList_String( lIntensityValues ) );
                }

                break;

            case LeddarCore::LdProperty::TYPE_BOOL:
                if( LeddarCore::LdBoolProperty *lBoolProp = dynamic_cast< LeddarCore::LdBoolProperty * >( lProp ) )
                {
                    PyDict_SetItemString( lValues, "type", PyUnicode_FromString( "bool" ) );
                    SET_CURRENT_VALUES( lBoolProp, lValues, "current", PyBool_FromLong );
                    PyDict_SetItemString( lValues, "data", VectorToList_String( { "true", "false" } ) );
                }

                break;

            case LeddarCore::LdProperty::TYPE_FLOAT:
                if( LeddarCore::LdFloatProperty *lFloatprop = dynamic_cast< LeddarCore::LdFloatProperty * >( lProp ) )
                {
                    PyDict_SetItemString( lValues, "type", PyUnicode_FromString( "range" ) );
                    SET_CURRENT_VALUES( lFloatprop, lValues, "current", PyFloat_FromDouble );
                    PyDict_SetItemString( lValues, "current", PyLong_FromLong( lFloatprop->Value() ) );
                    PyDict_SetItemString( lValues, "data", VectorToList_Float( { lFloatprop->MinValue(), lFloatprop->MaxValue() } ) );
                }

                break;

            case LeddarCore::LdProperty::TYPE_INTEGER:
                if( LeddarCore::LdIntegerProperty *lIntProp = dynamic_cast< LeddarCore::LdIntegerProperty * >( lProp ) )
                {
                    PyDict_SetItemString( lValues, "type", PyUnicode_FromString( "range" ) );
                    SET_CURRENT_VALUES( lIntProp, lValues, "current", PyLong_FromLong );
                    PyDict_SetItemString( lValues, "data", VectorToList_Long( { static_cast<long>( lIntProp->MinValue() ), static_cast<long>( lIntProp->MaxValue() ) } ) );
                }

                break;

            case LeddarCore::LdProperty::TYPE_BITFIELD:
                if( LeddarCore::LdBitFieldProperty *lBitProp = dynamic_cast< LeddarCore::LdBitFieldProperty * >( lProp ) )
                {
                    PyDict_SetItemString( lValues, "type", PyUnicode_FromString( "bitfield" ) );
                    SET_CURRENT_VALUES( lBitProp, lValues, "current", PyLong_FromLong );
                }

                break;

            case LeddarCore::LdProperty::TYPE_TEXT:
                if( LeddarCore::LdTextProperty *lTxtProp = dynamic_cast< LeddarCore::LdTextProperty * >( lProp ) )
                {
                    PyDict_SetItemString( lValues, "type", PyUnicode_FromString( "text" ) );
                    PyDict_SetItemString( lValues, "current", PyUnicode_FromString( lTxtProp->Value().c_str() ) );
                }

            case LeddarCore::LdProperty::TYPE_BUFFER:
                if( LeddarCore::LdBufferProperty *lBufProp = dynamic_cast< LeddarCore::LdBufferProperty * >( lProp ) )
                {
                    PyDict_SetItemString( lValues, "type", PyUnicode_FromString( "text" ) );
                    PyDict_SetItemString( lValues, "current", PyUnicode_FromString( lBufProp->GetStringValue().c_str() ) );
                }

                break;

            default:
            {
                PyErr_SetString( PyExc_RuntimeError, "Unhandled property type" );
                return nullptr;
            }
        }

        return lValues;
    }
    catch( std::exception &e )
    {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
        return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *SetPropertyValue( sLeddarDevice *self, PyObject *args )
///
/// \brief  Sets property value
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 int: property id
///                 string: new value
///                 int: (optional) index
///
/// \return True on success.
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *SetPropertyValue( sLeddarDevice *self, PyObject *args )
{
    if( !CheckSensor( self ) )
        return nullptr;

    int lPropertyId = 0, lIndex = 0;
    char *lPropValue;

    PARSE_PROPERTY_HELPER( args, "is|i", "ss|i", &lPropertyId, &lPropValue, &lIndex );

    try
    {
        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );
        self->mSensor->GetProperties()->GetProperty( lPropertyId )->SetStringValue( lIndex, lPropValue );
        self->mSensor->SetConfig();
        self->mSensor->WriteConfig();
        Py_RETURN_TRUE;
    }
    catch( std::exception &e )
    {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
        return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *SetAccumulationExponent( sLeddarDevice *self, PyObject *args )
///
/// \brief  Sets the accumulation exponent
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 int: New accumulation exponent
///
/// \return True on success.
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *SetAccumulationExponent( sLeddarDevice *self, PyObject *args )
{
    if( !CheckSensor( self ) )
        return nullptr;

    int lValue = 0;

    if( !PyArg_ParseTuple( args, "i", &lValue ) )
        return nullptr;

    try
    {
        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );
        self->mSensor->GetProperties()->GetProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->SetStringValue( 0, LeddarUtils::LtStringUtils::IntToString( lValue ) );
        self->mSensor->SetConfig();
        self->mSensor->WriteConfig();

        if( self->mSensor->GetProperties()->GetProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->GetStringValue() != LeddarUtils::LtStringUtils::IntToString( lValue ) )
            Py_RETURN_FALSE;
        else
            Py_RETURN_TRUE;
    }
    catch( std::exception &e )
    {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
        return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *SetOversamplingExponent( sLeddarDevice *self, PyObject *args )
///
/// \brief  Sets the oversampling exponent
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 int: New oversampling exponent
///
/// \return True on success.
///
/// \author David Levy
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *SetOversamplingExponent( sLeddarDevice *self, PyObject *args )
{
    if( !CheckSensor( self ) )
        return nullptr;

    int lValue = 0;

    if( !PyArg_ParseTuple( args, "i", &lValue ) )
        return nullptr;

    try
    {
        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );
        self->mSensor->GetProperties()->GetProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->SetStringValue( 0, LeddarUtils::LtStringUtils::IntToString( lValue ) );
        self->mSensor->SetConfig();
        self->mSensor->WriteConfig();

        if( self->mSensor->GetProperties()->GetProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->GetStringValue() != LeddarUtils::LtStringUtils::IntToString( lValue ) )
            Py_RETURN_FALSE;
        else
            Py_RETURN_TRUE;
    }
    catch( std::exception &e )
    {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
        return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *GetIPConfig( sLeddarDevice *self, PyObject *args )
///
/// \brief  Gets IP configuration
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    No argument.
///
/// \return Null if it fails, else the IP configuration.
///
/// \author David Levy
/// \date   December 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *GetIPConfig( sLeddarDevice *self, PyObject *args )
{
    if( !CheckSensor( self ) )
        return nullptr;

    try
    {
        uint32_t lIPConfig = 0;
        {
            std::lock_guard<std::mutex> lock( self->mDataThreadMutex );

            if( LeddarCore::LdBufferProperty *p = dynamic_cast< LeddarCore::LdBufferProperty * >( self->mSensor->GetProperties()->GetProperty( LeddarCore::LdPropertyIds::ID_IP_ADDRESS ) ) )
            {
                lIPConfig = *( ( uint32_t * )p->Value( 0 ) );
            }
        }

        if( lIPConfig == 0 )
        {
            return PyUnicode_FromString( "Dynamic" );
        }
        else if( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_LCA2_DISCRETE == self->mSensor->GetConnection()->GetDeviceType() )
        {
            //Special treatment for LCA2 discrete, the IP stack is weird and does not take input as network byte order, but depends on the endianess of the hardware
            lIPConfig = LeddarUtils::LtIntUtilities::SwapEndian( lIPConfig );

        }

        return PyUnicode_FromString( LeddarUtils::LtStringUtils::Ip4AddrToString( lIPConfig ).c_str() );

    }
    catch( std::exception &e )
    {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
        return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *SetIPConfig( sLeddarDevice *self, PyObject *args )
///
/// \brief  Sets IP configuration
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 int: dynamic / static
///                 string: (optional) new IP address
///
/// \return Null if it fails, else a pointer to a PyObject.
///
/// \author David Levy
/// \date   December 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *SetIPConfig( sLeddarDevice *self, PyObject *args )
{
    if( !CheckSensor( self ) )
        return nullptr;

    bool lDynamic = 0;
    const char *lNewIp = nullptr;

    if( !PyArg_ParseTuple( args, "b|s", &lDynamic, &lNewIp ) )
        return nullptr;

    try
    {
        uint32_t lIPValue = 0;

        if( lDynamic )
        {
            lIPValue = 0;
        }
        else
        {
            lIPValue = LeddarUtils::LtStringUtils::StringToIp4Addr( lNewIp );

            if( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_LCA2_DISCRETE == self->mSensor->GetConnection()->GetDeviceType() )
            {
                lIPValue = LeddarUtils::LtIntUtilities::SwapEndian( lIPValue );
            }
        }

        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );

        if( LeddarCore::LdBufferProperty *p = dynamic_cast< LeddarCore::LdBufferProperty * >( self->mSensor->GetProperties()->GetProperty( LeddarCore::LdPropertyIds::ID_IP_ADDRESS ) ) )
        {
            p->SetValue( 0, ( uint8_t * )&lIPValue, sizeof( lIPValue ) );
            self->mSensor->SetConfig();
            self->mSensor->WriteConfig();
        }

        Py_RETURN_TRUE;
    }
    catch( std::exception &e )
    {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
        return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *GetDataMask( sLeddarDevice *self, PyObject *args )
///
/// \brief  Gets data mask
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    No argument.
///
/// \return The current data mask.
///
/// \author Maxime Lemonnier
/// \date   April 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *GetDataMask( sLeddarDevice *self, PyObject *args )
{
    return PyLong_FromLong( self->mDataMask );
}

// *****************************************************************************
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
PyObject *SetDataMask( sLeddarDevice *self, PyObject *args )
{
    if( !CheckSensor( self ) )
        return nullptr;

    int lDataMask = 0;

    if( !PyArg_ParseTuple( args, "i", &lDataMask ) )
        return nullptr;

    {
        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );

        try
        {
            self->mSensor->SetDataMask( lDataMask );
        }
        catch( const std::exception &e )
        {
            PyErr_SetString( PyExc_RuntimeError, e.what() );
            return nullptr;
        }

    }
    self->mDataMask = lDataMask;
    Py_RETURN_TRUE;
}

// *****************************************************************************
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
PyObject *GetStates( sLeddarDevice *self, PyObject *args )
{
    if( !CheckSensor( self ) )
        return nullptr;

    size_t lNRetries = 5;

    if( !PyArg_ParseTuple( args, "|n", &lNRetries ) )
        return nullptr;

    return RetryNTimes( [&]()
    {
        ScopedDataMask sdm( self, LeddarDevice::LdSensor::DM_STATES );
        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );

        if( !self->mSensor->GetData() )
            throw std::runtime_error( "No new states available!" );

        return PackageStates( self->mSensor->GetResultStates() );
    }, lNRetries );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *PackageStates( LeddarConnection::LdResultStates *aResultStates )
///
/// \brief  Package the last received states to be sent to python
///
/// \exception  std::runtime_error    Raised when we cannot allocate memory for python list.
///
/// \param [in] aResultStates   Pointer to the sensor's result states.
///
/// \return  A dict with all available states
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////

#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)

#define ADD_PROPERTY(NAME, PROPERTY_ID, PROPERTY_TYPE, TO_PYTHON_TYPE) \
    static bool PPCAT(ADD_PROPERTY_, PROPERTY_ID) = false;\
    try\
    {\
        if( PROPERTY_TYPE * property = dynamic_cast<PROPERTY_TYPE *>(aResultStates->GetProperties()->FindProperty( PROPERTY_ID )))\
        {\
            size_t count = property->Count();\
            if(count > 1)\
            {\
                PyObject *listObj = PyList_New( count );\
                for( size_t i = 0; i < count; i++ )\
                    PyList_SetItem( listObj, i, TO_PYTHON_TYPE( property->Value( i ) ) );\
                PyDict_SetItemString( lStates, NAME, listObj );\
            }\
            else\
                PyDict_SetItemString( lStates, NAME, TO_PYTHON_TYPE( property->Value( 0 ) ) );\
        }\
    }\
    catch (const std::exception& e)\
    {\
        if(!PPCAT(ADD_PROPERTY_, PROPERTY_ID)) \
        {\
            PPCAT(ADD_PROPERTY_, PROPERTY_ID) = true;\
            DebugTrace(e.what());\
        }\
    }\


PyObject *PackageStates( LeddarConnection::LdResultStates *aResultStates )
{
    PyObject *lStates = PyDict_New();

    using namespace LeddarCore::LdPropertyIds;

    ADD_PROPERTY( "timestamp", ID_RS_TIMESTAMP, LeddarCore::LdIntegerProperty,  PyLong_FromLong );
    ADD_PROPERTY( "timestamp64", ID_RS_TIMESTAMP64, LeddarCore::LdIntegerProperty,  PyLong_FromLong );
    ADD_PROPERTY( "system_temp", ID_RS_SYSTEM_TEMP, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "predict_temp", ID_RS_PREDICT_TEMP, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "cpu_load", ID_RS_CPU_LOAD, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "discrete_outputs", ID_RS_DISCRETE_OUTPUTS, LeddarCore::LdIntegerProperty,  PyLong_FromLong );
    ADD_PROPERTY( "acq_current_params", ID_RS_ACQ_CURRENT_PARAMS, LeddarCore::LdIntegerProperty,  PyLong_FromLong );
    ADD_PROPERTY( "apd_temp", ID_RS_APD_TEMP, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "backup", ID_RS_BACKUP, LeddarCore::LdIntegerProperty,  PyLong_FromLong );
    ADD_PROPERTY( "apd_gain", ID_RS_APD_GAIN, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "noise_level", ID_RS_NOISE_LEVEL, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "adc_rssi", ID_RS_ADC_RSSI, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "snr", ID_RS_SNR, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "v3m_temp", ID_RS_V3M_TEMP, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "pmic_temp", ID_RS_PMIC_TEMP, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );

    return lStates;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *GetEchoes( sLeddarDevice *self, PyObject *args )
///
/// \brief  Get the last echoes from sensor.
///
/// \exception  std::runtime_error  Raised when a runtime error condition occurs.
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 int/size_t: (optional) number of retry
///                 int: (optional) time between retry (ms)
///
/// \return nullptr if there is no new data, else see ­\ref PackageEchoes
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *GetEchoes( sLeddarDevice *self, PyObject *args )
{
    if( !CheckSensor( self ) )
        return nullptr;

    size_t lNRetries = 5;
    int lMsBetweenRetries = 15;

    if( !PyArg_ParseTuple( args, "|ni", &lNRetries, &lMsBetweenRetries ) )
        return nullptr;

    return RetryNTimes( [&]()
    {
        ScopedDataMask sdm( self, LeddarDevice::LdSensor::DM_ECHOES );

        if( !self->mSensor->GetData() )
        {
            _sleep_ms( lMsBetweenRetries );
            throw std::runtime_error( "No new echoes available!" );
        }

        return PackageEchoes( self->mSensor->GetResultEchoes() );
    }, lNRetries );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *PackageEchoes( LeddarConnection::LdResultEchoes *aResultEchoes )
///
/// \brief  Package echoes
///
/// \exception  std::logic_error    Raised when unable to allocate memory for Python list.
///
/// \param [in,out] aResultEchoes   Pointer to the sensor's result echoes.
///
/// \return A dict with keys: indices, flags, data and timestamp
///
/// \author David Levy, Maxime Lemonnier
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////

struct LeddarPyEcho
{
    uint32_t index;
    float distance;
    float amplitude;
    uint16_t timestamp;
    uint16_t flag;
};
PyObject *PackageEchoes( LeddarConnection::LdResultEchoes *aResultEchoes )
{
    std::vector<LeddarConnection::LdEcho> &lEchoes = *( aResultEchoes->GetEchoes() );
    npy_intp dimsIndices = aResultEchoes->GetEchoCount();
    PyObject *lEchoesDict = PyDict_New();

    if( !lEchoesDict )
        throw std::logic_error( "Unable to allocate memory for Python list" );

    PyDict_SetItemString( lEchoesDict, "timestamp", PyLong_FromLong( aResultEchoes->GetTimestamp() ) );
    PyDict_SetItemString( lEchoesDict, "distance_scale", PyLong_FromLong( aResultEchoes->GetDistanceScale() ) );
    PyDict_SetItemString( lEchoesDict, "amplitude_scale", PyLong_FromLong( aResultEchoes->GetAmplitudeScale() ) );
    PyDict_SetItemString( lEchoesDict, "led_power", PyLong_FromLong( aResultEchoes->GetCurrentLedPower() ) );
    PyDict_SetItemString( lEchoesDict, "v_fov", PyLong_FromLong( aResultEchoes->GetVFOV() ) );
    PyDict_SetItemString( lEchoesDict, "h_fov", PyLong_FromLong( aResultEchoes->GetHFOV() ) );
    PyDict_SetItemString( lEchoesDict, "v", PyLong_FromLong( aResultEchoes->GetVChan() ) );
    PyDict_SetItemString( lEchoesDict, "h", PyLong_FromLong( aResultEchoes->GetHChan() ) );


    npy_intp dims[1] = {dimsIndices};

    PyObject *op = Py_BuildValue( "[(s, s), (s, s), (s, s), (s, s), (s, s)]"
                                  , "indices", "u4"
                                  , "distances", "f4"
                                  , "amplitudes", "f4"
                                  , "timestamps", "u2"
                                  , "flags", "u2" );
    PyArray_Descr *descr;
    PyArray_DescrConverter( op, &descr );
    Py_DECREF( op );
    PyObject *lEchoesArray = PyArray_SimpleNewFromDescr( 1, dims, descr );
    PyDict_SetItemString( lEchoesDict, "data", lEchoesArray );
    Py_DECREF( lEchoesArray );

    for( int i = 0; i < dimsIndices; ++i )
    {
        LeddarPyEcho *ech_ptr = static_cast<LeddarPyEcho *>PyArray_GETPTR1( ( PyArrayObject * )lEchoesArray, i );

        ech_ptr->index = uint32_t( lEchoes[i].mChannelIndex );
        ech_ptr->distance = float( lEchoes[i].mDistance ) / aResultEchoes->GetDistanceScale();
        ech_ptr->amplitude = float( lEchoes[i].mAmplitude ) / aResultEchoes->GetAmplitudeScale();
        ech_ptr->timestamp = uint16_t( 0 ); //TODO it should contain offset from main timestamp
        ech_ptr->flag = uint16_t( lEchoes[i].mFlag );
    }

    return lEchoesDict;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *SetCallBackState( sLeddarDevice *self, PyObject *args )
///
/// \brief  Sets the call back function for new states
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 python object: the python callback function
///
/// \return True on success.
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *SetCallBackState( sLeddarDevice *self, PyObject *args )
{
    PyObject *lCallback = nullptr;

    if( !PyArg_ParseTuple( args, "O", &lCallback ) )
        return nullptr;

    if( self->mCallBackState != nullptr )
    {
        Py_DECREF( self->mCallBackState );
    }

    Py_INCREF( lCallback );

    self->mCallBackState = lCallback;

    Py_RETURN_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *SetCallBackEcho( sLeddarDevice *self, PyObject *args )
///
/// \brief  Set the callback function when new echoes are received
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 python object: the python callback function
///
/// \return True on success
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *SetCallBackEcho( sLeddarDevice *self, PyObject *args )
{
    PyObject *lCallback = nullptr;

    if( !PyArg_ParseTuple( args, "O", &lCallback ) )
        return nullptr;

    if( self->mCallBackEcho != nullptr )
    {
        Py_DECREF( self->mCallBackEcho );
    }

    Py_INCREF( lCallback );

    self->mCallBackEcho = lCallback;

    Py_RETURN_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void DataThread( sLeddarDevice *self )
///
/// \brief  Worker thread that fetch data from sensor and call callbacks when need data is received
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
///
/// \author David Levy
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void DataThread( sLeddarDevice *self )
{
    uint16_t lErrorCount = 0;
    CallBackManger lCallBackManager( self );

    auto lLastPing = std::chrono::system_clock::now();

    while( true )
    {

        try
        {
            uint32_t lDelay = 0;
            {
                std::lock_guard<std::mutex> lock( self->mDataThreadMutex );
                lDelay = self->mDataThreadSharedData.mDelay;

                if( self->mDataThreadSharedData.mStop )
                {
                    DebugTrace( "Request to stop thread received, stopping..." );
                    break;
                }
            }


            bool lNewData = false;

            try
            {
                self->mDataThreadMutex.lock(); //we can't use a std::lock_guard here, since we have to free the lock BEFORE acquiring GIL
                self->mGetDataLocked = true;
                bool lNewData = self->mSensor->GetData(); //mutex will be handled by CallBackManager

                if( lNewData )
                    lErrorCount = 0;
            }
            catch( std::exception &e )
            {
                DebugTrace( std::string( "Error in data thread (GetData()): " ) + e.what() );
                ++lErrorCount;
            }
            catch( ... )
            {
                DebugTrace( "Unhandled exception in data thread (GetData())" );
                ++lErrorCount;
            }

            if( !lNewData )
                _sleep_ms( 1 );

            if( self->mGetDataLocked )
            {
                self->mDataThreadMutex.unlock(); // if no new data is found, or a exception was thrown, CallBackManager could not unlock the mutex
                self->mGetDataLocked = false;
            }


            LeddarUtils::LtTimeUtils::WaitBlockingMicro( lDelay * ( lErrorCount + 1 ) );

        }
        catch( ... )
        {
            DebugTrace( "Unhandled exception!" );
        }
    }

    DebugTrace( "Thread stopped!" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *StartDataThread( sLeddarDevice *self, PyObject *args )
///
/// \brief  Starts data thread
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    No argument.
///
/// \return Null if it fails, else a pointer to a PyObject.
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *StartDataThread( sLeddarDevice *self, PyObject *args )
{
    if( !CheckSensor( self ) )
        return nullptr;

    DebugTrace( "Starting thread" );
    {
        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );
        self->mDataThreadSharedData.mStop = false;
    }

    self->mDataThread = std::thread( DataThread, self );

    Py_RETURN_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *StopDataThread( sLeddarDevice *self, PyObject *args )
///
/// \brief  Stops data thread
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    No argument.
///
/// \return Null if it fails, else a pointer to a PyObject.
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *StopDataThread( sLeddarDevice *self, PyObject *args )
{
    {
        DebugTrace( "Obtaining mutex" );
        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );
        self->mDataThreadSharedData.mStop = true;
    }

    if( self->mDataThread.joinable() )
    {
        DebugTrace( "Joining thread" );
        self->mDataThread.join();
        DebugTrace( "Thread joined" );
    }

    Py_RETURN_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *SetDataThreadDelay( sLeddarDevice *self, PyObject *args )
///
/// \brief  Sets data thread delay between two data fetch.
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 int: the delay (us)
///
/// \return Null if it fails, else a pointer to a PyObject.
///
/// \author David Levy
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *SetDataThreadDelay( sLeddarDevice *self, PyObject *args )
{
    int lDelay = 0;

    if( !PyArg_ParseTuple( args, "i", &lDelay ) )
    {
        PyErr_Print();
        return nullptr;
    }

    if( lDelay < 0 )
        Py_RETURN_FALSE;

    {
        std::lock_guard<std::mutex> lock( self->mDataThreadMutex );
        self->mDataThreadSharedData.mDelay = lDelay;
    }
    Py_RETURN_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *StartStopRecording( sLeddarDevice *self, PyObject *args )
///
/// \brief  Starts or stop recording
///
/// \param [in,out] self    The class instance that this method operates on.
/// \param [in,out] args    The arguments.
///                 string(optional): Path to the file. If empty, will generate a ljr record with device name and date - time
///
/// \return Null if it fails, else a pointer to a PyObject.
///
/// \author David Levy
/// \date   December 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *StartStopRecording( sLeddarDevice *self, PyObject *args )
{
    const char *aPath = nullptr;

    if( !PyArg_ParseTuple( args, "|s", &aPath ) )
    {
        PyErr_Print();
        return nullptr;
    }

    if( self->mRecorder != nullptr )
    {
        self->mRecorder->StopRecording();
        delete self->mRecorder;
        self->mRecorder = nullptr;
        Py_RETURN_TRUE;
    }

    std::string lPath( "" );

    if( aPath != nullptr )
        lPath = std::string( aPath );


    self->mRecorder = new LeddarRecord::LdLjrRecorder( self->mSensor );
    self->mRecorder->StartRecording( lPath );
    Py_RETURN_TRUE;
}
