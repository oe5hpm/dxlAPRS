/**
\file    rinex.c
\brief   GNSS core 'c' function library: RINEX VERSION 2.11 related functions.
\author  Glenn D. MacGougan (GDM)
\date    2007-12-03
\since   2007-12-02

\b REFERENCES \n
- http://www.aiub-download.unibe.ch/rinex/rinex211.txt

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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#include <math.h>
#include "rinex.h"
#include "gnss_error.h"
#include "time_conversion.h"
#include "constants.h"

#define RINEX_HEADER_SIZE (32768) //!< The maximum size of a RINEX header buffer [bytes].
#define RINEX_LINEBUF_SIZE (8192) //!< The maximum size of a string used in RINEX decoding [bytes].
#define RINEX_MAX_NR_SATS    (64) //!< The maximum array size for "struct_RINEX_satellite RINEX_sat[RINEX_MAX_NR_SATS]".
#define RINEX_MAX_NR_OBS     (64) //!< The maximum array size for "struct_RINEX_obs RINEX_obs[RINEX_MAX_NR_OBS]".



static const double RINEX_MIN_URA[16] = {0.00, 2.40, 3.40, 4.85, 6.85,  9.65, 13.65, 24.00, 48.00, 96.00, 192.00, 384.00,  768.00, 1536.00, 3072.00, 6144.00};
static const double RINEX_MAX_URA[16] = {2.40, 3.40, 4.85, 6.85, 9.65, 13.65, 24.00, 48.00, 96.00, 192.0, 384.00, 768.00, 1536.00, 3072.00, 6144.00, 1.0e100};



/// A container for a single RINEX data observation.
typedef struct
{
  unsigned char loss_of_lock_indicator; //!< The loss of lock indicator (0-7).
  unsigned char signal_strength;        //!< The sign strength (1-9).
  double value;                         //!< The data value of the observation.
  RINEX_enumObservationType type;       //!< The type of observation.
  GNSS_enumSystem system;               //!< The GNSS system.
  unsigned short id;                    //!< The satellite id.
  BOOL isValid;                         //!< A boolean indicating if this observation is valid (The value could be zero or blank).
} struct_RINEX_obs;

/// A container for a single RINEX satellite descriptor.
typedef struct
{
  RINEX_enumSatelliteSystemType type;
  unsigned short id;
} struct_RINEX_satellite;


/// A static function to trim the whitespace from the left and right of a C string.
static BOOL RINEX_trim_left_right(
  char* str,            //!< (input/output) The input C string.
  unsigned max_length,  //!< (input) The maximum length of the input string.
  unsigned *str_length  //!< (output) The length of the string after trimming the whitespace.
  );

/// A static function to get the header lines indicated by the record descriptor.
static BOOL RINEX_get_header_lines(
  const char* header,              //!<  (input) The full RINEX header buffer.
  const unsigned header_size,      //!<  (input) The size of the valid data in the RINEX header buffer.
  const char* record_desciptor,    //!<  (input) The record descriptor. e.g. "RINEX VERSION / TYPE"
  char* lines_buffer,              //!< (output) The output buffer. 
  const unsigned max_lines_buffer, //!<  (input) The maximum size of the output buffer.
  unsigned* nr_lines               //!< (output) The number of lines read that correspond to the record descriptor.
  );

/// A static function to erase a substring from a string.
static BOOL RINEX_erase(
  char *erase_me, //!< (input) Erase this string from the input string.
  char* str       //!< (input/output) The input C string.  
  ); 
 
/// A static function to decode the "# / TYPES OF OBSERV" part of the RINEX OBS header.
static BOOL RINEX_GetObservationTypes(
  const char* header_buffer,          //!< (input) The full RINEX header buffer.
  const unsigned header_buffer_size,  //!< (input) The size of the valid data in the RINEX header buffer.
  RINEX_structDecodedHeader* header   //!< (input/output) The container for decoded header information.
  );


/// \brief  A static function to interpret special records 
/// (embedded Header records within the observation data).
/// This function is called when the epoch flag is greater than 1.
static BOOL RINEX_DealWithSpecialRecords(
  FILE* fid,                               //!< (input) An open (not NULL) file pointer to the RINEX data.
  RINEX_structDecodedHeader* RINEX_header, //!< (input/output) The decoded RINEX header information. The wavelength markers can change as data is decoded.
  BOOL *wasEndOfFileReached,               //!< Has the end of the file been reached (output).
  unsigned *filePosition,                  //!< The file position for the start of the message found (output).  
  const unsigned nr_special_records        //!< The number of special records.
  );

/// \brief  Decode the next observation set for one satellite from the file.
static BOOL RINEX_GetNextObserationSetForOneSatellite(
  FILE* fid,                                   //!< (input) An open (not NULL) file pointer to the RINEX data.
  RINEX_structDecodedHeader* RINEX_header,     //!< (input/output) The decoded RINEX header information. The wavelength markers can change as data is decoded.
  BOOL *wasEndOfFileReached,                   //!< (input/output) Has the end of the file been reached.
  unsigned *filePosition,                      //!< (input/output) The file position for the start of the message found (output).  
  struct_RINEX_obs* RINEX_obs,                 //!< (input/output) A pointer to the array of RINEX observations.
  const unsigned RINEX_max_nr_obs,             //!< (input) The maximum size of the RINEX_obs array.
  unsigned *RINEX_nr_obs,                      //!< (output) The number of valid obs in the RINEX_obs array.  
  const RINEX_enumSatelliteSystemType sattype, //!< (input) The satellite type.
  const unsigned short id                      //!< (input) The satellite id.
  );

/// \brief  A static function to replace float values exponents denoted with 'D' with 'E'.
static BOOL RINEX_ReplaceDwithE( char *str, const unsigned length );


/**
\brief   A static function to convert URA in meters to the URA index.
\author  Glenn D. MacGougan

\n REFERENCES \n
- GPS ICD 200C, p83. section 20.3.3.3.1.3
*/
static BOOL RINEX_ConvertURA_meters_to_URA_index( 
  double ura_m,         //!< The user range accuracy [m].
  unsigned char *ura    //!< The user range accuracy index.
  );



/**
\brief Convert RINEX signal strength to Carrier to Noise Density Ratio (CNo).
The conversion from RINEX signal strength to Carrier to Noise Density Ratio (CNo)
is ad-hoc. We will use the NovAtel OEM4 definition for CNo (dB-Hz). Nominally,
NovAtel maintains phase lock above 28 dB-Hz and a reasonable maximum signal strength
occurs at about 50 dB-Hz. We'll map linearly using 5 to 9 signals strength mapping to
28 to 50 dB-Hz. (50-28)/(9-5) = (y-28)/(x-5) where x is known, solve for y.
y = 5.5x+0.5.

\author  Glenn D. MacGougan
\return  TRUE(1) if successful, FALSE(0) otherwise.
*/
static BOOL RINEX_ConvertSignalStrengthToUsableCNo(
  float *cno,  //!< (input/output) The carrier to noise density ratio (dB-Hz)
  const unsigned char signal_strength
  );
  


BOOL RINEX_trim_left_right(
  char* str,            //!< (input) The input C string.
  unsigned max_length,  //!< (input) The maximum length of the input string.
  unsigned *str_length  //!< (output) The length of the string after trimming the whitespace.
  )
{
  int i = 0;
  int j = 0;
  int start = 0;
  size_t length = 0;
  if( str == NULL )
  {
    GNSS_ERROR_MSG( "if( str == NULL )" );
    return FALSE;
  }
  if( str_length == NULL )
  {
    GNSS_ERROR_MSG( "if( str_length == NULL )" );
    return FALSE;
  }

  // Remove leading whitesapce
  length = strlen( str );
  if( length > max_length )
  {
    GNSS_ERROR_MSG( "if( length > max_length )" );
    return FALSE;
  }
  if( length == 0 )
  {
    GNSS_ERROR_MSG( "if( length == 0 )" );
    return TRUE;
  }

  for( i = 0; i < (int)length; i++ )
  {
    if( isspace(str[i]) )
      continue;
    else
      break;
  }
  start = i;
  i = 0;
  for( j = start; j < (int)length; j++ )
  {
    str[i] = str[j];
    i++;
  }
  str[i] = '\0';

  // Remove trailing whitespace.
  length = strlen( str );
  for( i = (int)(length-1); i > 0; i-- )
  {
    if( isspace(str[i]) )
    {
      str[i] = '\0';
    }      
    else
    {
      break;
    }
  }

  length = strlen( str );
  *str_length = (unsigned)length;
  
  return TRUE;
}


BOOL RINEX_get_header_lines(
  const char* header_buffer,       //!<  (input) The full RINEX header buffer.
  const unsigned header_size,      //!<  (input) The size of the valid data in the RINEX header buffer.
  const char* record_desciptor,    //!<  (input) The record descriptor. e.g. "RINEX VERSION / TYPE"
  char* lines_buffer,              //!< (output) The output buffer. 
  const unsigned max_lines_buffer, //!<  (input) The maximum size of the output buffer.
  unsigned* nr_lines               //!< (output) The number of lines read that correspond to the record descriptor.
  )
{
  char *pch = NULL;
  char *strptr = NULL;
  unsigned i = 0;
  int j = 0;
  int record_descriptor_index = 0;
  int scount = 0;
  unsigned nr_valid_header_lines = 0;
  size_t length_record_desciptor = 0;
  
  typedef struct
  {
    char str[128]; // RINEX header strings should be 80 chars.
    size_t length;
  } struct_header_line_token;

  struct_header_line_token token[256];
  char buffer[RINEX_HEADER_SIZE];

  if( header_buffer == NULL )
  {
    GNSS_ERROR_MSG( "if( header_buffer == NULL )" );
    return FALSE;
  }
  if( header_size == 0 )
  {
    GNSS_ERROR_MSG( "if( header_size == 0 )" );
    return FALSE;
  }
  if( record_desciptor == NULL )
  {
    GNSS_ERROR_MSG( "if( record_desciptor == NULL )" );
    return FALSE;
  }
  if( lines_buffer == NULL )
  {
    GNSS_ERROR_MSG( "if( lines_buffer == NULL )" );
    return FALSE;
  }
  if( nr_lines == NULL )
  {
    GNSS_ERROR_MSG( "if( nr_lines == NULL )" );
    return FALSE;
  }

  if( header_size+1 > RINEX_HEADER_SIZE )
  {
    GNSS_ERROR_MSG( "if( header_size+1 > RINEX_HEADER_SIZE )" );
    return FALSE;
  }

  memcpy( buffer, header_buffer, header_size );
  buffer[header_size] = '\0';

  *nr_lines = 0;

  length_record_desciptor = strlen( record_desciptor );
  if( length_record_desciptor == 0 )
  {
    GNSS_ERROR_MSG( "if( length_record_desciptor == 0 )" );
    return FALSE;
  }

  strptr = strstr( header_buffer, record_desciptor );
  if( strptr == NULL )
  {
    return TRUE; // No line found with this descriptor at all.
  }

  // Tokenize the copied input buffer.
  pch = strtok( buffer,"\r\n");
  while( pch != NULL )
  {
    strptr = strstr( pch, record_desciptor );
    if( strptr != NULL )
    {
      token[i].length = strlen( pch );
      if( token[i].length < 128 )
      {
        strcpy( token[i].str, pch );
        i++;
      }
    }
    pch = strtok (NULL, "\r\n");
  }
  nr_valid_header_lines = i;

  if( nr_valid_header_lines == 0 )
  {
    GNSS_ERROR_MSG( "if( nr_valid_header_lines == 0 )" );
    return FALSE;
  }

  // Search the tokenized strings (lines) for the record descriptor.
  for( i = 0; i < nr_valid_header_lines; i++ )
  {
    strptr = strstr( token[i].str, record_desciptor );
    if( strptr != NULL )
    {
      // check where it was found in the line.
      record_descriptor_index = (int)(strptr - token[i].str);

      if( record_descriptor_index != 60 )
      {
        // record decriptor must be at columns 61-80
        // either extra/missing space(s) or the record descriptor is
        // present within a COMMENT line.

        if( strcmp( record_desciptor, "COMMENT" ) == 0 )
        {
          // possible COMMENT repeated on same line.
          // ok to decode this line for this record descriptor
        }
        else
        {
          // Check if within a comment line.
          strptr = strstr( token[i].str, "COMMENT" );
          if( strptr != NULL )
          {
            j = (int)(strptr - token[i].str);
            if( j == 60 )
            {
              // This is a comment line.
              continue;
            }
            else
            {
              // strange record, ignore
              continue;
            }
          }
          else
          {
            // This is likely a few missing or extra spaces.
            if( record_descriptor_index > 55 && record_descriptor_index < 65 )
            {
              // OK to decode this line for this record descriptor.
            }
            else
            {
              // too many extra spaces
              continue;
            }
          }
        }
      }

      // Add this line to the lines_buffer.
      if( (scount + token[i].length + 1) >= max_lines_buffer )
      {
        GNSS_ERROR_MSG( "if( (scount + token[i].length + 1) >= max_lines_buffer )" );
        return FALSE;
      }

      scount += sprintf( lines_buffer+scount, "%s\n", token[i].str );
      *nr_lines += 1;
    }
  }  
  return TRUE;
}


BOOL RINEX_erase(
  char *erase_me, //!< (input) Erase this string from the input string.
  char* str       //!< (input/output) The input C string.
  )
{
  int i = 0;
  int j = 0;
  char *strptr = NULL;
  size_t len = 0;
  size_t len_erase_me = 0;

  if( erase_me == NULL )
  {
    GNSS_ERROR_MSG( "if( erase_me == NULL )" );
    return FALSE;
  }
  if( str == NULL )
  {
    GNSS_ERROR_MSG( "if( str == NULL )" );
    return FALSE;
  }
  
  len_erase_me = strlen( erase_me );
  if( len_erase_me == 0 )
    return TRUE;

  len = strlen(str);
  if( len == 0 )
    return TRUE;

  if( len_erase_me > len )
    return TRUE;

  strptr = strstr( str, erase_me );
  
  while( strptr != NULL )
  {
    j = (int)(strptr - str);   // start of the string to be erased
    i = j + (int)len_erase_me; // end of the string to be erased

    for( ; i < (int)len; i++ )
    {
      str[j] = str[i];
      if( str[j] == '\0' )
        break;
      j++;
    }
    str[j] = '\0';
    len = strlen(str);  

    strptr = strstr( str, erase_me );
  }
  return TRUE;
}


