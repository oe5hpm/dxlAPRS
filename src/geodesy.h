/**
\file    geodesy.h
\brief   GNSS core 'c' function library: geodesy related functions.
\author  Glenn D. MacGougan (GDM)
\date    2007-11-28
\since   2005-07-30

\b REFERENCES \n
- NovAtel OEM3 reference manual (www.novatel.com). \n
- Schwartz, K. P. (1997). ENGO 421 Lecture Notes - Fundamentals of Geodesy. 
  Chapter 3, pp. 33-66 & Appendix B, pp. 178-183. Geomatics Engineering, 
  University of Calgary. \n
- Hsu, David Y. (1992). Closed Form Solution for Geodetic Coordinate 
  Transformation. ION NTM 1992. pp. 397-400. \n
- Misra, Pratap & Per Enge (2004). Global Positioning System Signals 
  Measurements and Performance. Ganga-Jamuna Press. 
  ISBN 0-9709544-0-9. PP. 115-116. \n

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

#ifndef _C_GEODESY_H_
#define _C_GEODESY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "basictypes.h"


/*************************************************************************************************/
// preprocessor constant definitions, any related enumerations and descriptors

/**
\enum   GEODESY_enumReferenceEllipse
\brief  Enumerated for various reference ellipsoids

Comma delimited data (copy data below to a text file and rename the extension to .csv for easy 
viewing in an excel spreadsheet for example. \n
GEODESY_REFERENCE_ELLIPSE_AIRY,                         6377563.396, 299.3249647,   6356256.9092444032, 0.00667053999776051,\n  
GEODESY_REFERENCE_ELLIPSE_MODIFED_AIRY,                 6377340.189, 299.3249647,   6356034.4479456525, 0.00667053999776060,\n  
GEODESY_REFERENCE_ELLIPSE_AUSTRALIAN_NATIONAL,          6378160.0,   298.25,        6356774.7191953063, 0.00669454185458760,\n  
GEODESY_REFERENCE_ELLIPSE_BESSEL_1841,                  6377397.155, 299.1528128,   6356078.9628181886, 0.00667437223180205,\n  
GEODESY_REFERENCE_ELLIPSE_CLARKE_1866,                  6378206.4,   294.9786982,   6356583.7999989809, 0.00676865799760959,\n  
GEODESY_REFERENCE_ELLIPSE_CLARKE_1880,                  6378249.145, 293.465,       6356514.8695497755, 0.00680351128284912,\n  
GEODESY_REFERENCE_ELLIPSE_EVEREST_INDIA_1830,           6377276.345, 300.8017,      6356075.4131402392, 0.00663784663019987,\n  
GEODESY_REFERENCE_ELLIPSE_EVEREST_BRUNEI_E_MALAYSIA,    6377298.556, 300.8017,      6356097.5503008962, 0.00663784663019965,\n  
GEODESY_REFERENCE_ELLIPSE_EVEREST_W_MALAYSIA_SINGAPORE, 6377304.063, 300.8017,      6356103.0389931547, 0.00663784663019970,\n  
GEODESY_REFERENCE_ELLIPSE_GRS_1980,                     6378137.0,   298.257222101, 6356752.3141403561, 0.00669438002290069,\n  
GEODESY_REFERENCE_ELLIPSE_HELMERT_1906,                 6378200.0,   298.30,        6356818.1696278909, 0.00669342162296610,\n  
GEODESY_REFERENCE_ELLIPSE_HOUGH_1960,                   6378270.0,   297.00,        6356794.3434343431, 0.00672267002233347,\n  
GEODESY_REFERENCE_ELLIPSE_INTERNATIONAL_1924,           6378388.0,   297.00,        6356911.9461279465, 0.00672267002233323,\n  
GEODESY_REFERENCE_ELLIPSE_SOUTH_AMERICAN_1969,          6378160.0,   298.25,        6356774.7191953063, 0.00669454185458760,\n  
GEODESY_REFERENCE_ELLIPSE_WGS72,                        6378135.0,   298.26,        6356750.5200160937, 0.00669431777826668,\n  
GEODESY_REFERENCE_ELLIPSE_WGS84,                        6378137.0,   298.257223563, 6356752.3142451793, 0.00669437999014132,\n  
*/
typedef enum 
{
  GEODESY_REFERENCE_ELLIPSE_WGS84 = 0,                         //!< WGS84
  GEODESY_REFERENCE_ELLIPSE_AIRY_1830=1,                       //!< Airy 1830
  GEODESY_REFERENCE_ELLIPSE_MODIFED_AIRY=2,                    //!< Modified Airy                         
  GEODESY_REFERENCE_ELLIPSE_AUSTRALIAN_NATIONAL=3,             //!< Australian National
  GEODESY_REFERENCE_ELLIPSE_BESSEL_1841=4,                     //!< Bessel 1841
  GEODESY_REFERENCE_ELLIPSE_CLARKE_1866=5,                     //!< Clarke 1866
  GEODESY_REFERENCE_ELLIPSE_CLARKE_1880=6,                     //!< Clarke 1880
  GEODESY_REFERENCE_ELLIPSE_EVEREST_INDIA_1830=7,              //!< Everest(India 1830)
  GEODESY_REFERENCE_ELLIPSE_EVEREST_BRUNEI_E_MALAYSIA=8,       //!< Everest(Brunei & E.Malaysia)
  GEODESY_REFERENCE_ELLIPSE_EVEREST_W_MALAYSIA_SINGAPORE=9,    //!< Everest(W.Malaysia & Singapore)
  GEODESY_REFERENCE_ELLIPSE_GRS_1980=10,                       //!< Geodetic Reference System 1980
  GEODESY_REFERENCE_ELLIPSE_HELMERT_1906=11,                   //!< Helmert 1906
  GEODESY_REFERENCE_ELLIPSE_HOUGH_1960=12,                     //!< Hough 1960
  GEODESY_REFERENCE_ELLIPSE_INTERNATIONAL_1924=13,             //!< International 1924
  GEODESY_REFERENCE_ELLIPSE_SOUTH_AMERICAN_1969=14,            //!< South American 1969
  GEODESY_REFERENCE_ELLIPSE_WGS72=15                           //!< World Geodetic System 1972

} GEODESY_enumReferenceEllipse;

