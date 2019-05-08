////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   LeddarTech/LtMathUtils.cpp
///
/// \brief  Implements the Math utilities functions
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LtMathUtils.h"

#include <cmath>
#include <stdexcept>

const double LeddarUtils::LtMathUtils::PI = std::acos( -1 );

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn double LeddarUtils::LtMathUtils::DegreeToRadian( double aAngle )
///
/// \brief  Degree to radian
///
/// \param  aAngle  The angle in degree.
///
/// \return The angle in radian.
///
/// \author David Levy
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
double LeddarUtils::LtMathUtils::DegreeToRadian( double aAngle )
{
    return aAngle * PI / 180.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarUtils::LtMathUtils::LtPointXYZ LeddarUtils::LtMathUtils::SphericalToCartesian( double aRho, double aTheta, double aDelta )
///
/// \brief  Convert spherical to cartesian coordinates angle taken from this page :
///     https://upload.wikimedia.org/wikipedia/commons/8/8c/Spherical_Coordinates_%28Latitude%2C_Longitude%29.svg but rotate axis so z is the
///     sensor axis
///
/// \exception  std::out_of_range   Thrown when the input argument are out of range
///
/// \param  aRho    Distance to the point.
/// \param  aTheta  Angle from sensor axis on horizontal plane.
/// \param  aDelta  Angle from the point to the horizontal plane.
///
/// \return The spherical coordinates.
///
/// \author David Levy
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarUtils::LtMathUtils::LtPointXYZ LeddarUtils::LtMathUtils::SphericalToCartesian( double aRho, double aTheta, double aDelta )
{
    if( aRho < 0 || aTheta < -PI || aTheta > PI || aDelta < -PI / 2.0 || aDelta > PI / 2.0 )
    {
        throw std::out_of_range( "Out of range arguments. Required: rho > 0, -pi < theta < pi, -pi/2 < delta < pi/2" );
    }

    const double x = aRho * cos( aDelta ) * sin( aTheta );
    const double y = aRho * sin( aDelta );
    const double z = aRho * cos( aDelta ) * cos( aTheta );

    return LeddarUtils::LtMathUtils::LtPointXYZ( x, y, z );
}
