// *****************************************************************************
// Module..: LeddarPy
//
/// \file    LeddarPy.h
///
/// \brief   Wrapper python for the SDK
///
/// \author  Maxime Lemonnier
///
/// \since   January 2019
//
// Copyright (c) 2019 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once


#include <Python.h>

PyObject *GetDeviceTypeDict( PyObject *self, PyObject *args );

PyObject *GetPropertyIdDict( PyObject *self, PyObject *args );

PyObject *GetMaskDict( PyObject *self, PyObject *args );