/**
\var    GEODESY_REFERENCE_ELLIPSE_STRING_DESCRIPTION
\brief  string array with text description corresponding to GEODESY_enumReferenceEllipse
\see    GEODESY_enumReferenceEllipse
*/
static char *GEODESY_REFERENCE_ELLIPSE_STRING_DESCRIPTION[64] =
{
  "World Geodetic System 1984",
  "Airy 1830",
  "Modified Airy",
  "Australian National",
  "Bessel 1841",
  "Clarke 1866",
  "Clarke 1880",
  "Everest(India 1830)",
  "Everest(Brunei & E.Malaysia)",
  "Everest(W.Malaysia & Singapore)",
  "Geodetic Reference System 1980",
  "Helmert 1906",
  "Hough 1960",
  "International 1924",
  "South American 1969",
  "World Geodetic System 1972",  
};


#define GEODESY_REFERENCE_ELLIPSE_WGS84_A                                (6378137.0)
#define GEODESY_REFERENCE_ELLIPSE_WGS84_F_INV                            (298.257223563)       
#define GEODESY_REFERENCE_ELLIPSE_WGS84_B                                (6356752.31424518)
#define GEODESY_REFERENCE_ELLIPSE_WGS84_E2                               (0.00669437999014132)

#define GEODESY_REFERENCE_ELLIPSE_AIRY_1830_A                            (6377563.396)
#define GEODESY_REFERENCE_ELLIPSE_AIRY_1830_F_INV                        (299.3249647)
#define GEODESY_REFERENCE_ELLIPSE_AIRY_1830_B                            (6356256.9092444)
#define GEODESY_REFERENCE_ELLIPSE_AIRY_1830_E2                           (0.00667053999776051)

