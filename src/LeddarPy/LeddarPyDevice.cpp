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
#include "LtMathUtils.h"
#include "LtExceptions.h"

#include "LdLibModbusSerial.h"
#include "LdSpiFTDI.h"
#include "LdLibUsb.h"
#include "LdEthernet.h"

#include "LdSensor.h"


#include "LdSensorLeddarAuto.h"
#include "LdSensorPixell.h"

#include "LdLjrRecorder.h"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define NO_IMPORT_ARRAY
#define PY_ARRAY_UNIQUE_SYMBOL leddar_ARRAY_API
#include <numpy/arrayobject.h>

#include <algorithm>
#include <chrono>


#ifdef _WIN32
#include <Ws2tcpip.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 2
#define HAVE_HEAP_TYPES
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
#define HAVE_STRVAR
#endif

//Python Member function list
PyMethodDef Device_methods[] =
{
    {
        "connect", ( PyCFunction )Connect, METH_VARARGS, ( char const * )"Connect to a sensor.\n"
        "param1: [All] value returned by GetDeviceList['name'], [Serial] serial port com, [USB] serial number, [SPI-FTDI] FTDI cable id (use get_devices(\"SpiFTDI\")), [CANBus komodo] Baudrate in kbps, [Ethernet] Ip address\n"
        "param2: (optional but recommended) Device type | connection type - Both are mandatory for CANBus protocol (from device_types dictionnary)\n"
        "param3: (optional) [Serial] modbus address (default 1), [CANBus komodo] Tx (default 0x750) ,[Ethernet] port (default 48630)\n"
        "param4: (optional) [Serial] baudrate (default 115200), [CANBus komodo] Rx (default 0x740),[Ethernet] Communication timeout\n"
        "Returns: False if it didnt connect to the requested sensor. True if connected"
    },
    { "disconnect", ( PyCFunction )Disconnect, METH_NOARGS, "Disconnect from sensor.\nReturns: True " },
    {
        "get_property_value", ( PyCFunction )GetPropertyValue, METH_VARARGS, "Get the value of a property.\n"
        "param1: Property id (from leddar.property_ids)\n"
        "param2: The index (optional)\n"
        "Returns: (string) Property value "
    },
    {
        "get_property_count", ( PyCFunction )GetPropertyCount, METH_VARARGS, "Get the count of a property.\n"
        "param1: Property id (from leddar.property_ids)\n"
        "Returns: (int) Property count "
    },
    {
        "get_property_available_values", ( PyCFunction )GetPropertyAvailableValues, METH_VARARGS, "Get the available values of the property.\n"
        "param1: Property id (from leddar.property_ids)\n"
        "Returns  False if property is constant or limits have no meanings, else a dict with \"type\" (range or list) and \"data\" containing the actual values, \n"
        "a range (min & max) or all possible values"
    },
    {
        "get_properties_snapshot", ( PyCFunction )GetPropertiesSnapshot, METH_VARARGS, "Get the current value of all available properties.\n"
        "Returns  dict with property ids for keys and all property values for values. \n"
        "If a property has multiple values, it will be a list"
    },
    {
        "set_property_value", ( PyCFunction )SetPropertyValue, METH_VARARGS, "Set the value of the property.\n"
        "param1: Property id (from leddar.property_ids)\n"
        "param2: the new property value (as a string)\n"
        "param3: The index (optional)\n"
        "Returns: True on success"
    },
    {
        "set_accumulation_exponent", ( PyCFunction )SetAccumulationExponent, METH_VARARGS, "Set the accumulation exponent value.\n"
        "param1: (int) the new value  see get_property_available_values() \n"
        "Returns: True on success"
    },
    {
        "set_oversampling_exponent", ( PyCFunction )SetOversamplingExponent, METH_VARARGS, "Set the oversampling exponent value.\n"
        "param1: (int) the new value  see get_property_available_values() \n"
        "Returns: True on success"
    },
    {
        "get_IP_config", ( PyCFunction )GetIPConfig, METH_NOARGS, "Get IP address configuration.\n"
        "Returns: (string) either \"Dynamic\" or the static ip"
    },
    {
        "set_IP_config", ( PyCFunction )SetIPConfig, METH_VARARGS, "Set IP address configuration.\n"
        "param1: (bool) dynamic ip (true) or static ip (false) \n"
        "param2: (string) (optional) the static ip\n"
        "Returns: True on success"
    },
    {
        "get_data_mask", ( PyCFunction )GetDataMask, METH_NOARGS, "Get the requested data mask.\n"
        "Returns: (int) the current data mask."
    },
    {
        "set_data_mask", ( PyCFunction )SetDataMask, METH_VARARGS, "Set the requested data mask.\n"
        "param1: (int) data mask value (from leddar.data_masks) \n"
        "Returns: True on success"
    },
    {
        "get_states", ( PyCFunction )GetStates, METH_VARARGS, "Get the last states from sensor.\n"
        "param1: (int) number of retries (optional, default to 5)\n"
        "Returns: False if there is no new states. Else return a dict with keys 'timestamp', 'cpu_load' and  'system_temp' and possibly 'apd_temps' (a list of 3 elements)"
    },
    {
        "get_echoes", ( PyCFunction )GetEchoes, METH_VARARGS, "Get last echoes from sensor.\n"
        "param1: (int) number of retries (optional, default to 5)\n"
        "param2: (int) ms between retries (optional, default to 15)\n"
        "Returns: Exception if there is no new data, else a dict with keys\n"
        "timestamp: the 32-bit base timestamp\n"
        "distance_scale: the scale that was applied to distances\n"
        "amplitude_scale: the scale that was applied to amplitudes\n"
        "led_power: the led power used\n"
        "v_fov: the vertical field of view\n"
        "h_fov: the horizontal field of view\n"
        "v: the vertical resolution\n"
        "h: the horizontal resolution\n"
        "data: a structured ndarray with fields:\n"
        "'indices' : (ndarray with shape (n_echoes, ) and dtype 'uint32') the channel index for each echo\n"
        "'distances' : (ndarray with shape (n_echoes, ) and dtype 'float32') the distance for each echo\n"
        "'amplitudes' : (ndarray with shape (n_echoes, ) and dtype 'float32') the amplitude for each echo\n"
        "'timestamps' : (ndarray with shape (n_echoes, ) and dtype 'uint16') the timestamp offset for each echo\n"
        "'flags' : (ndarray with shape (n_echoes, ) and dtype 'uint16') the flag for each echo\n"
    },
    {
        "get_calib_values", ( PyCFunction )GetCalibValues, METH_VARARGS, "returns the calibration values"
        "param1: (int) the type of calibration (see leddar.calib_types) \n"
        "Returns: the calibration"
    },
    {
        "set_callback_state", ( PyCFunction )SetCallBackState, METH_VARARGS, "Set a python function as a callback when new states are received.\n"
        "param1: (function) callback function with signatue f(new_state) (new_state same format as get_states()'s return value)\n"
        "Returns: True on success"
    },
    {
        "set_callback_echo", ( PyCFunction )SetCallBackEcho, METH_VARARGS, "Set a python function as a callback when new echoes are received.\n"
        "param1: (function) callback function with signatue f(new_echoes) (new_echoes same format as get_echoes()'s return value)\n"
        "Returns: True on success"
    },
    {
        "set_callback_exception", ( PyCFunction )SetCallBackException, METH_VARARGS, "Set a python function as a callback when an exception is raised in the data thread.\n"
        "param1: (function) callback function with signatue f(exception) (exception is a dictionnary with the exception text, and eventually additional info)\n"
        "Returns: True on success"
    },
    { "start_data_thread", ( PyCFunction )StartDataThread, METH_NOARGS, "Start the thread that fetch data from sensor in the background." },
    { "stop_data_thread", ( PyCFunction )StopDataThread, METH_NOARGS, "Stop the thread that fetch data from sensor in the background." },
    {
        "set_data_thread_delay", ( PyCFunction )SetDataThreadDelay, METH_VARARGS, "Set the waiting time (in �s) between two fetch request.\n"
        "param1: (int) time in microseconds between to requests\n"
        "Returns: True"
    },
    {
        "start_stop_recording", ( PyCFunction )StartStopRecording, METH_VARARGS, "Start or stop the recording.\n"
        "param1: (string)(optional) Path to the file. If empty, will generate a ltl record with device name and date - time\n"
        "Returns: True"
    },

    { NULL }  //Sentinel
};

