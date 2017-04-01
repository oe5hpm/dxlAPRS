/**
\file    navigation.c
\brief   GNSS core 'c' function library: navigation functions.
\author  Glenn D. MacGougan (GDM)
\date    2007-11-29
\since   2005-08-23

\b "LICENSE INFORMATION" \n
Copyright (c) 2007, refer to 'author' doxygen tags \n
All rights reserved. \n

Redistribution and use in source and binary forms, with or without
modification, are permitted provided the following conditions are met: \n

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer. \n
- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution. \n
- The name(s) of the contributor(s) may not be used to endorse or promote 
  products derived from this software without specific prior written 
  permission. \n

THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS 
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.
*/

#include <math.h>
#include "gnss_error.h"
#include "constants.h"
#include "geodesy.h"
#include "navigation.h"

void NAVIGATION_ComputeDerivativesOf_Range_WithRespectTo_XYZ( 
  const double x,     // User X coordinate WGS84 ECEF      [m]
  const double y,     // User Y coordinate WGS84 ECEF      [m]
  const double z,     // User Z coordinate WGS84 ECEF      [m]
  const double satX,  // Satellite X coordinate WGS84 ECEF [m]
  const double satY,  // Satellite Y coordinate WGS84 ECEF [m]
  const double satZ,  // Satellite Z coordinate WGS84 ECEF [m]  
  double* dPdx,       // Derivative of P wrt X             []
  double* dPdy,       // Derivative of P wrt Y             []
  double* dPdz,       // Derivative of P wrt Z             []    
  double* range )     // computed user to satellite range  [m]  
{
  double dx;
  double dy;
  double dz;
         
  dx = satX - x;
  dy = satY - y;
  dz = satZ - z;
  
  *range  = sqrt( dx*dx + dy*dy + dz*dz );
  
  // dPdx = d/dx( sqrt( (x_s-x)^2 + (y_s-y)^2 + (z_s-z)^2 ) = -dx/range
  *dPdx   = -dx / (*range);
  *dPdy   = -dy / (*range);	
  *dPdz   = -dz / (*range);      
}




void NAVIGATION_ComputeDerivativesOf_Range_WithRespectToLatitudeLongitudeHeight( 
  const double latitude,  // User geodetic latitude                   [rad]
  const double longitude, // User geodetic longtiude                  [rad]
  const double height,    // User geodetic height                     [m]
  const double satX,      // Satellite X coordinate WGS84 ECEF        [m]
  const double satY,      // Satellite Y coordinate WGS84 ECEF        [m]
  const double satZ,      // Satellite Z coordinate WGS84 ECEF        [m]  
  double* dlat,           // d(P)/d(lat) but not in units of [m/rad], [m/m]
  double* dlon,           // d(P)/d(lon) but not in units of [m/rad], [m/m]
  double* dhgt,           // d(P)/d(hgt)                              [m/m]
  double* range )         // computed user to satellite range         [m]
{
  double sinlat;
  double coslat; 
  double sinlon;
  double coslon;
  double dx;
  double dy;
  double dz;  
  double x; // User X position WGS84 ECEF [m]
  double y; // User Y position WGS84 ECEF [m]
  double z; // User Z position WGS84 ECEF [m]
  
  sinlat = sin(latitude);
  coslat = cos(latitude);
  sinlon = sin(longitude);
  coslon = cos(longitude);

  GEODESY_ConvertGeodeticCurvilinearToEarthFixedCartesianCoordinates(
    GEODESY_REFERENCE_ELLIPSE_WGS84,
    latitude,
    longitude,
    height,
    &x,
    &y, 
    &z );
  
  dx = satX - x;
  dy = satY - y;
  dz = satZ - z;

  // d/dx(P) = d/dx( sqrt( (x_s-x)^2 + (y_s-y)^2 + (z_s-z)^2 ) = -dx/range
  // likewise for d/dy, d/dz
  dx *= -1.0;
  dy *= -1.0;
  dz *= -1.0;

  *range  = sqrt( dx*dx + dy*dy + dz*dz );

  /*
  GEODESY_ComputePrimeVerticalRadiusOfCurvature( GEODESY_REFERENCE_ELLIPSE_WGS84, latitude, &N );
  GEODESY_ComputeMeridianRadiusOfCurvature( GEODESY_REFERENCE_ELLIPSE_WGS84, latitude, &M );  

  The derivatives of range with respect to lat and lon in [rad/m]
  lead to numeric instabilities in matrix inversion. Values in [m/rad] are huge!
  *dlat = (M + hgt) * ( -dx*sinlat*coslon - dy*sinlat*sinlon + dz*coslat ) / (*range);
  *dlon = (N + hgt) * ( -dx*coslat*sinlon + dy*coslat*coslon ) / (*range);  
  to scale dlat into a change in [m/m]
  *dlat /= (M + hgt)
  to scale dlon into a change in [m/m] 
  *dlon /= (N + hgt)*coslat
  
  This is effectively the same as rotating the derivatives with repect to xyz to the local frame
  This method requires a good approximation for the initial position when using this parametrization.
  */
  
  *dlat = ( -dx*sinlat*coslon - dy*sinlat*sinlon + dz*coslat ) / (*range);
  *dlon = ( -dx*sinlon        + dy*coslon                    ) / (*range);  
  *dhgt = (  dx*coslat*coslon + dy*coslat*sinlon + dz*sinlat ) / (*range);  
}







