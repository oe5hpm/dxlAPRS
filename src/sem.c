/**
\file    sem.c
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

#include <stdio.h>
#include <string.h>
#include "gnss_error.h"
#include "sem.h"
#include "constants.h"

#ifndef WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif


BOOL SEM_ReadAlmanacDataFromFile(
  const char* semFilePath,    //!< path to the input SEM ASCII file
  SEM_structAlmanac* alm,     //!< pointer to an array of SEM almanac structs
  unsigned char  max_to_read, //!< length of the array
  unsigned char* number_read  //!< number of almanac items read
  )
{
  FILE* in;                   // the input file pointer
  int n;                      // counter to indicate the number of items read by sscanf
  unsigned char i;            // counter
  unsigned utmp;              // tmp used in fscanf input
  unsigned number_of_records; // number of records in file
  unsigned week;              // gps week [week]
  unsigned toa;               // time of almanac applicability [s]
  double dtmp;                // tmp used in fscanf input
  char description[128];      // file descriptor
  
  // initialize
  *number_read = 0;  

  // check stupid error
  if( max_to_read < 1 )
  {
    GNSS_ERROR_MSG( "if( max_to_read < 1 )" );
    return FALSE;
  }

#ifndef _CRT_SECURE_NO_DEPRECATE
  if( fopen_s( &in, semFilePath, "r" ) != 0 )
  {
    char msg[128];
    sprintf( msg, "Unable to open %s.", semFilePath );
    GNSS_ERROR_MSG( msg );
    return FALSE;
  }
  if( !in )
  {    
    char msg[128];
    sprintf( msg, "Unable to open %s.", semFilePath );
    GNSS_ERROR_MSG( msg );
    return FALSE;
  }

  n = fscanf_s( in, "%u", &number_of_records ); if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}
  if( number_of_records < 1 )
  {
    GNSS_ERROR_MSG( "if( number_of_records < 1 )" );
    return FALSE;
  }

  n = fscanf_s( in, "%s", description ); 
  if(n != 1)
  {
    GNSS_ERROR_MSG( "fscanf failed." );
    return FALSE;
  }
  n = fscanf_s( in, "%u", &week ); 
  if(n != 1)
  {
    GNSS_ERROR_MSG( "fscanf failed." );
    return FALSE;
  }
  n = fscanf_s( in, "%u", &toa );  
  if(n != 1)
  {
    GNSS_ERROR_MSG( "fscanf failed." );
    return FALSE;
  }

  for( i = 0; i < number_of_records && i < max_to_read; i++ )
  {
    alm[i].week = (unsigned short) week;
    alm[i].toa  = toa;
    n = fscanf_s( in, "%u", &utmp );  if(n != 1){break;} alm[i].prn = (unsigned short)utmp; if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;}
    n = fscanf_s( in, "%u", &utmp );  if(n != 1){break;} alm[i].svn = (unsigned short)utmp; if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;}
    n = fscanf_s( in, "%u", &utmp );  if(n != 1){break;} alm[i].ura = (unsigned char)utmp;  if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;}
    
    n = fscanf_s( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].ecc      = dtmp;    if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;}
    n = fscanf_s( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].i0       = dtmp*PI; if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;} // convert to radians
    n = fscanf_s( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].omegadot = dtmp*PI; if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;} // convert to radians
    n = fscanf_s( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].sqrta    = dtmp;    if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;}
    n = fscanf_s( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].omega0   = dtmp*PI; if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;} // convert to radians
    n = fscanf_s( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].w        = dtmp*PI; if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;} // convert to radians
    n = fscanf_s( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].m0       = dtmp*PI; if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;} // convert to radians
    n = fscanf_s( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].af0      = dtmp;    if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;}
    n = fscanf_s( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].af1      = dtmp;    if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;}

    n = fscanf_s( in, "%u", &utmp );  if(n != 1){break;} alm[i].health      = (unsigned char)utmp; if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;}
    n = fscanf_s( in, "%u", &utmp );  if(n != 1){break;} alm[i].config_code = (unsigned char)utmp; if(n != 1){GNSS_ERROR_MSG( "fscanf failed." );return FALSE;}

    *number_read = (unsigned char)(i+1);
  }
  
  fclose(in);
#else
  in = fopen( semFilePath, "r" );
  if( !in )
  {
    char msg[128];
    sprintf( msg, "Unable to open %s.", semFilePath );
    GNSS_ERROR_MSG( msg );    
    return FALSE;
  }

  n = fscanf( in, "%u", &number_of_records ); if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}
  if( number_of_records < 1 )
  {
    GNSS_ERROR_MSG( "if( number_of_records < 1 )" );
    return FALSE;
  }

  n = fscanf( in, "%127s", description ); if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}
  n = fscanf( in, "%u", &week ); if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}
  n = fscanf( in, "%u", &toa );  if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}


  for( i = 0; i < number_of_records && i < max_to_read; i++ )
  {
    alm[i].week = (unsigned short) week;
    alm[i].toa  = toa;
    n = fscanf( in, "%u", &utmp );  if(n != 1){break;} alm[i].prn = (unsigned short)utmp; if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}
    n = fscanf( in, "%u", &utmp );  if(n != 1){break;} alm[i].svn = (unsigned short)utmp; if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}
    n = fscanf( in, "%u", &utmp );  if(n != 1){break;} alm[i].ura = (unsigned char)utmp;  if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}
    
    n = fscanf( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].ecc      = dtmp;    if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}
    n = fscanf( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].i0       = dtmp*PI; if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;} // convert to radians
    n = fscanf( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].omegadot = dtmp*PI; if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;} // convert to radians
    n = fscanf( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].sqrta    = dtmp;    if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}
    n = fscanf( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].omega0   = dtmp*PI; if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;} // convert to radians
    n = fscanf( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].w        = dtmp*PI; if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;} // convert to radians
    n = fscanf( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].m0       = dtmp*PI; if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;} // convert to radians
    n = fscanf( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].af0      = dtmp;    if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}
    n = fscanf( in, "%lf", &dtmp ); if(n != 1){break;} alm[i].af1      = dtmp;    if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}

    n = fscanf( in, "%u", &utmp );  if(n != 1){break;} alm[i].health      = (unsigned char)utmp; if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}
    n = fscanf( in, "%u", &utmp );  if(n != 1){break;} alm[i].config_code = (unsigned char)utmp; if(n != 1){GNSS_ERROR_MSG("fscanf failed.");return FALSE;}

    *number_read = (unsigned char)(i+1);
  }
  
  fclose(in);
#endif

  return TRUE;
}


BOOL SEM_WriteAlmanacDataToFile(
  const char* semFilePath,       //!< path to the output SEM ASCII file
  SEM_structAlmanac* alm,        //!< pointer to an array of SEM almanac structs
  unsigned char number_to_write  //!< length of the array
  )
{
  char buffer[1024];
  unsigned char i;

  FILE* out;

#ifndef _CRT_SECURE_NO_DEPRECATE
  if( fopen_s( &out, semFilePath, "w") != 0 )
  {
    char msg[128];
    sprintf( msg, "Unable to open %s.", semFilePath );
    GNSS_ERROR_MSG( msg );        
    return FALSE;
  }
  if( !out )
  {
    char msg[128];
    sprintf( msg, "Unable to open %s.", semFilePath );
    GNSS_ERROR_MSG( msg );        
    return FALSE;
  }
#else
  out = fopen(semFilePath, "w");
  if( !out )
  {
    char msg[128];
    sprintf( msg, "Unable to open %s.", semFilePath );
    GNSS_ERROR_MSG( msg );    
    return FALSE;
  }
#endif

  fprintf( out, "%d %s\n", number_to_write, "CURRENT.ALM" );
  fprintf( out, "%d %d\n\n", alm[0].week, alm[0].toa );

  for( i = 0; i < number_to_write; i++ )
  {
    SEM_WriteSingleAlmanacElementToBuffer( alm[i], buffer, 1024 );
    fprintf( out, "%s", buffer );
  }

  fclose(out);
  return TRUE;
}


BOOL SEM_WriteSingleAlmanacElementToBuffer(
  SEM_structAlmanac alm,     //!< SEM almanac struct
  char* buffer,              //!< buffer to write the SEM struct information
  unsigned short bufferSize  //!< size of the buffer, must be greater than 1024 bytes
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
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "%d\n%d\n%d\n", alm.prn, alm.svn, alm.ura );                          if( dcount < 0 ){ GNSS_ERROR_MSG("sprintf failed.");return FALSE; }else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "%-23.14g %23.14g %23.14g\n", alm.ecc, alm.i0/PI, alm.omegadot/PI );  if( dcount < 0 ){ GNSS_ERROR_MSG("sprintf failed.");return FALSE; }else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "%-23.14g %23.14g %23.14g\n", alm.sqrta, alm.omega0/PI, alm.w/PI );   if( dcount < 0 ){ GNSS_ERROR_MSG("sprintf failed.");return FALSE; }else{ scount += dcount; }
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "%-23.14g %23.14g %23.14g\n", alm.m0/PI, alm.af0, alm.af1 );          if( dcount < 0 ){ GNSS_ERROR_MSG("sprintf failed.");return FALSE; }else{ scount += dcount; }  
  dcount = sprintf_s( buffer+scount, bufferSize-scount, "%d\n%d\n\n", alm.health, alm.config_code );                          if( dcount < 0 ){ GNSS_ERROR_MSG("sprintf failed.");return FALSE; }else{ scount += dcount; }
#else
  if( bufferSize < 1024 )
  {
//    sprintf( buffer, "" );
    GNSS_ERROR_MSG( "if( bufferSize < 1024 )" );
    return FALSE;
  }
  scount += sprintf( buffer+scount, "%d\n%d\n%d\n", alm.prn, alm.svn, alm.ura );
  scount += sprintf( buffer+scount, "%-23.14g %23.14g %23.14g\n", alm.ecc, alm.i0/PI, alm.omegadot/PI );
  scount += sprintf( buffer+scount, "%-23.14g %23.14g %23.14g\n", alm.sqrta, alm.omega0/PI, alm.w/PI );
  scount += sprintf( buffer+scount, "%-23.14g %23.14g %23.14g\n", alm.m0/PI, alm.af0, alm.af1 );
  scount += sprintf( buffer+scount, "%d\n%d\n\n", alm.health, alm.config_code );    
#endif

  return TRUE;
}

