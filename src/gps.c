/**
\file    gps.c
\brief   GNSS core 'c' function library: GPS specific functions.
\author  Glenn D. MacGougan (GDM)
\date    2005-08-14
\since   2005-07-31

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
#include "gps.h"
#include "constants.h"
#include "geodesy.h"


/*************************************************************************************************/
// local preprocessor constant definitions, any related enumerations and descriptors

#ifndef  GPS_CLOCK_CORRECTION_RELATIVISTIC_CONSTANT_F
#define  GPS_CLOCK_CORRECTION_RELATIVISTIC_CONSTANT_F  (-4.442807633e-10)  //!< combined constant defined in ICD-GPS-200C p. 88     [s]/[sqrt(m)]
#endif
#ifndef  GPS_UNIVERSAL_GRAVITY_CONSTANT
#define  GPS_UNIVERSAL_GRAVITY_CONSTANT                (3.986005e14)       //!< gravity constant defined on ICD-GPS-200C p. 98      [m^3/s^2]
#endif
#ifndef  GPS_RATIO_OF_SQUARED_FREQUENCIES_L1_OVER_L2   
#define  GPS_RATIO_OF_SQUARED_FREQUENCIES_L1_OVER_L2   (1.6469444444444444444444444444444) //!< (f_L1/f_L2)^2 = (1575.42/1227.6)^2 = (77/60)^2
#endif
#ifndef  GPS_WGS84_EARTH_ROTATION_RATE
#define  GPS_WGS84_EARTH_ROTATION_RATE                 (7.2921151467e-05)  //!< constant defined on ICD-GPS-200C p. 98            [rad/s]
#endif

#define TWO_TO_THE_POWER_OF_55  (36028797018963968.0)
#define TWO_TO_THE_POWER_OF_43  (8796093022208.0)
#define TWO_TO_THE_POWER_OF_33  (8589934592.0)
#define TWO_TO_THE_POWER_OF_31  (2147483648.0)
#define TWO_TO_THE_POWER_OF_29  (536870912.0)
#define TWO_TO_THE_POWER_OF_19  (524288.0)

//
/*************************************************************************************************/



void GPS_ComputeSatelliteClockCorrectionAndDrift(
  const unsigned short transmission_gpsweek,   //!< GPS week when signal was transmit (0-1024+)            [weeks]
  const double         transmission_gpstow,    //!< GPS time of week when signal was transmit              [s]  
  const unsigned short ephem_week,             //!< ephemeris: GPS week (0-1024+)                          [weeks]
  const unsigned       toe,                    //!< ephemeris: time of week                                [s]
  const unsigned       toc,                    //!< ephemeris: clock reference time of week                [s]
  const double         af0,                    //!< ephemeris: polynomial clock correction coefficient     [s],   Note: parameters from ephemeris preferred vs almanac (22 vs 11 bits)
  const double         af1,                    //!< ephemeris: polynomial clock correction coefficient     [s/s], Note: parameters from ephemeris preferred vs almanac (16 vs 11 bits)
  const double         af2,                    //!< ephemeris: polynomial clock correction coefficient     [s/s^2]  
  const double         ecc,                    //!< ephemeris: eccentricity of satellite orbit             []
  const double         sqrta,                  //!< ephemeris: square root of the semi-major axis of orbit [m^(1/2)]
  const double         delta_n,                //!< ephemeris: mean motion difference from computed value  [rad]
  const double         m0,                     //!< ephemeris: mean anomaly at reference time              [rad]
  const double         tgd,                    //!< ephemeris: group delay differential between L1 and L2  [s]
  const unsigned char  mode,                   //!< 0=L1 only, 1=L2 only (see p. 90, ICD-GPS-200C)
  double*  clock_correction,  //!< satellite clock correction       [m]
  double*  clock_drift        //!< satellite clock drift correction [m/s]
   )
{               
  unsigned char i; // counter 

  double tot;    // time of transmission (including gps week) [s] 
  double tk;     // time from ephemeris reference epoch       [s]
  double tc;     // time from clock reference epoch           [s]
  double d_tr;   // relativistic correction term              [s]
  double d_tsv;  // SV PRN code phase time offset             [s]
  double a;      // semi-major axis of orbit                  [m]
  double n;      // corrected mean motion                     [rad/s]
  double M;      // mean anomaly,                             [rad]   (Kepler's equation for eccentric anomaly, solved by iteration)
  double E;      // eccentric anomaly                         [rad]      

  // compute the times from the reference epochs 
  // By including the week in the calculation, week rollover and old ephmeris bugs are mitigated
  // The result should be between -302400 and 302400 if the ephemeris is within one week of transmission   
  tot = transmission_gpsweek*SECONDS_IN_WEEK + transmission_gpstow;
  tk  = tot - (ephem_week*SECONDS_IN_WEEK + toe);
  tc  = tot - (ephem_week*SECONDS_IN_WEEK + toc);

  // compute the corrected mean motion term
  a = sqrta*sqrta;
  n = sqrt( GPS_UNIVERSAL_GRAVITY_CONSTANT / (a*a*a) ); // computed mean motion
  n += delta_n; // corrected mean motion
  
  // Kepler's equation for eccentric anomaly 
  M = m0 + n*tk; // mean anomaly
  E = M;
  for( i = 0; i < 7; i++ )
  {
    E = M + ecc * sin(E);
  }
 
  // relativistic correction
  d_tr = GPS_CLOCK_CORRECTION_RELATIVISTIC_CONSTANT_F * ecc * sqrta * sin(E); // [s]
  d_tr *= LIGHTSPEED;

  // clock correcton 
  d_tsv = af0 + af1*tc + af2*tc*tc; // [s]
        
  if( mode == 0 ) 
  {
    // L1 only
    d_tsv -= tgd; // [s]
  }
  else if( mode == 1 ) 
  {
    // L2 only
    d_tsv -= tgd*GPS_RATIO_OF_SQUARED_FREQUENCIES_L1_OVER_L2; // [s]
  }

  // clock correction
  *clock_correction = d_tsv*LIGHTSPEED + d_tr; // [m]

  // clock drift
  *clock_drift = (af1 + 2.0*af2*tc) * LIGHTSPEED; // [m/s]
}




