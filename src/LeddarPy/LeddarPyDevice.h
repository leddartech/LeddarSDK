// *****************************************************************************
// Module..: LeddarPy
//
/// \file    LeddarPyDevice.h
///
/// \brief   Definitions of Device object used in LeddarPy module
///
/// \author  David Levy
///
/// \since   November 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include <Python.h>
#include "structmember.h"

#include <stdint.h>
#include <thread>
#include <mutex>

//Forward declaration
namespace LeddarDevice
{
    class LdSensor;
}

namespace LeddarConnection
{
    class LdResultEchoes;
    class LdResultStates;
}

namespace LeddarRecord
{
    class LdRecorder;
}

struct sSharedDataBase
{
    std::mutex mMutex;
    std::thread mThread;
    bool mStop = true; // Request to stop thread
};
struct sSharedData : public sSharedDataBase
{
    uint32_t mDelay = 5000u;        // Delay between two request (in microseconds)
    PyObject *mCallBackState = nullptr;
    PyObject *mCallBackEcho = nullptr;
    bool mGetDataLocked = false;                //reserved for use of DataThread()
};


typedef struct sLeddarDevice
{
    PyObject_HEAD

    LeddarDevice::LdSensor *mSensor = nullptr;    //Pointer to the sensor
    LeddarRecord::LdRecorder *mRecorder = nullptr;
    bool mStream = false;                       //if the sensor is streaming (M16 and LCA3 UDP) we need to set data mask and start thread
    uint32_t mDataMask = 0;                 //Current Datamask requested by user

    sSharedData mDataThreadSharedData;  //Data shared between thread. Need to use mutex to read / write

    size_t v, h;
    float v_fov, h_fov;
    std::string mIP;
} sLeddarDevice;


//List all member functions
void Device_dealloc( sLeddarDevice *self );
PyObject *Device_new( PyTypeObject *type, PyObject *args, PyObject *kwds );
int Device_init( PyObject *self, PyObject *args, PyObject *kwds );
PyObject *Connect( sLeddarDevice *self, PyObject *args );
PyObject *Disconnect( sLeddarDevice *self, PyObject *args );

PyObject *GetPropertiesSnapshot( sLeddarDevice *self );
PyObject *GetPropertyValue( sLeddarDevice *self, PyObject *args );
PyObject *GetPropertyCount( sLeddarDevice *self, PyObject *args );
PyObject *GetPropertyAvailableValues( sLeddarDevice *self, PyObject *args );
PyObject *SetPropertyValue( sLeddarDevice *self, PyObject *args );
PyObject *SetAccumulationExponent( sLeddarDevice *self, PyObject *args );
PyObject *SetOversamplingExponent( sLeddarDevice *self, PyObject *args );
PyObject *SetIPConfig( sLeddarDevice *self, PyObject *args );
PyObject *GetIPConfig( sLeddarDevice *self, PyObject *args );
PyObject *GetDataMask( sLeddarDevice *self, PyObject *args );
PyObject *SetDataMask( sLeddarDevice *self, PyObject *args );
PyObject *GetStates( sLeddarDevice *self, PyObject *args );
PyObject *GetEchoes( sLeddarDevice *self, PyObject *args );
PyObject *GetCalibValues( sLeddarDevice *self, PyObject *args );

PyObject *SetCallBackState( sLeddarDevice *self, PyObject *args );
PyObject *SetCallBackEcho( sLeddarDevice *self, PyObject *args );
PyObject *StartDataThread( sLeddarDevice *self, PyObject *args );
PyObject *StopDataThread( sLeddarDevice *self, PyObject *args );
PyObject *SetDataThreadDelay( sLeddarDevice *self, PyObject *args );

PyObject *PackageEchoes( LeddarDevice::LdSensor *aResultEchoes );
PyObject *PackageStates( LeddarConnection::LdResultStates *aResultStatess );
PyObject *StartStopRecording( sLeddarDevice *self, PyObject *args );

//Python Member function list
static PyMethodDef Device_methods[] =
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
//Object declaration for python
static PyTypeObject LeddarDeviceType =
{
    PyVarObject_HEAD_INIT( NULL, 0 )
    "leddar.Device",              //tp_name
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
    "Device object.",               //tp_doc
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
