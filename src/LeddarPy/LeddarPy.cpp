// *****************************************************************************
// Module..: LeddarPy
//
/// \file    LeddarPy.cpp
///
/// \brief   Wrapper python for the SDK
///
/// \author  David Levy
///
/// \since   November 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************


#include "LeddarPy.h"
#include "LeddarPyDevice.h"
#include "PythonHelper.h"
#include "Connecters.h"
#include "LdLibModbusSerial.h"
#include "LdSpiFTDI.h"
#include "LdLibUsb.h"
#include "LdEthernet.h"

#include "LtComLeddarTechPublic.h"
#include "LtComLeddarTechPublic.h"
#include "LdPropertyIds.h"
#include "LtStringUtils.h"
#include "LtTimeUtils.h"
#include "LtIntUtilities.h"
#include "LtExceptions.h"

#include "LdSensor.h"

#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define PY_ARRAY_UNIQUE_SYMBOL leddar_ARRAY_API
#include <numpy/arrayobject.h>

#include <algorithm>
#include <vector>

bool gDebug = false;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn static PyObject *EnableDebugTrace( PyObject *self, PyObject *args )
///
/// \brief  Enables the debug trace
///
/// \param [in,out] self    The class instance that this method operates on.
/// \param [in,out] args    The arguments: (bool) Enable / disable
///
/// \return True on success.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
static PyObject *EnableDebugTrace( PyObject *self, PyObject *args )
{
    int enable = false;

    if( !PyArg_ParseTuple( args, "i", &enable ) )
        return nullptr;

    gDebug = ( enable != 0 );
    Py_RETURN_TRUE;
}



PyObject *GetDeviceTypeDict( PyObject *self, PyObject *args )
{
    PyObject *lDeviceType = PyDict_New();
    if( !lDeviceType )
        return nullptr;

    PyDict_SetItemString( lDeviceType, "M16", PyLong_FromLong( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16 ) );
    PyDict_SetItemString( lDeviceType, "LeddarOne", PyLong_FromLong( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SCH_EVALKIT ) );
    PyDict_SetItemString( lDeviceType, "Vu8", PyLong_FromLong( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VU8 ) );
    PyDict_SetItemString( lDeviceType, "M16Laser", PyLong_FromLong( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_LASER ) );
    PyDict_SetItemString( lDeviceType, "Vu8Komodo", PyLong_FromLong( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VU8 | CONNECTION_TYPE_CAN_KOMODO ) );
    PyDict_SetItemString( lDeviceType, "M16Komodo", PyLong_FromLong( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16 | CONNECTION_TYPE_CAN_KOMODO ) );

    PyDict_SetItemString( lDeviceType, "Ethernet", PyLong_FromLong( CONNECTION_TYPE_ETHERNET ) );
    PyDict_SetItemString( lDeviceType, "Serial", PyLong_FromLong( CONNECTION_TYPE_SERIAL ) );
    PyDict_SetItemString( lDeviceType, "Usb", PyLong_FromLong( CONNECTION_TYPE_USB ) );
    PyDict_SetItemString( lDeviceType, "SpiFTDI", PyLong_FromLong( CONNECTION_TYPE_SPI_FTDI ) );
    PyDict_SetItemString( lDeviceType, "CanKomodo", PyLong_FromLong( CONNECTION_TYPE_CAN_KOMODO ) );
    return lDeviceType;
}