// static 
BOOL RINEX_GetObservationTypes(
  const char* header_buffer,          //!< (input) The full RINEX header buffer.
  const unsigned header_buffer_size,  //!< (input) The size of the valid data in the RINEX header buffer.
  RINEX_structDecodedHeader* header   //!< (input/output) The container for decoded header information.  
  )
{
  char lines_buffer[RINEX_LINEBUF_SIZE];
  unsigned nr_lines = 0;
  BOOL result;
  char *pch = NULL;
  unsigned count = 0;
  char token[128];
  unsigned len=0;
  BOOL isFirst = TRUE;
  
  result = RINEX_get_header_lines(
    header_buffer,
    header_buffer_size,
    "# / TYPES OF OBSERV",
    lines_buffer,
    RINEX_LINEBUF_SIZE,
    &nr_lines
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
    return FALSE;  
  }
  // strip the record description from the string
  result = RINEX_erase( "# / TYPES OF OBSERV", lines_buffer );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_erase returned FALSE." );
    return FALSE;
  }
  
  // Determine the number of observation types.
  if( sscanf( lines_buffer, "%u", &(header->nr_obs_types) ) != 1 )
  {
    GNSS_ERROR_MSG( "sscanf failed." );
    return FALSE;
  }
  
  // Clean up the string a little.
  result = RINEX_trim_left_right( lines_buffer, RINEX_LINEBUF_SIZE, &len );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_trim_left_right returned FALSE." );
    return FALSE;
  }

  // Tokenize the string.
  pch = strtok( lines_buffer, " \t\r\n\f" );
  while( pch != NULL && count < header->nr_obs_types )
  {
    if( !isFirst )
    {
      strcpy( token, pch );
      result = RINEX_trim_left_right( token, 128, &len );
      if( result == FALSE )
      {
        GNSS_ERROR_MSG( "RINEX_trim_left_right returned FALSE." );
        return FALSE;
      }
      if( strlen(token) > 0 )
      {
        if( strcmp( token, "L1" ) == 0 )
          header->obs_types[count] =  RINEX_OBS_TYPE_L1;
        else if( strcmp( token, "L2" ) == 0 )
          header->obs_types[count] =  RINEX_OBS_TYPE_L2;
        else if( strcmp( token, "C1" ) == 0 )
          header->obs_types[count] =  RINEX_OBS_TYPE_C1;
        else if( strcmp( token, "P1" ) == 0 )
          header->obs_types[count] =  RINEX_OBS_TYPE_P1;
        else if( strcmp( token, "P2" ) == 0 )
          header->obs_types[count] =  RINEX_OBS_TYPE_P2;
        else if( strcmp( token, "D1" ) == 0 )
          header->obs_types[count] =  RINEX_OBS_TYPE_D1;
        else if( strcmp( token, "D2" ) == 0 )
          header->obs_types[count] =  RINEX_OBS_TYPE_D2;
        else if( strcmp( token, "T1" ) == 0 )
          header->obs_types[count] =  RINEX_OBS_TYPE_T1;
        else if( strcmp( token, "T2" ) == 0 )
          header->obs_types[count] =  RINEX_OBS_TYPE_T2;
        else if( strcmp( token, "S1" ) == 0 )
          header->obs_types[count] =  RINEX_OBS_TYPE_S1;
        else if( strcmp( token, "S2" ) == 0 )
          header->obs_types[count] =  RINEX_OBS_TYPE_S2;
        else
          header->obs_types[count] =  RINEX_OBS_TYPE_UNKNOWN;

        count++;
      }
    }
    pch = strtok (NULL, " ,.-");    
    isFirst = FALSE;
  }

  if( count != header->nr_obs_types )
  {
    GNSS_ERROR_MSG( "if( count != header->nr_obs_types )" );
    return FALSE;
  }

  return TRUE;
}


BOOL RINEX_DealWithSpecialRecords(
  FILE* fid,                               //!< (input) An open (not NULL) file pointer to the RINEX data.
  RINEX_structDecodedHeader* RINEX_header, //!< (input/output) The decoded RINEX header information. The wavelength markers can change as data is decoded.
  BOOL *wasEndOfFileReached,               //!< Has the end of the file been reached (output).
  unsigned *filePosition,                  //!< The file position for the start of the message found (output).  
  const unsigned nr_special_records        //!< The number of special records.
  )
{
  char line_buffer[RINEX_LINEBUF_SIZE];
  BOOL result = FALSE;
  size_t length = 0;
  unsigned i = 0;

  if( fid == NULL )
  {
    GNSS_ERROR_MSG( "if( fid == NULL )" );
    return FALSE;
  }
  if( RINEX_header == NULL )
  {
    GNSS_ERROR_MSG( "if( RINEX_header == NULL )" );
    return FALSE;
  }
  if( wasEndOfFileReached == NULL )
  {
    GNSS_ERROR_MSG( "if( wasEndOfFileReached == NULL )" );
    return FALSE;
  }
  if( filePosition == NULL )
  {
    GNSS_ERROR_MSG( "if( filePosition == NULL )" );
    return FALSE;
  }

  // check nothing to do.
  if( nr_special_records == 0 )
    return TRUE;

  for( i = 0; i < nr_special_records; i++ )
  {

    if( fgets( line_buffer, RINEX_LINEBUF_SIZE, fid ) == NULL )
    {
      if( feof(fid) )
      {
        *wasEndOfFileReached = TRUE;
        return TRUE;
      }
      else
      {
        GNSS_ERROR_MSG( "unexpected" );
        return FALSE;
      }
    }
    *filePosition = ftell(fid);

    if( strstr(line_buffer, "COMMENT") != NULL )
    {
      // This line is a comment. Ignore and continue.
    }
    else if( strstr(line_buffer, "WAVELENGTH FACT L1/2") != NULL )
    {
      // The wavelength factors have changed for some satellites.

      // GDM todo deal with these changes
    }
    else if( strstr(line_buffer, "MARKER NAME") != NULL )
    {
      // The marker name has changed.
      result = RINEX_erase("MARKER NAME", line_buffer);
      if( result == FALSE )
      {
        GNSS_ERROR_MSG( "RINEX_erase returned FALSE." );
        return FALSE;
      }
      result = RINEX_trim_left_right(line_buffer, RINEX_LINEBUF_SIZE, (unsigned *)&length );
      if( result == FALSE )
      {
        GNSS_ERROR_MSG( "RINEX_trim_left_right returned FALSE." );
        return FALSE;
      }
      if( length < 64 )
      {
        strcpy(RINEX_header->marker_name, line_buffer);
      }
      else
      {
        GNSS_ERROR_MSG( "length > 64" );
        return FALSE;
      }
    }
    else if( strstr(line_buffer, "MARKER NUMBER") != NULL )
    {
      // ignore for now
    }
    else if( strstr(line_buffer, "ANTENNA: DELTA H/E/N") != NULL )
    {
      if( sscanf( line_buffer, "%lf %lf %lf", 
        &(RINEX_header->antenna_delta_h), 
        &(RINEX_header->antenna_ecc_e), 
        &(RINEX_header->antenna_ecc_n) ) != 3 )
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE;
      }
    }
    else if( strstr(line_buffer, "APPROX POSITION XYZ") != NULL )
    {
      if( sscanf( line_buffer, "%lf %lf %lf", 
        &(RINEX_header->x), 
        &(RINEX_header->y), 
        &(RINEX_header->z) ) != 3 )
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE;
      }
    }
    else 
    {
      // The rest not handled yet.
    }
  }
  return TRUE;
}