#define GEODESY_REFERENCE_ELLIPSE_MODIFED_AIRY_A                         (6377340.189)
#define GEODESY_REFERENCE_ELLIPSE_MODIFED_AIRY_F_INV                     (299.3249647)       
#define GEODESY_REFERENCE_ELLIPSE_MODIFED_AIRY_B                         (356034.44794565)
#define GEODESY_REFERENCE_ELLIPSE_MODIFED_AIRY_E2                        (0.0066705399977606)

#define GEODESY_REFERENCE_ELLIPSE_AUSTRALIAN_NATIONAL_A                  (6378160.0)
#define GEODESY_REFERENCE_ELLIPSE_AUSTRALIAN_NATIONAL_F_INV              (298.25)      
#define GEODESY_REFERENCE_ELLIPSE_AUSTRALIAN_NATIONAL_B                  (6356774.7191953063)      
#define GEODESY_REFERENCE_ELLIPSE_AUSTRALIAN_NATIONAL_E2                 (0.0066945418545876)      

#define GEODESY_REFERENCE_ELLIPSE_BESSEL_1841_A                          (6377397.155)
#define GEODESY_REFERENCE_ELLIPSE_BESSEL_1841_F_INV                      (299.1528128)      
#define GEODESY_REFERENCE_ELLIPSE_BESSEL_1841_B                          (6356078.9628181886)
#define GEODESY_REFERENCE_ELLIPSE_BESSEL_1841_E2                         (0.00667437223180205)

#define GEODESY_REFERENCE_ELLIPSE_CLARKE_1866_A                          (6378206.4)
#define GEODESY_REFERENCE_ELLIPSE_CLARKE_1866_F_INV                      (294.9786982)            
#define GEODESY_REFERENCE_ELLIPSE_CLARKE_1866_B                          (6356583.7999989809)
#define GEODESY_REFERENCE_ELLIPSE_CLARKE_1866_E2                         (0.00676865799760959)

#define GEODESY_REFERENCE_ELLIPSE_CLARKE_1880_A                          (6378249.145)
#define GEODESY_REFERENCE_ELLIPSE_CLARKE_1880_F_INV                      (293.465)      
#define GEODESY_REFERENCE_ELLIPSE_CLARKE_1880_B                          (6356514.8695497755)
#define GEODESY_REFERENCE_ELLIPSE_CLARKE_1880_E2                         (0.00680351128284912)

#define GEODESY_REFERENCE_ELLIPSE_EVEREST_INDIA_1830_A                   (6377276.345)
#define GEODESY_REFERENCE_ELLIPSE_EVEREST_INDIA_1830_F_INV               (300.8017)      
#define GEODESY_REFERENCE_ELLIPSE_EVEREST_INDIA_1830_B                   (6356075.4131402392)
#define GEODESY_REFERENCE_ELLIPSE_EVEREST_INDIA_1830_E2                  (0.00663784663019987)

#define GEODESY_REFERENCE_ELLIPSE_EVEREST_BRUNEI_E_MALAYSIA_A            (6377298.556)
#define GEODESY_REFERENCE_ELLIPSE_EVEREST_BRUNEI_E_MALAYSIA_F_INV        (300.8017)      
#define GEODESY_REFERENCE_ELLIPSE_EVEREST_BRUNEI_E_MALAYSIA_B            (6356097.5503008962)
#define GEODESY_REFERENCE_ELLIPSE_EVEREST_BRUNEI_E_MALAYSIA_E2           (0.00663784663019965)

#define GEODESY_REFERENCE_ELLIPSE_EVEREST_W_MALAYSIA_SINGAPORE_A         (6377304.063)
#define GEODESY_REFERENCE_ELLIPSE_EVEREST_W_MALAYSIA_SINGAPORE_F_INV     (300.8017)      
#define GEODESY_REFERENCE_ELLIPSE_EVEREST_W_MALAYSIA_SINGAPORE_B         (6356103.0389931547)
#define GEODESY_REFERENCE_ELLIPSE_EVEREST_W_MALAYSIA_SINGAPORE_E2        (0.00663784663019970)

#define GEODESY_REFERENCE_ELLIPSE_GRS_1980_A                             (6378137.0)
#define GEODESY_REFERENCE_ELLIPSE_GRS_1980_F_INV                         (298.257222101)              
#define GEODESY_REFERENCE_ELLIPSE_GRS_1980_B                             (6356752.3141403561)
#define GEODESY_REFERENCE_ELLIPSE_GRS_1980_E2                            (0.00669438002290069)