#define LEDDAR_DEVICE_DOC "Device object."
#ifdef HAVE_STRVAR
PyDoc_STRVAR(Device_doc, LEDDAR_DEVICE_DOC);
#else
static char *Device_doc = LEDDAR_DEVICE_DOC;
#endif

//Object type definition for python
#define LEDDAR_DEVICE_TYPE_NAME "leddar.Device"
#ifdef HAVE_HEAP_TYPES
static PyType_Slot Device_slots[] =
{
    {Py_tp_dealloc,  (void *)Device_dealloc},
    {Py_tp_methods,  (void *)Device_methods},
    {Py_tp_init,     (void *)Device_init},
    {Py_tp_new,      (void *)Device_new},
    {Py_tp_traverse, (void *)Device_traverse},
    {Py_tp_doc,      (void *)Device_doc},
    {0, NULL},  //Sentinel
};

static PyType_Spec LeddarDeviceTypeSpec =
{
    LEDDAR_DEVICE_TYPE_NAME,                                        //name
    sizeof(sLeddarDevice),                                          //basicsize
    0,                                                              //itemsize
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE | Py_TPFLAGS_HAVE_GC,  //flags
    Device_slots,                                                   //slots
};
#else  //Use static type
static PyTypeObject LeddarDeviceType =
{
    PyVarObject_HEAD_INIT( NULL, 0 )
    LEDDAR_DEVICE_TYPE_NAME,        //tp_name
    sizeof( sLeddarDevice ),        //tp_basicsize
    0,                              //tp_itemsize
    ( destructor )Device_dealloc,   //tp_dealloc
    0,                              //tp_print
    0,                              //tp_getattr
    0,                              //tp_setattr
    0,                              //tp_compare (python 2) or *tp_as_async (python 3)
    0,                              //tp_repr
    0,                              //tp_as_number
    0,                              //tp_as_sequence
    0,                              //tp_as_mapping
    0,                              //tp_hash
    0,                              //tp_call
    0,                              //tp_str
    0,                              //tp_getattro
    0,                              //tp_setattro
    0,                              //tp_as_buffer
    Py_TPFLAGS_DEFAULT,             //tp_flags
    LEDDAR_DEVICE_DOC,              //tp_doc
    0,                              //tp_traverse
    0,                              //tp_clear
    0,                              //tp_richcompare
    0,                              //tp_weaklistoffset
    0,                              //tp_iter
    0,                              //tp_iternext
    Device_methods,                 //tp_methods
    0,                              //tp_members
    0,                              //tp_getset
    0,                              //tp_base
    0,                              //tp_dict
    0,                              //tp_descr_get
    0,                              //tp_descr_set
    0,                              //tp_dictoffset
    Device_init,                    //tp_init
    0,                              //tp_alloc
    Device_new,                     //tp_new
};
#endif

