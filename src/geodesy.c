/**
\file    geodesy.c
\brief   GNSS core 'c' function library: geodesy related functions.
\author  Glenn D. MacGougan (GDM)
\date    2007-11-28
\since   2005-08-14

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
#include "geodesy.h"
#include "constants.h"


/*************************************************************************************************/
// static function definitions (for functions used in this file only)
// static functions are functions that are only visable to other functions in the same file.

// Many functions contained herein only need a, and e2.
static BOOL GEODESY_GetReferenceEllipseParameters_A_E2( 
  const GEODESY_enumReferenceEllipse ellipse,
  double* a,      // semi-major axis of the reference ellipse                     [m]
  double* e2      // eccentricity of the reference ellipse (e2 = (a*a-b*b)/(a*a)) [] 
  );

// Many functions contained herein only need a, b and e2.
static BOOL GEODESY_GetReferenceEllipseParameters_A_B_E2( 
  const GEODESY_enumReferenceEllipse ellipse,
  double* a,      // semi-major axis of the reference ellipse                     [m]
  double* b,      // semi-minor axis of the reference ellipse                     [m]
  double* e2      // eccentricity of the reference ellipse (e2 = (a*a-b*b)/(a*a)) [] 
  );

// return TRUE(1) if valid, FALSE(0) otherwise.
static BOOL GEODESY_IsLatitudeValid( 
  const double latitude //!< expecting a value -pi/2 to pi/2 [rad]
  );
/*************************************************************************************************/




BOOL GEODESY_GetReferenceEllipseParameters( 
  const GEODESY_enumReferenceEllipse ellipse, //!< reference ellipse enumerated []
  double* a,      //!< semi-major axis of the reference ellipse                     [m]
  double* b,      //!< semi-minor axis of the reference ellipse (b = a - a*f_inv)   [m] 
  double* f_inv,  //!< inverse of the flattening of the reference ellipse           []
  double* e2      //!< eccentricity of the reference ellipse (e2 = (a*a-b*b)/(a*a)) [] 
  )
{
  switch( ellipse )
  {
  case GEODESY_REFERENCE_ELLIPSE_AIRY_1830: 
    *a     = GEODESY_REFERENCE_ELLIPSE_AIRY_1830_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_AIRY_1830_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_AIRY_1830_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_AIRY_1830_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_MODIFED_AIRY:
    *a     = GEODESY_REFERENCE_ELLIPSE_MODIFED_AIRY_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_MODIFED_AIRY_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_MODIFED_AIRY_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_MODIFED_AIRY_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_AUSTRALIAN_NATIONAL:
    *a     = GEODESY_REFERENCE_ELLIPSE_AUSTRALIAN_NATIONAL_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_AUSTRALIAN_NATIONAL_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_AUSTRALIAN_NATIONAL_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_AUSTRALIAN_NATIONAL_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_BESSEL_1841:
    *a     = GEODESY_REFERENCE_ELLIPSE_BESSEL_1841_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_BESSEL_1841_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_BESSEL_1841_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_BESSEL_1841_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_CLARKE_1866:
    *a     = GEODESY_REFERENCE_ELLIPSE_CLARKE_1866_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_CLARKE_1866_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_CLARKE_1866_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_CLARKE_1866_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_CLARKE_1880:
    *a     = GEODESY_REFERENCE_ELLIPSE_CLARKE_1880_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_CLARKE_1880_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_CLARKE_1880_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_CLARKE_1880_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_EVEREST_INDIA_1830:
    *a     = GEODESY_REFERENCE_ELLIPSE_EVEREST_INDIA_1830_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_EVEREST_INDIA_1830_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_EVEREST_INDIA_1830_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_EVEREST_INDIA_1830_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_EVEREST_BRUNEI_E_MALAYSIA:
    *a     = GEODESY_REFERENCE_ELLIPSE_EVEREST_BRUNEI_E_MALAYSIA_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_EVEREST_BRUNEI_E_MALAYSIA_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_EVEREST_BRUNEI_E_MALAYSIA_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_EVEREST_BRUNEI_E_MALAYSIA_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_EVEREST_W_MALAYSIA_SINGAPORE:
    *a     = GEODESY_REFERENCE_ELLIPSE_EVEREST_W_MALAYSIA_SINGAPORE_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_EVEREST_W_MALAYSIA_SINGAPORE_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_EVEREST_W_MALAYSIA_SINGAPORE_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_EVEREST_W_MALAYSIA_SINGAPORE_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_GRS_1980:
    *a     = GEODESY_REFERENCE_ELLIPSE_GRS_1980_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_GRS_1980_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_GRS_1980_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_GRS_1980_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_HELMERT_1906:
    *a     = GEODESY_REFERENCE_ELLIPSE_HELMERT_1906_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_HELMERT_1906_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_HELMERT_1906_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_HELMERT_1906_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_HOUGH_1960:
    *a     = GEODESY_REFERENCE_ELLIPSE_HOUGH_1960_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_HOUGH_1960_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_HOUGH_1960_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_HOUGH_1960_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_INTERNATIONAL_1924:
    *a     = GEODESY_REFERENCE_ELLIPSE_INTERNATIONAL_1924_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_INTERNATIONAL_1924_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_INTERNATIONAL_1924_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_INTERNATIONAL_1924_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_SOUTH_AMERICAN_1969:
    *a     = GEODESY_REFERENCE_ELLIPSE_SOUTH_AMERICAN_1969_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_SOUTH_AMERICAN_1969_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_SOUTH_AMERICAN_1969_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_SOUTH_AMERICAN_1969_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_WGS72:
    *a     = GEODESY_REFERENCE_ELLIPSE_WGS72_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_WGS72_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_WGS72_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_WGS72_E2;
    break;
    
  case GEODESY_REFERENCE_ELLIPSE_WGS84:
    *a     = GEODESY_REFERENCE_ELLIPSE_WGS84_A;
    *f_inv = GEODESY_REFERENCE_ELLIPSE_WGS84_F_INV;
    *b     = GEODESY_REFERENCE_ELLIPSE_WGS84_B;
    *e2    = GEODESY_REFERENCE_ELLIPSE_WGS84_E2;
    break;

  default:
    GNSS_ERROR_MSG( "Unexpected default case." );
    return FALSE;
    break;  
  }
  return TRUE;
}


