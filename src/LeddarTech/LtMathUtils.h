////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   LeddarTech/LtMathUtils.h
///
/// \brief  Math utilities functions
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace LeddarUtils
{
    namespace LtMathUtils
    {
        extern const double PI;

        template <class T>
        struct LtPointXY
        {
            LtPointXY( T aX, T aY ) : x( aX ), y( aY ) {}
            T x, y;
        };

        struct LtPointXYZ
        {
            LtPointXYZ( double aX, double aY, double aZ ) : x( aX ), y( aY ), z( aZ ) {}
            LtPointXYZ( ) : x( 0.0 ), y( 0.0 ), z( 0.0 ) {}
            double x, y, z;

            bool operator==( const LtPointXYZ &aPoint ) const {
                return aPoint.x == x && aPoint.y == y && aPoint.x == z;
            }
        };

        double DegreeToRadian( double aAngle );
        LtPointXYZ SphericalToCartesian( double aRho, double aTheta, double aDelta );
    }
}