PyObject *GetPropertyIdDict( PyObject *self, PyObject *args )
{
    PyObject *lPropertyId = PyDict_New();
    if( !lPropertyId )
        return nullptr;

    //Config
    PyDict_SetItemString( lPropertyId, "ID_ACCUMULATION_EXP", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP ) );
    PyDict_SetItemString( lPropertyId, "ID_OVERSAMPLING_EXP", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP ) );
    PyDict_SetItemString( lPropertyId, "ID_PRECISION", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_PRECISION ) );
    PyDict_SetItemString( lPropertyId, "ID_PRECISION_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_PRECISION_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_LED_INTENSITY", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_LED_INTENSITY ) );
    PyDict_SetItemString( lPropertyId, "ID_BASE_POINT_COUNT", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT ) );


    PyDict_SetItemString( lPropertyId, "ID_SATURATION_COMP_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_SATURATION_COMP_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_OVERSHOOT_MNG_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_OVERSHOOT_MNG_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_DEMERGING_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_DEMERGING_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_STATIC_NOISE_REMOVAL_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_STATIC_NOISE_REMOVAL_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_OVERSHOOT_MNG_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_OVERSHOOT_MNG_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_LED_AUTO_PWR_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_LED_AUTO_PWR_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_LED_AUTO_FRAME_AVG", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_LED_AUTO_FRAME_AVG ) );
    PyDict_SetItemString( lPropertyId, "ID_LED_AUTO_ECHO_AVG", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_LED_AUTO_ECHO_AVG ) );
    PyDict_SetItemString( lPropertyId, "ID_SATURATION_COMP_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_SATURATION_COMP_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_SATURATION_COMP_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_SATURATION_COMP_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_SEGMENT_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE ) );


    PyDict_SetItemString( lPropertyId, "ID_REF_PULSE_RATE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_REF_PULSE_RATE ) );
    PyDict_SetItemString( lPropertyId, "ID_ORIGIN_X", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_ORIGIN_X ) );
    PyDict_SetItemString( lPropertyId, "ID_ORIGIN_Y", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_ORIGIN_Y ) );
    PyDict_SetItemString( lPropertyId, "ID_ORIGIN_Z", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_ORIGIN_Z ) );
    PyDict_SetItemString( lPropertyId, "ID_YAW", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_YAW ) );
    PyDict_SetItemString( lPropertyId, "ID_PITCH", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_PITCH ) );
    PyDict_SetItemString( lPropertyId, "ID_ROLL", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_ROLL ) );

    //Constants
    PyDict_SetItemString( lPropertyId, "ID_PRECISION_LIMITS", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_PRECISION_LIMITS ) );
    PyDict_SetItemString( lPropertyId, "ID_DEVICE_TYPE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE ) );
    PyDict_SetItemString( lPropertyId, "ID_VERTICAL_CHANNEL_NBR", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_VSEGMENT ) );
    PyDict_SetItemString( lPropertyId, "ID_HORIZONTAL_CHANNEL_NBR", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_HSEGMENT ) );
    PyDict_SetItemString( lPropertyId, "ID_HFOV", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_HFOV ) );
    PyDict_SetItemString( lPropertyId, "ID_VFOV", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_VFOV ) );


    PyDict_SetItemString( lPropertyId, "ID_DEVICE_NAME", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_DEVICE_NAME ) );
    PyDict_SetItemString( lPropertyId, "ID_PART_NUMBER", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_PART_NUMBER ) );
    PyDict_SetItemString( lPropertyId, "ID_SOFTWARE_PART_NUMBER", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_SOFTWARE_PART_NUMBER ) );
    PyDict_SetItemString( lPropertyId, "ID_SERIAL_NUMBER", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_SERIAL_NUMBER ) );
    PyDict_SetItemString( lPropertyId, "ID_FIRMWARE_VERSION_INT", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_FIRMWARE_VERSION_INT ) );
    PyDict_SetItemString( lPropertyId, "ID_FPGA_VERSION", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_FPGA_VERSION ) );
    PyDict_SetItemString( lPropertyId, "ID_GROUP_ID_NUMBER", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_GROUP_ID_NUMBER ) );
    PyDict_SetItemString( lPropertyId, "ID_LED_AUTO_FRAME_AVG_LIMITS", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_LED_AUTO_FRAME_AVG_LIMITS ) );
    PyDict_SetItemString( lPropertyId, "ID_MAC_ADDRESS", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_MAC_ADDRESS ) );
    PyDict_SetItemString( lPropertyId, "ID_OPTIONS", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_OPTIONS ) );
    PyDict_SetItemString( lPropertyId, "ID_BASE_SAMPLE_DISTANCE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_BASE_SAMPLE_DISTANCE ) );
    PyDict_SetItemString( lPropertyId, "ID_MAX_ECHOES_PER_CHANNEL", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL ) );
    PyDict_SetItemString( lPropertyId, "ID_DISTANCE_SCALE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE ) );
    PyDict_SetItemString( lPropertyId, "ID_ECHO_AMPLITUDE_MAX", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_ECHO_AMPLITUDE_MAX ) );
    PyDict_SetItemString( lPropertyId, "ID_RAW_AMP_SCALE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_RAW_AMP_SCALE ) );
    PyDict_SetItemString( lPropertyId, "ID_FILTERED_AMP_SCALE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_FILTERED_AMP_SCALE ) );
    PyDict_SetItemString( lPropertyId, "ID_TEMPERATURE_SCALE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_TEMPERATURE_SCALE ) );
    PyDict_SetItemString( lPropertyId, "ID_SENSIVITY", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_SENSIVITY ) );
    PyDict_SetItemString( lPropertyId, "ID_IP_ADDRESS", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_IP_ADDRESS ) );
    PyDict_SetItemString( lPropertyId, "ID_IP_MODE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_IP_MODE ) );
    PyDict_SetItemString( lPropertyId, "ID_DATA_SERVER_PORT", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_DATA_SERVER_PORT ) );
    PyDict_SetItemString( lPropertyId, "ID_DATA_SERVER_PROTOCOL", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_DATA_SERVER_PROTOCOL ) );
    PyDict_SetItemString( lPropertyId, "ID_BUFFER_SIZE_TCP", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_BUFFER_SIZE_TCP ) );
    PyDict_SetItemString( lPropertyId, "ID_BUFFER_SIZE_UDP", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_BUFFER_SIZE_UDP ) );
    PyDict_SetItemString( lPropertyId, "ID_LICENSE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_LICENSE ) );
    PyDict_SetItemString( lPropertyId, "ID_LICENSE_INFO", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_LICENSE_INFO ) );
    PyDict_SetItemString( lPropertyId, "ID_PULSE_RATE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_PULSE_RATE ) );
    PyDict_SetItemString( lPropertyId, "ID_ACC_DIST_EXP", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_ACC_DIST_EXP ) );
    PyDict_SetItemString( lPropertyId, "ID_XTALK_REMOVAL_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_XTALK_REMOVAL_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_XTALK_OPTIC_SEG_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_XTALK_OPTIC_SEG_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_XTALK_OPTIC_LINE_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_XTALK_OPTIC_LINE_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_XTALK_OPTIC_ECH_SEG_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_XTALK_OPTIC_ECH_SEG_ENABLE ) );
    PyDict_SetItemString( lPropertyId, "ID_XTALK_OPTIC_ECH_LINE_ENABLE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_XTALK_OPTIC_ECH_LINE_ENABLE ) );

    PyDict_SetItemString( lPropertyId, "ID_SYSTEM_TIME", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_SYSTEM_TIME ) );
    PyDict_SetItemString( lPropertyId, "ID_SYNCHRONIZATION", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_SYNCHRONIZATION ) );



    return lPropertyId;
}