// static
BOOL GEODESY_GetReferenceEllipseParameters_A_E2( 
  const GEODESY_enumReferenceEllipse ellipse,
  double* a,      // semi-major axis of the reference ellipse                     [m]
  double* e2      // eccentricity of the reference ellipse (e2 = (a*a-b*b)/(a*a)) [] 
  )
{  
  double b;
  double f_inv;  
  BOOL result;

  result = GEODESY_GetReferenceEllipseParameters( 
    ellipse,
    a,
    &b,
    &f_inv,
    e2 );

  return result;
}


// static
BOOL GEODESY_GetReferenceEllipseParameters_A_B_E2( 
  const GEODESY_enumReferenceEllipse ellipse,
  double* a,      // semi-major axis of the reference ellipse                     [m]
  double* b,      // semi-minor axis of the reference ellipse                     [m]
  double* e2      // eccentricity of the reference ellipse (e2 = (a*a-b*b)/(a*a)) [] 
  )
{  
  double f_inv;  
  BOOL result;

  result = GEODESY_GetReferenceEllipseParameters( 
    ellipse,
    a,
    b,
    &f_inv,
    e2 );

  return result;
}

// static
BOOL GEODESY_IsLatitudeValid( 
  const double latitude //!< expecting a value -pi/2 to pi/2 [rad]
  )
{
  // check for valid latitude out of range
  if( latitude > HALFPI || latitude < -HALFPI )  
  {
    GNSS_ERROR_MSG( "if( latitude > HALFPI || latitude < -HALFPI )" );
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}







BOOL GEODESY_ConvertGeodeticCurvilinearToEarthFixedCartesianCoordinates(
  const GEODESY_enumReferenceEllipse  referenceEllipse,  //!< reference ellipse enumerated []
  const double latitude,   //!< geodetic latitude                [rad]
  const double longitude,  //!< geodetic longitude               [rad]
  const double height,     //!< geodetic height                  [m]
  double *x,               //!< earth fixed cartesian coordinate [m]
  double *y,               //!< earth fixed cartesian coordinate [m]
  double *z                //!< earth fixed cartesian coordinate [m]
  )
{  
  double a;      // semi-major axis of reference ellipse [m]
  double e2;     // first eccentricity of reference ellipse []
  double N;      // prime vertical radius of curvature [m]
  double sinlat; // sin of the latitude
  double dtmp;   // temp
  BOOL result;

  // get necessary reference ellipse parameters
  result = GEODESY_GetReferenceEllipseParameters_A_E2( referenceEllipse, &a, &e2 );
  if( result == FALSE )
  {
    *x = 0.0;
    *y = 0.0;
    *z = 0.0;
    GNSS_ERROR_MSG( "Reference ellipse invalid." );
    return FALSE;
  }
  
  // check for valid latitude out of range
  result = GEODESY_IsLatitudeValid( latitude );
  if( result == FALSE )
  {
    *x = 0.0;
    *y = 0.0;
    *z = 0.0;
    GNSS_ERROR_MSG( "Input latitude is invalid." );
    return FALSE;
  }

  sinlat = sin( latitude );             
  N = a / sqrt( 1.0 - e2 * sinlat*sinlat );      
  dtmp = (N + height) * cos(latitude);

  *x = dtmp * cos(longitude);
  *y = dtmp * sin(longitude);
  *z = ( (1.0 - e2)*N + height ) * sinlat;

  return TRUE;
}




BOOL GEODESY_ConvertEarthFixedCartesianToGeodeticCurvilinearCoordinates(
  const GEODESY_enumReferenceEllipse  referenceEllipse,  //!< reference ellipse enumerated []
  const double x,              // earth fixed cartesian coordinate [m]
  const double y,              // earth fixed cartesian coordinate [m]
  const double z,              // earth fixed cartesian coordinate [m]  
  double *latitude,            // geodetic latitude              [rad]
  double *longitude,           // geodetic longitude             [rad]
  double *height               // geodetic height                [m]
  )
{
  double a;      // semi-major axis of reference ellipse [m]
  double b;      // semi-minor axis of reference ellipse [m]
  double e2;     // first eccentricity of reference ellipse []
  double N;      // prime vertical radius of curvature [m]
  double p;      // sqrt( x^2 + y^2 ) [m]
  double dtmp;   // temp
  double sinlat; // sin(lat)
  double lat;    // temp geodetic latitude  [rad]
  double lon;    // temp geodetic longitude [rad]
  double hgt;    // temp geodetic height    [m]
  BOOL result;
  
  // get necessary reference ellipse parameters
  result = GEODESY_GetReferenceEllipseParameters_A_B_E2( referenceEllipse, &a, &b, &e2 );
  if( result == FALSE )
  {
    *latitude  = 0;
    *longitude = 0;  
    *height    = 0;  
    GNSS_ERROR_MSG( "Reference ellipse invalid." );    
    return FALSE;
  }
  
  if( x == 0.0 && y == 0.0 ) 
  {
    // at a pole    
    // most likely to happen while using a simulator
    
    // longitude is really unknown
    lon = 0.0; 
    
    if( z < 0 )
    {
      hgt = -z - b;
      lat = -HALFPI;
    }
    else
    {
      hgt = z - b;
      lat = HALFPI;
    }
  }
  else
  {
    p = sqrt( x*x + y*y );

    // unique solution for longitude
    // best formula for any longitude and applies well near the poles
    // pp. 178 reference [2]
    lon = 2.0 * atan2( y , ( x + p ) );
    
    // set approximate initial latitude assuming a height of 0.0
    lat = atan( z / (p * (1.0 - e2)) );
    hgt = 0.0;
    do
    { 
      dtmp = hgt;
      sinlat = sin(lat);
      N   = a / sqrt( 1.0 - e2*sinlat*sinlat );
      hgt = p / cos(lat) - N;
      lat = atan( z / (p * ( 1.0 - e2*N/(N + hgt) )) );      

    } while( fabs( hgt - dtmp ) > 0.0001 );  // 0.1 mm convergence for height
  }

  *latitude  = lat;
  *longitude = lon;  
  *height    = hgt;
  return TRUE;
}


BOOL GEODESY_ComputeNorthingEastingVertical(
  const GEODESY_enumReferenceEllipse  referenceEllipse,  //!< reference ellipse enumerated []
  const double referenceLatitude,  //!< datum geodetic latitude  [rad]
  const double referenceLongitude, //!< datum geodetic longitude [rad]
  const double referenceHeight,    //!< datum geodetic height    [m]
  const double latitude,           //!< geodetic latitude        [rad]
  const double longitude,          //!< geodetic longitude       [rad]
  const double height,             //!< geodetic height          [m]
  double *northing,                //!< local geodetic northing  [m]
  double *easting,                 //!< local geodetic easting   [m]
  double *vertical                 //!< local geodetic vertical  [m]
  )
{
  double x_ref;
  double y_ref;
  double z_ref;
  double x;
  double y;
  double z;
  double dx;
  double dy;
  double dz;
  double A;   // rotation angle [rad]
  double B;   // rotation angle [rad]
  double cosA;
  double sinA;
  double cosB;
  double sinB;
  BOOL result;

  *northing = 0;
  *easting  = 0;
  *vertical = 0;

  result = GEODESY_IsLatitudeValid( referenceLatitude );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "Input reference latitude is invalid" );
    return FALSE;  
  }
  result = GEODESY_IsLatitudeValid( latitude );  
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "Input latitude is invalid." );
    return FALSE;  
  }
  
  result = GEODESY_ConvertGeodeticCurvilinearToEarthFixedCartesianCoordinates(
    referenceEllipse,
    referenceLatitude,
    referenceLongitude,
    referenceHeight,
    &x_ref,
    &y_ref,
    &z_ref );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "GEODESY_ConvertGeodeticCurvilinearToEarthFixedCartesianCoordinates return FALSE." );
    return FALSE;
  }
  
  result = GEODESY_ConvertGeodeticCurvilinearToEarthFixedCartesianCoordinates(
    referenceEllipse,
    latitude,
    longitude,
    height,
    &x,
    &y,
    &z );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "GEODESY_ConvertGeodeticCurvilinearToEarthFixedCartesianCoordinates return FALSE." );
    return FALSE;
  }

  // A and B are rotation angles
  A = referenceLatitude - HALFPI;
  B = referenceLongitude - PI;

  cosA = cos(A);
  sinA = sin(A);
  cosB = cos(B);
  sinB = sin(B);

  // the cartesian vector between the two points in the geodetic 
  // frame is rotated to the local geodetic frame
  dx = x - x_ref;
  dy = y - y_ref;
  dz = z - z_ref;   

  *northing = cosA*cosB * dx  +  cosA*sinB * dy  -  sinA*dz;
  *easting  = sinB      * dx  -  cosB      * dy;
  *vertical = sinA*cosB * dx  +  sinA*sinB * dy  +  cosA*dz;   
  return TRUE;
}


