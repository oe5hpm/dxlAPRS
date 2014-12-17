/**
\file    yuma.c
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

#include <stdio.h>
#include <string.h>
#include "gnss_error.h"
#include "yuma.h"

#ifndef WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif

BOOL YUMA_ReadAlmanacDataFromFile(
  const char* yumaFilePath,   //!< path to the input YUMA ASCII file
  YUMA_structAlmanac* alm,    //!< pointer to an array of YUMA almanac structs
  unsigned char  max_to_read, //!< length of the array
  unsigned char* number_read  //!< number of almanac items read
  )
{
  FILE* in;            // the input file pointer
  char buffer[512];    // a buffer big enough to hold one line from the almanac file
  char synca, syncb;   // two characters used to sync the file (find the first "ID" string)
  int syncFound = 0;   // bool to indicate if "ID" was found in the file
  int n;               // counter to indicate the number of items read by sscanf_s
  int i;               // counter
  unsigned utmp;
  
  // initialize
  *number_read = 0;  

  // check stupid error
  if( max_to_read < 1 )
  {
    GNSS_ERROR_MSG( "if( max_to_read < 1 )" );
    return FALSE;
  }

#ifndef _CRT_SECURE_NO_DEPRECATE
  if( fopen_s( &in, yumaFilePath, "r" ) != 0 )
  {
    char msg[128];
    sprintf( msg, "Unable to open %s.", yumaFilePath );
    GNSS_ERROR_MSG( msg );
    return FALSE;
  }
  if( !in )
  {
    char msg[128];
    sprintf( msg, "Unable to open %s.", yumaFilePath );
    GNSS_ERROR_MSG( msg );    
    return FALSE;
  }
#else
  in = fopen( yumaFilePath, "r" );
  if( !in )
  {
    char msg[128];
    sprintf( msg, "Unable to open %s.", yumaFilePath );
    GNSS_ERROR_MSG( msg );    
    return FALSE;
  }
#endif

  // sync with the first "ID" found in the file
  synca = (char)fgetc(in);  
  while( !feof(in) )
  {
    syncb = (char)fgetc(in);
    if( synca == 'I' && syncb == 'D' )
    {
      syncFound = 1;
      break;
    }
    else
    {
      synca = syncb;
    }
  }
  if( !syncFound )
  {
    fclose(in);
    GNSS_ERROR_MSG( "if( !syncFound )" );
    return FALSE;
  }

  // rewind by two bytes to make things easy below
  fseek(in, -2, SEEK_CUR );

  i = 0;
  while( !feof(in) && i < max_to_read )
  {
#ifndef _CRT_SECURE_NO_DEPRECATE
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "ID:                         %u",  &utmp );              if( n != 1 ){break;} alm[i].prn = (unsigned short)utmp;
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "Health:                     %u",  &utmp );              if( n != 1 ){break;} alm[i].health = (unsigned char)utmp;
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "Eccentricity:               %lf", &(alm[i].ecc) );      if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "Time of Applicability(s):   %lf", &(alm[i].toa) );      if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "Orbital Inclination(rad):   %lf", &(alm[i].i0) );       if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "Rate of Right Ascen(r/s):   %lf", &(alm[i].omegadot) ); if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "SQRT(A)  (m 1/2):           %lf", &(alm[i].sqrta) );    if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "Right Ascen at Week(rad):   %lf", &(alm[i].omega0) );   if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "Argument of Perigee(rad):   %lf", &(alm[i].w) );        if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "Mean Anom(rad):             %lf", &(alm[i].m0) );       if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "Af0(s):                     %lf", &(alm[i].af0) );      if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "Af1(s/s):                   %lf", &(alm[i].af1) );      if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf_s( buffer, "week:                       %u",  &utmp );              if( n != 1 ){break;} alm[i].week = (unsigned short)utmp;
#else
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "ID:                         %u",  &utmp );              if( n != 1 ){break;} alm[i].prn = (unsigned short)utmp;
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "Health:                     %u",  &utmp );              if( n != 1 ){break;} alm[i].health = (unsigned char)utmp;
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "Eccentricity:               %lf", &(alm[i].ecc) );      if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "Time of Applicability(s):   %lf", &(alm[i].toa) );      if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "Orbital Inclination(rad):   %lf", &(alm[i].i0) );       if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "Rate of Right Ascen(r/s):   %lf", &(alm[i].omegadot) ); if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "SQRT(A)  (m 1/2):           %lf", &(alm[i].sqrta) );    if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "Right Ascen at Week(rad):   %lf", &(alm[i].omega0) );   if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "Argument of Perigee(rad):   %lf", &(alm[i].w) );        if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "Mean Anom(rad):             %lf", &(alm[i].m0) );       if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "Af0(s):                     %lf", &(alm[i].af0) );      if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "Af1(s/s):                   %lf", &(alm[i].af1) );      if( n != 1 ){break;}
    if( !fgets( buffer, 512, in ) ){break;}  n = sscanf( buffer, "week:                       %u",  &utmp );              if( n != 1 ){break;} alm[i].week = (unsigned short)utmp;
#endif
    alm[i].is_af0_af1_high_precision = 0;
    alm[i].reserved1 = 0;
    i++;

     // sync with the next "ID" found in the file
    syncFound = FALSE;
    synca = (char)fgetc(in);  
    while( !feof(in) )
    {
      syncb = (char)fgetc(in);
      if( synca == 'I' && syncb == 'D' )
      {
        syncFound = 1;
        break;
      }
      else
      {
        synca = syncb;
      }
    }
    if( !syncFound )
    {
      break;
    }

    // rewind by two bytes to make things easy below
    fseek(in, -2, SEEK_CUR );
  }

  *number_read = (unsigned char)i;

  fclose(in);

  return TRUE;
}


BOOL YUMA_WriteAlmanacDataToFile(
  const char* yumaFilePath,      //!< path to the output YUMA ASCII file
  YUMA_structAlmanac* alm,       //!< pointer to an array of YUMA almanac structs
  unsigned char number_to_write  //!< length of the array
  )
{
  char buffer[1024];
  unsigned char i;

  FILE* out;
#ifndef _CRT_SECURE_NO_DEPRECATE
  if( fopen_s( &out, yumaFilePath, "w") != 0 )
  {
    char msg[128];
    sprintf( msg, "Unable to open %s.", yumaFilePath );
    GNSS_ERROR_MSG( msg );    
    return FALSE;
  }
  if( !out )
  {
    char msg[128];
    sprintf( msg, "Unable to open %s.", yumaFilePath );
    GNSS_ERROR_MSG( msg );    
    return FALSE;
  }
#else
  out = fopen(yumaFilePath, "w");
  if( !out )
  {
    char msg[128];
    sprintf( msg, "Unable to open %s.", yumaFilePath );
    GNSS_ERROR_MSG( msg );    
    return FALSE;
  }
#endif

  for( i = 0; i < number_to_write; i++ )
  {
    YUMA_WriteSingleAlmanacElementToBuffer( alm[i], buffer, 1024 );
    fprintf( out, "%s", buffer );
  }

  fclose(out);
  return TRUE;
}

  
BOOL YUMA_WriteSingleAlmanacElementToBuffer(
  YUMA_structAlmanac alm,   //!< YUMA almanac struct
  char* buffer,             //!< buffer to write the YUMA struct information
  unsigned short bufferSize //!< size of the buffer, must be greater than 1024 bytes
  )
{
  int scount = 0;

#ifndef _CRT_SECURE_NO_DEPRECATE
  int dcount = 0;
  if( bufferSize < 1024 )
  {
//    sprintf_s( buffer, bufferSize, "" );
    GNSS_ERROR_MSG( "if( bufferSize < 1024 )" );
    return FALSE;
  }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "******** Week %03d almanac for PRN-%02d ********\n", alm.week, alm.prn ); if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "ID:                        % 03d\n",  alm.prn );       if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "Health:                    % 04d\n",  alm.health );    if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "Eccentricity:              % .10g\n", alm.ecc );       if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "Time of Applicability(s):  % .4f\n",  alm.toa );       if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "Orbital Inclination(rad):  % .10g\n", alm.i0 );        if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "Rate of Right Ascen(r/s):  % .10g\n", alm.omegadot );  if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "SQRT(A)  (m 1/2):          % .6f\n",  alm.sqrta );     if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "Right Ascen at Week(rad):  % .10g\n", alm.omega0 );    if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "Argument of Perigee(rad):  % .10g\n", alm.w );         if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "Mean Anom(rad):            % .10g\n", alm.m0 );        if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "Af0(s):                    % .10g\n", alm.af0 );       if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "Af1(s/s):                  % .10g\n", alm.af1 );       if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "week:                      % 03d\n\n",alm.week );      if( dcount < 0 ){GNSS_ERROR_MSG("sprintf failed.");return FALSE;}else{ scount += dcount; }
#else
  if( bufferSize < 1024 )
  {
//     sprintf( buffer, "" );
    GNSS_ERROR_MSG( "if( bufferSize < 1024 )" );
    return FALSE;
  }
  scount += sprintf( buffer+scount, "******** Week %03d almanac for PRN-%02d ********\n", alm.week, alm.prn );
  scount += sprintf( buffer+scount, "ID:                         %02d\n",  alm.prn );
  scount += sprintf( buffer+scount, "Health:                     %03d\n",  alm.health );
  scount += sprintf( buffer+scount, "Eccentricity:               %.10g\n", alm.ecc );
  scount += sprintf( buffer+scount, "Time of Applicability(s):   %.4f\n",  alm.toa );
  scount += sprintf( buffer+scount, "Orbital Inclination(rad):   %.10g\n", alm.i0 );
  scount += sprintf( buffer+scount, "Rate of Right Ascen(r/s):   %.10g\n", alm.omegadot );
  scount += sprintf( buffer+scount, "SQRT(A)  (m 1/2):           %.6f\n",  alm.sqrta );
  scount += sprintf( buffer+scount, "Right Ascen at Week(rad):   %.10g\n", alm.omega0 );
  scount += sprintf( buffer+scount, "Argument of Perigee(rad):   %.10g\n", alm.w );
  scount += sprintf( buffer+scount, "Mean Anom(rad):             %.10g\n", alm.m0 );
  scount += sprintf( buffer+scount, "Af0(s):                     %.10g\n", alm.af0 );
  scount += sprintf( buffer+scount, "Af1(s/s):                   %.10g\n", alm.af1 );
  scount += sprintf( buffer+scount, "week:                       %03d\n\n",alm.week );
#endif
  return TRUE;
}

