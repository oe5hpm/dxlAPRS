/**
\file    gps.h
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

#ifndef _C_GPS_H_
#define _C_GPS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "basictypes.h"


/// \brief    A set of satellite orbit parameters that is used 
/// to calculate GPS satellite positions and velocities. In the ephemeris 
/// structure below, the parameters for computing satellite clock corrections
/// are also included.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-14
/// \since    2005-08-14
/// 
/// \remarks
/// (1) struct packaging: compatible with 4 and 8 byte packing \n
/// 
/// \b REFERENCES \n
/// [1] ICD-GPS-200C, p. 93-102
/// 
typedef struct
{
  unsigned       toe;                //!< reference time ephemeris (0-604800)                                           [s]
  unsigned       toc;                //!< reference time (clock)   (0-604800)                                           [s]    

  unsigned short prn;                //!< GPS PRN number
  unsigned short week;               //!< 10 bit gps week 0-1023 (user must account for week rollover )                 [week]    
  unsigned short iodc;               //!< 10 bit issue of data (clock)                                                  []    
  unsigned short reserved1;          //!< reserved bytes    
  
  unsigned char  iode;               //!< 8 bit  issue of data (ephemeris)                                              []  
  unsigned char  health;             //!< 6 bit health parameter, 0 if healthy, unhealth othersize                      [0=healthy]    
  unsigned char  alert_flag;         //!< 1 = URA may be worse than indicated                                           [0,1]
  unsigned char  anti_spoof;         //!< anti-spoof flag from 0=off, 1=on                                              [0,1]    
  unsigned char  code_on_L2;         //!< 0=reserved, 1=P code on L2, 2=C/A on L2                                       [0,1,2]
  unsigned char  L2_P_data_flag;     //!< flag indicating if P is on L2 1=true                                          [0,1]
  unsigned char  fit_interval_flag;  //!< fit interval flag (four hour interval or longer) 0=4 fours, 1=greater         [0,1]
  unsigned char  ura;                //!< User Range Accuracy lookup code, 0 is excellent, 15 is use at own risk        [0-15], see p. 83 GPSICD200C  
  unsigned short age_of_data_offset; //!< age of data offset                                                            [s]
  unsigned short tow_week;           //!< The week corresponding to tow (0-1024+). Can be one week larger than week.    [week]
  unsigned       tow;                //!< The time of week derived formt the Z-count in the Hand Over Word (HOW)        [s]
  
  // clock parameters
  double         tgd;        //!< group delay                                                                   [s]
  double         af2;        //!< polynomial clock correction coefficient (rate of clock drift)                 [s/s^2]
  double         af1;        //!< polynomial clock correction coefficient (clock drift)                         [s/s] Note: parameters from ephemeris preferred vs almanac (16 vs 11 bits)
  double         af0;        //!< polynomial clock correction coefficient (clock bias)                          [s]   Note: parameters from ephemeris preferred vs almanac (22 vs 11 bits)
  
  // ephemeris parameters
  double         m0;         //!< mean anomaly at reference time                                                [rad]
  double         delta_n;    //!< mean motion difference from computed value                                    [rad/s]
  double         ecc;        //!< eccentricity                                                                  []
  double         sqrta;      //!< square root of the semi-major axis                                            [m^(1/2)]
  double         omega0;     //!< longitude of ascending node of orbit plane at weekly epoch                    [rad]
  double         i0;         //!< inclination angle at reference time                                           [rad]
  double         w;          //!< argument of perigee                                                           [rad]
  double         omegadot;   //!< rate of right ascension                                                       [rad/s]
  double         idot;       //!< rate of inclination angle                                                     [rad/s]
  double         cuc;        //!< amplitude of the cosine harmonic correction term to the argument of latitude  [rad]
  double         cus;        //!< amplitude of the sine harmonic correction term to the argument of latitude    [rad]
  double         crc;        //!< amplitude of the cosine harmonic correction term to the orbit radius          [m]
  double         crs;        //!< amplitude of the sine harmonic correction term to the orbit radius            [m]
  double         cic;        //!< amplitude of the cosine harmonic correction term to the angle of inclination  [rad]
  double         cis;        //!< amplitude of the sine harmonic correction term to the angle of inclination    [rad]

} GPS_structEphemeris;


/// \brief    A limited set of satellite orbit parameters that is used to 
/// calculate rough GPS satellite positions and velocities. The parameters 
/// for computing rough satellite clock corrections are also included.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-18
/// \since    2005-08-18
/// 
/// \remarks
/// (1) struct packaging: compatible with 4 and 8 byte packing
/// 
/// \b REFERENCES \n
/// [1] ICD-GPS-200C, p. 108
/// 
typedef struct
{
  unsigned toa;       //!< almanac time of applicability                               [s]
  unsigned reserved;  //!< reserved                                                    []
  double   ecc;       //!< eccentricity                                                []
  double   i0;        //!< inclination angle at reference time                         [rad]
  double   omegadot;  //!< rate of right ascension                                     [rad/s]
  double   sqrta;     //!< square root of the semi-major axis                          [m^(1/2)]
  double   omega0;    //!< longitude of ascending node of orbit plane at weekly epoch  [rad]
  double   w;         //!< argument of perigee                                         [rad]
  double   m0;        //!< mean anomaly at reference time                              [rad]
  double   af0;       //!< polynomial clock correction coefficient (clock bias)        [s]   Note: parameters from ephemeris preferred vs almanac (22 vs 11 bits)
  double   af1;       //!< polynomial clock correction coefficient (clock drift)       [s/s] Note: parameters from ephemeris preferred vs almanac (16 vs 11 bits)
} GPS_structAlmanac;




/// Computes the satellite clock and clock dirft corrections given the clock model and ephemeris 
/// information.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-12
/// \since    2005-08-12
/// 
/// \remarks
/// (1) The clock correction parameter is sensitive to the polynomial coefficient source which is
///     either ephemeris related from subframe 1 or from the gps almanac (subframes 4,5). The 
///     ephemeris parameters have higher precision 22 & 16 vs 11 & 11 bits. \n
/// (2) User must compensate for the GPS week rollover \n
/// 
/// \b REFERENCES \n
/// [1] ICD-GPS-200C, p. 88-101 \n
/// [2] Teunissen, P. J. G. & A. Kleusberg (editors) (1998). GPS for Geodesy, 2nd Edition. pp. 43-107
/// [3] Hofmann-Wellenhof, B. & J. Collins (1994). GPS Theory and Practice, 3rd Edition. \n
///     Springer-Verlag Wien New York, pp. 43-74 \n
/// [4] Wong, R.V.C & K.P. Schwarz (1985). Dynamic Positioning with an Integrated GPS-INS. Formulae and 
///     Baseline Tests. University of Calgary. UCSE Report #30003. pp. 22 \n
/// 
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
  );


/// Computes the satellite position and velocity in WGS84 based on the supplied ephemeris/almanac
/// parameters. 
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-07-31
/// \since    2005-07-31
///   
/// \b REFERENCES \n
/// [1] ICD-GPS-200C, p. 94-101 \n
/// [2] Teunissen, P. J. G. & A. Kleusberg (editors) (1998). GPS for Geodesy, 2nd Edition. pp. 43-107 \n
/// [3] Hofmann-Wellenhof, B. & J. Collins (1994). GPS Theory and Practice, 3rd Edition. 
///     Springer-Verlag Wien New York, pp. 43-74 \n
/// [4] Wong, R.V.C & K.P. Schwarz (1985). Dynamic Positioning with an Integrated GPS-INS. Formulae and 
///     Baseline Tests. University of Calgary. UCSE Report #30003. pp. 22 \n
/// 
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
  );


/// Computes the user to satellite range given the user and 
/// satellite position.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2006-11-23
/// \since    2006-11-23
/// 
void GPS_ComputeUserToSatelliteRange( 
  const double userX,    //!< user X position WGS84 ECEF         [m]
  const double userY,    //!< user Y position WGS84 ECEF         [m]
  const double userZ,    //!< user Z position WGS84 ECEF         [m]
  const double satX,     //!< satellite X position WGS84 ECEF    [m]
  const double satY,     //!< satellite Y positoin WGS84 ECEF    [m]
  const double satZ,     //!< satellite Z position WGS84 ECEF    [m] 
  double* range          //!< user to satellite range            [m]
  );


/// Computes the user to satellite range and range rate given the user and satellite
/// position and velocities.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-15
/// \since    2005-08-15
/// 
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
  );


/// Computes the satellite position and velocity in WGS84 based on almanac data.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-15
/// \since    2005-08-15
/// 
/// \remarks
/// (1) Assumes L1 for the clock correction mode \n
/// (2) This calculation solves for the satellite position at the true GPS time of week specified. 
///     It includes compensation for the satellite clock correction and the Saganc effect, which
///     is a function of the user position. \n
/// 
/// \b REFERENCES \n
/// [1] ICD-GPS-200C
/// 
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
  );


/// Computes the satellite position and velocity in WGS84 based on ephemeris data.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-15
/// \since    2005-08-15
/// 
/// \remarks
/// (1) Assumes L1 for the clock correction mode \n
/// (2) This calculation solves for the satellite position at the true GPS time of week specified. 
///     It includes compensation for the satellite clock correction and the Saganc effect, which
///     is a function of the user position. \n
/// (3) This function can be called with almanac data by inputting the almanac information and 
///     zero for the terms not available. toe and toc are the toa in that case. \n
/// 
/// \b REFERENCES \n
/// [1] ICD-GPS-200C
/// 
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
  );



/// Decodes the raw gps ephemeris (note, with the parity bits removed).
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-15
/// \since    2005-08-15
/// 
/// \returns  TRUE(1) if successful, FALSE(0) otherwise
/// 
/// \remarks
/// (1) Parity bits must be removed from the input subframe data \n
/// (2) Returns TRUE(1) if data is successfully decoded \n
/// (3) Returns FALSE(0) if the subframe id's are incorrect, i.e. must be subframe1, subframe2, subframe3 in that order \n
/// (4) Returns FALSE(0) if the iode's are not consistent for all subframes, note 8 LSB bits of the iodc should equal  
///     subframe2 and subframe3 iode \n
/// (5) Uses IBM PC format byte alignment \n
/// 
/// \b REFERENCES \n
/// [1] ICD-GPS-200C
/// 
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
  );


#ifdef __cplusplus
}
#endif


#endif // _C_GPS_H_