BOOL GEODESY_ComputePositionDifference(
  const GEODESY_enumReferenceEllipse  referenceEllipse,  //!< reference ellipse enumerated []
  const double referenceLatitude,  //!< reference point geodetic latitude  [rad]
  const double referenceLongitude, //!< reference point geodetic longitude [rad]
  const double referenceHeight,    //!< reference point geodetic height    [m]
  const double latitude,           //!< geodetic latitude        [rad]
  const double longitude,          //!< geodetic longitude       [rad]
  const double height,             //!< geodetic height          [m]
  double *difference_northing,     //!< difference in northing   [m]  (+2 m, means 2 m North of the reference)
  double *difference_easting,      //!< difference in easting    [m]  (+2 m, means 2 m East  of the reference)
  double *difference_vertical      //!< difference in vertical   [m]  (+2 m, means 2 m above of the reference)
  )
{
  BOOL result;
  result = GEODESY_ComputeNorthingEastingVertical(
    referenceEllipse,
    referenceLatitude, 
    referenceLongitude,
    referenceHeight,   
    latitude,          
    longitude,         
    height,            
    difference_northing,
    difference_easting,
    difference_vertical );  
  return result;
}




BOOL GEODESY_ComputeMeridianRadiusOfCurvature(
  const GEODESY_enumReferenceEllipse  referenceEllipse, //!< reference ellipse enumerated []
  const double latitude,  //!< geodetic latitude                     [rad]
  double*  M              //!< computed meridian radius of curvature [m]
  )
{
  double a;  // semi-major axis of reference ellipse    [m]
  double e2; // first eccentricity of reference ellipse []
  double dtmp;
  BOOL result;
  
  // get necessary reference ellipse parameters
  result = GEODESY_GetReferenceEllipseParameters_A_E2( referenceEllipse, &a, &e2 );
  if( result == FALSE )
  {
    *M = 0;
    GNSS_ERROR_MSG( "Reference ellipse invalid." );    
    return result;
  }
    
  dtmp = sin(latitude);
  dtmp = sqrt( 1.0 - e2 * dtmp * dtmp );  // W
  dtmp = dtmp*dtmp*dtmp;                  // W^3    
  
  *M = a * ( 1.0 - e2 ) / dtmp;
  return TRUE;
}