void GPS_ComputeSatellitePositionAndVelocity( 
  const unsigned short transmission_gpsweek,   //!< GPS week when signal was transmit (0-1024+)                                              [weeks]
  const double         transmission_gpstow,    //!< GPS time of week when signal was transmit                                                [s]  
  const unsigned short ephem_week,             //!< ephemeris: GPS week (0-1024+)                                                            [weeks]
  const unsigned       toe,                    //!< ephemeris: time of week                                                                  [s]
  const double         m0,                     //!< ephemeris: mean anomaly at reference time                                                [rad]
  const double         delta_n,                //!< ephemeris: mean motion difference from computed value                                    [rad]
  const double         ecc,                    //!< ephemeris: eccentricity                                                                  []
  const double         sqrta,                  //!< ephemeris: square root of the semi-major axis                                            [m^(1/2)]
  const double         omega0,                 //!< ephemeris: longitude of ascending node of orbit plane at weekly epoch                    [rad]
  const double         i0,                     //!< ephemeris: inclination angle at reference time                                           [rad]
  const double         w,                      //!< ephemeris: argument of perigee                                                           [rad]
  const double         omegadot,               //!< ephemeris: rate of right ascension                                                       [rad/s]
  const double         idot,                   //!< ephemeris: rate of inclination angle                                                     [rad/s]
  const double         cuc,                    //!< ephemeris: amplitude of the cosine harmonic correction term to the argument of latitude  [rad]
  const double         cus,                    //!< ephemeris: amplitude of the sine   harmonic correction term to the argument of latitude  [rad]
  const double         crc,                    //!< ephemeris: amplitude of the cosine harmonic correction term to the orbit radius          [m]
  const double         crs,                    //!< ephemeris: amplitude of the sine   harmonic correction term to the orbit radius          [m]
  const double         cic,                    //!< ephemeris: amplitude of the cosine harmonic correction term to the angle of inclination  [rad]
  const double         cis,                    //!< ephemeris: amplitude of the sine   harmonic correction term to the angle of inclination  [rad]
  const double         estimateOfTrueRange,    //!< best estimate of the signal propagation time (in m) for Sagnac effect compensation       [m]
  const double         estimteOfRangeRate,     //!< best estimate of the true signal Doppler (in m/s)   for Sagnac effect compensation       [m/s]
  double* x,  //!< satellite x            [m]
  double* y,  //!< satellite y            [m]
  double* z,  //!< satellite z            [m] 
  double* vx, //!< satellite velocity x   [m/s]
  double* vy, //!< satellite velocity y   [m/s]
  double* vz  //!< satellite velocity z   [m/s]
 )
{
  unsigned char j; // counter 

  double tot;        // time of transmission (including gps week) [s] 
  double tk;         // time from ephemeris reference epoch       [s]
  double a;          // semi-major axis of orbit                  [m]
  double n;          // corrected mean motion                     [rad/s]
  double M;          // mean anomaly,                             [rad]   (Kepler's equation for eccentric anomaly, solved by iteration)
	double E;          // eccentric anomaly                         [rad]      
  double v;          // true anomaly                              [rad]
  double u;          // argument of latitude, corrected           [rad]
  double r;          // radius in the orbital plane               [m]
  double i;          // orbital inclination                       [rad]
  double cos2u;      // cos(2*u)                                  []
  double sin2u;      // sin(2*u)                                  []
  double d_u;        // argument of latitude correction           [rad]
  double d_r;        // radius correction                         [m]
  double d_i;        // inclination correction                    [rad]
  double x_op;       // x position in the orbital plane           [m]
  double y_op;       // y position in the orbital plane           [m]
  double omegak;     // corrected longitude of the ascending node [rad]
  double cos_omegak; // cos(omegak)
  double sin_omegak; // sin(omegak)
  double cosu;       // cos(u)
  double sinu;       // sin(u)
  double cosi;       // cos(i)
  double sini;       // sin(i)
  double cosE;       // cos(E)
  double sinE;       // sin(E)
  double omegadotk;  // corrected rate of right ascension         [rad/s]
  double edot;       // edot = n/(1.0 - ecc*cos(E)),              [rad/s] 
  double vdot;       // d/dt of true anomaly                      [rad/s]
  double udot;       // d/dt of argument of latitude              [rad/s]
  double idotdot;    // d/dt of the rate of the inclination angle [rad/s^2]
  double rdot;       // d/dt of the radius in the orbital plane   [m/s]
  double tmpa;       // temp
  double tmpb;       // temp
  double vx_op;      // x velocity in the orbital plane           [m/s]
  double vy_op;      // y velocity in the orbital plane           [m/s]
         

  // compute the times from the reference epochs 
  // By including the week in the calculation, week rollover and older ephemeris bugs are mitigated
  // The result should be between -302400 and 302400 if the ephemeris is within one week of transmission   
  tot = transmission_gpsweek*SECONDS_IN_WEEK + transmission_gpstow;
  tk  = tot - (ephem_week*SECONDS_IN_WEEK + toe);
  
  // compute the corrected mean motion term
  a = sqrta*sqrta;
  n = sqrt( GPS_UNIVERSAL_GRAVITY_CONSTANT / (a*a*a) ); // computed mean motion
  n += delta_n; // corrected mean motion
  
  // Kepler's equation for eccentric anomaly 
  M = m0 + n*tk; // mean anomaly
  E = M;
  for( j = 0; j < 7; j++ )
  {
    E = M + ecc * sin(E);
  }

  cosE = cos(E);
  sinE = sin(E);
 
  // true anomaly
  v = atan2( (sqrt(1.0 - ecc*ecc)*sinE),  (cosE - ecc) );

  // argument of latitude
  u = v + w;
  // radius in orbital plane
  r = a * (1.0 - ecc * cos(E)); 
  // orbital inclination
  i = i0;

  // second harmonic perturbations
  //
  cos2u = cos(2.0*u);
  sin2u = sin(2.0*u);
  // argument of latitude correction  
  d_u = cuc * cos2u  +  cus * sin2u; 
  // radius correction  
  d_r = crc * cos2u  +  crs * sin2u; 
  // correction to inclination
  d_i = cic * cos2u  +  cis * sin2u;

  // corrected argument of latitude
  u += d_u;
  // corrected radius
  r += d_r;
  // corrected inclination
  i += d_i + idot * tk;

  // positions in orbital plane
  cosu = cos(u);
  sinu = sin(u);
  x_op = r * cosu;
  y_op = r * sinu;


  // compute the corrected longitude of the ascending node
  // This equation deviates from that in Table 20-IV p. 100 GPSICD200C with the inclusion of the 
  // signal propagation time (estimateOfTrueRange/LIGHTSPEED) term. This compensates for the Sagnac effect.
  // The omegak term is thus sensitive to the estimateOfTrueRange term which is usually unknown without
  // prior information. The average signal propagation time/range (70ms * c) can be used on first use
  // and this function must be called again to iterate this term. The sensitivity of the omegak term
  // typically requires N iterations - GDM_DEBUG{find out how many iterations are needed, how sensitive to the position?}
  omegak = omega0 + (omegadot - GPS_WGS84_EARTH_ROTATION_RATE)*tk - GPS_WGS84_EARTH_ROTATION_RATE*(toe + estimateOfTrueRange/LIGHTSPEED );

  // compute the WGS84 ECEF coordinates, 
  // vector r with components x & y is now rotated using, R3(-omegak)*R1(-i)
  cos_omegak = cos(omegak);
  sin_omegak = sin(omegak);
  cosi = cos(i);
  sini = sin(i);
  *x = x_op * cos_omegak - y_op * sin_omegak * cosi;
  *y = x_op * sin_omegak + y_op * cos_omegak * cosi;
  *z = y_op * sini;

  
  // Satellite Velocity Computations are below
  // see Reference
  // Remodi, B. M (2004). GPS Tool Box: Computing satellite velocities using the broadcast ephemeris. 
  // GPS Solutions. Volume 8(3), 2004. pp. 181-183 
  //
  // example source code was available at [http://www.ngs.noaa.gov/gps-toolbox/bc_velo/bc_velo.c]  

  // recomputed the cos and sin of the corrected argument of latitude
  cos2u = cos(2.0*u);
  sin2u = sin(2.0*u);
    
  edot  = n / (1.0 - ecc*cosE);
  vdot  = sinE*edot*(1.0 + ecc*cos(v)) / ( sin(v)*(1.0-ecc*cosE) );  
  udot  = vdot + 2.0*(cus*cos2u - cuc*sin2u)*vdot;
  rdot  = a*ecc*sinE*n/(1.0-ecc*cosE) + 2.0*(crs*cos2u - crc*sin2u)*vdot;
  idotdot = idot + (cis*cos2u - cic*sin2u)*2.0*vdot;    
    
  vx_op = rdot*cosu - y_op*udot;
  vy_op = rdot*sinu + x_op*udot;

  // corrected rate of right ascension including similarily as above, for omegak, 
  // compensation for the Sagnac effect
  omegadotk = omegadot - GPS_WGS84_EARTH_ROTATION_RATE*( 1.0 + estimteOfRangeRate/LIGHTSPEED );
  
  tmpa = vx_op - y_op*cosi*omegadotk;  
  tmpb = x_op*omegadotk + vy_op*cosi - y_op*sini*idotdot;
    
  *vx = tmpa * cos_omegak - tmpb * sin_omegak;  
  *vy = tmpa * sin_omegak + tmpb * cos_omegak;  
  *vz = vy_op*sini + y_op*cosi*idotdot;  
}