PyTypeObject *InitDeviceType()
{
#ifdef HAVE_HEAP_TYPES
    return ( PyTypeObject * )PyType_FromSpec(&LeddarDeviceTypeSpec);
#else  //Use static type
    if( PyType_Ready( &LeddarDeviceType ) < 0 ) //Initialize the type
        return nullptr;
    else
        return &LeddarDeviceType;
#endif
}

void CloseSocket( int socket )
{
    int lRet = 0;

#ifdef _WIN32
    lRet = closesocket( socket );
#else
    errno = 0;
    lRet = close( socket );
#endif

    if( lRet < 0 )
        throw std::runtime_error( "cannot close socket" );
}

void SetReceiveTimeout( int socket, uint32_t timeout_ms )
{
#ifdef _WIN32
    uint32_t lSockOptResult = setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, ( char * )&timeout_ms, sizeof( timeout_ms ) );

    if( lSockOptResult == SOCKET_ERROR )
#else
    struct timeval lTimeout;

    lTimeout.tv_sec = timeout_ms / 1000;
    lTimeout.tv_usec = ( timeout_ms % 1000 ) * 1000;
    int lSockOptResult = setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, &lTimeout, sizeof( lTimeout ) );

    if( lSockOptResult < 0 )
#endif
    {
        throw LeddarException::LtComException( "Failed to set socket option SO_RCVTIMEO (setsockopt): " );
    }
}

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
        if( mSelf->mDataThreadSharedData.mGetDataLocked ) { //additional safety, it is illegal to unlock a mutex twice, do not use this field outside of DataThread()
            mSelf->mDataThreadSharedData.mMutex.unlock();
            mSelf->mDataThreadSharedData.mGetDataLocked = false;
        }

        if( aSignal != LeddarCore::LdObject::NEW_DATA )
            return;

        PyGILState_STATE gstate;

        if( aSender == mStates ) {
            if( mSelf->mDataThreadSharedData.mCallBackState ) {
                //Thread safe python call
                gstate = PyGILState_Ensure();

                if( PyObject *o = PackageStates( mStates ) ) {
                    PyObject_CallFunctionObjArgs( mSelf->mDataThreadSharedData.mCallBackState, o, NULL );
                    Py_DECREF( o );
                }

                // Release the python thread. No Python API allowed beyond this point.
                PyGILState_Release( gstate );
            }
        }
        else if( aSender == mEchoes ) {
            if( mSelf->mDataThreadSharedData.mCallBackEcho ) {
                //Thread safe python call
                gstate = PyGILState_Ensure();

                if( PyObject *o = PackageEchoes( mSelf->mSensor ) ) {
                    PyObject_CallFunctionObjArgs( mSelf->mDataThreadSharedData.mCallBackEcho, o, NULL );
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
    new( self ) sLeddarDevice; //Placement new (runs the constructor without allocating the memory)

    if( self != nullptr )
        self->mDataMask = LeddarDevice::LdSensor::DM_NONE;

    return ( PyObject * )self;
}

int Device_traverse( PyObject *self, visitproc visit, void *arg )
{
    PyTypeObject *tp = Py_TYPE(self);
    Py_VISIT(tp);
    return 0;
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
    PyObject_GC_UnTrack(self);

    Py_XDECREF( self->mDataThreadSharedData.mCallBackState );
    Py_XDECREF( self->mDataThreadSharedData.mCallBackEcho );
    Py_XDECREF( self->mDataThreadSharedData.mCallBackException );

    if( self->mRecorder != nullptr )
    {
        delete self->mRecorder;
        self->mRecorder = nullptr;
    }

    if( self->mSensor != nullptr )
    {
        Disconnect( self, nullptr );
    }

    DebugTrace( "C++ Device Destructor." );
    self->~sLeddarDevice();
    Py_TYPE( self )->tp_free( ( PyObject * )self );

    DebugTrace( "tp_free called successfully" );
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
    if( CONNECTION_TYPE_SERIAL == lConnectionType )
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
        if( self->mDataThreadSharedData.mThread.joinable() )
        {
            StopDataThread( self, nullptr );
        }

        self->mSensor->Disconnect();
        delete self->mSensor;
        self->mSensor = nullptr;
        DebugTrace( "Disconnected" );
    }

    Py_RETURN_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *GetCalibValues( sLeddarDevice *self, PyObject *args )
///
/// \brief  Gets calibration values
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 int: property id
///
/// \return Null if it fails, else the calibration value.
///
/// \author Maxime Lemonnier
/// \date   December 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *GetCalibValues( sLeddarDevice *self, PyObject *args )
{
    if( self->mSensor == nullptr )
    {
        PyErr_SetString( PyExc_RuntimeError, "Not connected to a sensor." );
        return nullptr;
    }

    int lPropertyId = 0;

    if( !PyArg_ParseTuple( args, "i", &lPropertyId ) )
        return nullptr;

    try
    {
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );
        self->mSensor->GetCalib();
        LeddarCore::LdProperty *lProp = self->mSensor->GetProperties()->GetProperty( lPropertyId );


        switch( lProp->GetType() )
        {
            case LeddarCore::LdProperty::TYPE_FLOAT:
                if( LeddarCore::LdFloatProperty *lFloatprop = dynamic_cast< LeddarCore::LdFloatProperty * >( lProp ) )
                {
                    npy_intp dims = lFloatprop->Count();
                    PyObject *lPyArray =  PyArray_SimpleNew( 1, &dims, NPY_FLOAT32 );

                    if( !lPyArray )
                        throw std::logic_error( "Unable to allocate memory for Numpy Array" );

                    for( npy_intp i = 0; i < dims; i++ )
                        ( static_cast<float *>PyArray_GETPTR1( ( PyArrayObject * )lPyArray, i ) )[0] = lFloatprop->Value( i );

                    return lPyArray;
                }

                break;


            case LeddarCore::LdProperty::TYPE_BUFFER:
                if( LeddarCore::LdBufferProperty *lBufProp = dynamic_cast< LeddarCore::LdBufferProperty * >( lProp ) )
                {
                    npy_intp dims[2] = {( npy_intp )lBufProp->Count(), ( npy_intp )lBufProp->Size()};
                    PyObject *lPyArray =  PyArray_SimpleNew( 2, dims, NPY_UBYTE );

                    if( !lPyArray )
                        throw std::logic_error( "Unable to allocate memory for Numpy Array" );

                    for( npy_intp row_i = 0; row_i < dims[0]; row_i++ )
                    {
                        auto bytes = lBufProp->GetValue( row_i );

                        for( npy_intp col_i = 0; col_i < dims[1]; ++col_i )
                            ( static_cast<uint8_t *>PyArray_GETPTR2( ( PyArrayObject * )lPyArray, row_i, col_i ) )[0] = bytes[col_i];

                    }

                    return lPyArray;
                }

                break;

            default:
            {
                PyErr_SetString( PyExc_RuntimeError, "Unhandled property type" );
            }
        }


    }
    catch( std::exception &e )
    {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
    }

    return nullptr;
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
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );

        const auto *all_props = self->mSensor->GetProperties()->GetContent();

        PyObject *lValues = PyDict_New();
        if( !lValues )
            return nullptr;

        for( auto const &k_v : *all_props )
        {
            LeddarCore::LdProperty *property = k_v.second;
            size_t count = k_v.second->Count();

            if( count > 1 )
            {

                PyObject *listObj = PyList_New( count );
                if( !listObj )
                {
                    Py_DECREF( lValues );
                    return nullptr;
                }

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
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );
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
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );
        return PyLong_FromUnsignedLong( static_cast<long>( self->mSensor->GetProperties()->GetProperty( lPropertyId )->Count() ) );
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
        if( !listObj )\
            throw std::logic_error( "Unable to allocate memory for Python list" );\
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
        if( !lValues )
            return nullptr;

        LeddarCore::LdProperty *lProp = nullptr;
        {
            std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );
            lProp = self->mSensor->GetProperties()->GetProperty( lPropertyId );
        }

        PyDict_SetItemString( lValues, "features", PyLong_FromUnsignedLong( lProp->GetFeatures() ) );

        PyDict_SetItemString( lValues, "category", PyLong_FromUnsignedLong( lProp->GetCategory() ) );

        PyDict_SetItemString( lValues, "const", PyLong_FromUnsignedLong( lProp->GetCategory() == LeddarCore::LdProperty::CAT_CONSTANT ) );

        PyDict_SetItemString( lValues, "description", PyUnicode_FromString( lProp->GetDescription().c_str() ) );

        switch( lProp->GetType() )
        {
            case LeddarCore::LdProperty::TYPE_ENUM:
                if( LeddarCore::LdEnumProperty *lEnumProp = dynamic_cast< LeddarCore::LdEnumProperty * >( lProp ) )
                {
                    std::vector<std::string> lIntensityValues;

                    for( size_t i = 0; i < lEnumProp->EnumSize(); ++i )
                        lIntensityValues.push_back( lEnumProp->EnumText( i ) );

                    PyDict_SetItemString( lValues, "type", PyUnicode_FromString( "list" ) );
                    PyDict_SetItemString( lValues, "current", PyLong_FromUnsignedLong( lEnumProp->Value() ) );
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
                    SET_CURRENT_VALUES( lBitProp, lValues, "current", PyLong_FromUnsignedLong );
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
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );
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
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );
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
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );
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
            std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );

            if( LeddarCore::LdBufferProperty *p = dynamic_cast< LeddarCore::LdBufferProperty * >( self->mSensor->GetProperties()->GetProperty( LeddarCore::LdPropertyIds::ID_IP_ADDRESS ) ) )
            {
                auto lVal = p->GetValue( 0 );
                lIPConfig = *( (uint32_t *)lVal.data() );
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

        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );

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
    return PyLong_FromUnsignedLong( self->mDataMask );
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
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );

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
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );

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
                if( !listObj )\
                    throw std::logic_error( "Unable to allocate memory for Python list" );\
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
    if( !lStates )
        return nullptr;

    using namespace LeddarCore::LdPropertyIds;

    ADD_PROPERTY( "timestamp", ID_RS_TIMESTAMP, LeddarCore::LdIntegerProperty,  PyLong_FromUnsignedLong );
    ADD_PROPERTY( "timestamp64", ID_RS_TIMESTAMP64, LeddarCore::LdIntegerProperty,  PyLong_FromUnsignedLongLong );
    ADD_PROPERTY( "system_temp", ID_RS_SYSTEM_TEMP, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "predict_temp", ID_RS_PREDICT_TEMP, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "cpu_load", ID_RS_CPU_LOAD, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "discrete_outputs", ID_RS_DISCRETE_OUTPUTS, LeddarCore::LdIntegerProperty,  PyLong_FromUnsignedLong );
    ADD_PROPERTY( "acq_current_params", ID_RS_ACQ_CURRENT_PARAMS, LeddarCore::LdIntegerProperty,  PyLong_FromUnsignedLong );
    ADD_PROPERTY( "apd_temp", ID_RS_APD_TEMP, LeddarCore::LdFloatProperty,  PyFloat_FromDouble );
    ADD_PROPERTY( "backup", ID_RS_BACKUP, LeddarCore::LdIntegerProperty,  PyLong_FromUnsignedLong );
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
/// \exception  std::runtime_error  Raised when there is no new data
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 int/size_t: (optional) number of retry
///                 int: (optional) time between retry (ms)
///
/// \return A dict with data see ­\ref PackageEchoes
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
            LeddarUtils::LtTimeUtils::Wait( lMsBetweenRetries );
            throw std::runtime_error( "No new echoes available!" );
        }

        return PackageEchoes( self->mSensor );
    }, lNRetries );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *PackageEchoes( LeddarDevice::LdSensor *aSensor )
