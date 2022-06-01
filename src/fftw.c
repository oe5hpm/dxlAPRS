/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <fftw3.h>
#include <sys/types.h>
#include <stdlib.h>

static fftwf_plan p;

fftwf_complex* plan(uint n)
{
  fftwf_complex *io;

  io = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * n);
  p = fftwf_plan_dft_1d(n, io, io, FFTW_FORWARD, FFTW_ESTIMATE);
  return io;
}

void fft3()
{
  fftwf_execute(p);
}

void clean(fftwf_complex *inout)
{
  fftwf_destroy_plan(p);
  fftwf_free(inout);
}