#define GEODESY_REFERENCE_ELLIPSE_HELMERT_1906_A                         (6378200.0)
#define GEODESY_REFERENCE_ELLIPSE_HELMERT_1906_F_INV                     (298.30)       
#define GEODESY_REFERENCE_ELLIPSE_HELMERT_1906_B                         (6356818.1696278909)
#define GEODESY_REFERENCE_ELLIPSE_HELMERT_1906_E2                        (0.00669342162296610)

#define GEODESY_REFERENCE_ELLIPSE_HOUGH_1960_A                           (6378270.0)
#define GEODESY_REFERENCE_ELLIPSE_HOUGH_1960_F_INV                       (297.00)      
#define GEODESY_REFERENCE_ELLIPSE_HOUGH_1960_B                           (6356794.3434343431)
#define GEODESY_REFERENCE_ELLIPSE_HOUGH_1960_E2                          (0.00672267002233347)

#define GEODESY_REFERENCE_ELLIPSE_INTERNATIONAL_1924_A                   (6378388.0)
#define GEODESY_REFERENCE_ELLIPSE_INTERNATIONAL_1924_F_INV               (297.00)            
#define GEODESY_REFERENCE_ELLIPSE_INTERNATIONAL_1924_B                   (6356911.9461279465)
#define GEODESY_REFERENCE_ELLIPSE_INTERNATIONAL_1924_E2                  (0.00672267002233323)

#define GEODESY_REFERENCE_ELLIPSE_SOUTH_AMERICAN_1969_A                  (6378160.0)
#define GEODESY_REFERENCE_ELLIPSE_SOUTH_AMERICAN_1969_F_INV              (298.25)      
#define GEODESY_REFERENCE_ELLIPSE_SOUTH_AMERICAN_1969_B                  (6356774.7191953063)
#define GEODESY_REFERENCE_ELLIPSE_SOUTH_AMERICAN_1969_E2                 (0.00669454185458760)

#define GEODESY_REFERENCE_ELLIPSE_WGS72_A                                (6378135.0)
#define GEODESY_REFERENCE_ELLIPSE_WGS72_F_INV                            (298.26)      
#define GEODESY_REFERENCE_ELLIPSE_WGS72_B                                (6356750.5200160937)
#define GEODESY_REFERENCE_ELLIPSE_WGS72_E2                               (0.00669431777826668)

/*************************************************************************************************/



/** 
\brief    This is a look up table function to get reference ellipse parameters.
\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30
\return   TRUE(1) if successful, FALSE(0) otherwise.

\b Example \n
\code
  double a;      //!< semi-major axis of the reference ellipse                     [m]
  double b;      //!< semi-minor axis of the reference ellipse (b = a - a*f_inv)   [m] 
  double f_inv;  //!< inverse of the flattening of the reference ellipse           []
  double e2;     //!< eccentricity of the reference ellipse (e2 = (a*a-b*b)/(a*a)) [] 
  BOOL result;
  result = GEODESY_GetReferenceEllipseParameters( GEODESY_REFERENCE_ELLIPSE_WGS84_A, &a, &b, &f_inv, &e2 );
  if( result == TRUE )
  {
    printf( "ellipse = %s\n", GEODESY_REFERENCE_ELLIPSE_STRING_DESCRIPTION[GEODESY_REFERENCE_ELLIPSE_WGS84_A] );
    printf( "a       = %20.3f\n", a );
    printf( "b       = %20.9f\n", b );
    printf( "f_inv   = %20.11f\n", f_inv );
    printf( "e2      = %.15f\n\n", e2 );
  }
\endcode 

\b REFERENCES \n
- NovAtel OEM3 reference manual (www.novatel.com).
*/
BOOL GEODESY_GetReferenceEllipseParameters( 
  const GEODESY_enumReferenceEllipse ellipse, //!< reference ellipse enumerated    []
  double* a,      //!< semi-major axis of the reference ellipse                     [m]
  double* b,      //!< semi-minor axis of the reference ellipse (b = a - a*f_inv)   [m] 
  double* f_inv,  //!< inverse of the flattening of the reference ellipse           []
  double* e2      //!< eccentricity of the reference ellipse (e2 = (a*a-b*b)/(a*a)) [] 
  );



