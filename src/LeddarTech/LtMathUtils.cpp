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
/// \brief  Convert spherical to cartesian. The following image illustrate the representation of each variables used in this formula :
///     https://upload.wikimedia.org/wikipedia/commons/8/8c/Spherical_Coordinates_%28Latitude%2C_Longitude%29.svg
///
/// \exception  std::out_of_range   Thrown when the input argument are out of range
///
/// \param  aRho    Distance to the point.
/// \param  aTheta  Azimuthal angle from X axis to Y axis counterclockwise
/// \param  aDelta  Elevation angle from XY plane to the point
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

    const double x = aRho * cos( aDelta ) * cos( aTheta );
    const double y = aRho * cos( aDelta ) * sin( aTheta );
    const double z = aRho * sin( aDelta );
    

    return LeddarUtils::LtMathUtils::LtPointXYZ( x, y, z );
}