//static 
BOOL RINEX_GetNextObserationSetForOneSatellite(
  FILE* fid,                                   //!< (input) An open (not NULL) file pointer to the RINEX data.
  RINEX_structDecodedHeader* RINEX_header,     //!< (input/output) The decoded RINEX header information. The wavelength markers can change as data is decoded.
  BOOL *wasEndOfFileReached,                   //!< (input/output) Has the end of the file been reached.
  unsigned *filePosition,                      //!< (input/output) The file position for the start of the message found (output).  
  struct_RINEX_obs* RINEX_obs,                 //!< (input/output) A pointer to the array of RINEX observations.
  const unsigned RINEX_max_nr_obs,             //!< (input) The maximum size of the RINEX_obs array.
  unsigned *RINEX_nr_obs,                      //!< (output) The number of valid obs in the RINEX_obs array.  
  const RINEX_enumSatelliteSystemType sattype, //!< (input) The satellite type.
  const unsigned short id                      //!< (input) The satellite id.
  )
{
  unsigned i = 0;
  char line_buffer[RINEX_LINEBUF_SIZE];
  unsigned count = 0;
  char str_a[15];
  char str_b[15];
  char str_c[15];
  char str_d[15];
  char str_e[15];

  memset( str_a, 0, 15 );
  memset( str_b, 0, 15 );
  memset( str_c, 0, 15 );
  memset( str_d, 0, 15 );
  memset( str_e, 0, 15 );

  // Check input
  if( fid == NULL )
  {
    GNSS_ERROR_MSG( "if( fid == NULL )" );
    return FALSE;
  }
  if( RINEX_header == NULL )
  {
    GNSS_ERROR_MSG( "if( RINEX_header == NULL )" );
    return FALSE;
  }
  if( wasEndOfFileReached == NULL )
  {
    GNSS_ERROR_MSG( "if( wasEndOfFileReached == NULL )" );
    return FALSE;
  }
  if( filePosition == NULL )
  {
    GNSS_ERROR_MSG( "if( filePosition == NULL )" );
    return FALSE;
  }
  if( RINEX_obs == NULL )
  {
    GNSS_ERROR_MSG( "if( RINEX_obs == NULL )" );
    return FALSE;
  }
  if( RINEX_max_nr_obs == 0 )
  {
    GNSS_ERROR_MSG( "if( RINEX_max_nr_obs == 0 )" );
    return FALSE;
  }
  if( RINEX_nr_obs == NULL )
  {
    GNSS_ERROR_MSG( "if( RINEX_nr_obs == NULL )" );
    return FALSE;
  }
  if( RINEX_header->nr_obs_types >= RINEX_max_nr_obs )
  {
    GNSS_ERROR_MSG( "if( RINEX_header->nr_obs_types >= RINEX_max_nr_obs )" );
    return FALSE;
  }


  // Get the next line from the file.
  if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
  {
    if( feof(fid) )
    {
      *wasEndOfFileReached = TRUE;
      return TRUE;
    }
    else
    {
      GNSS_ERROR_MSG( "unexpected" );
      return FALSE;
    }
  }
  *filePosition = ftell(fid);

  // Set the default values to zero.
  memset( RINEX_obs, 0, sizeof(struct_RINEX_obs)*RINEX_header->nr_obs_types );

  switch( RINEX_header->nr_obs_types )
  {
  case 1:
    {
      count = sscanf( line_buffer, "%14c%c%c", 
        str_a,
        &RINEX_obs[0].loss_of_lock_indicator, 
        &RINEX_obs[0].signal_strength
        );        
      sscanf( str_a, "%lf", &(RINEX_obs[0].value) );
      break;
    }
  case 2:
    {
      count = sscanf( line_buffer, "%14c%c%c%14c%c%c", 
        str_a,
        &RINEX_obs[0].loss_of_lock_indicator, 
        &RINEX_obs[0].signal_strength,
        str_b,
        &RINEX_obs[1].loss_of_lock_indicator, 
        &RINEX_obs[1].signal_strength 
        );
      sscanf( str_a, "%lf", &(RINEX_obs[0].value) );
      sscanf( str_b, "%lf", &(RINEX_obs[1].value) );
      break;
    }
  case 3:
    {
      count = sscanf( line_buffer, "%14c%c%c%14c%c%c%14c%c%c", 
        str_a,
        &RINEX_obs[0].loss_of_lock_indicator, 
        &RINEX_obs[0].signal_strength,
        str_b,
        &RINEX_obs[1].loss_of_lock_indicator, 
        &RINEX_obs[1].signal_strength,
        str_c,
        &RINEX_obs[2].loss_of_lock_indicator, 
        &RINEX_obs[2].signal_strength
      );
      sscanf( str_a, "%lf", &(RINEX_obs[0].value) );
      sscanf( str_b, "%lf", &(RINEX_obs[1].value) );
      sscanf( str_c, "%lf", &(RINEX_obs[2].value) );      
      break;
    }
  case 4:
    {
      count = sscanf( line_buffer, "%14c%c%c%14c%c%c%14c%c%c%14c%c%c", 
        str_a, 
        &RINEX_obs[0].loss_of_lock_indicator, 
        &RINEX_obs[0].signal_strength,
        str_b, 
        &RINEX_obs[1].loss_of_lock_indicator, 
        &RINEX_obs[1].signal_strength,
        str_c, 
        &RINEX_obs[2].loss_of_lock_indicator, 
        &RINEX_obs[2].signal_strength,
        str_d, 
        &RINEX_obs[3].loss_of_lock_indicator, 
        &RINEX_obs[3].signal_strength
      );
      sscanf( str_a, "%lf", &(RINEX_obs[0].value) );
      sscanf( str_b, "%lf", &(RINEX_obs[1].value) );
      sscanf( str_c, "%lf", &(RINEX_obs[2].value) );
      sscanf( str_d ,"%lf", &(RINEX_obs[3].value) );
      break;
    }
    case 5:
    {
      count = sscanf( line_buffer, "%14c%c%c%14c%c%c%14c%c%c%14c%c%c%14c%c%c", 
        str_a,
        &RINEX_obs[0].loss_of_lock_indicator, 
        &RINEX_obs[0].signal_strength,
        str_b,
        &RINEX_obs[1].loss_of_lock_indicator, 
        &RINEX_obs[1].signal_strength,
        str_c,
        &RINEX_obs[2].loss_of_lock_indicator, 
        &RINEX_obs[2].signal_strength,
        str_d,
        &RINEX_obs[3].loss_of_lock_indicator, 
        &RINEX_obs[3].signal_strength,
        str_e,
        &RINEX_obs[4].loss_of_lock_indicator, 
        &RINEX_obs[4].signal_strength
      );
      sscanf( str_a, "%lf", &(RINEX_obs[0].value) );
      sscanf( str_b, "%lf", &(RINEX_obs[1].value) );
      sscanf( str_c, "%lf", &(RINEX_obs[2].value) );
      sscanf( str_d ,"%lf", &(RINEX_obs[3].value) );
      sscanf( str_e ,"%lf", &(RINEX_obs[4].value) );      
      break;
    }
    case 6:
    case 7:
    case 8:
    case 9:
    case 10: // intentional fall thru.   
    {

      count = sscanf( line_buffer, "%14c%c%c%14c%c%c%14c%c%c%14c%c%c%14c%c%c", 
        str_a, 
        &RINEX_obs[0].loss_of_lock_indicator, 
        &RINEX_obs[0].signal_strength,
        str_b, 
        &RINEX_obs[1].loss_of_lock_indicator, 
        &RINEX_obs[1].signal_strength,
        str_c, 
        &RINEX_obs[2].loss_of_lock_indicator, 
        &RINEX_obs[2].signal_strength,
        str_d, 
        &RINEX_obs[3].loss_of_lock_indicator, 
        &RINEX_obs[3].signal_strength,
        str_e, 
        &RINEX_obs[4].loss_of_lock_indicator, 
        &RINEX_obs[4].signal_strength
      );
      sscanf( str_a, "%lf", &(RINEX_obs[0].value) );
      sscanf( str_b, "%lf", &(RINEX_obs[1].value) );
      sscanf( str_c, "%lf", &(RINEX_obs[2].value) );
      sscanf( str_d ,"%lf", &(RINEX_obs[3].value) );
      sscanf( str_e ,"%lf", &(RINEX_obs[4].value) );      

      memset( str_a, 0, 15 );
      memset( str_b, 0, 15 );
      memset( str_c, 0, 15 );
      memset( str_d, 0, 15 );
      memset( str_e, 0, 15 );
      
      // Get the next line from the file.
      if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
      {
        if( feof(fid) )
        {
          *wasEndOfFileReached = TRUE;
          return TRUE;
        }
        else
        {
          GNSS_ERROR_MSG( "unexpected" );
          return FALSE;
        }      
      }

      switch( RINEX_header->nr_obs_types-5 )
      {
      case 1:
        {
          count = sscanf( line_buffer, "%14c%c%c", 
            str_a, 
            &RINEX_obs[5].loss_of_lock_indicator, 
            &RINEX_obs[5].signal_strength
            );        
          sscanf( str_a, "%lf", &(RINEX_obs[5].value) );
          break;
        }
      case 2:
        {
          count = sscanf( line_buffer, "%14c%c%c%14c%c%c", 
            str_a, 
            &RINEX_obs[5].loss_of_lock_indicator, 
            &RINEX_obs[5].signal_strength,
            str_b, 
            &RINEX_obs[6].loss_of_lock_indicator, 
            &RINEX_obs[6].signal_strength 
            );
          sscanf( str_a, "%lf", &(RINEX_obs[5].value) );
          sscanf( str_b, "%lf", &(RINEX_obs[6].value) );          
          break;
        }
      case 3:
        {
          count = sscanf( line_buffer, "%14c%c%c%14c%c%c%14c%c%c", 
            str_a, 
            &RINEX_obs[5].loss_of_lock_indicator, 
            &RINEX_obs[5].signal_strength,
            str_b, 
            &RINEX_obs[6].loss_of_lock_indicator, 
            &RINEX_obs[6].signal_strength,
            str_c, 
            &RINEX_obs[7].loss_of_lock_indicator, 
            &RINEX_obs[7].signal_strength
            );
          sscanf( str_a, "%lf", &(RINEX_obs[5].value) );
          sscanf( str_b, "%lf", &(RINEX_obs[6].value) );
          sscanf( str_c, "%lf", &(RINEX_obs[7].value) );          
          break;
        }
      case 4:
        {
          count = sscanf( line_buffer, "%14c%c%c%14c%c%c%14c%c%c%14c%c%c", 
            str_a, 
            &RINEX_obs[5].loss_of_lock_indicator, 
            &RINEX_obs[5].signal_strength,
            str_b, 
            &RINEX_obs[6].loss_of_lock_indicator, 
            &RINEX_obs[6].signal_strength,
            str_c, 
            &RINEX_obs[7].loss_of_lock_indicator, 
            &RINEX_obs[7].signal_strength,
            str_d, 
            &RINEX_obs[8].loss_of_lock_indicator, 
            &RINEX_obs[8].signal_strength
            );
          sscanf( str_a, "%lf", &(RINEX_obs[5].value) );
          sscanf( str_b, "%lf", &(RINEX_obs[6].value) );
          sscanf( str_c, "%lf", &(RINEX_obs[7].value) );
          sscanf( str_d ,"%lf", &(RINEX_obs[8].value) );          
          break;
        }
      case 5:
        {
          count = sscanf( line_buffer, "%14c%c%c%14c%c%c%14c%c%c%14c%c%c%14c%c%c", 
            str_a, 
            &RINEX_obs[5].loss_of_lock_indicator, 
            &RINEX_obs[5].signal_strength,
            str_b, 
            &RINEX_obs[6].loss_of_lock_indicator, 
            &RINEX_obs[6].signal_strength,
            str_c, 
            &RINEX_obs[7].loss_of_lock_indicator, 
            &RINEX_obs[7].signal_strength,
            str_d, 
            &RINEX_obs[8].loss_of_lock_indicator, 
            &RINEX_obs[8].signal_strength,
            str_e, 
            &RINEX_obs[9].loss_of_lock_indicator, 
            &RINEX_obs[9].signal_strength
            );
          sscanf( str_a, "%lf", &(RINEX_obs[5].value) );
          sscanf( str_b, "%lf", &(RINEX_obs[6].value) );
          sscanf( str_c, "%lf", &(RINEX_obs[7].value) );
          sscanf( str_d ,"%lf", &(RINEX_obs[8].value) );
          sscanf( str_e ,"%lf", &(RINEX_obs[9].value) );      
          break;
        }
      default:
        {
          GNSS_ERROR_MSG( "unexpected default case" );
          return FALSE;
          break;
        }
      }
      break;
    }
    case 11:
    {
      count = sscanf( line_buffer, "%14c%c%c%14c%c%c%14c%c%c%14c%c%c%14c%c%c", 
        str_a, 
        &RINEX_obs[0].loss_of_lock_indicator, 
        &RINEX_obs[0].signal_strength,
        str_b, 
        &RINEX_obs[1].loss_of_lock_indicator, 
        &RINEX_obs[1].signal_strength,
        str_c, 
        &RINEX_obs[2].loss_of_lock_indicator, 
        &RINEX_obs[2].signal_strength,
        str_d, 
        &RINEX_obs[3].loss_of_lock_indicator, 
        &RINEX_obs[3].signal_strength,
        str_e, 
        &RINEX_obs[4].loss_of_lock_indicator, 
        &RINEX_obs[4].signal_strength
        );
      sscanf( str_a, "%lf", &(RINEX_obs[0].value) );
      sscanf( str_b, "%lf", &(RINEX_obs[1].value) );
      sscanf( str_c, "%lf", &(RINEX_obs[2].value) );
      sscanf( str_d ,"%lf", &(RINEX_obs[3].value) );
      sscanf( str_e ,"%lf", &(RINEX_obs[4].value) );      

      memset( str_a, 0, 15 );
      memset( str_b, 0, 15 );
      memset( str_c, 0, 15 );
      memset( str_d, 0, 15 );
      memset( str_e, 0, 15 );

      // Get the next line from the file.
      if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
      {
        if( feof(fid) )
        {
          *wasEndOfFileReached = TRUE;
          return TRUE;
        }
        else
        {
          GNSS_ERROR_MSG( "unexpected" );
          return FALSE;
        }            
      }

      count = sscanf( line_buffer, "%14c%c%c%14c%c%c%14c%c%c%14c%c%c%14c%c%c", 
        str_a,
        &RINEX_obs[5].loss_of_lock_indicator, 
        &RINEX_obs[5].signal_strength,
        str_b,
        &RINEX_obs[6].loss_of_lock_indicator, 
        &RINEX_obs[6].signal_strength,
        str_c,
        &RINEX_obs[7].loss_of_lock_indicator, 
        &RINEX_obs[7].signal_strength,
        str_d,
        &RINEX_obs[8].loss_of_lock_indicator, 
        &RINEX_obs[8].signal_strength,
        str_e,
        &RINEX_obs[9].loss_of_lock_indicator, 
        &RINEX_obs[9].signal_strength
        );
      sscanf( str_a, "%lf", &(RINEX_obs[5].value) );
      sscanf( str_b, "%lf", &(RINEX_obs[6].value) );
      sscanf( str_c, "%lf", &(RINEX_obs[7].value) );
      sscanf( str_d ,"%lf", &(RINEX_obs[8].value) );
      sscanf( str_e ,"%lf", &(RINEX_obs[9].value) );
    
      memset( str_a, 0, 15 );

      // Get the next line from the file.
      if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
      {
        if( feof(fid) )
        {
          *wasEndOfFileReached = TRUE;
          return TRUE;
        }
        else
        {
          GNSS_ERROR_MSG( "unexpected" );
          return FALSE;
        }            
      }

      count = sscanf( line_buffer, "%14c%c%c", 
        str_a,
        &RINEX_obs[10].loss_of_lock_indicator, 
        &RINEX_obs[10].signal_strength
        );  
      sscanf( str_a, "%lf", &(RINEX_obs[10].value) );
      break;
    }
  default:
    {
      GNSS_ERROR_MSG( "unexpected default case" );
      return FALSE;
      break;
    }
  }

  for( i = 0; i < RINEX_header->nr_obs_types; i++ )
  {
    // Zero values denote invalid observations.
    if( RINEX_obs[i].value == 0 )
    {
      RINEX_obs[i].isValid = FALSE;
    }
    else
    {
      RINEX_obs[i].isValid = TRUE;
    }

    switch( RINEX_obs[i].loss_of_lock_indicator )
    {
    case '0': RINEX_obs[i].loss_of_lock_indicator = 0; break;
    case '1': RINEX_obs[i].loss_of_lock_indicator = 1; break;
    case '2': RINEX_obs[i].loss_of_lock_indicator = 2; break;
    case '3': RINEX_obs[i].loss_of_lock_indicator = 3; break;
    case '4': RINEX_obs[i].loss_of_lock_indicator = 4; break;
    case '5': RINEX_obs[i].loss_of_lock_indicator = 5; break;
    case '6': RINEX_obs[i].loss_of_lock_indicator = 6; break;
    case '7': RINEX_obs[i].loss_of_lock_indicator = 7; break;
    default:  RINEX_obs[i].loss_of_lock_indicator = 0; break;
    }

    switch( RINEX_obs[i].signal_strength )
    {
    case '0': RINEX_obs[i].signal_strength = 0; break;
    case '1': RINEX_obs[i].signal_strength = 1; break;
    case '2': RINEX_obs[i].signal_strength = 2; break;
    case '3': RINEX_obs[i].signal_strength = 3; break;
    case '4': RINEX_obs[i].signal_strength = 4; break;
    case '5': RINEX_obs[i].signal_strength = 5; break;
    case '6': RINEX_obs[i].signal_strength = 6; break;
    case '7': RINEX_obs[i].signal_strength = 7; break;
    case '8': RINEX_obs[i].signal_strength = 8; break;
    case '9': RINEX_obs[i].signal_strength = 9; break;
    default:  RINEX_obs[i].signal_strength = 0; break;
    }

    RINEX_obs[i].type = RINEX_header->obs_types[i];

    switch( sattype )
    {
    case RINEX_SATELLITE_SYSTEM_GPS:
      {
        RINEX_obs[i].system = GNSS_GPS;
        RINEX_obs[i].id = id;
        break;
      }
    case RINEX_SATELLITE_SYSTEM_GLO:
      {
        RINEX_obs[i].system = GNSS_GLONASS;
        RINEX_obs[i].id = id; // GLONASS slot number.
        break;
      }
    case RINEX_SATELLITE_SYSTEM_GEO:
      {
        RINEX_obs[i].system = GNSS_WAAS;    
        RINEX_obs[i].id = id + 100;
        break;
      }
    case RINEX_SATELLITE_SYSTEM_NSS:
      {
        continue; break; // Not supported. Ignore the data from this source. Continue to outer for loop.
      }
    default:
      {
        continue; break; // Not supported. Ignore the data from this source. Continue to outer for loop.
      }
    }
  }

  *RINEX_nr_obs = RINEX_header->nr_obs_types;

  return TRUE;
}


BOOL RINEX_ConvertSignalStrengthToUsableCNo(
  float *cno,                          //!< (input/output) The carrier to noise density ratio (dB-Hz)
  const unsigned char signal_strength  //!< (input) The RINEX signal strength indicator (0-9).
  )
{
  if( cno == NULL )
  {
    GNSS_ERROR_MSG( "if( cno == NULL )" );
    return FALSE;
  }

  if( signal_strength > 0 && signal_strength < 10 )
  {
    *cno = 5.5f*signal_strength + 0.5f;
  }
  return TRUE;
}


BOOL RINEX_GetHeader( 
  const char* filepath,           //!< Path to the RINEX file.
  char* buffer,                   //!< (input/output) A character buffer in which to place the RINEX header.
  const unsigned buffer_max_size, //!< (input)  The maximum size of the buffer [bytes]. This value should be large enough to hold the entire header, (8192 to 16384).
  unsigned *buffer_size,          //!< (output) The length of the header data placed in the buffer [bytes].
  double *version,                //!< (output) The RINEX version number. e.g. 1.0, 2.0, 2.2, 3.0, etc.
  RINEX_enumFileType *file_type   //!< (output) The RINEX file type. 
  )
{
  FILE* fid = NULL;                 // A file pointer for the RINEX file.
  char line_buffer[1024];               // A container for one line of the header.
  char *strptr = NULL;              // A pointer to a string.
  BOOL end_of_header_found = FALSE; // A boolean to indicate if the end of header was found.
  char type_char;
  
  size_t line_length = 0; // The length of one line.
  unsigned scount = 0;      // A counter/index used to compose the header buffer.

  fid = fopen( filepath, "r" );
  if( fid == NULL )
  {
    GNSS_ERROR_MSG( "if( fid == NULL )" );
    return FALSE;
  }

  // The first line of the file must be the RINEX VERSION / TYPE
  if( fgets( line_buffer, 1024, fid ) == NULL )
  {
    GNSS_ERROR_MSG( "fgets failed" );
    fclose(fid);
    return FALSE;
  }
  strptr = strstr( line_buffer, "RINEX VERSION / TYPE" );
  if( strptr == NULL )
  {
    GNSS_ERROR_MSG( "strstr failed." );
    fclose(fid);
    return FALSE;
  }

  // Add the first line to the buffer
  line_length = strlen( line_buffer );
  if( scount+line_length >= buffer_max_size )    
  {
    GNSS_ERROR_MSG( "if( scount+line_length >= buffer_max_size )" );
    fclose(fid);
    return FALSE;    
  }
  scount += sprintf( buffer+scount, "%s", line_buffer );

  // Extract the RINEX version and type.
  if( sscanf( line_buffer, "%lf %c", version, &type_char ) != 2 )
  {
    GNSS_ERROR_MSG( "sscanf failed" );
    fclose(fid);
    return FALSE;
  }
  *file_type = (RINEX_enumFileType)type_char;

  do
  {
    if( fgets( line_buffer, 1024, fid ) == NULL )
      break;

    if( strstr( line_buffer, "END OF HEADER" ) != NULL )
    {
      end_of_header_found = TRUE;
    }

    // Add the line of the buffer.
    line_length = strlen( line_buffer );
    if( scount+line_length >= buffer_max_size )   
    {
      GNSS_ERROR_MSG( "if( scount+line_length >= buffer_max_size )" );
      fclose(fid);
      return FALSE;    
    }
    scount += sprintf( buffer+scount, "%s", line_buffer );

  }while( !end_of_header_found );

  if( end_of_header_found )
  {
    *buffer_size = scount;
    fclose(fid);
    return TRUE;
  }
  else
  {
    GNSS_ERROR_MSG( "End of RINEX header not found." );
    fclose(fid);
    return FALSE;
  }
}