BOOL GEODESY_ComputePrimeVerticalRadiusOfCurvature(
  const GEODESY_enumReferenceEllipse  referenceEllipse, //!< reference ellipse enumerated []
  const double latitude,  //!< geodetic latitude                           [rad]
  double*  N              //!< computed prime vertical radius of curvature [m]
  )
{
  double a;  // semi-major axis of reference ellipse [m]
  double e2; // first eccentricity of reference ellipse []
  double W;
  BOOL result;
  
  // get necessary reference ellipse parameters
  result = GEODESY_GetReferenceEllipseParameters_A_E2( referenceEllipse, &a, &e2 );
  if( result == FALSE )
  {
    *N = 0;
    GNSS_ERROR_MSG( "Reference ellipse invalid." );    
    return result;  
  }
  
  W = sin(latitude);
  W = sqrt( 1.0 - e2 * W * W );      
  
  *N = a / W;
  return TRUE;
}


BOOL GEODESY_ComputeMeridianArcBetweenTwoLatitudes(
  const GEODESY_enumReferenceEllipse  referenceEllipse, //!< reference ellipse enumerated []
  const double referenceLatitude,  //!< datum geodetic latitude  [rad]
  const double latitude,           //!< geodetic latitude        [rad]
  double*      arc                 //!< computed meridian arc, North +ve, South -ve [m]
  )
{
  double a;  // semi-major axis of reference ellipse [m]
  double e2; // first eccentricity of reference ellipse []
  double e4; 
  double e6;
  double e8;
  double dtmp;
  double A;
  double B;
  double C;
  double D; 
  double E;
  double arc_ref; // arc from equator for the reference lat [m]
  double arc_p;   // arc from eqautor for point 'P' [m]
  BOOL result;

  *arc = 0;

  result = GEODESY_IsLatitudeValid( referenceLatitude );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "Reference latitude is invalid." );
    return result;
  }
  
  // get necessary reference ellipse parameters
  result = GEODESY_GetReferenceEllipseParameters_A_E2( referenceEllipse, &a, &e2 );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "Reference ellipse invalid." );    
    return result;  
  }
    
  e4 = e2*e2;
  e6 = e4*e2;
  e8 = e6*e2;
  dtmp = a*(1.0-e2);

  A =  dtmp * ( 1.0 + 0.75  * e2 + 0.703125   * e4 + 0.68359375     * e6 + 0.67291259765625 * e8 ); //  dtmp * (1.0 + 3.0/4.0*e2 + 45.0/64.0*e4  + 175.0/256.0*e6  + 11025.0/16384.0*e8 );
  B = -dtmp * (       0.375 * e2 + 0.46875    * e4 + 0.5126953125   * e6 + 0.538330078125   * e8 ); // -dtmp * (      3.0/8.0*e2 + 15.0/32.0*e4  + 525.0/1024.0*e6 + 2205.0/4096.0*e8 );
  C =  dtmp * (                    0.05859375 * e4 + 0.1025390625   * e6 + 0.13458251953125 * e8 ); // -dtmp * (                   15.0/256.0*e4 + 105.0/1024.0*e6 + 2205.0/16384.0*e8 );  
  D = -dtmp * (                                      0.011393229167 * e6 + 0.025634765625   * e8 ); // -dtmp * (                                   35.0/3072.0*e6  + 105.0/4096.0*e8 );  
  E =  dtmp * ( 2.4032593e-03 * e8 );                                                             

  arc_ref = A*referenceLatitude + B*sin(2.0*referenceLatitude) + C*sin(4.0*referenceLatitude) + D*sin(6.0*referenceLatitude) + E*sin(8.0*referenceLatitude);
  arc_p = A*latitude + B*sin(2.0*latitude) + C*sin(4.0*latitude) + D*sin(6.0*latitude) + E*sin(8.0*latitude);

  *arc = arc_p - arc_ref;
  return TRUE;
}