int NAVIGATION_PerformClosedFormPositionSolution_FromPseuodrangeMeasurements(
  double p1,          //!< 1st raw pseudorange measurement [m]
  double p2,          //!< 2nd raw pseudorange measurement [m]
  double p3,          //!< 3rd raw pseudorange measurement [m]
  double p4,          //!< 4th raw pseudorange measurement [m]
  double prc_satclk1, //!< 1st satellite clock corrections for psuedoranges [m]
  double prc_satclk2, //!< 2nd satellite clock corrections for psuedoranges [m]
  double prc_satclk3, //!< 3rd satellite clock corrections for psuedoranges [m]
  double prc_satclk4, //!< 4th satellite clock corrections for psuedoranges [m]
  double x1,          //!< 1st satellite X coordinates, WGS84 ECEF [m]
  double x2,          //!< 2nd satellite X coordinates, WGS84 ECEF [m]
  double x3,          //!< 3rd satellite X coordinates, WGS84 ECEF [m]
  double x4,          //!< 4th satellite X coordinates, WGS84 ECEF [m]
  double y1,          //!< 1st satellite Y coordinates, WGS84 ECEF [m]
  double y2,          //!< 2nd satellite Y coordinates, WGS84 ECEF [m]
  double y3,          //!< 3rd satellite Y coordinates, WGS84 ECEF [m]
  double y4,          //!< 4th satellite Y coordinates, WGS84 ECEF [m]
  double z1,          //!< 1st satellite Z coordinates, WGS84 ECEF [m]
  double z2,          //!< 2nd satellite Z coordinates, WGS84 ECEF [m]
  double z3,          //!< 3rd satellite Z coordinates, WGS84 ECEF [m]
  double z4,          //!< 4th satellite Z coordinates, WGS84 ECEF [m]
  double* latitude,   //!< The computed geodetic latitude [rad]
  double* longitude,  //!< The computed geodetic longitude [rad]
  double* height,     //!< The computed geodetic height [m]
  double* rx_clock_bias //!< The computed receiver clock bias [m]
  )
{
  // difference values
  double x12, x13, x14; // 'xij' = 'xi' - 'xj' [m]
  double y12, y13, y14;
  double z12, z13, z14;
  double p21, p31, p41; // 'pij' = 'pi' - 'pj' [m]

  double k1, k2, k3; // coefficients
  double c1, c2, c3;
  double f1, f2, f3;
  double m1, m2, m3;

  double d1;   // clock bias, solution 1 [m]
  double d2;   // clock bias, solution 2 [m]
  double detA; // determinant of A
  double tmp1;
  double tmp2;
  double tmp3;

  double A[3][3];
  double D[3][3]; // D is A^-1 * detA

  typedef struct
  {
    double x;
    double y;
    double z;    
  } struct_SOLN;

  struct_SOLN s1; 
  struct_SOLN s2;

  // apply the satellite clock corrections
  p1 = p1 + prc_satclk1;
  p2 = p2 + prc_satclk2;
  p3 = p3 + prc_satclk3;
  p4 = p4 + prc_satclk4;  
  
  x12 = x1 - x2;
  x13 = x1 - x3;
  x14 = x1 - x4;

  y12 = y1 - y2;
  y13 = y1 - y3;
  y14 = y1 - y4;
  
  z12 = z1 - z2;
  z13 = z1 - z3;
  z14 = z1 - z4;

  p21 = p2 - p1;
  p31 = p3 - p1;
  p41 = p4 - p1;

  k1 = x12*x12 + y12*y12 + z12*z12 - p21*p21;
  k2 = x13*x13 + y13*y13 + z13*z13 - p31*p31;
  k3 = x14*x14 + y14*y14 + z14*z14 - p41*p41;

  A[0][0] = 2.0*x12;
  A[1][0] = 2.0*x13;
  A[2][0] = 2.0*x14;

  A[0][1] = 2.0*y12;
  A[1][1] = 2.0*y13;
  A[2][1] = 2.0*y14;

  A[0][2] = 2.0*z12;
  A[1][2] = 2.0*z13;
  A[2][2] = 2.0*z14;

  tmp1 = A[1][1]*A[2][2] - A[2][1]*A[1][2];
  tmp2 = A[1][0]*A[2][2] - A[2][0]*A[1][2];
  tmp3 = A[1][0]*A[2][1] - A[2][0]*A[1][1];

  detA = A[0][0]*tmp1 - A[0][1]*tmp2 + A[0][2]*tmp3;
  if(detA==0) return FALSE;

  D[0][0] =  tmp1;
  D[1][0] = -tmp2;
  D[2][0] =  tmp3;

  D[0][1] = -A[0][1]*A[2][2] + A[2][1]*A[0][2];
  D[1][1] =  A[0][0]*A[2][2] - A[2][0]*A[0][2];
  D[2][1] = -A[0][0]*A[2][1] + A[2][0]*A[0][1];

  D[0][2] =  A[0][1]*A[1][2] - A[1][1]*A[0][2];
  D[1][2] = -A[0][0]*A[1][2] + A[1][0]*A[0][2];
  D[2][2] =  A[0][0]*A[1][1] - A[1][0]*A[0][1];

  c1 = (D[0][0]*p21 + D[0][1]*p31 + D[0][2]*p41) * 2.0 / detA;
  c2 = (D[1][0]*p21 + D[1][1]*p31 + D[1][2]*p41) * 2.0 / detA;
  c3 = (D[2][0]*p21 + D[2][1]*p31 + D[2][2]*p41) * 2.0 / detA;

  f1 = (D[0][0]*k1 + D[0][1]*k2 + D[0][2]*k3) / detA;
  f2 = (D[1][0]*k1 + D[1][1]*k2 + D[1][2]*k3) / detA;
  f3 = (D[2][0]*k1 + D[2][1]*k2 + D[2][2]*k3) / detA;

  m1 = c1*c1 + c2*c2 + c3*c3 - 1.0;
  m2 = -2.0*( c1*f1 + c2*f2 + c3*f3 );
  m3 = f1*f1 + f2*f2 + f3*f3;

  tmp1 = m2*m2 - 4.0*m1*m3;
  if( tmp1 < 0 )
  {
    // not good, there is no solution
//    GNSS_ERROR_MSG( "There is no solution." );
    return FALSE;
  }

  d1 = ( -m2 + sqrt( tmp1 ) ) * 0.5 / m1;
  d2 = ( -m2 - sqrt( tmp1 ) ) * 0.5 / m1;

  // two solutions
  s1.x = c1*d1 - f1 + x1;
  s1.y = c2*d1 - f2 + y1;
  s1.z = c3*d1 - f3 + z1;

  s2.x = c1*d2 - f1 + x1;
  s2.y = c2*d2 - f2 + y1;
  s2.z = c3*d2 - f3 + z1;

  tmp1 = sqrt( s1.x*s1.x + s1.y*s1.y + s1.z*s1.z );
  tmp2 = sqrt( s2.x*s2.x + s2.y*s2.y + s2.z*s2.z );

  // choose the correct solution based
  // on whichever solutino is closest to 
  // the Earth's surface
  tmp1 = fabs( tmp1 - 6371000.0 );
  tmp2 = fabs( tmp2 - 6371000.0 );

  if( tmp2 < tmp1 )
  {
    s1 = s2;
    d1 = d2;
  }

  *rx_clock_bias = d1;

  GEODESY_ConvertEarthFixedCartesianToGeodeticCurvilinearCoordinates(
    GEODESY_REFERENCE_ELLIPSE_WGS84,
    s1.x, s1.y, s1.z,
    latitude, longitude, height );

  if( *height < -1500.0 || *height > 50000.0 )
  {
    // height is out of the likely range for terrestrial users
//    GNSS_ERROR_MSG( "The height is out of the likely range for terrestrial users." );
    return FALSE;
  }
  
  return TRUE;
}

