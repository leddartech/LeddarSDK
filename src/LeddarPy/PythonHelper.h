// *****************************************************************************
// Module..: LeddarPy
//
/// \file    PythonHelper.h
///
/// \brief   Small functions to convert c data to python data type
///
/// \author  David Levy
///
/// \since   November 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include <Python.h>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "LdConnectionInfo.h"

extern bool gDebug;

static PyObject *VectorToList_Float( const std::vector<float> &data )
{
    PyObject *listObj = PyList_New( data.size() );

    if( !listObj )
        throw std::logic_error( "Unable to allocate memory for Python list" );

    for( unsigned int i = 0; i < data.size(); i++ )
    {
        PyObject *num = PyFloat_FromDouble( ( double )data[i] );

        if( !num )
        {
            Py_DECREF( listObj );
            throw std::logic_error( "Unable to allocate memory for Python list" );
        }

        PyList_SetItem( listObj, i, num );
    }

    return listObj;
}

static PyObject *VectorToList_Long( const std::vector<long> &data )
{
    PyObject *listObj = PyList_New( data.size() );

    if( !listObj )
        throw std::logic_error( "Unable to allocate memory for Python list" );

    for( unsigned int i = 0; i < data.size(); i++ )
    {
        PyObject *num = PyLong_FromLong( data[i] );

        if( !num )
        {
            Py_DECREF( listObj );
            throw std::logic_error( "Unable to allocate memory for Python list" );
        }

        PyList_SetItem( listObj, i, num );
    }

    return listObj;
}

static PyObject *VectorToList_String( const std::vector<std::string> &data )
{
    PyObject *listObj = PyList_New( data.size() );

    if( !listObj )
        throw std::logic_error( "Unable to allocate memory for Python string list" );

    for( unsigned int i = 0; i < data.size(); i++ )
    {
        PyObject *string = PyUnicode_FromString( data[i].c_str() );

        if( !string )
        {
            Py_DECREF( listObj );
            throw std::logic_error( "Unable to allocate memory for Python string list" );
        }

        PyList_SetItem( listObj, i, string );
    }

    return listObj;
}

static PyObject *ArrayToList_float( const double *aArray, const unsigned int aSize )
{
    PyObject *listObj = PyList_New( aSize );

    if( !listObj )
        throw std::logic_error( "Unable to allocate memory for Python list" );

    for( unsigned int i = 0; i < aSize; i++ )
    {
        PyObject *num = PyFloat_FromDouble( ( double )aArray[i] );

        if( !num )
        {
            Py_DECREF( listObj );
            throw std::logic_error( "Unable to allocate memory for Python list" );
        }

        PyList_SetItem( listObj, i, num );
    }

    return listObj;
}

static void DebugTrace( std::string aString )
{
    if( gDebug )
        std::cout << "DEBUG: " << aString << std::endl;
}

// *****************************************************************************
// Function: deleteAllButOneConnections
//
/// \brief   Delete all (but one) pointers stored in vector after a GetDeviceList()
///
/// param[in] Vector returned by GetDeviceList
/// param[in] Optional : Index of the pointer NOT to delete
///
// *****************************************************************************
static void deleteAllButOneConnections( std::vector<LeddarConnection::LdConnectionInfo *> &aConnections, size_t aButOne = -1 )
{
    for( size_t i = 0; i < aConnections.size(); ++i )
    {
        if( i != aButOne )
            delete aConnections[i];
    }
}