/**
\brief    This function converts curvilinear geodetic coordinates from latitude, longitude, 
and ellipsoidal height to cartesian geodetic coordinates x, y, and z for the 
reference ellipse specified. 
(e.g. GEODESY_REFERENCE_ELLIPSE_WGS84).

\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30
\return   TRUE(1) if successful, FALSE(0) otherwise.

\b Example \n
\code
  double latitude=0;   //!< geodetic latitude                [rad]
  double longitude=0;  //!< geodetic longitude               [rad]
  double height=0;     //!< geodetic height                  [m]
  double x;            //!< earth fixed cartesian coordinate [m]
  double y;            //!< earth fixed cartesian coordinate [m]
  double z;            //!< earth fixed cartesian coordinate [m]

  result = GEODESY_ConvertGeodeticCurvilinearToEarthFixedCartesianCoordinates(
    GEODESY_REFERENCE_ELLIPSE_WGS84_A,
    latitude, 
    longitude,
    height,   
    &x,       
    &y,       
    &z        
    );
  printf( "ellipse   = %s\n", GEODESY_REFERENCE_ELLIPSE_STRING_DESCRIPTION[GEODESY_REFERENCE_ELLIPSE_WGS84_A] );
  printf( "latitude  = %.12lf\n", latitude );
  printf( "longitude = %.12lf\n", longitude );
  printf( "height    = %.4lf\n", height );
  printf( "x         = %.4lf\n", x );
  printf( "y         = %.4lf\n", y );
  printf( "z         = %.4lf\n\n", z );
\endcode

\b REFERENCES \n
- Schwartz, K. P. (1997). ENGO 421 Lecture Notes - Fundamentals of Geodesy. 
  Chapter 3, pp. 33-66 & Appendix B, pp. 178-183. Geomatics Engineering, 
  University of Calgary.\n
*/
BOOL GEODESY_ConvertGeodeticCurvilinearToEarthFixedCartesianCoordinates(
  const GEODESY_enumReferenceEllipse  referenceEllipse,  //!< reference ellipse enumerated []
  const double latitude,   //!< geodetic latitude                [rad]
  const double longitude,  //!< geodetic longitude               [rad]
  const double height,     //!< geodetic height                  [m]
  double *x,               //!< earth fixed cartesian coordinate [m]
  double *y,               //!< earth fixed cartesian coordinate [m]
  double *z                //!< earth fixed cartesian coordinate [m]
  );

/**
\brief    This function converts cartesian geodetic coordinates from x, y, and z to 
curvilinear geodetic coordinates latitude, longitude, and ellipsoidal 
height for the reference ellipse specified. An iterative approach is used.
(e.g. GEODESY_REFERENCE_ELLIPSE_WGS84).

\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30
\return   TRUE(1) if successful, FALSE(0) otherwise.

\remarks
(1) Hsu (1992) shows that the iterative form is slightly faster than the closed form for 3 iterations. \n
(2) Testing by GDM for WGS84, all points lat = -90:1:90 and lon = -180:1:180 were tested
   lat, lon, hgt first converted to x, y, z then converted back to lat ,lon, hgt and the number of
   iterations noted. No errors observed. 2-5 iterations are needed with a mean of 4.32 iterations. \n

\b REFERENCES \n
- Schwartz, K. P. (1997). ENGO 421 Lecture Notes - Fundamentals of Geodesy. 
  Chapter 3, pp. 33-66 & Appendix B, pp. 178-183. Geomatics Engineering, 
  University of Calgary. \n
  Iterative Method - Chapter 3, pp. 33-66.
  Closed Form      - Appendix B, pp. 178-183. \n
- Hsu, David Y. (1992). Closed Form Solution for Geodetic Coordinate 
  Transformation. ION NTM 1992. pp. 397-400. \n
- Misra, Pratap & Per Enge (2004). Global Positioning System Signals 
  Measurements and Performance. Ganga-Jamuna Press. 
  ISBN 0-9709544-0-9. PP. 115-116. \n
*/
BOOL GEODESY_ConvertEarthFixedCartesianToGeodeticCurvilinearCoordinates(
  const GEODESY_enumReferenceEllipse  referenceEllipse,  //!< reference ellipse enumerated []
  const double x,              // earth fixed cartesian coordinate [m]
  const double y,              // earth fixed cartesian coordinate [m]
  const double z,              // earth fixed cartesian coordinate [m]  
  double *latitude,            // geodetic latitude                [rad]
  double *longitude,           // geodetic longitude               [rad]
  double *height               // geodetic height                  [m]
  );


