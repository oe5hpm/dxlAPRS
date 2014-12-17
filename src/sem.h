/**
\file    sem.h
\brief   GNSS core 'c' function library: SEM format almanacs.
\author  Glenn D. MacGougan (GDM)
\date    2006-11-10
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

#ifndef _C_SEM_H_
#define _C_SEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "basictypes.h"


/// \brief  A limited set of satellite orbit parameters that is used to 
///         calculate GPS satellite positions and velocities. In the 
///         ephemeris structure below, the parameters for computing 
///         satellite clock corrections are also included.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2006-11-10
/// \since    2005-08-15
/// 
/// \remarks
/// (1) Struct packaging: compatible with 4 and 8 byte packing \n
/// (2) The af0 and af1 parameters are typically from low precision sources like the true broadcast
///     GPS almanacs. The af0 and af1 parameters are preferred from ephemeris download as they have
///     higher precision. \n
/// 
/// \b REFERENCES \n
/// [1] http://www.navcen.uscg.gov/gps/almanacs.htm
/// 
typedef struct
{
  unsigned       toa;                        //!< almanac time of applicability (reference time)               [s]
  unsigned short week;                       //!< 10 bit gps week 0-1023 (user must account for week rollover) [week]    
  unsigned short prn;                        //!< GPS prn number                                               []  
  unsigned short reserved;                   //!< reserved
  unsigned short svn;                        //!< Satellite vehicle number                                     []
  unsigned char  ura;                        //!< User Range Accuracy lookup code,                             [0-15], see p. 83 GPSICD200C, 0 is excellent, 15 is use at own risk
  unsigned char  health;                     //!< 0=healthy, unhealthy otherwise                               [], subframe 4 and 5, page 25 six-bit health code
  unsigned char  config_code;                //!< configuration code                                           [], if >=9 Anti-Spoofing is on
  
  // this inicator is not part of the SEM standard but is added by the user if known
  unsigned char  is_af0_af1_high_precision;  //!< indicates precision of af0 and af1 [1=high precision,0=low precision] (22&16 bits, ephemeris source) vs (11&11 bits, almanac source), 0 is typical for most SEM sources
    
  double ecc;      //!< eccentricity                                                 []
  double i0;       //!< orbital inclination at reference time                        [rad]
  double omegadot; //!< rate of right ascension                                      [rad/s]
  double sqrta;    //!< square root of the semi-major axis                           [m^(1/2)]
  double omega0;   //!< longitude of ascending node of orbit plane at weekly epoch   [rad]
  double w;        //!< argument of perigee                                          [rad]
  double m0;       //!< mean anomaly at reference time                               [rad]
  double af0;      //!< polynomial clock correction coefficient (clock bias)         [s],   Note: parameters from ephemeris preferred vs almanac (22 vs 11 bits)
  double af1;      //!< polynomial clock correction coefficient (clock drift)        [s/s], Note: parameters from ephemeris preferred vs almanac (16 vs 11 bits)
    
} SEM_structAlmanac;  
  

/// Load an array of SEM format almanac structs from the ASCII file path specified.
///   
/// \author   Glenn D. MacGougan (GDM)
/// \date     2006-11-10
/// \since    2005-08-14
/// \return   TRUE(1) if successful, FALSE(0) otherwise
/// 
BOOL SEM_ReadAlmanacDataFromFile(
  const char* semFilePath,    //!< path to the input SEM ASCII file
  SEM_structAlmanac* alm,     //!< pointer to an array of SEM almanac structs
  unsigned char  max_to_read, //!< length of the array
  unsigned char* number_read  //!< number of almanac items read
  );


/// Write an array of SEM format almanac structs to the ASCII file path specified.
///   
/// \author   Glenn D. MacGougan (GDM)
/// \date     2006-11-10
/// \since    2005-08-15
/// \return   TRUE(1) if successful, FALSE(0) otherwise
/// 
BOOL SEM_WriteAlmanacDataToFile(
  const char* semFilePath,       //!< path to the output SEM ASCII file
  SEM_structAlmanac* alm,        //!< pointer to an array of SEM almanac structs
  unsigned char number_to_write  //!< length of the array
  );


/// Print a SEM format almanac struct to buffer supplied.
///   
/// \author   Glenn D. MacGougan (GDM)
/// \date     2006-11-10
/// \since    2005-08-15
/// \return   TRUE(1) if successful, FALSE(0) otherwise
/// 
BOOL SEM_WriteSingleAlmanacElementToBuffer(
  SEM_structAlmanac alm,    //!< SEM almanac struct
  char* buffer,              //!< buffer to write the SEM struct information
  unsigned short bufferSize  //!< size of the buffer, must be greater than 1024 bytes
  );

#ifdef __cplusplus
}
#endif


#endif // _C_SEM_H_
