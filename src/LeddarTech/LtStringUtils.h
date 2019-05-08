////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   LeddarTech/LtStringUtils.h
///
/// \brief  Utilities for std::string
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <string>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <sstream>
#include <iomanip>
#include <vector>
#include <bitset>

namespace LeddarUtils
{
    namespace LtStringUtils
    {
        int32_t StringToInt( const std::string &aData, int aBase );
        uint32_t StringToUInt( const std::string &aData, int aBase );
        float StringToFloat( const std::string &aData );

        template <typename T> std::string IntToString( T aData, int aBase = 10, bool aLeadingZero = false );

        // Left trim
        inline std::string &LeftTrim( std::string &s )
        {
            s.erase( s.begin(), std::find_if( s.begin(), s.end(), std::not1( std::ptr_fun<int, int>( std::isspace ) ) ) );
            return s;
        }

        // Right trim
        inline std::string &RightTrim( std::string &s )
        {
            s.erase( std::find_if( s.rbegin(), s.rend(), std::not1( std::ptr_fun<int, int>( std::isspace ) ) ).base(), s.end() );
            return s;
        }

        // Trim in both side
        inline std::string &Trim( std::string &s )
        {
            return LeftTrim( RightTrim( s ) );
        }

        // Replace all occurences of a character by another one
        inline void Replace( std::string &aInput, char aCharToReplace, char aReplacementChar )
        {
            std::replace( aInput.begin(), aInput.end(), aCharToReplace, aReplacementChar );
        }

        inline std::string ToUpper( std::string aValue )
        {
            std::transform( aValue.begin(), aValue.end(), aValue.begin(), ::toupper );
            return aValue;
        }

        inline std::string ToLower( std::string aValue )
        {
            std::transform( aValue.begin(), aValue.end(), aValue.begin(), ::tolower );
            return aValue;
        }

        std::vector<std::string> Split( const std::string &aLine, char aSeparator = ' ' );

        void HexStringToByteArray( const std::string &aHexString, uint8_t *aHexByte );
        std::string ByteArrayToHexString( const uint8_t *aHexByte, uint32_t aLength );

        uint32_t StringToIp4Addr( const std::string &aIpAddrStr );
        std::string Ip4AddrToString( uint32_t aIpAddr );
        void Ip4PortStringToValues( const std::string &aIp4Port, std::string *aIp, uint16_t *aPort );
#ifdef _WIN32
        std::string Utf8Encode( const std::wstring &lWstr );
        std::wstring Utf8Decode( const std::string &lStr );
#endif

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \fn template <typename T> std::string IntToString( T aData, int aBase, bool aLeadingZero )
        ///
        /// \brief  Convert int to string
        ///         Note : negative int8 in base 16 are displayed as int16
        ///
        /// \tparam T   Generic integer type parameter.
        /// \param  aData           Int to convert.
        /// \param  aBase           Number base to display the integer
        /// \param  aLeadingZero    True to leading zero.
        ///
        /// \return A std::string.
        ///
        /// \author David Levy
        /// \date   November 2018
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        template <typename T> std::string
        IntToString( T aData, int aBase, bool aLeadingZero )
        {
            if( aBase == 8 || aBase == 10 || aBase == 16 )
            {
                std::stringstream lResult;

                if( aLeadingZero )
                {
                    lResult << ( aBase == 16 ? "0x" : "" ) << std::setw( sizeof( T ) ) << std::setfill( '0' ) << std::setbase( aBase ) << aData;
                }
                else
                {
                    lResult << ( aBase == 16 ? "0x" : "" ) << std::setbase( aBase ) << aData;
                }

                return lResult.str();
            }
            else if( aBase == 2 )
            {
                std::string lRes = std::bitset<sizeof( aData ) * 8>( aData ).to_string();

                if( aLeadingZero == false )
                {
                    lRes.erase( 0, std::min( lRes.find_first_not_of( '0' ), lRes.size() - 1 ) );
                }

                return lRes;
            }
            else
            {
                throw std::invalid_argument( "Unsupported base to convert int to string." );
            }
        }

        template <> inline std::string
        IntToString<uint8_t>( uint8_t aData, int aBase, bool aLeadingZero )
        {
            if( aBase == 8 || aBase == 10 || aBase == 16 )
            {
                std::stringstream lResult;

                if( aLeadingZero )
                {
                    lResult << ( aBase == 16 ? "0x" : "" ) << std::setw( sizeof( uint8_t ) ) << std::setfill( '0' ) << std::setbase( aBase ) << static_cast<uint16_t>( aData );
                }
                else
                {
                    lResult << ( aBase == 16 ? "0x" : "" ) << std::setbase( aBase ) << static_cast<uint16_t>( aData );
                }

                return lResult.str();
            }
            else if( aBase == 2 )
            {
                std::string lRes = std::bitset<sizeof( aData ) * 8>( aData ).to_string();

                if( aLeadingZero == false )
                {
                    lRes.erase( 0, std::min( lRes.find_first_not_of( '0' ), lRes.size() - 1 ) );
                }

                return lRes;
            }
            else
            {
                throw std::invalid_argument( "Unsupported base to convert int to string." );
            }
        }

        template <> inline std::string
        IntToString<int8_t>( int8_t aData, int aBase, bool aLeadingZero )
        {
            if( aBase == 8 || aBase == 10 || aBase == 16 )
            {
                std::stringstream lResult;

                if( aLeadingZero )
                {
                    lResult << ( aBase == 16 ? "0x" : "" ) << std::setw( sizeof( int8_t ) ) << std::setfill( '0' ) << std::setbase( aBase ) << static_cast<int16_t>( aData );
                }
                else
                {
                    lResult << ( aBase == 16 ? "0x" : "" ) << std::setbase( aBase ) << static_cast<int16_t>( aData );
                }

                return lResult.str();
            }
            else if( aBase == 2 )
            {
                std::string lRes = std::bitset<sizeof( aData ) * 8>( aData ).to_string();

                if( aLeadingZero == false )
                {
                    lRes.erase( 0, std::min( lRes.find_first_not_of( '0' ), lRes.size() - 1 ) );
                }

                return lRes;
            }
            else
            {
                throw std::invalid_argument( "Unsupported base to convert int to string." );
            }
        }
    }
}
