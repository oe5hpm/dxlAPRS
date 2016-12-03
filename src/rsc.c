/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define N 255
#define R 24
#define K (N-R)

void *init_rs_char(int symsize,int gfpoly,int fcr,int prim,int nroots,int pad);
int decode_rs_char(void *arg,
                   unsigned char *data, int *eras_pos, int no_eras);

void *rs;

void initrsc()
{
rs = init_rs_char( 8, 0x11d, 0, 1, R, 0);
}


int decodersc(unsigned char *data, int *eras_pos, int no_eras)
{
 return decode_rs_char(rs, data, eras_pos, no_eras);
}
