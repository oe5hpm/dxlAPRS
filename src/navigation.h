/**
\file    navigation.h
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

#ifndef _C_NAVIGATION_H_
#define _C_NAVIGATION_H_

#ifdef __cplusplus
extern "C" {
#endif


/// \brief    Computes the derivative of the pseudorange with respect 
///           to X, Y, Z. Also computes the user to satellite range for 
///           convenience.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-23
/// \since    2005-08-23
/// 
void NAVIGATION_ComputeDerivativesOf_Range_WithRespectTo_XYZ( 
  const double x,     //!< User X coordinate WGS84 ECEF      [m]
  const double y,     //!< User Y coordinate WGS84 ECEF      [m]
  const double z,     //!< User Z coordinate WGS84 ECEF      [m]
  const double satX,  //!< Satellite X coordinate WGS84 ECEF [m]
  const double satY,  //!< Satellite Y coordinate WGS84 ECEF [m]
  const double satZ,  //!< Satellite Z coordinate WGS84 ECEF [m]  
  double* dPdx,       //!< Derivative of P wrt X             []
  double* dPdy,       //!< Derivative of P wrt Y             []
  double* dPdz,       //!< Derivative of P wrt Z             []    
  double* range       //!< computed user to satellite range  [m]  
  );


/// \brief    Computes the derivative of the pseudorange with respect 
///           to latitude, longitude, and height/ Also computes the user 
///           to satellite range for convenience.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-23
/// \since    2005-08-23
/// 
void NAVIGATION_ComputeDerivativesOf_Range_WithRespectToLatitudeLongitudeHeight( 
  const double latitude,  //!< User geodetic latitude                   [rad]
  const double longitude, //!< User geodetic longtiude                  [rad]
  const double height,    //!< User geodetic height                     [m]
  const double satX,      //!< Satellite X coordinate WGS84 ECEF        [m]
  const double satY,      //!< Satellite Y coordinate WGS84 ECEF        [m]
  const double satZ,      //!< Satellite Z coordinate WGS84 ECEF        [m]  
  double* dlat,           //!< d(P)/d(lat) but not in units of [m/rad], [m/m]
  double* dlon,           //!< d(P)/d(lon) but not in units of [m/rad], [m/m]
  double* dhgt,           //!< d(P)/d(hgt)                              [m/m]
  double* range           //!< computed user to satellite range         [m]
  );




/// \brief    Compute a closed form position solution using four 
///           raw pseudoranges.
/// 
/// \author   Glenn D. MacGougan (GDM)
/// \date     2005-08-23
/// \since    2005-08-23
/// 
/// \return   TRUE(1) if successful, FALSE(0) otherwise.
///
/// \remarks
/// (1) The satellite clock corrections in meters are required for each pseudorange \n
/// (2) This function was written for easy portability into Matlab
/// 
/// \b REFERENCES \n
/// [1] Mezentsev, O. and J. Collin (2004). A Closed Form Solution Of Non-Linear GPS 
///     Pseudorange Equations. White paper proof - not published.
/// 
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
  );

#ifdef __cplusplus
}
#endif


#endif // _C_NAVIGATION_H_
