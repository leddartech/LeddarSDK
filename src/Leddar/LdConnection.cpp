// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnection.cpp
///
/// \brief   Base class of LdConnection
///
/// \author  Patrick Boulay
///
/// \since   February 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdConnection.h"

#include <cstring>

// *****************************************************************************
// Function: LdConnection::LdConnection
//
/// \brief   Constructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarConnection::LdConnection::LdConnection( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface ) :
    mConnectionInfo( aConnectionInfo ),
    mDeviceType( 0 ),
    mInterface( aInterface ),
    mTransferInputBuffer( nullptr ),
    mTransferOutputBuffer( nullptr ),
    mTransferBufferSize( 0 ),
    mOwner( false )
{
}

// *****************************************************************************
// Function: LdConnection::~LdConnection
//
/// \brief   Destructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarConnection::LdConnection::~LdConnection()
{
    if( mOwner )
    {
        delete mInterface;
        mInterface = nullptr;
        delete mConnectionInfo;
        mConnectionInfo = nullptr;
    }

    if( mTransferInputBuffer != nullptr )
    {
        delete[] mTransferInputBuffer;
        mTransferInputBuffer = nullptr;
    }

    if( mTransferOutputBuffer != nullptr )
    {
        delete[] mTransferOutputBuffer;
        mTransferOutputBuffer = nullptr;
    }
}

// *****************************************************************************
// Function: LeddarConnection::ResizeInternalBuffers
//
/// \brief   Resize the internal buffers.
///
/// \param  aSize Size of the buffer to receive.
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

void
LeddarConnection::LdConnection::ResizeInternalBuffers( const uint32_t &aSize )
{
    uint8_t *mTransferInputBufferTemp = new uint8_t[ aSize ];
    uint8_t *mTransferOutputBufferTemp = new uint8_t[ aSize ];
    memcpy( mTransferInputBufferTemp, mTransferInputBuffer, ( aSize > mTransferBufferSize ? mTransferBufferSize : aSize ) );
    memcpy( mTransferOutputBufferTemp, mTransferOutputBuffer, ( aSize > mTransferBufferSize ? mTransferBufferSize : aSize ) );
    delete[] mTransferInputBuffer;
    delete[] mTransferOutputBuffer;
    mTransferInputBuffer = mTransferInputBufferTemp;
    mTransferOutputBuffer = mTransferOutputBufferTemp;
    mTransferBufferSize = aSize;
}