PyObject *GetProtocolTypeDict( PyObject *self, PyObject *args )
{
    PyObject *lCom = PyDict_New();

    //Comm
    PyDict_SetItemString( lCom, "LT_COMM_PROTOCOL_INVALID", PyLong_FromLong( LtComLeddarTechPublic::LT_COMM_PROTOCOL_INVALID ) );
    PyDict_SetItemString( lCom, "LT_COMM_PROTOCOL_TCP", PyLong_FromLong( LtComLeddarTechPublic::LT_COMM_PROTOCOL_TCP ) );
    PyDict_SetItemString( lCom, "LT_COMM_PROTOCOL_UDP", PyLong_FromLong( LtComLeddarTechPublic::LT_COMM_PROTOCOL_UDP ) );

    return lCom;
}

PyObject *GetMaskDict( PyObject *self, PyObject *args )
{
    PyObject *lMask = PyDict_New();
    if( !lMask )
        return nullptr;

    PyDict_SetItemString( lMask, "DM_NONE", PyLong_FromLong( LeddarDevice::LdSensor::DM_NONE ) );
    PyDict_SetItemString( lMask, "DM_STATES", PyLong_FromLong( LeddarDevice::LdSensor::DM_STATES ) );
    PyDict_SetItemString( lMask, "DM_ECHOES", PyLong_FromLong( LeddarDevice::LdSensor::DM_ECHOES ) );
    PyDict_SetItemString( lMask, "DM_ALL", PyLong_FromLong( LeddarDevice::LdSensor::DM_ALL ) );
    return lMask;
}


