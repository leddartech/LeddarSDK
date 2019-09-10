////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdObject.h
///
/// \brief  Declares the LdObject class
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"

#include <cstddef>
#include <list>
#include <map>
#include <stdexcept>

namespace LeddarCore
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdObject.
    ///
    /// \brief  Base class of all LeddarTech SDK objects.
    ///
    /// \author Patrick Boulay
    /// \date   January 2016
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdObject
    {
    public:
        enum SIGNALS {  CONNECTED,
                        DISCONNECTED,
                        VALUE_CHANGED,
                        LIMITS_CHANGED,
                        NEW_DATA
                     };

        LdObject( void );
        virtual ~LdObject( void );

        void ConnectSignal( LdObject *aSender, const SIGNALS aSignal );
        void DisconnectSignal( LdObject *aSender, const SIGNALS aSignal );
        size_t GetConnectedObjectsSize( void ) const { return mReceiverMap.size(); }
        virtual void Callback( LdObject * /*aSender*/, const SIGNALS /*aSignal*/, void * /*aExtraData*/ ) {};

    protected:
        void EmitSignal( const SIGNALS aSignal, void *aExtraData = nullptr );

    private:
		LdObject(const LdObject &aObj);
		LdObject& operator=(const LdObject &aObj);

        void DisconnectAll( void );

        std::list< LdObject * > mConnectedObject;
        std::multimap< LdObject *, SIGNALS> mReceiverMap;
    };

}