/**
\brief    Given a reference point (datum) for a local geodetic coordinate system
compute northing easting and vertical of another point given its latitude,
longitude, and height for the reference ellipse specified.
(e.g. GEODESY_REFERENCE_ELLIPSE_WGS84).

\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30
\return   TRUE(1) if successful, FALSE(0) otherwise.

\b REFERENCES \n
[1] Schwartz, K. P. (1997). ENGO 421 Lecture Notes - Fundamentals of Geodesy. 
   Chapter 4, pp 79. \n
*/
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
  );



/**
\brief  Compute the difference between two points in a local geodetic frame
using one point as the reference. Can be used to compute position error 
with respect to a known reference but keep in mind a local geodetic 
frame is used.
 
\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30
\return   TRUE(1) if successful, FALSE(0) otherwise.

\b REFERENCES \n
\see GEODESY_ComputeNorthingEastingVertical
*/
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
  );
 



/**
\brief  Computes meridian radius of curvature for the reference ellipse specified.
(e.g. GEODESY_REFERENCE_ELLIPSE_WGS84).

\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30
\return   TRUE(1) if successful, FALSE(0) otherwise.

\b REFERENCES \n
- Schwartz, K. P. (1997). ENGO 421 Lecture Notes - Fundamentals of Geodesy. 
  Chapter 3, pp. 33-66 & Appendix B, pp. 178-183. Geomatics Engineering, 
  University of Calgary. \n
*/
BOOL GEODESY_ComputeMeridianRadiusOfCurvature(
  const GEODESY_enumReferenceEllipse  referenceEllipse, //!< reference ellipse enumerated []
  const double latitude,  //!< geodetic latitude                     [rad]
  double*  M              //!< computed meridian radius of curvature [m]
  );

/**
\brief  Computes the prime vertical radius of curvature for the reference ellipse specified.
(e.g. GEODESY_REFERENCE_ELLIPSE_WGS84).

\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30
\return   TRUE(1) if successful, FALSE(0) otherwise.

\b REFERENCES \n
- Schwartz, K. P. (1997). ENGO 421 Lecture Notes - Fundamentals of Geodesy. 
  Chapter 3, pp. 33-66 & Appendix B, pp. 178-183. Geomatics Engineering, 
  University of Calgary. \n
*/
BOOL GEODESY_ComputePrimeVerticalRadiusOfCurvature(
  const GEODESY_enumReferenceEllipse  referenceEllipse, //!< reference ellipse enumerated []
  const double latitude,  //!< geodetic latitude                           [rad]
  double*  N              //!< computed prime vertical radius of curvature [m]
  );


/**
\brief  Compute the meridian arc between two latitudes for the reference ellipse specified.
(e.g. GEODESY_REFERENCE_ELLIPSE_WGS84).
 
\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30
\return   TRUE(1) if successful, FALSE(0) otherwise.

\b REFERENCES \n
- Schwartz, K. P. (1997). ENGO 421 Lecture Notes - Fundamentals of Geodesy. 
  Chapter 3, pp. 33-66 & Appendix B, pp. 178-183. Geomatics Engineering, 
  University of Calgary. \n
*/
BOOL GEODESY_ComputeMeridianArcBetweenTwoLatitudes(
  const GEODESY_enumReferenceEllipse  referenceEllipse, //!< reference ellipse enumerated []
  const double referenceLatitude,  //!< datum geodetic latitude  [rad]
  const double latitude,           //!< geodetic latitude        [rad]
  double*      arc                 //!< computed meridian arc, North +ve, South -ve [m]
  );