///
/// \brief  Package echoes
///
/// \exception  std::logic_error    Raised when unable to allocate memory for Python list.
///
/// \param [in,out] aSensor   Pointer to the sensor.
///
/// \return A dict with keys: timestamp, distance_scale, amplitude_scale, led_power, v_fov, h_fov, v, h, data
///
/// \author David Levy, Maxime Lemonnier
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push,1)

struct LeddarPyEcho
{
    uint32_t index;
    float distance;
    float amplitude;
    uint64_t timestamp;
    uint16_t flag;
    float x;
    float y;
    float z;
};
#pragma pack(pop)

PyObject *PackageEchoes( LeddarDevice::LdSensor *aSensor )
{
    std::vector<LeddarConnection::LdEcho> &lEchoes = *( aSensor->GetResultEchoes()->GetEchoes() );
    npy_intp dimsIndices = aSensor->GetResultEchoes()->GetEchoCount();
    PyObject *lEchoesDict = PyDict_New();

    if( !lEchoesDict )
        throw std::logic_error( "Unable to allocate memory for Python list" );

    auto *lTS64 = dynamic_cast<const LeddarCore::LdIntegerProperty*>(aSensor->GetResultEchoes()->GetProperties()->FindProperty(LeddarCore::LdPropertyIds::ID_RS_TIMESTAMP64));
    auto *lCurrentLedPower = dynamic_cast<const LeddarCore::LdIntegerProperty*>(aSensor->GetResultEchoes()->GetProperties()->FindProperty(LeddarCore::LdPropertyIds::ID_CURRENT_LED_INTENSITY));
    PyDict_SetItemString( lEchoesDict, "timestamp", PyLong_FromUnsignedLongLong( lTS64 != nullptr && lTS64->ValueT<uint64_t>() != 0 ? lTS64->ValueT<uint64_t>() :
                          aSensor->GetResultEchoes()->GetTimestamp() ) );
    PyDict_SetItemString( lEchoesDict, "distance_scale", PyLong_FromUnsignedLong( aSensor->GetResultEchoes()->GetDistanceScale() ) );
    PyDict_SetItemString( lEchoesDict, "amplitude_scale", PyLong_FromUnsignedLong( aSensor->GetResultEchoes()->GetAmplitudeScale() ) );
    if( lCurrentLedPower )
        PyDict_SetItemString( lEchoesDict, "led_power", PyLong_FromUnsignedLong( lCurrentLedPower->Value() ) );
    PyDict_SetItemString( lEchoesDict, "v_fov", PyFloat_FromDouble( aSensor->GetResultEchoes()->GetVFOV() ) );
    PyDict_SetItemString( lEchoesDict, "h_fov", PyFloat_FromDouble( aSensor->GetResultEchoes()->GetHFOV() ) );
    PyDict_SetItemString( lEchoesDict, "v", PyLong_FromUnsignedLong( aSensor->GetResultEchoes()->GetVChan() ) );
    PyDict_SetItemString( lEchoesDict, "h", PyLong_FromUnsignedLong( aSensor->GetResultEchoes()->GetHChan() ) );


    npy_intp dims[1] = {dimsIndices};

    PyObject *op = Py_BuildValue( "[(s, s), (s, s), (s, s), (s, s), (s, s), (s, s), (s, s), (s, s)]"
                                  , "indices", "u4"
                                  , "distances", "f4"
                                  , "amplitudes", "f4"
                                  , "timestamps", "u8"
                                  , "flags", "u2"
                                  , "x", "f4"
                                  , "y", "f4"
                                  , "z", "f4" );
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
        ech_ptr->distance = float( lEchoes[i].mDistance ) / aSensor->GetResultEchoes()->GetDistanceScale();
        ech_ptr->amplitude = float( lEchoes[i].mAmplitude ) / aSensor->GetResultEchoes()->GetAmplitudeScale();
        ech_ptr->timestamp = lEchoes[i].mTimestamp;
        ech_ptr->flag = uint16_t( lEchoes[i].mFlag );
        ech_ptr->x = lEchoes[i].mX;
        ech_ptr->y = lEchoes[i].mY;
        ech_ptr->z = lEchoes[i].mZ;
    }

    return lEchoesDict;
}


