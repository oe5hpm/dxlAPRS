/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*
  ka9q-fec:
    gcc -c init_rs_char.c
    gcc -c encode_rs_char.c
    gcc -c decode_rs_char.c
  gcc init_rs_char.o encode_rs_char.o decode_rs_char.o ecc-rs_vaisala.c -o ecc-rs_vaisala
*/

#include <stdio.h>
#include <string.h>
/*
#include "fec.h"
*/
#define N 255
#define R 24
#define K (N-R)

void *rs;

void initrsc()
{
rs = init_rs_char( 8, 0x11d, 0, 1, R, 0);
}


int decodersc(unsigned char *data, int *eras_pos, int no_eras)
{
 return decode_rs_char(rs, data, eras_pos, no_eras);
}