BOOL GEODESY_ComputeParallelArcBetweenTwoLongitudes(
  const GEODESY_enumReferenceEllipse  referenceEllipse, //!< reference ellipse enumerated []
  const double referenceLatitude,  //!< reference geodetic latitude  [rad]
  const double referenceLongitude, //!< reference geodetic longitude [rad]
  const double longitude,          //!< geodetic longitude           [rad]
  double*      arc                 //!< computed parallel arc, East +ve, West -ve [m]
  )
{
  double a;  // semi-major axis of reference ellipse [m]
  double e2; // first eccentricity of reference ellipse []
  double N;  // computed prime vertical radius of curvature [m]
  BOOL result;

  *arc = 0;
  
  // get necessary reference ellipse parameters
  result = GEODESY_GetReferenceEllipseParameters_A_E2( referenceEllipse, &a, &e2 );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "Reference ellipse invalid." );    
    return result;    
  }

  result = GEODESY_IsLatitudeValid( referenceLatitude );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "Reference latitude is invalid." );
    return result;    
  }
  
  N = sin(referenceLatitude);
  N = a / sqrt( 1.0 - e2 * N * N );

  *arc = N * cos(referenceLatitude) * (longitude - referenceLongitude);  
  return TRUE;
}




BOOL GEODESY_RotateVectorFromLocalGeodeticFrameToEarthFixedFrame(
  const double referenceLatitude,  //!< reference geodetic latitude                 [rad]
  const double referenceLongitude, //!< reference geodetic longitude                [rad]
  const double dN,                 //!< local geodetic northing vector component    [m]
  const double dE,                 //!< local geodetic easting  vector component    [m]
  const double dUp,                //!< local geodetic vertical vector component    [m]
  double* dX,                      //!< earth centered earth fixed vector component [m]
  double* dY,                      //!< earth centered earth fixed vector component [m]
  double* dZ                       //!< earth centered earth fixed vector component [m]
  )
{
  double sinlat;
  double coslat;
  double sinlon;
  double coslon;
  BOOL result;

  result = GEODESY_IsLatitudeValid( referenceLatitude );
  if( result == FALSE )
  {
    *dX = 0;
    *dY = 0;
    *dZ = 0;
    GNSS_ERROR_MSG( "Reference latitude is invalid." );
    return result;
  }

  sinlat = sin(referenceLatitude);
  coslat = cos(referenceLatitude);
  sinlon = sin(referenceLongitude);
  coslon = cos(referenceLongitude);
  
  *dX = -sinlat*coslon * dN  -  sinlon * dE  +  coslat*coslon * dUp;
  *dY = -sinlat*sinlon * dN  +  coslon * dE  +  coslat*sinlon * dUp;
  *dZ =  coslat        * dN                  +  sinlat        * dUp;
  return TRUE;
}




