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
#include <map>
#include <set>
#include <stdexcept>
#include <mutex>

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
        enum SIGNALS
        {
            INVALID = -1,
            CONNECTED,
            DISCONNECTED,
            VALUE_CHANGED,
            LIMITS_CHANGED,
            NEW_DATA,
            EXCEPTION
        };

        LdObject( void );
        virtual ~LdObject( void );

        void ConnectSignal( LdObject *aSender, const SIGNALS aSignal ) const;
        void DisconnectSignal( LdObject *aSender, const SIGNALS aSignal ) const;
        size_t GetConnectedObjectsSize( void ) const { return mReceiverMap.size(); }
        virtual void Callback( LdObject * /*aSender*/, const SIGNALS /*aSignal*/, void * /*aExtraData*/ ){};

      protected:
        virtual void EmitSignal( const SIGNALS aSignal, void *aExtraData = nullptr );

      private:
        
        void DisconnectAll( void ) const;
        mutable std::mutex mObjectMutex;
        mutable std::set<LdObject *> mConnectedObject;
        mutable std::multimap<LdObject *, SIGNALS> mReceiverMap;
    };

} // namespace LeddarCore