void GPS_ComputeUserToSatelliteRange( 
  const double userX,    //!< user X position WGS84 ECEF         [m]
  const double userY,    //!< user Y position WGS84 ECEF         [m]
  const double userZ,    //!< user Z position WGS84 ECEF         [m]
  const double satX,     //!< satellite X position WGS84 ECEF    [m]
  const double satY,     //!< satellite Y positoin WGS84 ECEF    [m]
  const double satZ,     //!< satellite Z position WGS84 ECEF    [m] 
  double* range          //!< user to satellite range            [m]
  )
{
  double dx;
  double dy;
  double dz;
         
  dx = satX - userX;
  dy = satY - userY;
  dz = satZ - userZ;

  // compute the range
  *range = sqrt( dx*dx + dy*dy + dz*dz );
}


void GPS_ComputeUserToSatelliteRangeAndRangeRate( 
  const double userX,    //!< user X position WGS84 ECEF         [m]
  const double userY,    //!< user Y position WGS84 ECEF         [m]
  const double userZ,    //!< user Z position WGS84 ECEF         [m]
  const double userVx,   //!< user X velocity WGS84 ECEF         [m/s]
  const double userVy,   //!< user Y velocity WGS84 ECEF         [m/s]
  const double userVz,   //!< user Z velocity WGS84 ECEF         [m/s]
  const double satX,     //!< satellite X position WGS84 ECEF    [m]
  const double satY,     //!< satellite Y positoin WGS84 ECEF    [m]
  const double satZ,     //!< satellite Z position WGS84 ECEF    [m] 
  const double satVx,    //!< satellite X velocity WGS84 ECEF    [m/s]
  const double satVy,    //!< satellite Y velocity WGS84 ECEF    [m/s]
  const double satVz,    //!< satellite Z velocity WGS84 ECEF    [m/s]
  double* range,         //!< user to satellite range            [m]
  double* range_rate     //!< user to satellite range rate       [m/s]
  )
{
  double dx;
  double dy;
  double dz;
         
  dx = satX - userX;
  dy = satY - userY;
  dz = satZ - userZ;

  // compute the range
  *range = sqrt( dx*dx + dy*dy + dz*dz );
  
  // compute the range rate
  // this method uses the NovAtel style sign convention!
  *range_rate = (userVx - satVx)*dx + (userVy - satVy)*dy + (userVz - satVz)*dz;
  *range_rate /= *range;      
}



