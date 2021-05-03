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
    PyObject *mCallBackException = nullptr;
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
} sLeddarDevice;


//List all member functions
void Device_dealloc( sLeddarDevice *self );
PyObject *Device_new( PyTypeObject *type, PyObject *args, PyObject *kwds );
int Device_init( PyObject *self, PyObject *args, PyObject *kwds );
int Device_traverse( PyObject *self, visitproc visit, void *arg );
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
PyObject *SetCallBackException( sLeddarDevice *self, PyObject *args );
PyObject *StartDataThread( sLeddarDevice *self, PyObject *args );
PyObject *StopDataThread( sLeddarDevice *self, PyObject *args );
PyObject *SetDataThreadDelay( sLeddarDevice *self, PyObject *args );

PyObject *PackageEchoes( LeddarDevice::LdSensor *aResultEchoes );
PyObject *PackageStates( LeddarConnection::LdResultStates *aResultStatess );
PyObject *StartStopRecording( sLeddarDevice *self, PyObject *args );

PyTypeObject *InitDeviceType();
