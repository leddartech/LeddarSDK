// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnectionInfo.cpp
///
/// \brief   Interface of the connection info retreived by the device list.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdConnectionInfo.h"


// *****************************************************************************
// Function: LdConnectionInfo::LdConnectionInfo
//
/// \brief   Constructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarConnection::LdConnectionInfo::LdConnectionInfo( eConnectionType aConnectionType, const std::string &aDisplayName ) :
    mDisplayName( aDisplayName ),
    mAddress( "" ),
    mType( aConnectionType )
{
}

// *****************************************************************************
// Function: LdConnectionInfo::~LdConnectionInfo
//
/// \brief   Destructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarConnection::LdConnectionInfo::~LdConnectionInfo()
{
}