void GPS_ComputeSatellitePositionVelocityAzimuthElevationDoppler_BasedOnAlmanacData(
  const double         userX,        //!< user X position WGS84 ECEF                                   [m]
  const double         userY,        //!< user Y position WGS84 ECEF                                   [m]
  const double         userZ,        //!< user Z position WGS84 ECEF                                   [m]
  const unsigned short gpsweek,      //!< user gps week (0-1024+)                                      [week]
  const double         gpstow,       //!< user time of week                                            [s]
  const double         toa,          //!< time of applicability                                        [s]  
  const unsigned short almanac_week, //!< gps week of almanac (0-1024+)                                [week]
  const unsigned short prn,          //!< GPS prn number                                               []
  const double         ecc,          //!< eccentricity                                                 []
  const double         i0,           //!< orbital inclination at reference time                        [rad]
  const double         omegadot,     //!< rate of right ascension                                      [rad/s]
  const double         sqrta,        //!< square root of the semi-major axis                           [m^(1/2)]
  const double         omega0,       //!< longitude of ascending node of orbit plane at weekly epoch   [rad]
  const double         w,            //!< argument of perigee                                          [rad]
  const double         m0,           //!< mean anomaly at reference time                               [rad]
  const double         af0,          //!< polynomial clock correction coefficient (clock bias)         [s],   Note: parameters from ephemeris preferred vs almanac (22 vs 11 bits)
  const double         af1,          //!< polynomial clock correction coefficient (clock drift)        [s/s], Note: parameters from ephemeris preferred vs almanac (16 vs 11 bits)
  double* clock_correction,  //!< clock correction for this satellite for this epoch           [m]
  double* clock_drift,       //!< clock drift correction for this satellite for this epoch     [m/s]
  double* satX,              //!< satellite X position WGS84 ECEF                              [m]
  double* satY,              //!< satellite Y position WGS84 ECEF                              [m]
  double* satZ,              //!< satellite Z position WGS84 ECEF                              [m]
  double* satVx,             //!< satellite X velocity WGS84 ECEF                              [m/s]
  double* satVy,             //!< satellite Y velocity WGS84 ECEF                              [m/s]
  double* satVz,             //!< satellite Z velocity WGS84 ECEF                              [m/s]
  double* azimuth,           //!< satelilte azimuth                                            [rad]
  double* elevation,         //!< satelilte elevation                                          [rad]
  double* doppler            //!< satellite doppler with respect to the user position          [m/s], Note: User must convert to Hz
  )
{
  double tow;        // user time of week adjusted with the clock corrections [s]
  double range;      // range estimate between user and satellite             [m]
  double range_rate; // range_rate esimate between user and satellite         [m/s]
  double x;          // sat X position [m]
  double y;          // sat Y position [m]
  double z;          // sat Z position [m]
  double vx;         // sat X velocity [m/s]
  double vy;         // sat Y velocity [m/s]
  double vz;         // sat Z velocity [m/s]

  unsigned short week; // user week adjusted with the clock correction if needed [week]

  unsigned char i; // counter

  i = (unsigned char)prn; // get rid of a debug msg :)

  // initialize to zero
  x = y = z = vx = vy = vz = 0.0;

  GPS_ComputeSatelliteClockCorrectionAndDrift(
    gpsweek,
    gpstow,
    almanac_week,
    (unsigned)toa,
    (unsigned)toa,
    af0,
    af1,
    0.0,
    ecc,
    sqrta,
    0.0,
    m0,
    0.0,
    0,
    clock_correction,
    clock_drift );

  // adjust for week rollover  
  week = gpsweek;
  tow = gpstow + (*clock_correction)/LIGHTSPEED;
  if( tow < 0.0 )
  {
    tow += SECONDS_IN_WEEK;
    week--;
  }
  if( tow > 604800.0 )
  {
    tow -= SECONDS_IN_WEEK;
    week++;
  }

  // iterate to include the Sagnac correction
  // since the range is unknown, an approximate of 70 ms is good enough 
  // to start the iterations so that 2 iterations are enough
  range = 0.070*LIGHTSPEED; 
  range_rate = 0.0;
  for( i = 0; i < 2; i++ )
  {
    GPS_ComputeSatellitePositionAndVelocity(
      week,
      tow,
      almanac_week,
      (unsigned)toa,
      m0,
      0.0,
      ecc,
      sqrta,
      omega0,
      i0,
      w,
      omegadot,
      0.0,
      0.0,
      0.0,
      0.0,
      0.0,
      0.0,
      0.0,
      range,
      range_rate,
      &x,
      &y,
      &z,
      &vx,
      &vy,
      &vz );

    GPS_ComputeUserToSatelliteRangeAndRangeRate(
      userX,
      userY,
      userZ,
      0.0,
      0.0,
      0.0,
      x,
      y,
      z,
      vx,
      vy,
      vz,
      &range,
      &range_rate );    
  }

  GEODESY_ComputeAzimuthAndElevationAnglesBetweenToPointsInTheEarthFixedFrame(
    GEODESY_REFERENCE_ELLIPSE_WGS84,
    userX,
    userY,
    userZ,
    x,
    y,
    z,
    elevation, // sets the elevation 
    azimuth ); // sets the azimuth

  *satX = x;
  *satY = y;
  *satZ = z;
  *satVx = vx;
  *satVy = vy;
  *satVz = vz;
  
  *doppler = range_rate;

}



