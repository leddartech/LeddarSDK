////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdInterfaceCan.h
///
/// \brief  Declares the LdInterfaceCan class - Interface class for the CAN protocol
///             For multi-sensors setup, one connection is the "master" and behaves as a router.
///             When data is received, its filled in the mBuffer of the corresponding interface
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#ifdef BUILD_CANBUS

#include "LdConnection.h"
#include "LdConnectionInfoCan.h"

#include "comm/Canbus/LtComCanbus.h"

#include <queue>

namespace LeddarConnection
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdInterfaceCan.
    ///
    /// \brief  Interface class for CANbus protocol. This class is responsible for the "routing" of the data to the correct can interface (for multi sensor setup)
    ///
    /// \author David Levy
    /// \date   November 2018
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdInterfaceCan : public LdConnection
    {
    public:
        virtual ~LdInterfaceCan();

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \fn virtual bool LdInterfaceCan::Read( const LdInterfaceCan *aRequestingInterface ) = 0;
        ///
        /// \brief  Interface function to Read from a CANBus
        ///         To implement this function (you can look at the implementation in LdCanKomodo.cpp file):
        ///             -Check if interface is the master. If we are not, forward the request to the master
        ///             -Once the data is read, forward it to the corresponding interface using ForwardDataMaster
        ///
        /// \param  aRequestingInterface    The interface that requested the read.
        ///
        /// \return True if the requesting interface received data. Else false
        ///
        /// \author David Levy
        /// \date   November 2018
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual bool    Read( const LdInterfaceCan *aRequestingInterface ) = 0;
        bool            Read( void ) {return Read( this ); }
        virtual void    Write( uint16_t aId, const std::vector<uint8_t> &aData ) = 0; //If master, writes to CANbus, else ask master to write
        virtual bool    WriteAndWaitForAnswer( uint16_t aId, const std::vector<uint8_t> &aData ) = 0;
        virtual bool    IsConnected( void ) const override {return mIsConnected;}
        bool            IsMaster(void) const { return mMaster == nullptr; }

    protected:
        explicit LdInterfaceCan( const LdConnectionInfoCan *aConnectionInfo, LdConnection *aExistingInterface = nullptr );
        LeddarConnection::LdInterfaceCan *ForwardDataMaster( uint16_t aId, const std::vector<uint8_t> &aData );

        LdInterfaceCan  *mMaster;  ///Pointer to the "master" connection, responsible to connect / disconnect and write / read. If nullptr, it means we are the master
        bool            mIsConnected;

    private:
        std::vector<LtComCanBus::sCanIds> mRegisteredIds;       ///Store a pointer to a "slave" connection and his ids (and to itself).

        void    RegisterConnection( LeddarConnection::LdInterfaceCan *aNewInterface );
        void    UnRegisterConnection( const LeddarConnection::LdInterfaceCan *aInterface );
        void    ChangeMaster(const std::vector<LtComCanBus::sCanIds>& aRegisteredIds);
        void    ForwardDataSlave( LtComCanBus::sCanData aData );

    };
}

#endif