////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdCanKomodo.h
///
/// \brief  Declares the LdCanKomodo class
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#if defined(BUILD_CANBUS_KOMODO) && defined(BUILD_CANBUS)

#include "LdInterfaceCan.h"

namespace LeddarConnection
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdCanKomodo
    ///
    /// \brief  An implementation of the CAN protocol using Komodo adapter.
    ///
    /// \author David Levy
    /// \date   October 2018
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdCanKomodo : public LdInterfaceCan
    {
    public:
        explicit LdCanKomodo( const LdConnectionInfoCan *aConnectionInfo, LdConnection *aExistingConnection = nullptr );
        virtual ~LdCanKomodo();

        virtual void    Connect( void ) override;
        virtual void    Disconnect( void ) override;

        virtual bool    Read( const LdInterfaceCan *aRequestingInterface ) override ;
        virtual void    Write( uint16_t aId, const std::vector<uint8_t> &aData ) override;
        virtual bool    WriteAndWaitForAnswer( uint16_t aId, const std::vector<uint8_t> &aData ) override; //Not recommended to use this function when the sensor is in "stream" mode

        static std::vector<LdConnectionInfo *> GetDeviceList( void );

    private:
        int  mHandle; // mHandle > 0 if it is valid

        void WasteEvent( void );
    };
}

#endif