BOOL RINEX_DecodeHeader_ObservationFile(
  const char* header_buffer,         //!< (input) The character buffer containing the RINEX header.
  const unsigned header_buffer_size, //!< (input) The size of the character buffer containing the RINEX header [bytes]. Not the maximum size, the size of the valid data in the buffer.
  RINEX_structDecodedHeader* header  //!< (output) The decoded header data.
  )
{
  BOOL result = FALSE;
  char lines_buffer[RINEX_LINEBUF_SIZE];
  unsigned nr_lines = 0;
  char rinex_type_char = 0;
  char time_system_str[128];
  unsigned len = 0;
  int itmp[5];  
  
  if( header_buffer == NULL )
  {
    GNSS_ERROR_MSG( "if( header_buffer == NULL )" );
    return FALSE;
  }
  if( header_buffer_size == 0 )
  {
    GNSS_ERROR_MSG( "if( header_buffer_size == 0 )" );
    return FALSE;
  }
  if( header == NULL )
  {
    GNSS_ERROR_MSG( "if( header == NULL )" );
    return FALSE;
  }

  memset( header, 0, sizeof(RINEX_structDecodedHeader) );

  result = RINEX_get_header_lines(
    header_buffer,
    header_buffer_size,
    "RINEX VERSION / TYPE",
    lines_buffer,
    RINEX_LINEBUF_SIZE,
    &nr_lines
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
    return FALSE;
  }
  if( nr_lines != 1 )
  {
    GNSS_ERROR_MSG( "if( nr_lines != 1 )" );
    return FALSE;
  }
  if( sscanf( lines_buffer, "%lf %c", &(header->version), &rinex_type_char ) != 2 )
  {
    GNSS_ERROR_MSG( "sscanf failed" );
    return FALSE;
  }
  header->type = (RINEX_enumFileType)rinex_type_char;


  result = RINEX_get_header_lines(
    header_buffer,
    header_buffer_size,
    "MARKER NAME",
    lines_buffer,
    RINEX_LINEBUF_SIZE,
    &nr_lines
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
    return FALSE;
  }
  if( nr_lines != 1 )
  {
    GNSS_ERROR_MSG( "if( nr_lines != 1 )" );
    return FALSE;
  }
  result = RINEX_erase("MARKER NAME", lines_buffer);
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_erase returned FALSE." );
    return FALSE;
  }
  result = RINEX_trim_left_right(lines_buffer, RINEX_LINEBUF_SIZE, &len );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_trim_left_right returned FALSE" );
    return FALSE;
  }
  if( len < 64 )
  {
    strcpy(header->marker_name, lines_buffer);
  }
  else
  {
    GNSS_ERROR_MSG( "if( len < 64 )" );
    return FALSE;
  }

  result = RINEX_get_header_lines(
    header_buffer,
    header_buffer_size,
    "APPROX POSITION XYZ",
    lines_buffer,
    RINEX_LINEBUF_SIZE,
    &nr_lines
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_get_header_lines" );
    return FALSE;  
  }
  if( nr_lines == 1 )
  {    
    if( sscanf( lines_buffer, "%lf %lf %lf", &(header->x), &(header->y), &(header->z) ) != 3 )
    {
      GNSS_ERROR_MSG( "sscanf failed" );
      return FALSE;
    }
  }

  result = RINEX_get_header_lines(
    header_buffer,
    header_buffer_size,
    "ANTENNA: DELTA H/E/N",
    lines_buffer,
    RINEX_LINEBUF_SIZE,
    &nr_lines
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
    return FALSE;
  }
  if( nr_lines != 1 )
  {
    GNSS_ERROR_MSG( "if( nr_lines != 1 )" );
    return FALSE;
  }
  if( sscanf( lines_buffer, "%lf %lf %lf", &(header->antenna_delta_h), &(header->antenna_ecc_e), &(header->antenna_ecc_n) ) != 3 )
  {
    GNSS_ERROR_MSG( "sscanf failed." );
    return FALSE;
  }

  
  result = RINEX_get_header_lines(
    header_buffer,
    header_buffer_size,
    "WAVELENGTH FACT L1/2",
    lines_buffer,
    RINEX_LINEBUF_SIZE,
    &nr_lines
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
    return FALSE;  
  }
  if( nr_lines == 1 )
  {
    // Only default values specified.
    if( sscanf( lines_buffer, "%d %d", (int *)&(header->default_wavefactor_L1), (int *)&(header->default_wavefactor_L2) ) != 2 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      return FALSE;
    }
  }
  else
  {
    // First read the default values specified.
    if( sscanf( lines_buffer, "%d %d", (int *)&(header->default_wavefactor_L1), (int *)&(header->default_wavefactor_L2) ) != 2 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      return FALSE;
    }

    //GDM_TODO deal with multiline WAVELENTH FACT L1/2
  }

  result = RINEX_GetObservationTypes( header_buffer, header_buffer_size, header );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_GetObservationTypes returned FALSE." );
    return FALSE;
  }



  result = RINEX_get_header_lines(
    header_buffer,
    header_buffer_size,
    "INTERVAL",
    lines_buffer,
    RINEX_LINEBUF_SIZE,
    &nr_lines
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
    return FALSE;
  }
  if( nr_lines == 1 )
  {    
    if( sscanf( lines_buffer, "%lf", &(header->interval) ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      return FALSE;
    }
    if( header->interval <= 0 )
      header->interval = -1;  // Set to unknown value.
  }
  else
  {
    header->interval = -1; // Set to unknown value.
  }



  result = RINEX_get_header_lines(
    header_buffer,
    header_buffer_size,
    "TIME OF FIRST OBS",
    lines_buffer,
    RINEX_LINEBUF_SIZE,
    &nr_lines
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
    return FALSE; 
  }

  if( sscanf( lines_buffer, "%d %d %d %d %d %f %s", 
    &(itmp[0]),
    &(itmp[1]),
    &(itmp[2]),
    &(itmp[3]),
    &(itmp[4]),
    &(header->time_of_first_obs.seconds),
    time_system_str ) != 7 )
  {
    GNSS_ERROR_MSG( "sscanf failed." );
    return FALSE;
  }  
  header->time_of_first_obs.year   = (unsigned short)itmp[0];
  header->time_of_first_obs.month  = (unsigned char)itmp[1];
  header->time_of_first_obs.day    = (unsigned char)itmp[2];
  header->time_of_first_obs.hour   = (unsigned char)itmp[3];
  header->time_of_first_obs.minute = (unsigned char)itmp[4];
   
    
  result = RINEX_trim_left_right( time_system_str, 128, &len );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_trim_left_right returned FALSE." );
    return FALSE; 
  }
  if( strcmp( time_system_str, "TIME") == 0  ) // no string present, defaults to GPS
    header->time_of_first_obs.time_system = RINEX_TIME_SYSTEM_GPS;
  else if( strcmp( time_system_str, "GPS" ) == 0 )
    header->time_of_first_obs.time_system = RINEX_TIME_SYSTEM_GPS;
  else if( strcmp( time_system_str, "GLO" ) == 0 )
    header->time_of_first_obs.time_system = RINEX_TIME_SYSTEM_GLO;
  else
    header->time_of_first_obs.time_system = RINEX_TIME_SYSTEM_UNKNOWN;

  return TRUE;
}