PyObject *GetCalibTypeDict( PyObject *self, PyObject *args )
{
    PyObject *lCalib = PyDict_New();
    if( !lCalib )
        return nullptr;

    PyDict_SetItemString( lCalib, "ID_TIMEBASE_DELAY", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_TIMEBASE_DELAY ) );
    PyDict_SetItemString( lCalib, "ID_STATIC_NOISE", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_STATIC_NOISE ) );
    PyDict_SetItemString( lCalib, "ID_INTENSITY_COMPENSATIONS", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_INTENSITY_COMPENSATIONS ) );
    PyDict_SetItemString( lCalib, "ID_CHANNEL_ANGLE_AZIMUT", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_CHANNEL_ANGLE_AZIMUT ) );
    PyDict_SetItemString( lCalib, "ID_CHANNEL_ANGLE_ELEVATION", PyLong_FromLong( LeddarCore::LdPropertyIds::ID_CHANNEL_ANGLE_ELEVATION ) );

    return lCalib;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn static PyObject *GetDevices( PyObject *self, PyObject *args )
///
/// \brief  Gets the devices from the specified interface
///
/// \param [in,out] self    The class instance that this method operates on.
/// \param [in,out] args    The argument: (string) Connection type to get device from. Use "" for all types
///
/// \return nullptr if it fails, else the device list.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
static PyObject *GetDevices( PyObject *self, PyObject *args )
{
    const char *type;

    if( !PyArg_ParseTuple( args, "s", &type ) )
        return nullptr;

    try
    {
        bool allTypes = strlen( type ) == 0;

        std::vector<LeddarConnection::LdConnectionInfo *> lConnectionsList;

        auto append = [&]( const std::vector<LeddarConnection::LdConnectionInfo *> &c )
        {
            lConnectionsList.insert( lConnectionsList.end(), c.begin(), c.end() );
        };

        if( allTypes || strcmp( type, "Serial" ) == 0 )
            append( LeddarConnection::LdLibModbusSerial::GetDeviceList() );
        else if( allTypes || strcmp( type, "SpiFTDI" ) == 0 )
            append( LeddarConnection::LdSpiFTDI::GetDeviceList() );
        else if( allTypes || strcmp( type, "Ethernet" ) == 0 )
            append( LeddarConnection::LdEthernet::GetDeviceList() );
        else if( allTypes || strcmp( type, "Usb" ) == 0 )
            append( LeddarConnection::LdLibUsb::GetDeviceList( 0x28F1, 0x0400 ) );
        else
            return nullptr;

        PyObject *lList = PyList_New( lConnectionsList.size() );
        if( !lList )
            return nullptr;

        for( size_t i = 0; i < lConnectionsList.size(); i++ )
        {
            LeddarConnection::LdConnectionInfo *c = lConnectionsList[i];
            PyObject *lConnection = PyDict_New();
            std::string lName;

            if( !lConnection )
            {
                Py_DECREF( lList );
                return nullptr;
            }

            switch( c->GetType() )
            {
                case LeddarConnection::LdConnectionInfo::CT_SPI_FTDI:
                case LeddarConnection::LdConnectionInfo::CT_LIB_MODBUS:
                    lName = c->GetDisplayName();
                    break;

                case LeddarConnection::LdConnectionInfo::CT_USB:
                    lName = dynamic_cast<LeddarConnection::LdConnectionInfoUsb *>( c )->GetSerialNumber();
                    break;

                case LeddarConnection::LdConnectionInfo::CT_ETHERNET_LEDDARTECH:
                    lName = dynamic_cast<LeddarConnection::LdConnectionInfoEthernet *>( c )->GetIP();
                    break;

                default:
                    throw std::logic_error( "Unreachable case" );
            }

            PyDict_SetItemString( lConnection, "name", PyUnicode_FromString( lName.c_str() ) );
            PyDict_SetItemString( lConnection, "type", PyLong_FromLong( c->GetType() ) );
            PyDict_SetItemString( lConnection, "address", PyUnicode_FromString( c->GetAddress().c_str() ) );

            PyList_SetItem( lList, i, lConnection );
        }

        deleteAllButOneConnections( lConnectionsList, -1 /*delete all*/ );

        return lList;
    }
    catch( const std::exception &e )
    {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
    }
    catch( ... )
    {
        PyErr_SetString( PyExc_RuntimeError, "unknown exception" );
    }

    return nullptr;

}