void GPS_ComputeSatellitePositionVelocityAzimuthElevationDoppler_BasedOnEphmerisData(
  const double         userX,        //!< user X position WGS84 ECEF  [m]
  const double         userY,        //!< user Y position WGS84 ECEF  [m]
  const double         userZ,        //!< user Z position WGS84 ECEF  [m]
  const unsigned short gpsweek,      //!< gps week of signal transmission (0-1024+)                              [week]
  const double         gpstow,       //!< time of week of signal transmission  (gpstow-psr/c)                    [s]
  const unsigned short ephem_week,   //!< ephemeris: GPS week (0-1024+)                                          [weeks]
  const unsigned       toe,          //!< ephemeris: time of week                                                [s]
  const unsigned       toc,          //!< ephemeris: clock reference time of week                                [s]
  const double         af0,          //!< ephemeris: polynomial clock correction coefficient                     [s],   Note: parameters from ephemeris preferred vs almanac (22 vs 11 bits)
  const double         af1,          //!< ephemeris: polynomial clock correction coefficient                     [s/s], Note: parameters from ephemeris preferred vs almanac (16 vs 11 bits)
  const double         af2,          //!< ephemeris: polynomial clock correction coefficient                     [s/s^2]  
  const double         tgd,          //!< ephemeris: group delay differential between L1 and L2                  [s]
  const double         m0,           //!< ephemeris: mean anomaly at reference time                              [rad]
  const double         delta_n,      //!< ephemeris: mean motion difference from computed value                  [rad/s]
  const double         ecc,          //!< ephemeris: eccentricity                                                []
  const double         sqrta,        //!< ephemeris: square root of the semi-major axis                          [m^(1/2)]
  const double         omega0,       //!< ephemeris: longitude of ascending node of orbit plane at weekly epoch  [rad]
  const double         i0,           //!< ephemeris: inclination angle at reference time                         [rad]
  const double         w,            //!< ephemeris: argument of perigee                                         [rad]
  const double         omegadot,     //!< ephemeris: rate of right ascension                                     [rad/s]
  const double         idot,         //!< ephemeris: rate of inclination angle                                   [rad/s]
  const double         cuc,          //!< ephemeris: amplitude of the cosine harmonic correction term to the argument of latitude  [rad]
  const double         cus,          //!< ephemeris: amplitude of the sine   harmonic correction term to the argument of latitude  [rad]
  const double         crc,          //!< ephemeris: amplitude of the cosine harmonic correction term to the orbit radius          [m]
  const double         crs,          //!< ephemeris: amplitude of the sine   harmonic correction term to the orbit radius          [m]
  const double         cic,          //!< ephemeris: amplitude of the cosine harmonic correction term to the angle of inclination  [rad]
  const double         cis,          //!< ephemeris: amplitude of the sine   harmonic correction term to the angle of inclination  [rad]
  double* clock_correction,  //!< clock correction for this satellite for this epoch           [m]
  double* clock_drift,       //!< clock drift correction for this satellite for this epoch     [m/s]
  double* satX,              //!< satellite X position WGS84 ECEF                              [m]
  double* satY,              //!< satellite Y position WGS84 ECEF                              [m]
  double* satZ,              //!< satellite Z position WGS84 ECEF                              [m]
  double* satVx,             //!< satellite X velocity WGS84 ECEF                              [m/s]
  double* satVy,             //!< satellite Y velocity WGS84 ECEF                              [m/s]
  double* satVz,             //!< satellite Z velocity WGS84 ECEF                              [m/s]
  double* azimuth,           //!< satelilte azimuth                                            [rad]
  double* elevation,         //!< satelilte elevation                                          [rad]
  double* doppler            //!< satellite doppler with respect to the user position          [m/s], Note: User must convert to Hz
  )
{
  double tow;         // user time of week adjusted with the clock corrections [s]
  double range;       // range estimate between user and satellite             [m]
  double range_rate;  // range_rate esimate between user and satellite         [m/s]
  double x;           // sat X position [m]
  double y;           // sat Y position [m]
  double z;           // sat Z position [m]
  double vx;          // sat X velocity [m/s]
  double vy;          // sat Y velocity [m/s]
  double vz;          // sat Z velocity [m/s]

  unsigned short week; // user week adjusted with the clock correction if needed [week]

  unsigned char i; // counter

  // initialize to zero
  x = y = z = vx = vy = vz = 0.0;

  GPS_ComputeSatelliteClockCorrectionAndDrift(
    gpsweek,
    gpstow,
    ephem_week,
    toe,
    toc,
    af0,
    af1,
    af2,
    ecc,
    sqrta,
    delta_n,
    m0,
    tgd,
    0,
    clock_correction,
    clock_drift );

  // adjust for week rollover  
  week = gpsweek;
  tow = gpstow + (*clock_correction)/LIGHTSPEED;
  if( tow < 0.0 )
  {
    tow += SECONDS_IN_WEEK;
    week--;
  }
  if( tow > 604800.0 )
  {
    tow -= SECONDS_IN_WEEK;
    week++;
  }

  // iterate to include the Sagnac correction
  // since the range is unknown, an approximate of 70 ms is good enough to start 
  // the iterations so that 2 iterations are enough for sub mm accuracy
  range = 0.070*LIGHTSPEED; 
  range_rate = 0.0;
  for( i = 0; i < 2; i++ )
  {
    GPS_ComputeSatellitePositionAndVelocity(
      week,
      tow,
      ephem_week,
      toe,
      m0,
      delta_n,
      ecc,
      sqrta,
      omega0,
      i0,
      w,
      omegadot,
      idot,
      cuc,
      cus,
      crc,
      crs,
      cic,
      cis,
      range,
      range_rate,
      &x,
      &y,
      &z,
      &vx,
      &vy,
      &vz );

    GPS_ComputeUserToSatelliteRangeAndRangeRate(
      userX,
      userY,
      userZ,
      0.0,
      0.0,
      0.0,
      x,
      y,
      z,
      vx,
      vy,
      vz,
      &range,
      &range_rate );    
  }

  GEODESY_ComputeAzimuthAndElevationAnglesBetweenToPointsInTheEarthFixedFrame(
    GEODESY_REFERENCE_ELLIPSE_WGS84,
    userX,
    userY,
    userZ,
    x,
    y,
    z,
    elevation, // sets the elevation 
    azimuth ); // sets the azimuth

  *satX = x;
  *satY = y;
  *satZ = z;
  *satVx = vx;
  *satVy = vy;
  *satVz = vz;
  
  *doppler = range_rate;
}