BOOL RINEX_GetNextObservationSet(
  FILE* fid,                               //!< (input) An open (not NULL) file pointer to the RINEX data.
  RINEX_structDecodedHeader* RINEX_header, //!< (input/output) The decoded RINEX header information. The wavelength markers can change as data is decoded.
  BOOL *wasEndOfFileReached,               //!< Has the end of the file been reached (output).
  BOOL *wasObservationFound,               //!< Was a valid observation found (output).
  unsigned *filePosition,                  //!< The file position for the start of the message found (output).  
  GNSS_structMeasurement* obsArray,        //!< A pointer to a user provided array of GNSS_structMeasurement (input/output).
  const unsigned char maxNrObs,            //!< The maximum number of elements in the array provided (input).
  unsigned *nrObs,                         //!< The number of valid elements set in the array (output).
  unsigned short* rx_gps_week,             //!< The receiver GPS week (0-1024+) [weeks].
  double* rx_gps_tow                       //!< The receiver GPS time of week (0-603799.99999) [s].
  )
{
  char line_buffer[RINEX_LINEBUF_SIZE]; // A character buffer to hold a line from the RINEX file.
  size_t length = 0;                // A string length.
  RINEX_TIME epoch;                 // The RINEX time.
  RINEX_enumEpochFlag epoch_flag;   // A RINEX epoch flag.
  char *pch = NULL;                 // A string pointer used in tokenizing a C string.
  unsigned count = 0;               // A counter.
  int itmp = 0;                     // A temporary integer.
  int i = 0;                        // A counter.
  int j = 0;                        // A counter.
  int obsArray_index = 0;           // A counter.
  char numstr[64];                  // A string to hold a number.
  char tmpstr[128];                 // A temporary string.  
  BOOL isL1data_present = FALSE;
  BOOL isL2data_present = FALSE;
  BOOL overwriteCNoL1 = TRUE;
  BOOL overwriteCNoL2 = TRUE;
  BOOL isEpochValidToDecode = FALSE;
  BOOL isContinuationLinePresent = FALSE;
  int nr_special_records = 0;
  
  double tow = 0; // A time of week (0-604399.99999) [s].
  unsigned short week = 0; // The GPS week (0-1024+) [weeks].

  RINEX_enumSatelliteSystemType next_sat_type = RINEX_SATELLITE_SYSTEM_UNKNOWN;
  

  typedef struct
  {
    char str[128];
    size_t length;
    BOOL isValid;
  } struct_token;
  struct_token token[64]; // An array of string tokens.
  unsigned nr_tokens = 0; // The number of valid tokens.

  struct_RINEX_satellite RINEX_sat[RINEX_MAX_NR_SATS];
  unsigned RINEX_sat_index = 0;      // The index into RINEX_sat.
  unsigned RINEX_nr_satellites = 0;  // The number of valid values in the RINEX_sat array.
  
  struct_RINEX_obs RINEX_obs[RINEX_MAX_NR_OBS];
  unsigned RINEX_obs_index = 0; // The index into RINEX_obs.
  unsigned RINEX_nr_obs = 0;    // The number of valid obs in RINEX_obs array.

  BOOL result;                       

  numstr[0] = '\0';
  numstr[1] = '\0';
  numstr[2] = '\0';
  numstr[3] = '\0';

  // Check the input.
  if( fid == NULL )
  {
    GNSS_ERROR_MSG( "if( fid == NULL )" );
    return FALSE; 
  }    
  if( wasEndOfFileReached == NULL )
  {
    GNSS_ERROR_MSG( "if( wasEndOfFileReached == NULL )" );
    return FALSE; 
  }    
  if( wasObservationFound == NULL )
  {
    GNSS_ERROR_MSG( "if( wasObservationFound == NULL )" );
    return FALSE; 
  }    
  if( filePosition == NULL )
  {
    GNSS_ERROR_MSG( "if( filePosition == NULL )" );
    return FALSE; 
  }    
  if( obsArray == NULL )
  {
    GNSS_ERROR_MSG( "if( obsArray == NULL )" );
    return FALSE; 
  }    
  if( nrObs == NULL )
  {
    GNSS_ERROR_MSG( "if( nrObs == NULL )" );
    return FALSE; 
  }    
  if( RINEX_header->type != RINEX_FILE_TYPE_OBS )
  {
    GNSS_ERROR_MSG( "if( RINEX_header->type != RINEX_FILE_TYPE_OBS )" );
    return FALSE; 
  }    

  *wasObservationFound = FALSE;
  *wasEndOfFileReached = FALSE; 

  // Set the token array to zero.
  memset( token, 0, sizeof(struct_token)*RINEX_MAX_NR_SATS );

  // Set the RINEX_obs array to zero.
  memset( RINEX_obs, 0, sizeof(struct_RINEX_obs)*RINEX_MAX_NR_OBS );
  
  // The epoch's tme system type is the same as the header's time of first RINEX_obs.
  epoch.time_system = RINEX_header->time_of_first_obs.time_system;

  if( epoch.time_system != RINEX_TIME_SYSTEM_GPS )
  {
    // Not supported for now!
    GNSS_ERROR_MSG( "if( epoch.time_system != RINEX_TIME_SYSTEM_GPS ) - NOT SUPPORTED YET." );
    return FALSE;
  }

  // Read in the RINEX epoch, epoch flag, and satellite ids string.
  do
  {
    do // advance over empty lines if any
    {
      if( fgets( line_buffer, RINEX_LINEBUF_SIZE, fid ) == NULL )
      {
        if( feof(fid) )
        {
          *wasEndOfFileReached = TRUE;
          return TRUE;
        }
        else
        {
          GNSS_ERROR_MSG( "unexpected" );
          return FALSE;
        }
      }
      result = RINEX_trim_left_right( line_buffer, RINEX_LINEBUF_SIZE, (unsigned *)&length );
      if( result == FALSE )
      {
        GNSS_ERROR_MSG( "RINEX_trim_left_right returned FALSE." );
        return FALSE;
      }
    }while( length == 0 );
  
    if( length == 0 )
    {
      GNSS_ERROR_MSG( "if( length == 0 )" );
      return FALSE;
    }

    // To make life easier later:
    // Search and replace "G ", "R ", "T ", "S " with
    //                    "G_", "R_", "T_", "S_"
    // These are the satellite ids.
    for( i = 0; i < (int)(length-1); i++ )
    {
      if( line_buffer[i] == 'G' || line_buffer[i] == 'R' || line_buffer[i] == 'T' || line_buffer[i] == 'S' )
      {
        if( line_buffer[i+1] == ' ' )
        {
          line_buffer[i+1] = '_';
        }
      }
    }
  
    // Tokenize the input line buffer.
    pch = strtok( line_buffer, " \t\r\n\f" );  
    nr_tokens = 0;
    while( pch != NULL && count < 64 )
    {
      token[nr_tokens].length = strlen(pch);
      if( token[nr_tokens].length < 128 )
      {
        strcpy( token[nr_tokens].str, pch );
        token[nr_tokens].isValid = TRUE;
      }
      else
      {
        GNSS_ERROR_MSG( "unexpected." );
        return FALSE;
      }

      pch = strtok( NULL, " \t\r\n\f" );
      nr_tokens++;
    }
    if( nr_tokens >= 64 )
    {
      GNSS_ERROR_MSG( "if( nr_tokens >= 64 )" );
      return FALSE;
    }

    // For events without significant epoch the epoch fields can be left blank.
    // Thus, there are a few cases: 
    // (a) No epoch information,  2 integers (an event flag and the nr_special_records to follow 0-999).
    // (b) All the epoch information + (a) (year month day hour minute seconds epoch_flag nr_special_records)
    // (c) All the epoch information, epoch flag, number of satellites, the satellite list, and optionally the rx clock offset.
    if( nr_tokens == 2 )
    {
      // This can only indicate a epoch flag with a number of special records to follow.
      // Read in the epoch flag and the number of special records to follow.
      if( sscanf( token[0].str, "%d", &itmp ) != 1 )
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE;
      }
      epoch_flag = (RINEX_enumEpochFlag)itmp;        

      if( sscanf( token[1].str, "%d", &nr_special_records ) != 1 )
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE;
      }

      // Deal with special records if any
      result = RINEX_DealWithSpecialRecords(
        fid,
        RINEX_header, 
        wasEndOfFileReached,
        filePosition,
        nr_special_records 
        );

      continue;
    }
    else 
    {
      // The number of tokens must now contain
      // year month day hour minute seconds epoch_flag --other stuff--,
      // --other stuff-- depending on the epoch flag
      if( nr_tokens < 8 )
      {
        GNSS_ERROR_MSG( "if( nr_tokens < 8 )" );
        return FALSE;  
      }

      // Exract the epoch information from the tokenized line buffer.
      i = 0;
      if( sscanf( token[i].str, "%d", &itmp ) != 1 ) 
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE; 
      }
      epoch.year = (unsigned short)itmp;
      i++;
      if( sscanf( token[i].str, "%d", &itmp ) != 1 ) 
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE; 
      }
      epoch.month = (unsigned char)itmp;
      i++;
      if( sscanf( token[i].str, "%d", &itmp ) != 1 ) 
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE; 
      }
      epoch.day = (unsigned char)itmp;
      i++;
      if( sscanf( token[i].str, "%d", &itmp ) != 1 ) 
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE; 
      }
      epoch.hour = (unsigned char)itmp;
      i++;
      if( sscanf( token[i].str, "%d", &itmp ) != 1 ) 
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE; 
      }
      epoch.minute = (unsigned char)itmp;
      i++;
      if( sscanf( token[i].str, "%f", &(epoch.seconds) ) != 1 ) 
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE; 
      }
      i++;
      if( sscanf( token[i].str, "%d", &(itmp) ) != 1 ) 
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE; 
      }
      i++;
      epoch_flag = (RINEX_enumEpochFlag)itmp;

      if( epoch_flag == 6 )
      {
        isEpochValidToDecode = TRUE;        
        // Cycle slip correction observations to follow.
        // This is dealt with later below.
      }
      else if( epoch_flag > 1 )
      {
        if( sscanf( token[i].str, "%d", &nr_special_records ) != 1 )
        {
          GNSS_ERROR_MSG( "sscanf failed." );
          return FALSE;       
        }

        // Deal with special records if any
        result = RINEX_DealWithSpecialRecords(
          fid,
          RINEX_header, 
          wasEndOfFileReached,
          filePosition,
          nr_special_records 
          );
        continue;
      }
      else
      {
        isEpochValidToDecode = TRUE;
      }
    }

  }while( !isEpochValidToDecode );

  if( token[7].length == 0 )
  {
    GNSS_ERROR_MSG( "if( token[7].length == 0 )" );
    return FALSE;
  }

  // Some RINEX data has a clock offset term following the number of satellites and there id's
  // Sometimes, this value is part of the eighth token
  j = 0;
  for( i = 0; i < (int)token[7].length; i++ )
  {    
    if( token[7].str[i] == '-' || token[7].str[i] == '+' )
    {
      // ignore the rest of the token string
      tmpstr[j] = '\0';
      j++;
      break;
    }
    if( j < 128 )
    {
      tmpstr[j] = token[7].str[i];
      j++;
    }
  }
  if( j < 128 )
  {
    tmpstr[j] = '\0';
    j++;
  }
  strcpy( token[7].str, tmpstr );
  token[7].length = strlen( token[7].str );
  
  // The eighth token contains the number of satellites and there id's
  j = 0;
  for( i = 0; i < (int)token[7].length; i++ )
  {
    // Satellite can be denoted by the following letters (RINEX_v_ 2.1)
    // 'G': GPS
    // 'R': GLONASS
    // 'S': Geostationary signal payload
    // 'T': NNSS Transit
    //
    // e.g. string here is 5G_8G12G13R_8S20 means 5 satellite observations, 
    // with GPS PRN's 8, 12, 14, GLONASS id 7, and SBAS id 20 (by the way: making up the id's here).
    if( token[7].str[i] == '_' )
    {
      continue;
    }  
    if( token[7].str[i] == '-' || token[7].str[i] == '+' || token[7].str[i] == '.' || token[7].str[i] == 'E' || token[7].str[i] == 'e' )
    {
      // Any float numbers should not be present on this line.
      // unless the clock offset is output (e.g. Trimble R8 receivers)
      GNSS_ERROR_MSG( "float numbers should not be present on this line." );
      break;
      //return FALSE;
    }    

    if( isdigit( token[7].str[i] ) )
    {
      numstr[j] = token[7].str[i];
      j++;
    }
    else
    {
      if( token[7].str[i] != RINEX_SATELLITE_SYSTEM_GPS && 
        token[7].str[i] != RINEX_SATELLITE_SYSTEM_GLO && 
        token[7].str[i] != RINEX_SATELLITE_SYSTEM_GEO && 
        token[7].str[i] != RINEX_SATELLITE_SYSTEM_NSS )
      {
        GNSS_ERROR_MSG( "unexpected" );
        return FALSE;
      }

      numstr[j] = '\0';
      j = 0;
      // A number always precedes a non-number here. Decode the number.
      if( sscanf( numstr, "%d", &itmp ) != 1 )
      {
        GNSS_ERROR_MSG( "sscanf failed." );
        return FALSE;
      }
      if( count == 0 )
      {
        // This is the number of observations        
        RINEX_nr_satellites = itmp;
        if( RINEX_nr_satellites > 12 )
        {
          // A continuation line will be used!
          isContinuationLinePresent = TRUE;
        }

        if( RINEX_nr_satellites >= 64 )
        {
          GNSS_ERROR_MSG( "if( RINEX_nr_satellites >= 64 )" );
          return FALSE; // a very unlikely error condition.
        }
      }
      else
      {
        // This is a satellite id of type (current value of) next_sat_type.
        RINEX_sat[count-1].id = (unsigned short)itmp;
        RINEX_sat[count-1].type = next_sat_type;
      }
      count++;
      if( count > RINEX_nr_satellites+1 )
      {
        GNSS_ERROR_MSG( "if( count > RINEX_nr_satellites+1 )" );
        return FALSE; // a very unlikely error condition.
      }
      next_sat_type = (RINEX_enumSatelliteSystemType)token[7].str[i];
    }
  }
  if( count == 0 )
  {
    GNSS_ERROR_MSG( "if( count == 0 )" );
    return FALSE;
  }

  // The last satellite id must still be interpreted.
  numstr[j] = '\0';
  j = 0;
  if( sscanf( numstr, "%d", &itmp ) != 1 )
  {
    GNSS_ERROR_MSG( "sscanf failed" );
    return FALSE;
  }
  RINEX_sat[count-1].id = (unsigned short)itmp;
  RINEX_sat[count-1].type = next_sat_type;
  count++;

  if( count > RINEX_nr_satellites+1 )
  {
    // Error in number of satellite ids read compared to RINEX_nr_obs.
    GNSS_ERROR_MSG( "if( count > RINEX_nr_satellites+1 )" );
    return FALSE;
  }

  // Deal with the possibility of continuation lines.
  if( count != RINEX_nr_satellites+1 )
  {
    if( RINEX_nr_satellites <= 12 )
    {
      GNSS_ERROR_MSG( "if( RINEX_nr_satellites <= 12 )" );
      return FALSE;
    }
    else
    {
      // Get the next line from the file.
      if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
      {
        if( feof(fid) )
        {
          *wasEndOfFileReached = TRUE;
          return TRUE;
        }
        else
        {
          GNSS_ERROR_MSG( "unexpected" );
          return FALSE;
        }
      }

      length = strlen(line_buffer);
      if( length == 0 )
      {
        GNSS_ERROR_MSG( "if( length == 0 )" );
        return FALSE;
      }

      if( RINEX_trim_left_right( line_buffer, RINEX_LINEBUF_SIZE, (unsigned *)&length ) == FALSE )
      {
        GNSS_ERROR_MSG( "RINEX_trim_left_right returned FALSE." );
        return FALSE;
      }

      for( i = 0; i < (int)length; i++ )
      {
        if( isspace(line_buffer[i]) )
        {
          continue;
        }  
        if( line_buffer[i] == '-' || line_buffer[i] == '+' || line_buffer[i] == '.' || line_buffer[i] == 'E' || line_buffer[i] == 'e' )
        {
          // Any float numbers should not be present on this line.
          GNSS_ERROR_MSG( "float numbers should not be present on this line." );
          return FALSE;
        }

        if( !isdigit( line_buffer[i] ) ) // In this case the satellite system type letter is first.
        {
          if( line_buffer[i] != RINEX_SATELLITE_SYSTEM_GPS && 
            line_buffer[i] != RINEX_SATELLITE_SYSTEM_GLO && 
            line_buffer[i] != RINEX_SATELLITE_SYSTEM_GEO && 
            line_buffer[i] != RINEX_SATELLITE_SYSTEM_NSS &&
            line_buffer[i] != RINEX_SATELLITE_SYSTEM_MIXED )
          {
            GNSS_ERROR_MSG( "unexpected" );
            return FALSE;
          }

          if( j != 0 )
          {
            numstr[j] = '\0';
            j = 0;
            if( sscanf( numstr, "%d", &itmp ) != 1 )
            {
              GNSS_ERROR_MSG( "sscanf failed." );
              return FALSE;
            }
            RINEX_sat[count-1].id = (unsigned short)itmp;
            RINEX_sat[count-1].type = next_sat_type;

            count++;
            if( count > RINEX_nr_satellites+1 )
            {
              GNSS_ERROR_MSG( "if( count > RINEX_nr_satellites+1 )" );
              return FALSE; // a very unlikely error condition.
            }
          }          
          next_sat_type = (RINEX_enumSatelliteSystemType)line_buffer[i];
        }
        else
        {
          numstr[j] = line_buffer[i];
          j++;
        }
      }
    }
    // The last satellite id must still be interpreted.
    numstr[j] = '\0';
    j = 0;
    if( sscanf( numstr, "%d", &itmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      return FALSE;
    }
    RINEX_sat[count-1].id = (unsigned short)itmp;
    RINEX_sat[count-1].type = next_sat_type;
    count++;

    if( count != RINEX_nr_satellites+1 )
    {
      GNSS_ERROR_MSG( "if( count != RINEX_nr_satellites+1 )" );
      return FALSE;
    }
  }

  if( RINEX_header->nr_obs_types >= 64 )    
  {
    GNSS_ERROR_MSG( "if( RINEX_header->nr_obs_types >= 64 )" );
    return FALSE; // A very unlikely condition.
  }



  // TIME: The time of the measurement is the receiver time of the received signals.
  // It is identical for the phase and range measurements and is identical for
  // all satellites observed at that epoch. It is expressed in GPS time (not
  // Universal Time). 
  //
  // It is stored in UTC style (year, month, day, etc) BUT is receiver time.

  if( epoch.year >= 80 && epoch.year < 2000 )
  {
    epoch.year += 1900;
  }
  else if( epoch.year >= 0 && epoch.year < 79 )
  {
    epoch.year += 2000;
  }
  else
  {
    GNSS_ERROR_MSG( "unexpected" );
    return FALSE;
  }
  TIMECONV_GetGPSTimeFromRinexTime(
    epoch.year,
    epoch.month,
    epoch.day,
    epoch.hour,
    epoch.minute,
    epoch.seconds,
    &week,
    &tow 
    );

  // Set the receiver time values.
  *rx_gps_week = week;
  *rx_gps_tow = tow;
  

  obsArray_index = 0;
  for( RINEX_sat_index = 0; RINEX_sat_index < (int)RINEX_nr_satellites; RINEX_sat_index++ )
  {
    isL1data_present = FALSE;
    isL2data_present = FALSE;
    overwriteCNoL1 = TRUE;
    overwriteCNoL2 = TRUE;  

    // Set measurement data default to 0.
    memset( &(obsArray[obsArray_index]), 0, sizeof(GNSS_structMeasurement) );

    result = RINEX_GetNextObserationSetForOneSatellite(
      fid,
      RINEX_header,
      wasEndOfFileReached,
      filePosition,      
      RINEX_obs,
      RINEX_MAX_NR_OBS,
      &RINEX_nr_obs,
      RINEX_sat[RINEX_sat_index].type,
      RINEX_sat[RINEX_sat_index].id
      );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_GetNextObserationSetForOneSatellite returned FALSE." );
      return FALSE;
    }

    if( epoch_flag == 6 ) 
    {
      // Cycle slip style observations are present.

      // GDM - ignore for now.
      continue;
    }

    // Set the time.
    obsArray[obsArray_index].tow  =  tow;
    obsArray[obsArray_index].week = week;

    // The channel index is simply the order of the data in this case.
    obsArray[obsArray_index].channel = (unsigned short)obsArray_index;

    obsArray[obsArray_index].id = RINEX_sat[RINEX_sat_index].id;

    // Set default validity flags.
    obsArray[obsArray_index].flags.isEphemerisValid        = 0; // not yet known
    obsArray[obsArray_index].flags.isAlmanacValid          = 0; // not yet known
    obsArray[obsArray_index].flags.isAboveElevationMask    = 0; // not yet known
    obsArray[obsArray_index].flags.isAboveCNoMask          = 0; // not yet known
    obsArray[obsArray_index].flags.isAboveLockTimeMask     = 0; // not yet known
    obsArray[obsArray_index].flags.isNotUserRejected       = 1; // assume not rejected
    obsArray[obsArray_index].flags.isNotPsrRejected        = 1; // assume not rejected
    obsArray[obsArray_index].flags.isNotAdrRejected        = 1; // assume not rejected
    obsArray[obsArray_index].flags.isNotDopplerRejected    = 1; // assume not rejected
    obsArray[obsArray_index].flags.isNoCycleSlipDetected   = 1; // assume no slip
    obsArray[obsArray_index].flags.isPsrUsedInSolution     = 0; // not yet known
    obsArray[obsArray_index].flags.isDopplerUsedInSolution = 0; // not yet known
    obsArray[obsArray_index].flags.isAdrUsedInSolution     = 0; // not yet known
    obsArray[obsArray_index].flags.useTropoCorrection          = 1; // default to yes
    obsArray[obsArray_index].flags.useBroadcastIonoCorrection  = 1; // default to yes



    // The GNSS observation array is channel based. 
    // We must look for matching observation sets to place within the channel based container.
    // e.g. L1, P1, C1, D1 and S1
    // first look for L1, P1, C1, D1 and S1
    
    // Deal with S1 measurements first, so that if present, the RINEX signal strength values
    // are not interpretted.
    for( RINEX_obs_index = 0; RINEX_obs_index < RINEX_nr_obs; RINEX_obs_index++ )
    {
      if( RINEX_obs[RINEX_obs_index].type == RINEX_OBS_TYPE_S1 && RINEX_obs[RINEX_obs_index].isValid )
      {
        obsArray[obsArray_index].system   = RINEX_obs[RINEX_obs_index].system;
        obsArray[obsArray_index].freqType = GNSS_GPSL1; 
        
        overwriteCNoL1 = FALSE;

        // GDM_TODO - A receiver dependant look up table is needed here to convert to 
        // Carrier to noise density ratio values in dB-Hz.
        obsArray[obsArray_index].cno = (float)RINEX_obs[RINEX_obs_index].value; // [receiver dependant!]

        isL1data_present = TRUE;
      }
    }

    for( RINEX_obs_index = 0; RINEX_obs_index < RINEX_nr_obs; RINEX_obs_index++ )
    {
      if( !RINEX_obs[RINEX_obs_index].isValid )
        continue;

      switch(RINEX_obs[RINEX_obs_index].type)
      {
      case RINEX_OBS_TYPE_L1:
        {
          obsArray[obsArray_index].system   = RINEX_obs[RINEX_obs_index].system;
          obsArray[obsArray_index].freqType = GNSS_GPSL1; 

          obsArray[obsArray_index].adr = RINEX_obs[RINEX_obs_index].value; // cycles

          // Set the validity flags
          obsArray[obsArray_index].flags.isActive       = TRUE;
          obsArray[obsArray_index].flags.isCodeLocked   = TRUE;
          obsArray[obsArray_index].flags.isPhaseLocked  = TRUE;
          obsArray[obsArray_index].flags.isParityValid  = TRUE; // Assume valid. No half cycle slips (invalid parity changes to valid partiy causes 1/2 cycle jump).
          obsArray[obsArray_index].flags.isAdrValid     = TRUE;
          obsArray[obsArray_index].flags.isAutoAssigned = TRUE; // Assumed.
          obsArray[obsArray_index].flags.isNoCycleSlipDetected = TRUE;

          isL1data_present = TRUE;

          // Loss of lock indicator really pertains to the phase only.
          switch( RINEX_obs[RINEX_obs_index].loss_of_lock_indicator )
          {
          case 0:
            {
              // OK 
              break;
            }
          case 1: // Loss lock between previous and current observation: cycle clip possible.
            {
              // Assume cycle slip took place.
              obsArray[obsArray_index].flags.isNoCycleSlipDetected = FALSE;
              break;
            }
          case 2:
            {
              // Opposite wavelength factor to the
              // one defined for the satellite by a 
              // previous WAVELENGTH FACT L1/2 line.
              // Valid for the current epoch only.  
              // GDM_TODO parity failure here?
              break;
            }
          case 3:
            { 
              // both 2 and 1
              obsArray[obsArray_index].flags.isNoCycleSlipDetected = FALSE;
              // GDM_TODO 
              break;
            }
          case 5:
            {
              // both 4 and 1
              obsArray[obsArray_index].flags.isNoCycleSlipDetected = FALSE;
              // Intentional fall thru.
            }
          case 4:
            { 
              // Observation under Antispoofing (may suffer from increased noise).
              // GDM_TODO 
              break;
            }
          case 6:
            {
              // both 4 and 2
              // GDM_TODO 
              break;
            }
          case 7:
            {
              // both 4, 2 and 1
              obsArray[obsArray_index].flags.isNoCycleSlipDetected = FALSE;
              // GDM_TODO 
              break;
            }
          default:
            {
              // could be 'blank' or whitespace like '\r' or '\n'
              // ignore
              break;
            }
          }

          if( overwriteCNoL1 )
          {
            result = RINEX_ConvertSignalStrengthToUsableCNo( &(obsArray[obsArray_index].cno), RINEX_obs[RINEX_obs_index].signal_strength );
            if( result == FALSE )
            {
              GNSS_ERROR_MSG( "RINEX_ConvertSignalStrengthToUsableCNo returned FALSE." );
              return FALSE;
            }
          }

          break;  
        }
      case RINEX_OBS_TYPE_C1:
        {
          obsArray[obsArray_index].system   = RINEX_obs[RINEX_obs_index].system;
          obsArray[obsArray_index].freqType = GNSS_GPSL1; 
          obsArray[obsArray_index].codeType = GNSS_CACode; 

          obsArray[obsArray_index].psr = RINEX_obs[RINEX_obs_index].value; // m

          // The observation time convention is 'transmit' time.
          obsArray[obsArray_index].tow  =  tow - obsArray[obsArray_index].psr/LIGHTSPEED;

          // Set the validity flags
          obsArray[obsArray_index].flags.isActive       = TRUE;
          obsArray[obsArray_index].flags.isCodeLocked   = TRUE;
          obsArray[obsArray_index].flags.isPsrValid     = TRUE;
          obsArray[obsArray_index].flags.isAutoAssigned = TRUE; // Assumed.
          isL1data_present = TRUE;

          if( overwriteCNoL1 )
          {
            result = RINEX_ConvertSignalStrengthToUsableCNo( &(obsArray[obsArray_index].cno), RINEX_obs[RINEX_obs_index].signal_strength );
            if( result == FALSE )
            {
              GNSS_ERROR_MSG( "RINEX_ConvertSignalStrengthToUsableCNo returned FALSE." );
              return FALSE;
            }
          }

          break;
        }
      case RINEX_OBS_TYPE_P1:
        {
          obsArray[obsArray_index].system   = RINEX_obs[RINEX_obs_index].system;
          obsArray[obsArray_index].freqType = GNSS_GPSL1; 
          obsArray[obsArray_index].codeType = GNSS_PCode; 

          obsArray[obsArray_index].psr = RINEX_obs[RINEX_obs_index].value; // m

          
          // The observation time convention is 'tranmsit' time.
          obsArray[obsArray_index].tow  =  tow - obsArray[obsArray_index].psr/LIGHTSPEED;

          // Set the validity flags
          obsArray[obsArray_index].flags.isActive       = TRUE;
          obsArray[obsArray_index].flags.isCodeLocked   = TRUE;
          obsArray[obsArray_index].flags.isPsrValid     = TRUE;
          obsArray[obsArray_index].flags.isAutoAssigned = TRUE; // Assumed.
          isL1data_present = TRUE;

          if( overwriteCNoL1 )
          {
            result = RINEX_ConvertSignalStrengthToUsableCNo( &(obsArray[obsArray_index].cno), RINEX_obs[RINEX_obs_index].signal_strength );
            if( result == FALSE )
            {
              GNSS_ERROR_MSG( "RINEX_ConvertSignalStrengthToUsableCNo returned FALSE." );
              return FALSE;
            }
          }

          break;
        }
      case RINEX_OBS_TYPE_D1:
        {
          obsArray[obsArray_index].system   = RINEX_obs[RINEX_obs_index].system;
          obsArray[obsArray_index].freqType = GNSS_GPSL1; 
          obsArray[obsArray_index].doppler = (float)RINEX_obs[RINEX_obs_index].value; // m

          // Trimble R8 receiver data when converted to RINEX have epochs of invalid doppler 
          // where the value output is 0.0. To compensate for this all value of exactly zero
          // are deemed invalid doppler.

          // Set the validity flags
          if( obsArray[obsArray_index].doppler == 0.0 )
          {
            obsArray[obsArray_index].flags.isDopplerValid = FALSE;
          }
          else
          {
            obsArray[obsArray_index].flags.isDopplerValid = TRUE;
            isL1data_present = TRUE;
          }
          break;
        }
      default:
        {
          break;
        }
      }
    }
    if( isL1data_present )
    {
      // Check if no information about cno is present for L1.
      if( obsArray[obsArray_index].cno == 0.0 )
      {
        obsArray[obsArray_index].cno = 32; // A nominally low but useable value [dB-Hz].
      }

      obsArray_index++;
      if( obsArray_index >= maxNrObs )
      {
        GNSS_ERROR_MSG( "if( obsArray_index >= maxNrObs )" );
        return FALSE;
      }
      
      // Set measurement data default to 0.
      memset( &(obsArray[obsArray_index]), 0, sizeof(GNSS_structMeasurement) );

      // Set the time.
      obsArray[obsArray_index].tow  =  tow;
      obsArray[obsArray_index].week = week;

      obsArray[obsArray_index].id = RINEX_sat[RINEX_sat_index].id;

      // The channel index is simply the order of the data in this case.
      obsArray[obsArray_index].channel = (unsigned short)obsArray_index;

      // Set default validity flags.
      obsArray[obsArray_index].flags.isEphemerisValid        = 0; // not yet known
      obsArray[obsArray_index].flags.isAlmanacValid          = 0; // not yet known
      obsArray[obsArray_index].flags.isAboveElevationMask    = 0; // not yet known
      obsArray[obsArray_index].flags.isAboveCNoMask          = 0; // not yet known
      obsArray[obsArray_index].flags.isAboveLockTimeMask     = 0; // not yet known
      obsArray[obsArray_index].flags.isNotUserRejected       = 1; // assume not rejected
      obsArray[obsArray_index].flags.isNotPsrRejected        = 1; // assume not rejected
      obsArray[obsArray_index].flags.isNotAdrRejected        = 1; // assume not rejected
      obsArray[obsArray_index].flags.isNotDopplerRejected    = 1; // assume not rejected
      obsArray[obsArray_index].flags.isNoCycleSlipDetected   = 1; // assume no slip
      obsArray[obsArray_index].flags.isPsrUsedInSolution     = 0; // not yet known
      obsArray[obsArray_index].flags.isDopplerUsedInSolution = 0; // not yet known
      obsArray[obsArray_index].flags.isAdrUsedInSolution     = 0; // not yet known
      obsArray[obsArray_index].flags.useTropoCorrection      = 1; // default to yes
      obsArray[obsArray_index].flags.useBroadcastIonoCorrection  = 1; // default to yes
    }

    
    // Deal with S2 measurements first, so that if present, the RINEX signal strength values
    // are not interpretted.
    for( RINEX_obs_index = 0; RINEX_obs_index < RINEX_nr_obs; RINEX_obs_index++ )
    {
      if( RINEX_obs[RINEX_obs_index].type == RINEX_OBS_TYPE_S2 && RINEX_obs[RINEX_obs_index].isValid )
      {
        obsArray[obsArray_index].system   = RINEX_obs[RINEX_obs_index].system;
        obsArray[obsArray_index].freqType = GNSS_GPSL2; 
        obsArray[obsArray_index].id       = RINEX_obs[RINEX_obs_index].id;

        overwriteCNoL2 = FALSE;

        // GDM_TODO - A receiver dependant look up table is needed here to convert to 
        // Carrier to noise density ratio values in dB-Hz.
        obsArray[obsArray_index].cno = (float)RINEX_obs[RINEX_obs_index].value; // [receiver dependant!]

        isL2data_present = TRUE;
      }
    }


    // Look for L2, P2, D2 and S2
    for( RINEX_obs_index = 0; RINEX_obs_index < RINEX_nr_obs; RINEX_obs_index++ )
    {
      if( !RINEX_obs[RINEX_obs_index].isValid )
        continue;

      switch(RINEX_obs[RINEX_obs_index].type)
      {
      case RINEX_OBS_TYPE_L2:
        {
          obsArray[obsArray_index].system   = RINEX_obs[RINEX_obs_index].system;
          obsArray[obsArray_index].freqType = GNSS_GPSL2; 
          obsArray[obsArray_index].adr = RINEX_obs[RINEX_obs_index].value; // cycles

          // Set the validity flags
          obsArray[obsArray_index].flags.isActive       = TRUE;
          obsArray[obsArray_index].flags.isCodeLocked   = TRUE;
          obsArray[obsArray_index].flags.isPhaseLocked  = TRUE;
          obsArray[obsArray_index].flags.isParityValid  = TRUE; // Assume valid. No half cycle slips (invalid parity changes to valid partiy causes 1/2 cycle jump).
          obsArray[obsArray_index].flags.isAdrValid     = TRUE;
          obsArray[obsArray_index].flags.isAutoAssigned = TRUE; // Assumed.
          obsArray[obsArray_index].flags.isNoCycleSlipDetected = TRUE;
          isL2data_present = TRUE;

          // Loss of lock indicator really pertains to the phase only.
          switch( RINEX_obs[RINEX_obs_index].loss_of_lock_indicator )
          {
          case 0: 
            {
              break; // OK 
            }
          case 1: // Loss lock between previous and current observation: cycle clip possible.
            {
              // Assume cycle slip took place.
              obsArray[obsArray_index].flags.isNoCycleSlipDetected = FALSE;
              break;
            }
          case 2:
            {
              // Opposite wavelength factor to the
              // one defined for the satellite by a 
              // previous WAVELENGTH FACT L1/2 line.
              // Valid for the current epoch only.  
              // GDM_TODO parity failure here?
              break;
            }
          case 3:
            { 
              // both 2 and 1
              obsArray[obsArray_index].flags.isNoCycleSlipDetected = FALSE;
              // GDM_TODO 
              break;
            }
          case 5:
            {
              // both 4 and 1
              obsArray[obsArray_index].flags.isNoCycleSlipDetected = FALSE;
              // Intentional fall thru.
            }
          case 4:
            { 
              // Observation under Antispoofing (may suffer from increased noise).
              // GDM_TODO 
              break;
            }
          case 6:
            {
              // both 4 and 2
              // GDM_TODO 
              break;
            }
          case 7:
            {
              // both 4, 2 and 1
              obsArray[obsArray_index].flags.isNoCycleSlipDetected = FALSE;
              // GDM_TODO 
              break;
            }
          default:
            {
              // could be 'blank' or whitespace like '\r' or '\n'
              // ignore
              break;
            }
          }

          if( overwriteCNoL2 )
          {
            result = RINEX_ConvertSignalStrengthToUsableCNo( &(obsArray[obsArray_index].cno), RINEX_obs[RINEX_obs_index].signal_strength );
            if( result == FALSE )
            {
              GNSS_ERROR_MSG( "RINEX_ConvertSignalStrengthToUsableCNo returned FALSE." );
              return FALSE;
            }
          }

          break;  
        }
      case RINEX_OBS_TYPE_P2:
        {
          obsArray[obsArray_index].system   = RINEX_obs[RINEX_obs_index].system;
          obsArray[obsArray_index].freqType = GNSS_GPSL2; 
          obsArray[obsArray_index].codeType = GNSS_PCode; 

          obsArray[obsArray_index].psr = RINEX_obs[RINEX_obs_index].value; // m

          // The observation time convention is 'tranmsit' time.
          obsArray[obsArray_index].tow  =  tow - obsArray[obsArray_index].psr/LIGHTSPEED;

          // Set the validity flags
          obsArray[obsArray_index].flags.isActive       = TRUE;
          obsArray[obsArray_index].flags.isCodeLocked   = TRUE;
          obsArray[obsArray_index].flags.isPsrValid     = TRUE;
          obsArray[obsArray_index].flags.isAutoAssigned = TRUE; // Assumed.
          isL2data_present = TRUE;

          if( overwriteCNoL2 )
          {
            result = RINEX_ConvertSignalStrengthToUsableCNo( &(obsArray[obsArray_index].cno), RINEX_obs[RINEX_obs_index].signal_strength );
            if( result == FALSE )
            {
              GNSS_ERROR_MSG( "RINEX_ConvertSignalStrengthToUsableCNo returned FALSE." );
              return FALSE;
            }
          }

          break;
        }
      case RINEX_OBS_TYPE_D2:
        {
          obsArray[obsArray_index].system   = RINEX_obs[RINEX_obs_index].system;
          obsArray[obsArray_index].freqType = GNSS_GPSL2; 
          obsArray[obsArray_index].doppler = (float)RINEX_obs[RINEX_obs_index].value; // m

          // Trimble R8 receiver data when converted to RINEX have epochs of invalid doppler 
          // where the value output is 0.0. To compensate for this all value of exactly zero
          // are deemed invalid doppler.

          // Set the validity flags
          if( obsArray[obsArray_index].doppler == 0.0 )
          {
            obsArray[obsArray_index].flags.isDopplerValid = FALSE;
          }
          else
          {
            obsArray[obsArray_index].flags.isDopplerValid = TRUE;
            isL2data_present = TRUE;
          }
          break;
        }
      default:
        {
          break;
        }
      }
    }

    if( isL2data_present )
    {
      // Check if no information about cno is present for L2.
      if( obsArray[obsArray_index].cno == 0.0 )
      {
        obsArray[obsArray_index].cno = 32; // A nominally low but useable value [dB-Hz].
      }

      obsArray_index++;
      if( obsArray_index >= maxNrObs )
      {
        GNSS_ERROR_MSG( "if( obsArray_index >= maxNrObs )" );
        return FALSE;
      }
    }

    // Note that T1 and T2 measurements are not supported.
  }

  *nrObs = obsArray_index;
  *wasObservationFound = TRUE;

  return TRUE;
}