//List all functions available to Python
static PyMethodDef leddar_Methods[] =
{
    // The first property is the name exposed to python, the second is the C++ function name
    {
        "enable_debug_trace", EnableDebugTrace, METH_VARARGS, "Enable/disable debug traces\n"
        "param1: (bool) enable/disable"
    },
    {
        "get_devices", GetDevices, METH_VARARGS, "Lists devices\n"
        "param1: (string) the device type (Serial, SpiFTDI, Ethernet or Usb) - Case sensitive"
        "Returns: List of dicts containing 'name', 'type' and 'address' fields"
    },
    { nullptr } // Sentinel
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef leddar_module =
{
    PyModuleDef_HEAD_INIT,
    "leddar",                                 // Module name
    "Python wrapper for LeddarTech SDK",        // Module description
    -1,
    leddar_Methods,                            // Structure that defines the methods
    NULL,                /* m_reload */
    NULL,                /* m_traverse */
    NULL,                /* m_clear */
    NULL,                /* m_free */
};
#define CREATE_MODULE PyModule_Create( &leddar_module );
#define RETURN_VALUE(v) v
PyMODINIT_FUNC PyInit_leddar( void )
#else
#define CREATE_MODULE Py_InitModule3( "leddar", leddar_Methods, "Python wrapper for LeddarTech SDK." );
#define RETURN_VALUE(v) ;
PyMODINIT_FUNC initleddar( void )
#endif
{
    if( !PyEval_ThreadsInitialized() )
    {
        PyEval_InitThreads();
    }

    Py_Initialize();

    PyObject *lModule = CREATE_MODULE;


    if( lModule == nullptr )
        return RETURN_VALUE( nullptr );

    import_array();

    PyTypeObject *deviceType = InitDeviceType();
    if( !deviceType )
    {
        Py_DECREF( lModule );
        return RETURN_VALUE( nullptr );
    }

    PyModule_AddObject( lModule, "device_types", GetDeviceTypeDict( lModule, nullptr ) );
    PyModule_AddObject( lModule, "property_ids", GetPropertyIdDict( lModule, nullptr ) );
    PyModule_AddObject( lModule, "data_masks", GetMaskDict( lModule, nullptr ) );
    PyModule_AddObject( lModule, "calib_types", GetCalibTypeDict( lModule, nullptr ) );
    if( !PyType_HasFeature( deviceType, Py_TPFLAGS_HEAPTYPE ) )
        Py_INCREF( deviceType );
    PyModule_AddObject( lModule, "Device", ( PyObject * )deviceType );

    return RETURN_VALUE( lModule );
}

