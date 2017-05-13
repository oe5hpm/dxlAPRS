/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include "X2C.h"
#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <string.h>

struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

int32_t readjpg(char *fn, JSAMPARRAY *row_pointers, int32_t *maxx, int32_t *maxy, int32_t *maxxbyte)
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  FILE * infile;                /* source file */
  int ret;
  int linec;
  JSAMPARRAY buffer;            /* Output row buffer */
  int row_stride;               /* physical row width in output buffer */

  if ((infile = fopen(fn, "rb")) == NULL) {
/*
    fprintf(stderr, "can't open %s\n", fn);
*/
    return -1;
  }

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return -2;
  }
  ret=0;
  jpeg_create_decompress(&cinfo);

  jpeg_stdio_src(&cinfo, infile);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);

  if (cinfo.output_height > *maxy || cinfo.output_width > *maxx || cinfo.output_components != 3)
    ret=-3;

  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  linec=0;
  while (cinfo.output_scanline < cinfo.output_height) 
    {   
    (void) jpeg_read_scanlines(&cinfo, (JSAMPARRAY) buffer, 1);
    if (linec < *maxy)
      memcpy(row_pointers[linec++], buffer[0], *maxxbyte);
    }

  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);
  return ret;
}
