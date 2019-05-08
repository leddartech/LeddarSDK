// *****************************************************************************
// Module..: Leddar
//
/// \file    LdResultStates.cpp
///
/// \brief   Echoes result.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdResultStates.h"
#include "LdProperty.h"
#include "LdPropertyIds.h"
#include "LtExceptions.h"

#include <sstream>

// *****************************************************************************
// Function: LdResultEchoes::LdResultEchoes
//
/// \brief   Constructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarConnection::LdResultStates::LdResultStates( void ) :
    mIsInitialized( false )
{
}

// *****************************************************************************
// Function: LdResultEchoes::LdResultEchoes
//
/// \brief   Destructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarConnection::LdResultStates::~LdResultStates( void )
{
}

// *****************************************************************************
// Function: LdResultEchoes::Init
//
/// \brief   Init the result state class
///
/// \author  Patrick Boulay
///
/// \since   August 2016
// *****************************************************************************

void
LeddarConnection::LdResultStates::Init( uint32_t aTemperatureScale, uint32_t aCpuLoadScale )
{
    LeddarCore::LdFloatProperty *lTemperature = dynamic_cast<LeddarCore::LdFloatProperty *>( mProperties.FindProperty( LeddarCore::LdPropertyIds::ID_RS_SYSTEM_TEMP ) );

    if( lTemperature != nullptr )
    {
        lTemperature->SetScale( aTemperatureScale );
        lTemperature->ForceValue( 0, 0.0 );
    }


    LeddarCore::LdFloatProperty *lCPULoad = dynamic_cast<LeddarCore::LdFloatProperty *>( mProperties.FindProperty( LeddarCore::LdPropertyIds::ID_RS_CPU_LOAD ) );

    if( lCPULoad != nullptr )
    {
        lCPULoad->SetScale( aCpuLoadScale );
        lCPULoad->ForceValue( 0, 0.0 );
    }

    mIsInitialized = true;
}

// *****************************************************************************
// Function: LdResultEchoes::ToString
//
/// \brief   Format the result into string (for deguging).
///
/// \return  Formatted result.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

std::string
LeddarConnection::LdResultStates::ToString( void ) const
{
    const std::map< uint32_t, LeddarCore::LdProperty *> *lProperties = mProperties.GetContent();
    std::stringstream lResult;

    for( std::map< uint32_t, LeddarCore::LdProperty *>::const_iterator lIter = lProperties->begin(); lIter != lProperties->end(); ++lIter )
    {
        lResult << lIter->second->GetDescription() << ": " << lIter->second->GetStringValue() << std::endl;
    }

    return lResult.str();
}
