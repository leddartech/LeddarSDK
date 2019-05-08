// *****************************************************************************
// Module..: LeddarTech
//
/// \file    LtExceptions.h
///
/// \brief   Defines all the exceptions
///
/// \author  Patrick Boulay
///
/// \since   May 2016
//
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include <string>
#include <stdint.h>
#include <stdexcept>

namespace LeddarException
{
    enum eErrorType
    {
        ERROR_COM_UNKNOWN   = 0,
        ERROR_COM_READ      = 1,
        ERROR_COM_WRITE     = 2
    };

    class LtException : public std::exception
    {
    public:
        explicit LtException( const std::string &aErrorMsg ) : std::exception( std::runtime_error( aErrorMsg ) ), mInformation( aErrorMsg ) {UpdateDisplayInformation(); };
        explicit LtException( std::exception aException ) : std::exception( aException ), mInformation( aException.what() ) { UpdateDisplayInformation(); };
        virtual ~LtException() throw( ) {};
        virtual const char *what() const  throw( ) override {  return mDisplayInformation.c_str(); }

        std::string GetErrorMsg( void ) const { return mDisplayInformation; }
        void SetExtraInformation( const std::string &aExtraInformation ) { mExtraInformation = aExtraInformation; UpdateDisplayInformation(); }
        std::string GetExtraInformation( void ) const { return mExtraInformation; }

    protected:
        void UpdateDisplayInformation( void ) { mDisplayInformation = mInformation + " " + mExtraInformation; }
        std::string mInformation;
        std::string mExtraInformation;
        std::string mDisplayInformation;
    };

    class LtInfoException : public LtException
    {
    public:
        explicit LtInfoException( const std::string &aErrorMsg ) : LtException( aErrorMsg )  {};
        explicit LtInfoException( std::exception aException ) : LtException( aException ) {};
        virtual ~LtInfoException() throw( ) {};
    };

    class LtComException : public LtException
    {
    public:
        explicit LtComException( const std::string &aErrorMsg, int aErrType = ERROR_COM_UNKNOWN, bool aDisconnect = false ) : LtException( aErrorMsg ),
            mDisconnect( aDisconnect ),
            mErrType( aErrType )
        {};
        explicit LtComException( std::exception aException, bool aDisconnect = false ) : LtException( aException ), mDisconnect( aDisconnect ), mErrType( 0 ) {};
        bool GetDisconnect( void ) { return mDisconnect;  }
        int GetErrType( void ) { return mErrType; }
    protected:
        bool mDisconnect;
        int mErrType;
    };

    class LtTimeoutException : public LtComException
    {
    public:
        LtTimeoutException( const std::string &aErrorMsg, bool aDisconnect = false ) : LtComException( aErrorMsg, aDisconnect ) {};
        LtTimeoutException( std::exception aException, bool aDisconnect = false ) : LtComException( aException, aDisconnect ) {};
    };

    class LtConnectionFailed : public LtComException
    {
    public:
        LtConnectionFailed( const std::string &aErrorMsg, bool aDisconnect = false ) : LtComException( aErrorMsg, aDisconnect ) {};
        LtConnectionFailed( std::exception aException, bool aDisconnect = false ) : LtComException( aException, aDisconnect ) {};
    };

    class LtNotConnectedException : public LtComException
    {
    public:
        LtNotConnectedException( const std::string &aErrorMsg, bool aDisconnect = false ) : LtComException( aErrorMsg, aDisconnect ) {};
        LtNotConnectedException( std::exception aException, bool aDisconnect = false ) : LtComException( aException, aDisconnect ) {};
    };

    class LtConfigException : public LtComException
    {
    public:
        LtConfigException( const std::string &aErrorMsg, bool aDisconnect = false ) : LtComException( aErrorMsg, aDisconnect ) {};
        LtConfigException( std::exception aException, bool aDisconnect = false ) : LtComException( aException, aDisconnect ) {};
    };

    class LtCrcException : public LtComException
    {
    public:
        LtCrcException( const std::string &aErrorMsg, bool aDisconnect = false ) : LtComException( aErrorMsg, aDisconnect ) {};
        LtCrcException( std::exception aException, bool aDisconnect = false ) : LtComException( aException, aDisconnect ) {};
    };

}
