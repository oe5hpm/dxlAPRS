/**
\file    yuma.h
\brief   GNSS core 'c' function library: YUMA format almanacs.
\author  Glenn D. MacGougan (GDM)
\date    2007-11-29
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

#ifndef _C_YUMA_H_
#define _C_YUMA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "basictypes.h"


/// \brief    A limited set of satellite orbit parameters that is used to calculate GPS satellite 
/// positions and velocities. In the ephemeris structure below, the parameters for computing 
/// satellite clock corrections are also included.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-14
/// \since    2005-08-14
/// 
/// \remarks
/// (1) Struct packaging: compatible with 4 and 8 byte packing \n
/// (2) The af0 and af1 parameters are typically from low precision sources like the true broadcast
///     GPS almanacs. The af0 and af1 parameters are preferred from ephemeris download as they have
///     higher precision. \n
/// 
/// \b REFERENCES \n
/// [1] http://www.navcen.uscg.gov/gps/almanacs.htm \n
/// 
typedef struct
{
  unsigned short    reserved1;       //!< reserved
  unsigned short    week;            //!< 10 bit gps week 0-1023 (user must account for week rollover) [week]    
  unsigned short    prn;             //!< GPS prn number                                               []
  unsigned char     health;          //!< 0=healthy, unhealthy otherwise                               []
  
  /// This indicates precision of af0 and af1.
  /// [1=high precision,0=low precision] (22&16 bits, ephemeris source) vs (11&11 bits, almanac source)
  /// 0 is typical for most YUMA sources.
  /// This inicator is not part of the standard but is added by the user if known.
  unsigned char     is_af0_af1_high_precision;
  
  double ecc;                        //!< eccentricity                                                 []
  double toa;                        //!< time of applicability                                        [s]
  double i0;                         //!< orbital inclination at reference time                        [rad]
  double omegadot;                   //!< rate of right ascension                                      [rad/s]
  double sqrta;                      //!< square root of the semi-major axis                           [m^(1/2)]
  double omega0;                     //!< longitude of ascending node of orbit plane at weekly epoch   [rad]
  double w;                          //!< argument of perigee                                          [rad]
  double m0;                         //!< mean anomaly at reference time                               [rad]
  double af0;                        //!< polynomial clock correction coefficient (clock bias)         [s],   Note: parameters from ephemeris preferred vs almanac (22 vs 11 bits)
  double af1;                        //!< polynomial clock correction coefficient (clock drift)        [s/s], Note: parameters from ephemeris preferred vs almanac (16 vs 11 bits)

} YUMA_structAlmanac;  
  

/// \brief    Load an array of YUMA format almanac structs from the ASCII file path specified.
///   
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-14
/// \since    2005-08-14
/// \return   TRUE(1) if successful, FALSE(0) otherwise
/// 
BOOL YUMA_ReadAlmanacDataFromFile(
  const char* yumaFilePath,   //!< path to the input YUMA ASCII file
  YUMA_structAlmanac* alm,    //!< pointer to an array of YUMA almanac structs
  unsigned char  max_to_read, //!< length of the array
  unsigned char* number_read  //!< number of almanac items read
  );


/// \brief    Write an array of YUMA format almanac structs to the ASCII file path specified.
///   
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-14
/// \since    2005-08-14
/// \return   TRUE(1) if successful, FALSE(0) otherwise
/// 
BOOL YUMA_WriteAlmanacDataToFile(
  const char* yumaFilePath,      //!< path to the output YUMA ASCII file
  YUMA_structAlmanac* alm,       //!< pointer to an array of YUMA almanac structs
  unsigned char number_to_write  //!< length of the array
  );


/// \brief    Print a YUMA format almanac struct to buffer supplied.
///   
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-15
/// \since    2005-08-15
/// \return   TRUE(1) if successful, FALSE(0) otherwise
/// 
BOOL YUMA_WriteSingleAlmanacElementToBuffer(
  YUMA_structAlmanac alm,   //!< YUMA almanac struct
  char* buffer,             //!< buffer to write the YUMA struct information
  unsigned short bufferSize //!< size of the buffer, must be greater than 1024 bytes
  );

#ifdef __cplusplus
}
#endif


#endif // _C_YUMA_H_