bool DataThreadIsStopped( sSharedDataBase &shared )
{
    std::lock_guard<std::mutex> lock( shared.mMutex );

    if( !shared.mStop )
    {
        PyErr_SetString( PyExc_RuntimeError, "Operation refused, DataThread is running." );
    }

    return shared.mStop;
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

    if( !DataThreadIsStopped( self->mDataThreadSharedData ) || !PyArg_ParseTuple( args, "O", &lCallback ) )
        return nullptr;

    if( self->mDataThreadSharedData.mCallBackState != nullptr )
    {
        Py_DECREF( self->mDataThreadSharedData.mCallBackState );
    }

    Py_INCREF( lCallback );

    self->mDataThreadSharedData.mCallBackState = lCallback;

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

    if( !DataThreadIsStopped( self->mDataThreadSharedData ) || !PyArg_ParseTuple( args, "O", &lCallback ) )
        return nullptr;

    if( self->mDataThreadSharedData.mCallBackEcho != nullptr )
    {
        Py_DECREF( self->mDataThreadSharedData.mCallBackEcho );
    }

    Py_INCREF( lCallback );

    self->mDataThreadSharedData.mCallBackEcho = lCallback;

    Py_RETURN_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn PyObject *SetCallBackException( sLeddarDevice *self, PyObject *args )
///
/// \brief  Sets call back when an exception is raised from the data thread
///
/// \param [in,out] self    If non-null, the class instance that this method operates on.
/// \param [in,out] args    If non-null, the arguments.
///                 python object: the python callback function
///
/// \returns    True on success
///
/// \author David Lévy
/// \date   March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *SetCallBackException( sLeddarDevice *self, PyObject *args )
{
    PyObject *lCallback = nullptr;

    if( !DataThreadIsStopped( self->mDataThreadSharedData ) || !PyArg_ParseTuple( args, "O", &lCallback ) )
        return nullptr;

    if( self->mDataThreadSharedData.mCallBackException != nullptr )
    {
        Py_DECREF( self->mDataThreadSharedData.mCallBackException );
    }

    Py_INCREF( lCallback );

    self->mDataThreadSharedData.mCallBackException = lCallback;

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
    CallBackManger lCallBackManager( self );

    auto lLastPing = std::chrono::system_clock::now();

    auto lExceptionHandle = [self]( std::exception_ptr aExcept ) {
        if( self->mDataThreadSharedData.mCallBackException )
        {
            PyGILState_STATE gstate;
            // Thread safe python call
            gstate = PyGILState_Ensure();

            if( PyObject *o = PyDict_New() )
            {
                try
                {
                    if( aExcept )
                    {
                        std::rethrow_exception( aExcept );
                    }
                }
                catch( const LeddarException::LtComException &e )
                {
                    if(e.GetDisconnect())
                    {
                        if( self->mDataThreadSharedData.mGetDataLocked )
                        {
                            self->mDataThreadSharedData.mMutex.unlock(); // if no new data is found, or a exception was thrown, CallBackManager could not unlock the mutex
                            self->mDataThreadSharedData.mGetDataLocked = false;
                        }
                        self->mDataThreadSharedData.mStop = true;
                    }
                    PyDict_SetItemString( o, "what", PyUnicode_FromString( e.what() ) );
                    PyDict_SetItemString( o, "disconnect", PyBool_FromLong( e.GetDisconnect() ) );
                }
                catch( const std::exception &e )
                {
                    PyDict_SetItemString( o, "what", PyUnicode_FromString( e.what() ) );
                }
                catch( ... )
                {
                    if( self->mDataThreadSharedData.mGetDataLocked )
                    {
                        self->mDataThreadSharedData.mMutex.unlock(); // if no new data is found, or a exception was thrown, CallBackManager could not unlock the mutex
                        self->mDataThreadSharedData.mGetDataLocked = false;
                    }
                    self->mDataThreadSharedData.mStop = true;
                    PyDict_SetItemString( o, "what", PyUnicode_FromString( "Unknown error in data thread (GetData())" ) );
                    PyDict_SetItemString( o, "disconnect", PyBool_FromLong( true ) );
                }
                PyObject_CallFunctionObjArgs( self->mDataThreadSharedData.mCallBackException, o, NULL );
                Py_DECREF( o );
            }

            // Release the python thread. No Python API allowed beyond this point.
            PyGILState_Release( gstate );
        }
    };

    while( true )
    {

        try
        {
            uint32_t lDelay = 0;
            {
                std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );
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
                self->mDataThreadSharedData.mMutex.lock(); //we can't use a std::lock_guard here, since we have to free the lock BEFORE acquiring GIL
                self->mDataThreadSharedData.mGetDataLocked = true;
                lNewData = self->mSensor->GetData(); //mutex will be handled by CallBackManager
            }
            catch( ... )
            {
                lExceptionHandle( std::current_exception() );
                continue;
            }

            if( self->mDataThreadSharedData.mGetDataLocked )
            {
                self->mDataThreadSharedData.mMutex.unlock(); // if no new data is found, or a exception was thrown, CallBackManager could not unlock the mutex
                self->mDataThreadSharedData.mGetDataLocked = false;
            }


            LeddarUtils::LtTimeUtils::WaitBlockingMicro( lDelay );

            if( LeddarDevice::LdSensorLeddarAuto *lAutoSensor = dynamic_cast<LeddarDevice::LdSensorLeddarAuto *>( self->mSensor ) )
            {
                auto lNow = std::chrono::system_clock::now();
                std::chrono::duration<double> diff = lNow - lLastPing;

                if( diff.count() > 1.0 ) //every 2s
                {
                    lLastPing = lNow;
                    DebugTrace( "Sending Ping..." );

                    try
                    {
                        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );

                        if( LeddarDevice::LdSensorPixell *lPixell = dynamic_cast<LeddarDevice::LdSensorPixell *>( self->mSensor ) )
                        {
                            lPixell->GetStatus();
                        }
                        else
                        {
                            lAutoSensor->SendPing();
                        }
                    }
                    catch( ... )
                    {
                        lExceptionHandle( std::current_exception() );
                    }
                }
            }
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

    Py_BEGIN_ALLOW_THREADS;

    {
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );

        if( self->mDataThreadSharedData.mStop )
        {
            self->mDataThreadSharedData.mStop = false;
            DebugTrace( "Starting DataThread" );
            self->mDataThreadSharedData.mThread = std::thread( DataThread, self );
        }
        else
            DebugTrace( "DataThread already running, nothing to do." );
    }



    Py_END_ALLOW_THREADS;

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
    Py_BEGIN_ALLOW_THREADS;

    {
        DebugTrace( "Obtaining mMutex" );
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );
        self->mDataThreadSharedData.mStop = true;
    }

    if( self->mDataThreadSharedData.mThread.joinable() )
    {
        DebugTrace( "Joining DataThread" );
        self->mDataThreadSharedData.mThread.join();
        DebugTrace( "DataThread joined" );
    }


    Py_END_ALLOW_THREADS;

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
        std::lock_guard<std::mutex> lock( self->mDataThreadSharedData.mMutex );
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