/**
\brief  Compute the parallel arc between two longitudes for the reference ellipse specified.
(e.g. GEODESY_REFERENCE_ELLIPSE_WGS84).
 
\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30
\return   TRUE(1) if successful, FALSE(0) otherwise.

\b REFERENCES \n
- Schwartz, K. P. (1997). ENGO 421 Lecture Notes - Fundamentals of Geodesy. 
  Chapter 3, pp. 33-66 & Appendix B, pp. 178-183. Geomatics Engineering, 
  University of Calgary. \n
*/ 
BOOL GEODESY_ComputeParallelArcBetweenTwoLongitudes(
  const GEODESY_enumReferenceEllipse  referenceEllipse, //!< reference ellipse enumerated []
  const double referenceLatitude,  //!< reference geodetic latitude  [rad]
  const double referenceLongitude, //!< reference geodetic longitude [rad]
  const double longitude,          //!< geodetic longitude           [rad]
  double*      arc                 //!< computed parallel arc, East +ve, West -ve [m]
  );

/**
\brief  Rotates a vector from a Local Geodetic Frame (LG) to and Earth Centered Earth Fixed Frame (ECEF).
 
\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30
\return   TRUE(1) if successful, FALSE(0) otherwise.

\b REFERENCES \n
- Schwartz, K. P. (1997). ENGO 421 Lecture Notes - Fundamentals of Geodesy. 
  Chapter 3, pp. 33-66 & Appendix B, pp. 178-183. Geomatics Engineering, 
  University of Calgary. \n
*/ 
BOOL GEODESY_RotateVectorFromLocalGeodeticFrameToEarthFixedFrame(
  const double referenceLatitude,  //!< reference geodetic latitude                 [rad]
  const double referenceLongitude, //!< reference geodetic longitude                [rad]
  const double dN,                 //!< local geodetic northing vector component    [m]
  const double dE,                 //!< local geodetic easting  vector component    [m]
  const double dUp,                //!< local geodetic vertical vector component    [m]
  double* dX,                      //!< earth centered earth fixed vector component [m]
  double* dY,                      //!< earth centered earth fixed vector component [m]
  double* dZ                       //!< earth centered earth fixed vector component [m]
  );

/**
\brief  Rotates a vector from a Earth Centered Earth Fixed Frame (ECEF) to a Local Geodetic Frame (LG).
 
\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30
\return   TRUE(1) if successful, FALSE(0) otherwise.

\b REFERENCES \n
- Schwartz, K. P. (1997). ENGO 421 Lecture Notes - Fundamentals of Geodesy. 
  Chapter 3, pp. 33-66 & Appendix B, pp. 178-183. Geomatics Engineering, 
  University of Calgary. \n
*/ 
BOOL GEODESY_RotateVectorFromEarthFixedFrameToLocalGeodeticFrame(
  const double referenceLatitude,  //!< reference geodetic latitude                 [rad]
  const double referenceLongitude, //!< reference geodetic longitude                [rad]
  const double dX,                 //!< earth centered earth fixed vector component [m]
  const double dY,                 //!< earth centered earth fixed vector component [m]
  const double dZ,                 //!< earth centered earth fixed vector component [m]
  double* dN,                      //!< local geodetic northing vector component    [m]
  double* dE,                      //!< local geodetic easting  vector component    [m]
  double* dUp                      //!< local geodetic vertical vector component    [m]
  );

/**
\brief  Computes the azimuth and elevation angles in the local geodetic (LG) frame between the 'from' 
point to the 'to' point given these point in the earth fixed frame. 
e.g. between a user gps location (from) and a satellite (to).
 
\author   Glenn D. MacGougan (GDM)
\date     2005-07-30
\since    2005-07-30 
\return   TRUE(1) if successful, FALSE(0) otherwise.
 */
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
  );
  
#ifdef __cplusplus
}
#endif


#endif // _C_GEODESY_H_