BOOL GPS_DecodeRawGPSEphemeris( 
  const unsigned char subframe1[30],  //!< subframe 1 data, 30 bytes * 8bits/byte = 240 bits, thus parity bits have been removed
  const unsigned char subframe2[30],  //!< subframe 2 data, 30 bytes * 8bits/byte = 240 bits, thus parity bits have been removed
  const unsigned char subframe3[30],  //!< subframe 3 data, 30 bytes * 8bits/byte = 240 bits, thus parity bits have been removed
  unsigned short  prn,                //!< GPS PRN number (helps with debugging)
  unsigned*       tow,                //!< time of week in subframe1, the time of the leading bit edge of subframe 2     [s]
  unsigned short* iodc,               //!< 10 bit issue of data (clock), 8 LSB bits will match the iode                  []    
  unsigned char*  iode,               //!< 8 bit  issue of data (ephemeris)                                              []
  unsigned*       toe,                //!< reference time ephemeris (0-604800)                                           [s]
  unsigned*       toc,                //!< reference time (clock)   (0-604800)                                           [s]      
  unsigned short* week,               //!< 10 bit gps week 0-1023 (user must account for week rollover )                 [week]    
  unsigned char*  health,             //!< 6 bit health parameter, 0 if healthy, unhealth othersize                      [0=healthy]    
  unsigned char*  alert_flag,         //!< 1 = URA may be worse than indicated                                           [0,1]
  unsigned char*  anti_spoof,         //!< anti-spoof flag from 0=off, 1=on                                              [0,1]    
  unsigned char*  code_on_L2,         //!< 0=reserved, 1=P code on L2, 2=C/A on L2                                       [0,1,2]
  unsigned char*  ura,                //!< User Range Accuracy lookup code, 0 is excellent, 15 is use at own risk        [0-15], see p. 83 GPSICD200C
  unsigned char*  L2_P_data_flag,     //!< flag indicating if P is on L2 1=true                                          [0,1]
  unsigned char*  fit_interval_flag,  //!< fit interval flag (four hour interval or longer) 0=4 fours, 1=greater         [0,1]
  unsigned short* age_of_data_offset, //!< age of data offset                                                            [s]
  double* tgd,                //!< group delay                                                                   [s]
  double* af2,                //!< polynomial clock correction coefficient (rate of clock drift)                 [s/s^2]
  double* af1,                //!< polynomial clock correction coefficient (clock drift)                         [s/s]
  double* af0,                //!< polynomial clock correction coefficient (clock bias)                          [s]    
  double* m0,                 //!< mean anomaly at reference time                                                [rad]
  double* delta_n,            //!< mean motion difference from computed value                                    [rad/s]
  double* ecc,                //!< eccentricity                                                                  []
  double* sqrta,              //!< square root of the semi-major axis                                            [m^(1/2)]
  double* omega0,             //!< longitude of ascending node of orbit plane at weekly epoch                    [rad]
  double* i0,                 //!< inclination angle at reference time                                           [rad]
  double* w,                  //!< argument of perigee                                                           [rad]
  double* omegadot,           //!< rate of right ascension                                                       [rad/s]
  double* idot,               //!< rate of inclination angle                                                     [rad/s]
  double* cuc,                //!< amplitude of the cosine harmonic correction term to the argument of latitude  [rad]
  double* cus,                //!< amplitude of the sine harmonic correction term to the argument of latitude    [rad]
  double* crc,                //!< amplitude of the cosine harmonic correction term to the orbit radius          [m]
  double* crs,                //!< amplitude of the sine harmonic correction term to the orbit radius            [m]
  double* cic,                //!< amplitude of the cosine harmonic correction term to the angle of inclination  [rad]
  double* cis                 //!< amplitude of the sine harmonic correction term to the angle of inclination    [rad]
   )
{
/*
------------------------------------------------------------------
                           SUBFRAME1
------------------------------------------------------------------
TERM,               NR BITS,    BITS NO PARITY,   BITS WITH PARITY
preamble,           8,          1-8,              1-8,
TLM,                14,         9-22,             9-22
reserved,           2,          23-24,            23-24
--PARITY--          6,          -----             25-30
TOW,                17,         25-41,            31-47
alert_flag,         1,          42,               48
anti_spoof_flag,    1,          43,               49
subframeID,         3,          44-46,            50-52
parity_related,     2,          47-48,            53-54
--PARITY--          6,          -----             25-30
week,               10,         49-58,            61-70
code_on_L2,         2,          59,60,            71-72
ura,                4,          61-64,            73-76
health,             6,          65-70,            77-82
iodc_MSB,           2,          71-72,            83-84
--PARITY--          6,          -----             85-90
L2_P_data_flag,     1,          73,               91
reserved,           23,         74-96,            92-114
--PARITY--          6,          -----             115-120
reserved,           24,         97-120,           121-144
--PARITY--          6,          -----             25-30
reserved,           24,         121-144,          151-174
--PARITY--          6,          -----             25-30
reserved,           16,         145-160,          181-196
tgd,                8,          161-168,          197-204
--PARITY--          6,          -----             205-210
iodc_LSB,           8,          169-176,          211-218
toc,                16,         177-192,          219-234
--PARITY--          6,          -----             235-240
af2,                8,          193-200,          241-248
af1,                16,         201-216,          249-264
--PARITY--          6,          -----             265-270
af0,                22,         217-238,          271-292
parity_related,     2,          239-240,          293-294
--PARITY--          6,          -----             295-300
------------------------------------------------------------------
                           SUBFRAME2
------------------------------------------------------------------
TERM,               NR BITS,    BITS NO PARITY,   BITS WITH PARITY
preamble,           8,          1-8,              1-8,
TLM,                14,         9-22,             9-22
reserved,           2,          23-24,            23-24
--PARITY--          6,          -----             25-30
TOW,                17,         25-41,            31-47
alert_flag,         1,          42,               48
anti_spoof_flag,    1,          43,               49
subframeID,         3,          44-46,            50-52
parity_related,     2,          47-48,            53-54
--PARITY--          6,          -----             25-30
iode,               8,          49-56,            61-68
crs,                16,         57-72,            69-84
--PARITY--          6,          -----             95-90
delta_n,            16,         73-88,            91-106
m0_MSB,             8,          89-96,            107-114
--PARITY--          6,          -----             115-120
m0_LSB,             24,         97-120,           121-144
--PARITY--          6,          -----             145-150
cuc,                16,         121-136,          151-166
ecc_MSB,            8,          137-144,          167-174
--PARITY--          6,          -----             175-180
ecc_LSB,            24,         145-168,          181-204
--PARITY--          6,          -----             205-210
cus,                16,         169-184,          211-226
sqrta_MSB,          8,          185-192,          227-234
--PARITY--          6,          -----             235-240
sqrta_LSB,          24,         193-216,          241-264
--PARITY--          6,          -----             265-270
toe,                16,         217-232,          271-286
fit_interval_flag,  1,          233,              287
age_of_data_offset, 5,          234-238,          288-292
parity_related,     2,          239-240,          293-294
--PARITY--          6,          -----             295-300
------------------------------------------------------------------
                           SUBFRAME3
------------------------------------------------------------------
TERM,               NR BITS,    BITS NO PARITY,   BITS WITH PARITY
preamble,           8,          1-8,              1-8,
TLM,                14,         9-22,             9-22
reserved,           2,          23-24,            23-24
--PARITY--          6,          -----             25-30
TOW,                17,         25-41,            31-47
alert_flag,         1,          42,               48
anti_spoof_flag,    1,          43,               49
subframeID,         3,          44-46,            50-52
parity_related,     2,          47-48,            53-54
--PARITY--          6,          -----             25-30
cic,                16,         49-64,            61-76
omega0_MSB,         8,          65-72,            77-84
--PARITY--          6,          -----             85-90
omega0_LSB,         24,         73-96,            91-114
--PARITY--          6,          -----             115-120
cis,                16,         97-112,           121-136
i0_MSB,             8,          113-120,          137-144
--PARITY--          6,          -----             145-150
i0_LSB,             24,         121-144,          151-174
--PARITY--          6,          -----             175-180
crc,                16,         145-160,          181-196
w_MSB,              8,          161-168,          197-204
--PARITY--          6,          -----             205-210
w_LSB,              24,         169-192,          211-234
--PARITY--          6,          -----             235-240
omegadot,           24,         193-216,          241-264
--PARITY--          6,          -----             265-270
iode,               8,          217-224,          271-278
idot,               14,         225-238,          279-292
parity_related,     2,          239-240,          293-294
------------------------------------------------------------------
*/
  unsigned char subframe_id;    // subrame id
  unsigned char iode_subframe1; // 8 LSB bits of the iodc in subframe 1
  unsigned char iode_subframe2; // subframe2 iode
  unsigned char iode_subframe3; // subframe3 iode
  
  // temporary variables of different size
  char  s8;
  short s16;
  int   s32;
  unsigned short u16a, u16b;
  unsigned u32a, u32b, u32c, u32d;

  u16a = prn; // gets rid of a debug msg :)
    
  //------------------------------------------------------------------
  //                         SUBFRAME1
  //------------------------------------------------------------------

  // time of week,  actually a 19 bit value, 17 MSBs are available, 2 LSB bits are always zero
  u32a = subframe1[3] << 11;
  u32b = subframe1[4] << 3;
  u32c = (subframe1[5] & 0x80) >> 5;
  *tow = (u32a | u32b | u32c); // [z-count 1.5s intervals]
  *tow = (*tow * 3) / 2; // converted to [s]

  // alert_flag
  *alert_flag = (unsigned char)( (subframe1[5] & 0x40) >> 6 );

  // anti-spoof
  *anti_spoof = (unsigned char)( (subframe1[5] & 0x20) >> 5 );

  // confirm that this is subframe 1  
  subframe_id = (unsigned char)( (subframe1[5] & 0x1C) >> 2 );
  if( subframe_id != 1 )
  {
    GNSS_ERROR_MSG( "if( subframe_id != 1 )" );
    return FALSE;
  }

  // GPS Week
  u16a  = (unsigned short)( subframe1[6] << 2 );
  u16b  = (unsigned short)( subframe1[7] >> 6 );
  *week = (unsigned short)( u16a | u16b );
  
  /// code_on_L2
  *code_on_L2 = (unsigned char)( (subframe1[7] & 0x30) >> 4 );

  // ura
  *ura = (unsigned char)( (subframe1[7] & 0x0F) );

  // health
  *health = (unsigned char)( subframe1[8] >> 2 );

  // issue of data clock
  u16a  = (unsigned short)( (subframe1[8] & 0x03) << 8 );
  u16b  = (unsigned short)( subframe1[21] );
  *iodc = (unsigned short)( u16a | u16b ); // []
  
  // iode subframe1 for consistency checking
  iode_subframe1 = subframe1[21];

  // L2_P_data_flag 
  *L2_P_data_flag = (unsigned char)( (subframe1[9] & 0x80) >> 7 );

  // tgd
  s8   = subframe1[20]; // signed
  *tgd = s8 / TWO_TO_THE_POWER_OF_31;

  // toc
  u16a = (unsigned short)( subframe1[22] << 8 );
  u16b = (unsigned short)( subframe1[23] );
  *toc = (unsigned)( (u16a | u16b) ) * 16;  
  
  // af2
  s8  = subframe1[24]; // signed
  *af2 = s8;
  *af2 /= TWO_TO_THE_POWER_OF_55;
  
  // af1
  u16a = (unsigned short)( subframe1[25] << 8 );
  u16b = subframe1[26];   
  s16 = (unsigned short)( u16a | u16b ); // signed value
  *af1 = s16;
  *af1 /= TWO_TO_THE_POWER_OF_43;
  
  // af0
  u32a = subframe1[27] << 24;
  u32b = subframe1[28] << 16;
  u32c = subframe1[29] & 0xFC;
  u32c <<= 8; // align to the sign bit (two's complement integer)
  u32d = (u32a | u32b | u32c);
  s32 = (int)(u32d);
  s32 >>= 10; // 22 bit value
  *af0  = s32;
  *af0 /= TWO_TO_THE_POWER_OF_31;

  //------------------------------------------------------------------
  //                         SUBFRAME2
  //------------------------------------------------------------------

  // confirm that this is subframe 2
  subframe_id = (unsigned char)( (subframe2[5] & 0x1C) >> 2 );
  if( subframe_id != 2 )
  {
    GNSS_ERROR_MSG( "if( subframe_id != 2 )" );
    return FALSE;
  }

  // iode subframe2
  iode_subframe2 = subframe2[6];

  // crs
  u16a = (unsigned short)( subframe2[7] << 8 );
  u16b = subframe2[8];
  s16  = (unsigned short)( u16a | u16b ); // signed value
  *crs = s16;
  *crs /= 32.0; // [m]

  // delta_n
  u16a = (unsigned short)( subframe2[9] << 8 );
  u16b = subframe2[10];  
  s16  = (short)( u16a | u16b ); // signed value
  *delta_n  = s16;
  *delta_n *= PI / TWO_TO_THE_POWER_OF_43; // [rad/s]

  // m0
  u32a = subframe2[11] << 24;
  u32b = subframe2[12] << 16;
  u32c = subframe2[13] << 8;
  u32d = subframe2[14];
  s32 = (u32a | u32b | u32c | u32d); // signed value
  *m0  = s32;
  *m0 *= PI / TWO_TO_THE_POWER_OF_31; // [rad]

  // cuc
  u16a = (unsigned short)( subframe2[15] << 8 );
  u16b = subframe2[16];
  s16  = (short)( u16a | u16b ); // signed value
  *cuc  = s16;
  *cuc /= TWO_TO_THE_POWER_OF_29; // [rad]
  
  // ecc
  u32a = subframe2[17] << 24;
  u32b = subframe2[18] << 16;
  u32c = subframe2[19] << 8;
  u32d = subframe2[20];
  *ecc  = u32a | u32b | u32c | u32d;
  *ecc /= TWO_TO_THE_POWER_OF_33;  // []

  // cus
  u16a = (unsigned short)( subframe2[21] << 8 );
  u16b = subframe2[22];
  s16  = (short)( u16a | u16b );
  *cus  = s16;
  *cus /= TWO_TO_THE_POWER_OF_29; // [rad]
  
  // sqrta
  u32a = subframe2[23] << 24;
  u32b = subframe2[24] << 16;
  u32c = subframe2[25] << 8;
  u32d = subframe2[26];
  *sqrta = u32a | u32b | u32c | u32d; 
  *sqrta /= TWO_TO_THE_POWER_OF_19; // [sqrt(m)]

  // toe
  u16a = (unsigned short)( subframe2[27] << 8 );
  u16b = subframe2[28];
  *toe = (unsigned)( (u16a | u16b) ) * 16; // [s]

  // fit_interval_flag
  *fit_interval_flag  = (unsigned char)( subframe2[29] >> 7 );

  // age_of_data_offset
  *age_of_data_offset = (unsigned short)( (subframe2[29] & 0x74) >> 2 );
  *age_of_data_offset *= 900; // [s]

  //------------------------------------------------------------------
  //                         SUBFRAME3
  //------------------------------------------------------------------

  // confirm that this is subframe 3
  subframe_id = (unsigned char)( (subframe3[5] & 0x1C) >> 2 );
  if( subframe_id != 3 )
  {
    GNSS_ERROR_MSG( "if( subframe_id != 3 )" );
    return FALSE;
  }

  // cic
  u16a  = (unsigned short)( subframe3[6] << 8 );
  u16b  = subframe3[7];
  s16   = (short)( u16a | u16b ); // signed value
  *cic  = s16;
  *cic /= TWO_TO_THE_POWER_OF_29; // [rad]

  // omego0
  u32a = subframe3[8] << 24;
  u32b = subframe3[9] << 16;
  u32c = subframe3[10] << 8;
  u32d = subframe3[11];
  s32  = u32a | u32b | u32c | u32d; // signed value
  *omega0  = s32;
  *omega0 *= PI / TWO_TO_THE_POWER_OF_31; // [rad]

  // cis
  u16a  = (unsigned short)( subframe3[12] << 8 );
  u16b  = subframe3[13];
  s16   = (short)( u16a | u16b ); // signed value
  *cis  = s16;
  *cis /= TWO_TO_THE_POWER_OF_29; // [rad]

  // i0
  u32a = subframe3[14] << 24;
  u32b = subframe3[15] << 16;
  u32c = subframe3[16] << 8;
  u32d = subframe3[17];
  s32  = u32a | u32b | u32c | u32d;
  *i0  = s32;
  *i0 *= PI / TWO_TO_THE_POWER_OF_31; // [rad]
  
  // crc
  u16a  = (unsigned short)( subframe3[18] << 8 );
  u16b  = subframe3[19];
  s16   = (short)( u16a | u16b ); // signed value
  *crc  = s16;
  *crc /= 32.0; // [m]

  // w
  u32a = subframe3[20] << 24;
  u32b = subframe3[21] << 16;
  u32c = subframe3[22] << 8;
  u32d = subframe3[23];
  s32  = u32a | u32b | u32c | u32d; // signed value
  *w   = s32;
  *w  *= PI / TWO_TO_THE_POWER_OF_31; // [rad]

  // omegadot
  u32a = subframe3[24] << 24;
  u32b = subframe3[25] << 16;
  u32c = subframe3[26] << 8;  
  s32  = u32a | u32b | u32c; // signed value
  s32  = s32 >> 8;
  *omegadot  = s32;
  *omegadot *= PI / TWO_TO_THE_POWER_OF_43; // [rad/s]

  // iode subframe3
  iode_subframe3 = subframe3[27];

  // idot
  u16a  = (unsigned short)( subframe3[28] << 8 );
  u16b  = (unsigned short)( subframe3[29] & 0xFC );
  s16   = (short)( u16a | u16b ); // signed value
  s16   = (short)( s16 >> 2 );
  *idot = s16;
  *idot *= PI / TWO_TO_THE_POWER_OF_43; // [rad/s]  

  // check that the IODE values match for all three subframes  
  if( (iode_subframe1 == iode_subframe2) && (iode_subframe1 == iode_subframe3) )
  {
    *iode = iode_subframe1;
    return TRUE;
  }
  else
  {
    *iode = 0;
    GNSS_ERROR_MSG( "inconsistent subframe dataset" );
    return FALSE; // inconsistent subframe dataset
  }
}