BOOL RINEX_DecodeGPSNavigationFile(
  const char *filepath,                          //!< (input) The file path to the GPS Navigation message file.
  GNSS_structKlobuchar *iono_model,              //!< (input/output) A pointer to the ionospheric parameters struct.
  GPS_structEphemeris *ephemeris_array,          //!< (input/output) A pointer to the GPS ephemeris array.
  const unsigned int max_length_ephemeris_array, //!< (input) The maximum size of the GPS ephemeris array.
  unsigned int *length_ephemeris_array           //!< (input/output) The length of the GPS ephemeris array after decoding. The number of valid items.
  )
{
  char RINEX_header[RINEX_HEADER_SIZE];
  unsigned RINEX_header_length = 0;
  double version = 0.0;
  RINEX_enumFileType file_type = RINEX_FILE_TYPE_UNKNOWN;
  char line_buffer[RINEX_LINEBUF_SIZE];
  unsigned nr_lines = 0;
  BOOL result;
  FILE* fid = NULL;
  GPS_structEphemeris eph; // A single ephemeris record.
  RINEX_TIME epoch;
  unsigned i = 0;
  unsigned count = 0;
  double tow = 0;
  int itmp = 0;
  int itmp2 = 0;
  double dtmp = 0.0;
  unsigned short week = 0;
  unsigned ephemeris_array_index = 0;
  size_t length = 0;

  char station_name[5];
  unsigned short dayofyear = 0;
  unsigned char file_sequence_nr = 0;
  unsigned short year = 0;

  double header_A0; // DELTA-UTC: A0,A1,T,W
  double header_A1; // DELTA-UTC: A0,A1,T,W
  int header_week;  // DELTA-UTC: A0,A1,T,W
  int header_tow;   // DELTA-UTC: A0,A1,T,W
  
  char str[10][20]; // A 2D array of strings of length 20;

  memset( &eph, 0, sizeof(GPS_structEphemeris) );

  epoch.time_system = RINEX_TIME_SYSTEM_GPS;
  
  if( filepath == NULL )
  {
    GNSS_ERROR_MSG( "if( filepath == NULL )" );
    return FALSE;
  }
  if( iono_model == NULL )
  {
    GNSS_ERROR_MSG( "if( iono_model == NULL )" );
    return FALSE;
  }
  if( ephemeris_array == NULL )
  {
    GNSS_ERROR_MSG( "if( ephemeris_array == NULL )" );
    return FALSE;
  }
  if( length_ephemeris_array == NULL )
  {
    GNSS_ERROR_MSG( "if( length_ephemeris_array == NULL )" );
    return FALSE;
  }
  if( max_length_ephemeris_array == 0 )
  {
    GNSS_ERROR_MSG( "if( max_length_ephemeris_array == 0 )" );
    return FALSE;
  }

  iono_model->isValid = FALSE;

  result = RINEX_GetHeader( 
    filepath,
    RINEX_header,
    RINEX_HEADER_SIZE,
    &RINEX_header_length,
    &version,
    &file_type
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_GetHeader returned FALSE." );
    return FALSE;
  }

  if( file_type != RINEX_FILE_TYPE_GPS_NAV )
  {
    GNSS_ERROR_MSG( "if( file_type != RINEX_FILE_TYPE_GPS_NAV )" );
    return FALSE;
  }

  if( fabs( version - 2.1 ) < 1e-06 || 
    fabs( version - 2.0 ) < 1e-06 ||
    fabs( version - 2.11 ) < 1e-06  ||
    fabs( version - 2.12 ) < 1e-06 )
  {
    // this version is valid for decoding
  }
  else
  {    
    GNSS_ERROR_MSG( "RINEX version not supported." );
    return FALSE;
  }

  result = RINEX_get_header_lines(
    RINEX_header,
    RINEX_header_length,
    "ION ALPHA",
    line_buffer,
    RINEX_LINEBUF_SIZE,
    &nr_lines
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
    return FALSE;
  }
  if( nr_lines == 1 )
  {
    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
      return FALSE;
    }

    if( sscanf( line_buffer, "%lf %lf %lf %lf", 
      &(iono_model->alpha0), 
      &(iono_model->alpha1),
      &(iono_model->alpha2),
      &(iono_model->alpha3) ) != 4 )
    {
      GNSS_ERROR_MSG( "sscanf returned FALSE." );
      return FALSE; // bad header?
    }
    result = RINEX_get_header_lines(
      RINEX_header,
      RINEX_header_length,
      "ION BETA",
      line_buffer,
      RINEX_LINEBUF_SIZE,
      &nr_lines
      );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
      return FALSE;
    }
    if( nr_lines != 1 )
    {
      GNSS_ERROR_MSG( "if( nr_lines != 1 )" );
      return FALSE; // weird header
    }

    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
      return FALSE;
    }

    if( sscanf( line_buffer, "%lf %lf %lf %lf", 
      &(iono_model->beta0), 
      &(iono_model->beta1),
      &(iono_model->beta2),
      &(iono_model->beta3) ) != 4 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      return FALSE; // bad header?
    }

    result = RINEX_get_header_lines(
      RINEX_header,
      RINEX_header_length,
      "DELTA-UTC: A0,A1,T,W",
      line_buffer,
      RINEX_LINEBUF_SIZE,
      &nr_lines
      );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
      return FALSE;
    }
    if( nr_lines == 1 )
    {
      result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
      if( result == FALSE )
      {
        GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
        return FALSE;
      }

      if( sscanf( line_buffer, "%lf %lf %d %d",
        &header_A0,
        &header_A1,
        &header_tow,
        &header_week ) == 4 )
      {
        iono_model->week = (unsigned short) header_week;
        iono_model->tow = header_tow;
        iono_model->isValid = TRUE;
      }
    }

    if( iono_model->isValid == FALSE )
    {
      // Decode the year and day of year from the file name.
      result = RINEX_DecodeFileName( filepath, station_name, &dayofyear, &file_sequence_nr, &year, &file_type );
      if( result == TRUE )
      {
        result = TIMECONV_GetGPSTimeFromYearAndDayOfYear(
          year,
          dayofyear,
          &(iono_model->week),
          &dtmp
          );
        if( result == FALSE )
        {
          GNSS_ERROR_MSG( "TIMECONV_GetGPSTimeFromYearAndDayOfYear returned FALSE." );
          return FALSE;
        }
        iono_model->tow = (unsigned)dtmp;
        iono_model->isValid = TRUE;
      }
      else
      {
        // Get the Iono model time from the first ephemeris record in the file if there is one.
        iono_model->isValid = FALSE;
      }
    }
  }

  fid = fopen( filepath, "r" );
  if( fid == NULL )
  {
    GNSS_ERROR_MSG( "if( fid == NULL )" );
    return FALSE;
  }

  do
  {
    // Get the next line from the file.
    if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
    {
      if( feof(fid) )
      {      
        fclose(fid);
        return TRUE;
      }
      else
      {
        GNSS_ERROR_MSG( "unexpected" );
        fclose(fid);
        return FALSE;
      }
    }
  }
  while( strstr( line_buffer, "END OF HEADER" ) == NULL );


  while( !feof(fid) && !ferror(fid) && ephemeris_array_index < max_length_ephemeris_array )
  {
    // Get the next line from the file.
    if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
    {
      if( feof(fid) )
      {      
        break;
      }
      else
      {
        GNSS_ERROR_MSG( "unexpected" );
        fclose(fid);
        return FALSE;
      }
    }
    // Check that this line is now empty.
    length = strlen(line_buffer);
    for( i = 0; i < length; i++ )
    {
      if( isalnum(line_buffer[i]) )
        break;
    }
    if( i == length )
      continue; // This string is empty.

    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)length );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
      fclose(fid);
      return FALSE;
    }

    memset( str, 0, sizeof(char)*10*20 );  
    count = sscanf( line_buffer, "%2c%*c%2c%*c%2c%*c%2c%*c%2c%*c%2c%5c%19c%19c%19c",
      str[0],
      str[1],
      str[2],
      str[3],
      str[4],
      str[5],
      str[6],
      str[7],
      str[8],
      str[9]
      );

    if( count != 10 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );    
      fclose(fid);
      return FALSE; // bad record
    }

    i = 0;
    if( sscanf( str[i], "%d", &itmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );    
      fclose(fid);
      return FALSE;
    }
    eph.prn = (unsigned short)itmp;
    i++;
    if( sscanf( str[i], "%d", &itmp  ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );    
      fclose(fid);
      return FALSE;
    }
    epoch.year = (unsigned short)itmp;
    i++;
    if( sscanf( str[i], "%d", &itmp  ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );    
      fclose(fid);
      return FALSE;
    }
    epoch.month = (unsigned char)itmp;
    i++;
    if( sscanf( str[i], "%d", &itmp  ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );    
      fclose(fid);
      return FALSE;
    }
    epoch.day = (unsigned char)itmp;
    i++;
    if( sscanf( str[i], "%d", &itmp  ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );    
      fclose(fid);
      return FALSE;
    }
    epoch.hour = (unsigned char)itmp;
    i++;
    if( sscanf( str[i], "%d", &itmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );    
      fclose(fid);
      return FALSE;
    }
    epoch.minute = (unsigned char)itmp;
    i++;
    if( sscanf( str[i], "%f", &epoch.seconds ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );    
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.af0 ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );    
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.af1 ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );    
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.af2 ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );    
      fclose(fid);
      return FALSE;
    }

    if( epoch.year >= 80 && epoch.year < 2000 )
    {
      epoch.year += 1900;
    }
    else if( epoch.year >= 0 && epoch.year < 79 )
    {
      epoch.year += 2000;
    }
    else
    {
      GNSS_ERROR_MSG( "unexpected." );    
      fclose(fid);
      return FALSE;
    }
    result = TIMECONV_GetGPSTimeFromRinexTime(
      epoch.year,
      epoch.month,
      epoch.day,
      epoch.hour,
      epoch.minute,
      epoch.seconds,
      &week,
      &tow 
      );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "TIMECONV_GetGPSTimeFromRinexTime returned FALSE." );    
      fclose(fid);
      return FALSE;
    }
    eph.toc = (unsigned)tow;


    // -------------------
    // BROADCAST ORBIT - 1

    // Get the next line from the file.
    if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
    {
      if( feof(fid) )
      {      
        break;
      }
      else
      {
        GNSS_ERROR_MSG( "unexpected." );
        fclose(fid);
        return FALSE;
      }
    }
    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
      fclose(fid);
      return FALSE;
    }

    memset( str, 0, sizeof(char)*10*20 );  
    count = sscanf( line_buffer, "%*3c%19c%19c%19c%19c",
      str[0],
      str[1],
      str[2],
      str[3]
      );
    if( count != 4 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }

    i = 0;
    if( sscanf( str[i], "%lf", &dtmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    eph.iode = (unsigned char)dtmp;
    i++;
    if( sscanf( str[i], "%lf", &eph.crs ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.delta_n ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.m0 ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;

    // -------------------
    // BROADCAST ORBIT - 2

    // Get the next line from the file.
    if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
    {
      if( feof(fid) )
      {      
        break;
      }
      else
      {
        GNSS_ERROR_MSG( "unexpected." );
        fclose(fid);
        return FALSE;
      }
    }
    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
      fclose(fid);
      return FALSE;
    }

    memset( str, 0, sizeof(char)*10*20 );  
    count = sscanf( line_buffer, "%*3c%19c%19c%19c%19c",
      str[0],
      str[1],
      str[2],
      str[3]
      );
    if( count != 4 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );      
      fclose(fid);
      return FALSE;
    }

    i = 0;
    if( sscanf( str[i], "%lf", &eph.cuc ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );      
      fclose(fid);
      return FALSE;    
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.ecc ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );      
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.cus ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );      
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.sqrta ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );      
      fclose(fid);
      return FALSE;
    }
    i++;


    // -------------------
    // BROADCAST ORBIT - 3

    // Get the next line from the file.
    if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
    {
      if( feof(fid) )
      {      
        break;
      }
      else
      {
        GNSS_ERROR_MSG( "unexpected." );      
        fclose(fid);
        return FALSE;
      }
    }
    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );      
      fclose(fid);
      return FALSE;
    }

    memset( str, 0, sizeof(char)*10*20 );  
    count = sscanf( line_buffer, "%*3c%19c%19c%19c%19c",
      str[0],
      str[1],
      str[2],
      str[3]
      );
    if( count != 4 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }

    i = 0;
    if( sscanf( str[i], "%lf", &dtmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;    
    }
    eph.toe = (unsigned)dtmp;
    i++;
    if( sscanf( str[i], "%lf", &eph.cic ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.omega0 ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.cis ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;


    // -------------------
    // BROADCAST ORBIT - 4

    // Get the next line from the file.
    if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
    {
      if( feof(fid) )
      {      
        break;
      }
      else
      {
        GNSS_ERROR_MSG( "unexpected." );
        fclose(fid);
        return FALSE;
      }
    }
    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
      fclose(fid);
      return FALSE;
    }

    memset( str, 0, sizeof(char)*10*20 );  
    count = sscanf( line_buffer, "%*3c%19c%19c%19c%19c",
      str[0],
      str[1],
      str[2],
      str[3]
      );
    if( count != 4 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }

    i = 0;
    if( sscanf( str[i], "%lf", &eph.i0 ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;    
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.crc ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.w ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &eph.omegadot ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;


    // -------------------
    // BROADCAST ORBIT - 5

    // Get the next line from the file.
    if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
    {
      if( feof(fid) )
      {      
        break;
      }
      else
      {
        GNSS_ERROR_MSG( "unexpected." );
        fclose(fid);
        return FALSE;
      }
    }
    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
      fclose(fid);
      return FALSE;
    }

    memset( str, 0, sizeof(char)*10*20 );  
    count = sscanf( line_buffer, "%*3c%19c%19c%19c%19c",
      str[0],
      str[1],
      str[2],
      str[3]
      );
    if( count != 4 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }

    i = 0;
    if( sscanf( str[i], "%lf", &eph.idot ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &dtmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    eph.code_on_L2 = (unsigned char)dtmp;
    i++;
    if( sscanf( str[i], "%lf", &dtmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    eph.week = (unsigned short)dtmp;
    i++;
    if( sscanf( str[i], "%lf", &dtmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    eph.L2_P_data_flag = (unsigned char)dtmp;
    i++;

    // -------------------
    // BROADCAST ORBIT - 6

    // Get the next line from the file.
    if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
    {
      if( feof(fid) )
      {      
        break;
      }
      else
      {
        GNSS_ERROR_MSG( "unexpected" );
        fclose(fid);
        return FALSE;
      }
    }
    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
      fclose(fid);
      return FALSE;
    }

    memset( str, 0, sizeof(char)*10*20 );  
    count = sscanf( line_buffer, "%*3c%19c%19c%19c%19c",
      str[0],
      str[1],
      str[2],
      str[3]
      );
    if( count != 4 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }

    i = 0;
    if( sscanf( str[i], "%lf", &dtmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;
    // This ura is in meters and the GPS_ephemeris is a ura index. Convert it.
    result = RINEX_ConvertURA_meters_to_URA_index( dtmp, &eph.ura ); 
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ConvertURA_meters_to_URA_index returned FALSE." );
      fclose(fid);
      return FALSE;
    }

    if( sscanf( str[i], "%lf", &dtmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    eph.health = (unsigned char)dtmp;
    i++;
    if( sscanf( str[i], "%lf", &eph.tgd ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    i++;
    if( sscanf( str[i], "%lf", &dtmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf failed." );
      fclose(fid);
      return FALSE;
    }
    eph.iodc = (unsigned short)dtmp;
    i++;

    // -------------------
    // BROADCAST ORBIT - 7

    // Get the next line from the file.
    if( fgets(line_buffer, RINEX_LINEBUF_SIZE, fid) == NULL )
    {
      if( feof(fid) )
      {      
        break;
      }
      else
      {
        GNSS_ERROR_MSG( "unexpected" );
        fclose(fid);
        return FALSE;
      }
    }
    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
      fclose(fid);
      return FALSE;
    }

    memset( str, 0, sizeof(char)*10*20 );  
    count = sscanf( line_buffer, "%*3c%19c%19c",
      str[0],
      str[1]
      );
    if( count != 2 )
    {
      GNSS_ERROR_MSG( "sscanf returned FALSE." );
      fclose(fid);
      return FALSE;
    }

    i = 0;
    if( sscanf( str[i], "%lf", &dtmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf returned FALSE." );
      fclose(fid);
      return FALSE;    
    }
    
    eph.tow = (unsigned)dtmp;
    itmp  = (int)eph.tow;
    itmp2 = (int)eph.toe;
    if( (itmp-itmp2) < -4*SECONDS_IN_DAY )
    {
      eph.tow_week = eph.week+1;
    }
    else
    {
      eph.tow_week = eph.week;
    }
/*
    i++;
    if( sscanf( str[i], "%lf", &dtmp ) != 1 )
    {
      GNSS_ERROR_MSG( "sscanf returned FALSE." );
      fclose(fid);
      return FALSE;    
    }
    itmp = (int)dtmp;
    if( itmp <= 4 )
      eph.fit_interval_flag = 0;
    else
      eph.fit_interval_flag = 1;
*/    
    ephemeris_array[ephemeris_array_index] = eph;
    ephemeris_array_index++;    

    if( ephemeris_array_index == 0 && iono_model->isValid == FALSE )
    {
      // Get the model time from the first ephemeris data for the iono model.
      iono_model->week = eph.week;
      iono_model->tow = eph.toe;
      iono_model->isValid = TRUE;
    }    
  }

  *length_ephemeris_array = ephemeris_array_index;
  fclose(fid);
  return TRUE;
}


// static
BOOL RINEX_ReplaceDwithE( char *str, const unsigned length )
{
  unsigned i = 0;
  if( str == NULL )
  {
    GNSS_ERROR_MSG( "if( str == NULL )" );
    return FALSE;    
  }
  if( length == 0 )
  {
    GNSS_ERROR_MSG( "if( length == 0 )" );
    return FALSE;
  }

  for( i = 0; i < length; i++ )
  {
    if( str[i] == 'D' || str[i] == 'd' )
      str[i] = 'E';
  }
  return TRUE;
}

// static
BOOL RINEX_ConvertURA_meters_to_URA_index( 
  double ura_m,         //!< The user range accuracy [m].
  unsigned char *ura    //!< The user range accuracy index.
  )
{
  unsigned char i = 0;

  if( ura == FALSE )
  {
    GNSS_ERROR_MSG( "if( ura == FALSE )" );
    return FALSE;
  }

  *ura = 15; // by default.

  for( i = 0; i < 16; i++ )
  {
    if( ura_m >= RINEX_MIN_URA[i] && ura_m < RINEX_MAX_URA[i] )
    {
      *ura = i;
      break;
    }
  }

  return TRUE;
}


BOOL RINEX_DecodeFileName(
  const char *filepath,            //!< (input) A full filepath.
  char *station_name,              //!< (output) A 5 character C string. char station_name[5]. In which to place the 4-character station name designator. This must be at least 5 characters.
  unsigned short *dayofyear,       //!< (output) The day of year.
  unsigned char *file_sequence_nr, //!< (output) file sequence number within day. 0: file contains all the existing data of the current day.
  unsigned short *year,            //!< (output) The full year. e.g. 1999, 2001.
  RINEX_enumFileType *filetype     //!< (output) The RINEX file type.
  )
{
  char str[RINEX_LINEBUF_SIZE];
  char filename[RINEX_LINEBUF_SIZE];
  size_t length = 0;
  char *pch = NULL;
  char filetype_char = 0;
  unsigned count = 0;
  int itmp[3];

  // Check input.
  if( filepath == NULL )
  {
    GNSS_ERROR_MSG( "if( filepath == NULL )" );
    return FALSE;
  }
  if( station_name == NULL )
  {
    GNSS_ERROR_MSG( "if( station_name == NULL )" );
    return FALSE;
  }
  if( dayofyear == NULL )
  {
    GNSS_ERROR_MSG( "if( dayofyear == NULL )" );
    return FALSE;
  }
  if( file_sequence_nr == NULL )
  {
    GNSS_ERROR_MSG( "if( file_sequence_nr == NULL )" );
    return FALSE;
  }
  if( year == NULL )
  {
    GNSS_ERROR_MSG( "if( year == NULL )" );
    return FALSE;
  }
  if( filetype == NULL )
  {
    GNSS_ERROR_MSG( "if( filetype == NULL )" );
    return FALSE;
  }

  station_name[0] = station_name[1] = station_name[2] = station_name[3] = station_name[4]  = '\0';

  length = strlen( filepath );
  if( length >= RINEX_LINEBUF_SIZE )
  {
    GNSS_ERROR_MSG( "if( length >= RINEX_LINEBUF_SIZE )" );
    return FALSE;
  }

  // Copy the file path.
  strcpy( str, filepath );

  // Tokenize the string copy.
  pch = strtok( str,"\\/" );
  while (pch != NULL)
  {
    strcpy( filename, pch );
    pch = strtok( NULL, "\\/" );
  }

  length = strlen( filename );
  if( length != 12 )
  {
    GNSS_ERROR_MSG( "if( length != 12 )" );
    return FALSE;
  }

  // Decode the filename.
  count = sscanf( filename, "%4c%3d%1d%*c%2d%c", 
    station_name,
    &(itmp[0]),
    &(itmp[1]),
    &(itmp[2]),
    &filetype_char
    );
  if( count != 5 )
  {
    GNSS_ERROR_MSG( "sscanf failed." );
    return FALSE;
  }

  filetype_char = (char)toupper(filetype_char);

  *dayofyear = (unsigned short)itmp[0];
  *file_sequence_nr = (unsigned char)itmp[1];
  *year = (unsigned short)itmp[2];
  

  if( *year >= 80 && *year < 2000 )
  {
    *year += 1900;
  }
  else if( *year >= 0 && *year < 79 )
  {
    *year += 2000;
  }
  else
  {
    GNSS_ERROR_MSG( "unexpected" );
    return FALSE;
  }

  *filetype = (RINEX_enumFileType)filetype_char;

  switch( *filetype )
  {
  case RINEX_FILE_TYPE_OBS:
  case RINEX_FILE_TYPE_GPS_NAV:
  case RINEX_FILE_TYPE_MET:
  case RINEX_FILE_TYPE_GLO_NAV:
  case RINEX_FILE_TYPE_GEO_NAV:
  case RINEX_FILE_TYPE_GAL_NAV:
  case RINEX_FILE_TYPE_MIXED_NAV:
  case RINEX_FILE_TYPE_SBAS:
  case RINEX_FILE_TYPE_CLK:
  case RINEX_FILE_TYPE_SUMMARY:
    {
    return TRUE;
    }
  default:
    {
      GNSS_ERROR_MSG( "unexpected default case" );
      return FALSE;
    }
  }
}


BOOL RINEX_GetKlobucharIonoParametersFromNavFile(
  const char *filepath,             //!< (input) The file path to the GPS Navigation message file.
  GNSS_structKlobuchar *iono_model  //!< (input/output) A pointer to the ionospheric parameters struct.
  )
{
  char RINEX_header[RINEX_HEADER_SIZE];
  unsigned RINEX_header_length = 0;
  double version = 0.0;
  RINEX_enumFileType file_type = RINEX_FILE_TYPE_UNKNOWN;
  char line_buffer[RINEX_LINEBUF_SIZE];
  unsigned nr_lines = 0;
  BOOL result;
  double dtmp = 0.0;
  
  char station_name[5];
  unsigned short dayofyear = 0;
  unsigned char file_sequence_nr = 0;
  unsigned short year = 0;

  double header_A0; // DELTA-UTC: A0,A1,T,W
  double header_A1; // DELTA-UTC: A0,A1,T,W
  int header_week;  // DELTA-UTC: A0,A1,T,W
  int header_tow;   // DELTA-UTC: A0,A1,T,W
  
  memset( iono_model, 0, sizeof(GNSS_structKlobuchar) );

  if( filepath == NULL )
  {
    GNSS_ERROR_MSG( "if( filepath == NULL )" );
    return FALSE;
  }
  if( iono_model == NULL )
  {
    GNSS_ERROR_MSG( "if( iono_model == NULL )" );
    return FALSE;
  }


  result = RINEX_GetHeader( 
    filepath,
    RINEX_header,
    RINEX_HEADER_SIZE,
    &RINEX_header_length,
    &version,
    &file_type
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_GetHeader returned FALSE." );
    return FALSE;
  }

  if( file_type != RINEX_FILE_TYPE_GPS_NAV )
  {
    GNSS_ERROR_MSG( "if( file_type != RINEX_FILE_TYPE_GPS_NAV )" );
    return FALSE;
  }

  result = RINEX_get_header_lines(
    RINEX_header,
    RINEX_header_length,
    "ION ALPHA",
    line_buffer,
    RINEX_LINEBUF_SIZE,
    &nr_lines
    );
  if( result == FALSE )
  {
    GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
    return FALSE;
  }
  if( nr_lines == 1 )
  {
    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
      return FALSE;
    }

    if( sscanf( line_buffer, "%lf %lf %lf %lf", 
      &(iono_model->alpha0), 
      &(iono_model->alpha1),
      &(iono_model->alpha2),
      &(iono_model->alpha3) ) != 4 )
    {
      GNSS_ERROR_MSG( "sscanf failed" );
      return FALSE; // bad header?
    }
    result = RINEX_get_header_lines(
      RINEX_header,
      RINEX_header_length,
      "ION BETA",
      line_buffer,
      RINEX_LINEBUF_SIZE,
      &nr_lines
      );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
      return FALSE;
    }
    if( nr_lines != 1 )
    {
      GNSS_ERROR_MSG( "if( nr_lines != 1 )" );
      return FALSE; // weird header
    }

    result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
      return FALSE;
    }

    if( sscanf( line_buffer, "%lf %lf %lf %lf", 
      &(iono_model->beta0), 
      &(iono_model->beta1),
      &(iono_model->beta2),
      &(iono_model->beta3) ) != 4 )
    {
      GNSS_ERROR_MSG( "sscanf failed" );
      return FALSE; // bad header?
    }

    result = RINEX_get_header_lines(
      RINEX_header,
      RINEX_header_length,
      "DELTA-UTC: A0,A1,T,W",
      line_buffer,
      RINEX_LINEBUF_SIZE,
      &nr_lines
      );
    if( result == FALSE )
    {
      GNSS_ERROR_MSG( "RINEX_get_header_lines returned FALSE." );
      return FALSE;
    }
    if( nr_lines == 1 )
    {
      result = RINEX_ReplaceDwithE( line_buffer, (unsigned)strlen(line_buffer) );
      if( result == FALSE )
      {
        GNSS_ERROR_MSG( "RINEX_ReplaceDwithE returned FALSE." );
        return FALSE;
      }

      if( sscanf( line_buffer, "%lf %lf %d %d",
        &header_A0,
        &header_A1,
        &header_tow,
        &header_week ) == 4 )
      {
        iono_model->week = (unsigned short) header_week;
        iono_model->tow = header_tow;
        iono_model->isValid = TRUE;
      }
    }

    if( iono_model->isValid == FALSE )
    {
      // Decode the year and day of year from the file name.
      result = RINEX_DecodeFileName( filepath, station_name, &dayofyear, &file_sequence_nr, &year, &file_type );
      if( result == TRUE )
      {
        result = TIMECONV_GetGPSTimeFromYearAndDayOfYear(
          year,
          dayofyear,
          &(iono_model->week),
          &dtmp
          );
        if( result == FALSE )
        {
          GNSS_ERROR_MSG( "TIMECONV_GetGPSTimeFromYearAndDayOfYear returned FALSE." );
          return FALSE;
        }
        iono_model->tow = (unsigned)dtmp;
        iono_model->isValid = TRUE;
      }
      else
      {
        // Get the Iono model time from the first ephemeris record in the file if there is one.
        iono_model->isValid = FALSE;
      }
    }
  }

  return TRUE;
}

