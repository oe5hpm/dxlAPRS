/**
\file    gnss_types.h
\brief   Basic gnss types.

\author  Glenn D. MacGougan (GDM)
\date    2007-12-05
\since   2006-11-13

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

#ifndef _GNSS_TYPES_H_
#define _GNSS_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif



/// \brief    These are the supported receiver data types.
typedef enum GNSS_enumRxDataType
{
  GNSS_RXDATA_NOVATELOEM4 = 0, //!< NovAtel OEM4 data.
  GNSS_RXDATA_RINEX21     = 1, //!< RINEX version 2.1
  GNSS_RXDATA_RINEX211    = 2, //!< RINEX version 2.11
  GNSS_RXDATA_UNKNOWN
};
  

/// A 32 bit bit-field for GNSS measurement associated flags (all are boolean indicators).
typedef struct 
{
  unsigned isActive:1;                //!< This flag indicates that the channel is active for use. If this is not set, no other flags are valid for use.
  unsigned isCodeLocked:1;            //!< Indicates if the code tracking is locked.
  unsigned isPhaseLocked:1;           //!< Indicates if the phase tracking is locked.
  unsigned isParityValid:1;           //!< Indicates if the phase parity if valid.      
  unsigned isPsrValid:1;              //!< Indicates if the pseudorange valid for use.
  unsigned isAdrValid:1;              //!< Indicates if the ADR is valid for use.
  unsigned isDopplerValid:1;          //!< Indicates if the Doppler if valid for use.
  unsigned isGrouped:1;               //!< Indicates if this channel has another associated channel. eg. L1 and L2 measurements.
  unsigned isAutoAssigned:1;          //!< Indicates if the channel was receiver assigned (otherwise, the user forced this channel assignment).
  unsigned isCarrierSmoothed:1;       //!< Indicates if the pseudorange has carrier smoothing enabled.
  unsigned isEphemerisValid:1;        //!< Indicates if this channel has valid associated ephemeris information. 
  unsigned isAlmanacValid:1;          //!< Indicates if this channel has valid associated almanac information.
  unsigned isAboveElevationMask:1;    //!< Indicates if the satellite tracked is above the elevation mask.    
  unsigned isAboveCNoMask:1;          //!< Indciates if the channel's C/No is above a threshold value.
  unsigned isAboveLockTimeMask:1;     //!< Indicates if the channel's locktime is above a treshold value.
  unsigned isNotUserRejected:1;       //!< Indicates if the user has not forced the rejection of this channel or PRN.
  unsigned isNotPsrRejected:1;        //!< Indicates if the pseudorange was not rejetced (ie Fault Detection and Exclusion).
  unsigned isNotAdrRejected:1;        //!< Indicates if the ADR was not rejetced (ie Fault Detection and Exclusion).
  unsigned isNotDopplerRejected:1;    //!< Indicates if the Doppler was not rejected (ie Fault Detection and Exclusion).
  unsigned isNoCycleSlipDetected:1;   //!< Indicates that no cycle slip has occurred at this epoch.
  unsigned isPsrUsedInSolution:1;     //!< Indicates if some part (pseudorange) of this channel's measurement was used in the position solution.
  unsigned isDopplerUsedInSolution:1; //!< Indicates if some part (Doppler) of this channel's measurement was used in the velocity solution.
  unsigned isAdrUsedInSolution:1;     //!< Indicates if the the ADR is used in the solution.

  unsigned isDifferentialPsrAvailable:1;     //!< Indicates if a matching pseudrange observation is available from another receiver.
  unsigned isDifferentialDopplerAvailable:1; //!< Indicates if a matching Doppler observation is available from another receiver.
  unsigned isDifferentialAdrAvailable:1;     //!< Indicates if a matching ADR observation is available from another receiver.

  unsigned useTropoCorrection:1;         //!< Indicates that the tropospheric correction should be applied.
  unsigned useBroadcastIonoCorrection:1; //!< Indicates that the broadcast ionospheric correction should be applied.

  unsigned isBaseSatellite:1; //!< Indicates if this channel corresponds to a base satellite used in double differencing.

  unsigned isNotPsrDeweighted:1;
  unsigned isNotDopplerDeweighted:1;
  unsigned isNotAdrDeweighted:1;

} GNSS_structFlagsBitField;

/// \brief    An enumerated for a GNSS code modulation type.
///
/// This enumerated uses the NovAtel OEM4 convention.
typedef enum 
{
  GNSS_CACode     = 0,
  GNSS_PCode      = 1,
  GNSS_PCodeless  = 2,
  GNSS_UWBCodeType = 1614,
  GNSS_ReservedCodeType 
} GNSS_enumCodeType;

/// \brief    An enumeration for a GNSS frequency type.
///
/// This enumerated uses the NovAtel OEM4 convention.  
typedef enum 
{
  GNSS_GPSL1 = 0,
  GNSS_GPSL2 = 1,       
  GNSS_UWBFrequency = 1615,
  GNSS_ReservedFrequency
} GNSS_enumFrequency;

/// \brief    An enumeration for a GNSS system type.
///
/// This enumerated uses the NovAtel OEM4 convention.    
typedef enum 
{
  GNSS_GPS        = 0,
  GNSS_GLONASS    = 1,
  GNSS_WAAS       = 2,
  GNSS_Pseudolite = 3,
  GNSS_UWBSystem = 1616,  
  GNSS_ReservedSystem
} GNSS_enumSystem;


/// \brief    An enumeration for a GNSS measurement type.
typedef enum 
{
  GNSS_INVALID_MEASUREMENT    = 0,
  GNSS_PSR_MEASUREMENT        = 1,
  GNSS_DOPPLER_MEASUREMENT    = 2,
  GNSS_ADR_MEASUREMENT        = 3,  
  GNSS_LAT_CONSTRAINT         = 4,
  GNSS_LON_CONSTRAINT         = 5,
  GNSS_HGT_CONSTRAINT         = 6,
  GNSS_VN_CONSTRAINT          = 7,
  GNSS_VE_CONSTRAINT          = 8,
  GNSS_VUP_CONSTRAINT         = 9,  
  GNSS_RESERVED_MEASUREMENT_TYPE
} GNSS_enumMeasurementType;



/// \brief    Correction data associated with a single receiver measurement channel.
///           PRC is PseudRange Correction, RRC is RangeRate Correction.
///
/// This struct is 4 and 8 byte struct member packing compatible.
typedef struct 
{
  float prcTropoDry;      //!< Tropospheric dry delay correction (+ve is a delay) [m].
  float prcTropoWet;      //!< Tropospheric wet delay correction (+ve is a delay) [m].
  float prcIono;          //!< Ionospheric correction (+ve is a delay) [m].
  float prcSatClk;        //!< The satellite clock correction (+ve is a delay) [m].
  float prcReserved1;     //!< A reserved range correction for the pseudorange [m].
  float prcReserved2;     //!< A reserved range correction for the pseudorange [m].
  float rrcSatClkDrift;   //!< The satellite clock drift rate correction (+ve is a rate delay) [m/s].
  float rrcReserved1;     //!< A reserved range rate correction [m/s].
  float rrcReserved2;     //!< A reserved range rate correction [m/s].
  float dX;               //!< This channel's associated satellite orbital correction X (ECEF) [m].
  float dY;               //!< This channel's associated satellite orbital correction X (ECEF) [m].
  float dZ;               //!< This channel's associated satellite orbital correction X (ECEF) [m].    
} GNSS_structCorrections;    

/// \brief    Residuals data associated with a single receiver measurement channel.
///
/// This struct is 4 and 8 byte struct member packing compatible.
typedef struct 
{
  float psrResidual;      //!< The pseudorange residual [m].
  float adrResidual;      //!< The accumulated Doppler range residual [cycles].
  float dopplerResidual;  //!< The Doppler residual [Hz].    
  float reserved;         //!< Reserved value.
} GNSS_structResiduals;  

/// \brief    A struct for satelite data.
///
/// This struct is 4 and 8 byte struct member packing compatible.
typedef struct 
{
  int isValid;      //!< This indicates if the pvt information is valid [1=valid, 0=invalid].
  int ageOfEph;     //!< The difference between the current measurement time and the time of ephemeris.
  double x;         //!< The satellite position ECEF, x [m].
  double y;         //!< The satellite position ECEF, y [m].
  double z;         //!< The satellite position ECEF, z [m].
  double vx;        //!< The satellite velocity ECEF, vx [m].
  double vy;        //!< The satellite velocity ECEF, vy [m].
  double vz;        //!< The satellite velocity ECEF, vz [m].
  double clk;        //!< The satellite clock correction [m].
  double clkdrift;   //!< The satellite clock drift correction [m/s].
  double elevation;  //!< The elevation angle between the user and the satellite [rad].
  double azimuth;    //!< The azimuth angle between the user and the satellite, measured clockwise positive from the true North [rad].   
  double doppler;   //!< The satellite doppler with respect to the user position [m/s], Note: User must convert to Hz.
} GNSS_structSatellitePVT;


/// \brief    A container for klobuchar sytle ionospheric parameters.
///
/// This struct is 4 and 8 byte struct member packing compatible.  
typedef struct 
{
  unsigned short isValid; //!< Is this structure valid for use 1=YES, 0=NO.
  unsigned short week;    //!< The GPS week corresponding to the correction parameters [weeks].
  unsigned  tow;          //!< The GPS time of week corresponding to the correction parameters [s].
  double    alpha0;       //!< coefficients of a cubic equation representing the amplitude of the vertical delay [s]
  double    alpha1;       //!< coefficients of a cubic equation representing the amplitude of the vertical delay [s/semi-circle]
  double    alpha2;       //!< coefficients of a cubic equation representing the amplitude of the vertical delay [s/semi-circle^2]
  double    alpha3;       //!< coefficients of a cubic equation representing the amplitude of the vertical delay [s/semi-circle^3]  
  double    beta0;        //!< coefficients of a cubic equation representing the period of the model [s]
  double    beta1;        //!< coefficients of a cubic equation representing the period of the model [s/semi-circle]
  double    beta2;        //!< coefficients of a cubic equation representing the period of the model [s/semi-circle^2]
  double    beta3;        //!< coefficients of a cubic equation representing the period of the model [s/semi-circle^3]
} GNSS_structKlobuchar;

/// \brief    Measurement data associated with a single receiver measurement channel.
///
/// This struct is 4 and 8 byte struct member packing compatible.
typedef struct 
{
  unsigned short            channel;  //!< The channel number associated with this measurement.
  unsigned short            id;       //!< The unique id for this channel (eg PRN for GPS).    
  GNSS_enumSystem           system;   //!< The satellite system associated with this channel.
  GNSS_enumCodeType         codeType; //!< The code type for this channel.
  GNSS_enumFrequency        freqType; //!< The frequency type for this channel.
  GNSS_structFlagsBitField  flags;    //!< The flags associated with this channel. ie Validity flags, etc.
  short reserved1;  
  unsigned short week;  //!< The measurement gps week (at 'transmit' time) [weeks].
  double   tow;         //!< The measurement gps time of week (at 'transmit' time) [s].

  // The actual measurements.
  double   psr;               //!< The pseudorange measurement [m].
  double   adr;               //!< The carrier phase or accumulated Doppler range measurement [cycles].
  float    doppler;           //!< The Doppler measurement for this channel [Hz].
  float    cno;               //!< The carrier to noise density ratio for this channel [dB-Hz]
  float    locktime;          //!< The number of seconds of continous phase tracking (no known cycle slips) [s].

  // The variance information associated with the actual measurements.
  float    stdev_psr;         //!< The estimated pseudorange measurement standard deviation [m].
  float    stdev_adr;         //!< The estimated accumulated Doppler range measurement standard deviation [cycles].
  float    stdev_doppler;     //!< The estimated Doppler measurement standard deviation [Hz].

  // Derived information.
  double   psr_misclosure;     //!< The measured psr minus the computed psr estimate [m].
  double   doppler_misclosure; //!< The measured Doppler minus the computed Doppler estimate [m/s].
  double   range;              //!< The best estimate of the geometric range between the antenna and the satellite [m].
  double   rangerate;          //!< The best estimate of the geometric range rate between the antenna and the satellite [m/s].
  double   psr_smoothed;       //!< The carrier smoothed pseudorange if available [m].
  double   psr_predicted;      //!< The predicted pseudorange based on the satellite position, user position, and current clock offset [m].
  double   sd_ambiguity;       //!< The estimated single difference float ambiguity [m].
  double   dd_ambiguity;       //!< The estimated double difference float ambiguity [m].
  double   dd_ambiguity_fixed; //!< The estimated double difference fixed ambiguity [m].
  float    doppler_predicted;  //!< The predicted Doppler based on user position, velocity, satellite position, velocity and clock rate [Hz].
  
  double   psr_misclosure_lsq;     //!< The measured psr minus the computed psr estimate using least squares [m].
  double   doppler_misclosure_lsq; //!< The measured Doppler minus the computed Doppler estimate using least squares [m/s].
  
  short index_differential;      //!< The channel index of a matching differential observation. -1 means there is no matching channel.
  short index_time_differential; //!< The channel index of a matching time differential observation. -1 means there is no matching channel.

  short index_between_satellite_differential; //!< The channel index for between satellite difference of the satellite that is substracted from this one. -1 means there is no matching channel.
  short index_ambiguity_state;    //!< The index into the state vector for the float ambiguity state. -1 not estimated.
  short index_ambiguity_state_dd; //!< The index into the state vector for the double difference ambiguity state. -1 not estimated.
  short index_psr_B;              //!< A very convenient index into the B matrix used in double differencing. -1 if not valid.
  short index_Doppler_B;          //!< A very convenient index into the B matrix used in double differencing. -1 if not valid.
  short index_adr_B;              //!< A very convenient index into the B matrix used in double differencing. -1 if not valid.
  
  double adr_misclosure;         //!< The measured ADR minus the computed ADR estimate [m]. This is the between receiver differential adr misclosure.

  double adr_residual_sd;       //!< The measured single difference ADR minus the computed single difference ADR estimate after full sequential update.
  double adr_residual_dd;       //!< The measured double difference ADR minus the computed double difference ADR estimate after full sequential update (float solution).
  double adr_residual_dd_fixed; //!< The measured double difference ADR minus the computed double difference ADR estimate after full sequential update and ambiguity fixing.
  double adr_misclosure_temp;   //!< A temporary variable used to compute adr_misclosure_dd above/

  double sd_doppler_covar;       //!< Single difference doppler covariance.
  double sd_doppler_innovation;  //!< Single difference doppler innovation.
  double sd_doppler_innovation_standardized;  //!< A standardized single difference doppler innovation.

  double sd_psr_covar;       //!< Single difference psr covariance.
  double sd_psr_innovation;  //!< Single difference psr innovation.
  double sd_psr_innovation_standardized;  //!< A standardized single difference psr innovation.

  double sd_adr_covar;       //!< Single difference adr covariance.
  double sd_adr_innovation;  //!< Single difference adr innovation.
  double sd_adr_innovation_standardized;  //!< A standardized single difference adr innovation.

  double uwb_bias;
  double uwb_scale_factor;

  double H_p[3]; //!< The design matrix row relating the pseudorange measurements to the position solution. dP/d(lat), dP/d(lon), dP/d(hgt).
  double H_a[3]; //!< The design matrix row relating the adr measurements to the position solution. dP/d(lat), dP/d(lon), dP/d(hgt).
  double H_v[3]; //!< The design matrix row relating the Doppler measurements to the velocity solution. dD/d(lat), dD/d(lon), dD/d(hgt).

  GNSS_structCorrections  corrections; //!< The corrections associated with this channel.
  GNSS_structResiduals    residuals;   //!< The post-adjustment (filtering) measurement residual associated with this channel.

  GNSS_structSatellitePVT satellite;  //!< Each channel has an associated satellite pvt struct evaluated at the transmit time.
} GNSS_structMeasurement;




/// \brief    A struct for holding dilution of precision information.
///
/// This struct is 4 and 8 byte struct member packing compatible.
typedef struct 
{
  float gdop;     //!< geometric  dilution of precision [].
  float pdop;     //!< position   dilution of precision [].
  float ndop;     //!< northing   dilution of precision [].
  float edop;     //!< easting    dilution of precision [].
  float hdop;     //!< horizontal dilution of precision []. 
  float vdop;     //!< vertical   dilution of precision [].
  float tdop;     //!< time       dilution of precision [].
  float reserved; //!< reserved   dilution of precision [].
} GNSS_structDOP;

/// \brief    A container for time information.
///
/// This struct is 4 and 8 byte struct member packing compatible.
typedef struct 
{
  unsigned short  day_of_year;  //!< The number of days into the year (1-366) [days].
  unsigned short  utc_year;     //!< Universal Time Coordinated    [year]
  unsigned char   utc_month;    //!< Universal Time Coordinated    [1-12 months] 
  unsigned char   utc_day;      //!< Universal Time Coordinated    [1-31 days]
  unsigned char   utc_hour;     //!< Universal Time Coordinated    [hours]
  unsigned char   utc_minute;   //!< Universal Time Coordinated    [minutes]
  float           utc_seconds;  //!< Universal Time Coordinated    [s]      
  unsigned int    gps_week;     //!< The current GPS week associated with the observations [weeks]. 
  double          gps_tow;      //!< The GPS time of week (0-604800.0) (receiver time of observation) [s].      
} GNSS_structReceiverTime;



/// \brief    A struct for degrees, minutes, seconds, and a string.
///
/// This struct is 4 and 8 byte struct member packing compatible.
typedef struct 
{
  short  degrees;      //!< The degrees component [deg].
  short  minutes;      //!< The minutes component [min].
  float  seconds;      //!< The seconds component [s]
  char   dms_str[24];  //!< A DMS string e.g. dms_str = "-180'59'59.9999\""
} GNSS_structDMS;


/// \brief    A struct for holding position, velocity and time.
///
/// This struct is 4 and 8 byte struct member packing compatible.  
typedef struct 
{
  GNSS_structReceiverTime time; // The receiver time information.

  double  pos_apvf;       //!< The a-posteriori variance factor for the position solution.

  double  latitude;       //!< The user latitude [rad].
  double  longitude;      //!< The user longitude [rad].
  double  height;         //!< The user orthometric height [m].
  double  undulation;     //!< The undulation [m].

  double  latitudeDegs;    //!< The user latitude  [degrees].
  double  longitudeDegs;   //!< The user longitude [degrees].
  GNSS_structDMS lat_dms; //!< The user latitude  [dms].  
  GNSS_structDMS lon_dms; //!< The user longitude [dms].    

  double  x;              //!< The user's ECEF position, X [m].
  double  y;              //!< The user's ECEF position, Y [m].
  double  z;              //!< The user's ECEF position, Z [m].

  double  vel_apvf;       //!< The a-posteriori variance factor for the velocity solution.

  double  vn;             //!< The user's local geodetic velocity, velocity North [m/s].
  double  ve;             //!< The user's local geodetic velocity, velocity East [m/s].
  double  vup;            //!< The user's local geodetic velocity, velocity Up [m/s].

  double  vx;             //!< The user's ECEF velocity, vx [m/s].
  double  vy;             //!< The user's ECEF velocity, vy [m/s].
  double  vz;             //!< The user's ECEF velocity, vz [m/s].

  double  clockOffset;    //!< The receiver clock offset [m].
  double  clockDrift;     //!< the receiver clock drift  [m/s].

  GNSS_structDOP dop;     //!< All the associated DOP information for this solution.

  double std_lat;        //!< The standard deviation uncertainty in the latitude [m].
  double std_lon;        //!< The standard deviation uncertainty in the longitude [m]. 
  double std_hgt;        //!< The standard deviation uncertainty in the height [m].
  double std_vn;         //!< The standard deviation uncertainty in the northing velocity [m/s].
  double std_ve;         //!< The standard deviation uncertainty in the easting velocity [m/s].
  double std_vup;        //!< The standard deviation uncertainty in the up velocity [m/s].
  double std_clk;        //!< The standard deviation uncertainty in the clock offset [m].
  double std_clkdrift;   //!< The standard deviation uncertainty in the clock drift [m/s].

  unsigned char didGlobalTestPassForPosition;  //!< Does the position solution pass the Chi^2 goodness of fit test [TRUE(1),FALSE(0)].
  unsigned char didGlobalTestPassForVelocity;  //!< Does the velocity solution pass the Chi^2 goodness of fit test [TRUE(1),FALSE(0)].

  unsigned char isPositionFixed;            //!< Is this a position solution fully constrained to a fixed point [TRUE(1),FALSE(0)].
  unsigned char isHeightConstrained;        //!< Is this a height constrained solution   [TRUE(1),FALSE(0)].
  unsigned char isClockConstrained;         //!< Is this a clock constrained solution    [TRUE(1),FALSE(0)].
  unsigned char isSolutionBasedOnEphemeris; //!< Is the solution based on valid ephemeris information [TRUE(1),FALSE(0)].
  
  unsigned char nrPsrObsAvailable;      //!< This indicates the number of valid pseudorange observations valid before adjustment.
  unsigned char nrPsrObsUsed;           //!< This indicates the number of valid pseudorange observations valid after adjustment.
  unsigned char nrPsrObsRejected;       //!< This indicates the number of valid pseudorange observations rejected during adjustment.

  unsigned char nrAdrObsAvailable;      //!< This indicates the number of valid ADR observations valid before adjustment.
  unsigned char nrAdrObsUsed;           //!< This indicates the number of valid ADR observations valid after adjustment.
  unsigned char nrAdrObsRejected;       //!< This indicates the number of valid ADR observations rejected during adjustment.

  unsigned char nrDopplerObsAvailable;  //!< This indicates the number of valid Doppler observations valid before adjustment.
  unsigned char nrDopplerObsUsed;       //!< This indicates the number of valid Doppler observations valid after adjustment.
  unsigned char nrDopplerObsRejected;   //!< This indicates the number of valid Doppler observations rejected during adjustment.

  unsigned char reserved[1];                //!< Space for reserved flags.
  
} GNSS_structPVT;

#ifdef __cplusplus
}
#endif


#endif // _GNSS_TYPES_H_