BOOL GEODESY_RotateVectorFromEarthFixedFrameToLocalGeodeticFrame(
  const double referenceLatitude,  //!< reference geodetic latitude                 [rad]
  const double referenceLongitude, //!< reference geodetic longitude                [rad]
  const double dX,                 //!< earth centered earth fixed vector component [m]
  const double dY,                 //!< earth centered earth fixed vector component [m]
  const double dZ,                 //!< earth centered earth fixed vector component [m]
  double* dN,                      //!< local geodetic northing vector component    [m]
  double* dE,                      //!< local geodetic easting  vector component    [m]
  double* dUp                      //!< local geodetic vertical vector component    [m]
  )
{
  double sinlat;
  double coslat;
  double sinlon;
  double coslon;
  BOOL result;

  result = GEODESY_IsLatitudeValid( referenceLatitude );
  if( result == FALSE )
  {
    *dN = 0;
    *dE = 0;
    *dUp = 0;
    GNSS_ERROR_MSG( "Reference latitude is invalid." );
    return result;    
  }

  sinlat = sin(referenceLatitude);
  coslat = cos(referenceLatitude);
  sinlon = sin(referenceLongitude);
  coslon = cos(referenceLongitude);
  
  *dN  = -sinlat*coslon * dX  -  sinlat*sinlon * dY  +  coslat * dZ;
  *dE  = -sinlon        * dX  +  coslon        * dY;
  *dUp =  coslat*coslon * dX  +  coslat*sinlon * dY  +  sinlat * dZ;  

  return TRUE;
}



