// *****************************************************************************
// Module..: Leddar
//
/// \file    LdResultProvider.h
///
/// \brief   Base class of results.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdResultProvider.h"
#include "LdPropertyIds.h"

// *****************************************************************************
// Function: LdResultProvider::LdResultProvider
//
/// \brief   Constructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************
LeddarConnection::LdResultProvider::LdResultProvider()
{
    mTimestamp = new LeddarCore::LdIntegerProperty( LeddarCore::LdProperty::CAT_INFO, LeddarCore::LdProperty::F_NONE, LeddarCore::LdPropertyIds::ID_RS_TIMESTAMP, 0, 4, "Timestamp" );
    mTimestamp->ForceValue( 0, 0 );
    mProperties.AddProperty( mTimestamp );
}