BOOL GEODESY_ComputeAzimuthAndElevationAnglesBetweenToPointsInTheEarthFixedFrame(
  const GEODESY_enumReferenceEllipse  referenceEllipse, //!< reference ellipse enumerated []
  const double fromX, //!< earth centered earth fixed vector from point X component [m]
  const double fromY, //!< earth centered earth fixed vector from point Y component [m]
  const double fromZ, //!< earth centered earth fixed vector from point Z component [m]
  const double toX,   //!< earth centered earth fixed vector to point X component   [m]
  const double toY,   //!< earth centered earth fixed vector to point Y component   [m]
  const double toZ,   //!< earth centered earth fixed vector to point Z component   [m]
  double* elevation,  //!< elevation angle [rad]
  double* azimuth     //!< azimuth angle   [rad]
  )
{
  double lat; // reference geodetic latitude  ('from' point) [rad]
  double lon; // reference geodetic longitude ('from' point) [rad]
  double dX;  // ECEF X vector component between 'from' and 'to' point (m)
  double dY;  // ECEF Y vector component between 'from' and 'to' point (m)
  double dZ;  // ECEF Z vector component between 'from' and 'to' point (m)
  double dN;  // LG northing vector component between 'from' and 'to' point (m)
  double dE;  // LG easting  vector component between 'from' and 'to' point (m)
  double dUp; // LG vertical vector component between 'from' and 'to' point (m)
  double tmp; // temp value
  BOOL result;

  *elevation = 0;
  *azimuth = 0; 

  // get the reference geodetic curvilinear coordinates from the 'from' point
  result = GEODESY_ConvertEarthFixedCartesianToGeodeticCurvilinearCoordinates(
    referenceEllipse,
    fromX,
    fromY,
    fromZ,
    &lat,
    &lon,
    &tmp );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "GEODESY_ConvertEarthFixedCartesianToGeodeticCurvilinearCoordinates returned FALSE." );
    return result;      
  }

  // vector between the two points in the earth fixed frame
  dX = toX - fromX;
  dY = toY - fromY;
  dZ = toZ - fromZ;

  // rotate the vector to the local geodetic frame
  result = GEODESY_RotateVectorFromEarthFixedFrameToLocalGeodeticFrame(
    lat,
    lon,
    dX,
    dY,
    dZ,
    &dN,
    &dE,
    &dUp );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "GEODESY_RotateVectorFromEarthFixedFrameToLocalGeodeticFrame returned FALSE." );
    return result;    
  }
  
  // compute the elevation
  tmp = sqrt( dN*dN + dE*dE );
  *elevation = atan( dUp / tmp );
  
  // compute the azimuth
  *azimuth = atan2(dE, dN);

  // by convention, azimuth will be between 0 to 2 PI
  if( *azimuth < 0.0 )
    *azimuth += TWOPI;
  
  return TRUE;
}

