/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <iio.h>


static struct iio_context *ctx   = NULL;
static struct iio_channel *rx0_i = NULL;
static struct iio_channel *rx0_q = NULL;
static struct iio_channel *tx0_i = NULL;
static struct iio_channel *tx0_q = NULL;
static struct iio_buffer  *rxbuf = NULL;
static struct iio_buffer  *txbuf = NULL;
static struct iio_context *plutoctx;
static struct iio_device *phy;
static struct iio_context_info **info;
static struct iio_device *tx;
static struct iio_device *rx;
static char tmpstr[64];

#define MHZ(x) ((long long)(x*1000000.0 + .5))

/* helper function generating channel names */
static char* get_ch_name(const char* type, int id)
{
  snprintf(tmpstr, sizeof(tmpstr), "%s%d", type, id);
  return tmpstr;                                         /* grausam */
}

/* returns ad9361 phy device */
static struct iio_device* get_ad9361_phy(struct iio_context *ctx)
{
  struct iio_device *dev = iio_context_find_device(ctx, "ad9361-phy");
  return dev;
}



/* finds AD9361 phy IIO configuration channel with id chid */
static bool get_phy_chan(struct iio_context *ctx, bool istx, int chid, struct iio_channel **chn)
{
  if (!istx)
  {
    *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("voltage", chid), false); return *chn != NULL;
  }
  else
  {
    *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("voltage", chid), true);  return *chn != NULL;
  }
  return false;
}


/* finds AD9361 local oscillator IIO configuration channels */
static bool get_lo_chan(struct iio_context *ctx, bool istx, struct iio_channel **chn)
{
  if (!istx)
  {
    *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 0), true); return *chn != NULL;
  }
  else
  {
    *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 1), true); return *chn != NULL;
  }
  return false;
}


/* finds AD9361 streaming IIO devices */
static bool get_ad9361_stream_dev(struct iio_context *ctx, bool istx, struct iio_device **dev)
{
  if (istx)
  {
    *dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc"); return *dev != NULL;
  }
  else
  {
    *dev = iio_context_find_device(ctx, "cf-ad9361-lpc");  return *dev != NULL;
  }
  return false;
}


/* finds AD9361 streaming IIO channels */
static bool get_ad9361_stream_ch(struct iio_context *ctx, bool istx, struct iio_device *dev, int chid, struct iio_channel **chn)
{
        *chn = iio_device_find_channel(dev, get_ch_name("voltage", chid), istx);
        if (!*chn)
                *chn = iio_device_find_channel(dev, get_ch_name("altvoltage", chid), istx);
        return *chn != NULL;
}

/*
void setPlutoRxFreq(long long rxfreq)
{
int ret;
    ret=iio_channel_attr_write_longlong(iio_device_find_channel(phy, "altvoltage0", true),"frequency", rxfreq);
}

void setPlutoTxFreq(long long txfreq)
{
int ret;
    ret=iio_channel_attr_write_longlong(iio_device_find_channel(phy, "altvoltage1", true),"frequency", txfreq);
}

void setPlutoTxAtt(int att)
{
    iio_channel_attr_write_double(iio_device_find_channel(phy, "voltage0", true),"hardwaregain", (double)att);
}
*/

void PlutoTxOff(bool txoff)
{
  iio_channel_attr_write_bool(iio_device_find_channel(phy, "altvoltage1", true), "powerdown", txoff);
}

void PlutoRxOff(bool rxoff)
{
  iio_channel_attr_write_bool(iio_device_find_channel(phy, "altvoltage0", true),"powerdown", rxoff);
}

void setPlutoGpo(int p)
{
  char pins[10];

  sprintf(pins,"0x27 0x%x0",p);
  pins[9]=0;
  iio_device_debug_attr_write(phy,"direct_reg_access",pins);
}

/*
self.pluto_source_0 = iio.pluto_source('ip:pluto.local', 1000000000, 528000, 2000000, 0x800, True, True, True, "slow_attack", 64.0, '', True)
self.pluto_sink_0 = iio.pluto_sink('ip:pluto.local', 1000000000, 528000, 2000000, 0x800, False, 0, '', True)
*/



int Initiio(const char *url)
{
  int i;
  if (!url)
  { 
    // printf("* Acquiring IIO context\n");
    struct iio_scan_context *sctx = iio_create_scan_context("usb", 0);
    if (sctx==NULL) {
        fprintf(stderr, "Unable to create scan context\n");
        abort();
    }
    struct iio_context_info **info;
    int ret = iio_scan_context_get_info_list(sctx, &info);
    if (ret < 0) {
        iio_scan_context_destroy(sctx);
        fprintf(stderr, "Unable to scan for Pluto devices\n");
        abort();
    }

    if (ret == 0) {
        iio_context_info_list_free(info);
        iio_scan_context_destroy(sctx);
        fprintf(stderr, "No Pluto device found\n");
    }

    if (ret > 1) {
        printf("More than one Pluto found:\n");

        for (i = 0; i < (size_t) ret; i++) {
            printf("\t%d: %s [%s]\n", i,
                   iio_context_info_get_description(info[i]),
                   iio_context_info_get_uri(info[i]));
        }

        printf("We will use the first one.\n");
    }

    const char* uri = iio_context_info_get_uri(info[0]);
    // iio_context_info_list_free(info);
    iio_scan_context_destroy(sctx);
  }
  else
  {
    // Acquiring IIO context
    ctx = iio_create_context_from_uri(url);
    if (!ctx) return -2;
  };
  if (iio_context_get_devices_count(ctx) <= 0) return -2;

  phy = iio_context_find_device(ctx, "ad9361-phy");

  return 0;
}


void closeiio(void)
{
    if (info) iio_context_info_list_free(info);
    if (rxbuf) iio_buffer_destroy(rxbuf);
    if (txbuf) iio_buffer_destroy(txbuf);
    if (rx0_i) iio_channel_disable(rx0_i);
    if (rx0_q) iio_channel_disable(rx0_q);
    if (tx0_i) iio_channel_disable(tx0_i);
    if (tx0_q) iio_channel_disable(tx0_q);
    if (ctx) iio_context_destroy(ctx);
}

int startrx(uint32_t samples)
{
    get_ad9361_stream_dev(ctx, false, &rx);
    get_ad9361_stream_ch(ctx, false, rx, 0, &rx0_i);
    get_ad9361_stream_ch(ctx, false, rx, 1, &rx0_q);
    iio_channel_enable(rx0_i);
    iio_channel_enable(rx0_q);
    rxbuf = iio_device_create_buffer(rx, samples, false);
    if (!rxbuf) return -1;

    return 0;
}

int starttx(uint32_t samples)
{
    get_ad9361_stream_dev(ctx, true, &tx);
    get_ad9361_stream_ch(ctx, true, tx, 0, &tx0_i);
    get_ad9361_stream_ch(ctx, true, tx, 1, &tx0_q);
    iio_channel_enable(tx0_i);
    iio_channel_enable(tx0_q);
    txbuf = iio_device_create_buffer(tx, samples, false);
    if (!txbuf) return -1;

    return 0;
}


int setrxgainmode(const char* mode)
{
    return iio_channel_attr_write(iio_device_find_channel(phy, "voltage0", false),
                                 "gain_control_mode", mode);
}


int setrxgain(int16_t rxgain)
{
    return iio_channel_attr_write_longlong(iio_device_find_channel(phy, "voltage0",
                                          false), "hardwaregain", rxgain);
}

int settxgain(int16_t txgain)
{
    return iio_channel_attr_write_longlong(iio_device_find_channel(phy, "voltage0",
                                          true), "hardwaregain", txgain);
}

int setddsfreq(int32_t dds_freq_hz)
{
    struct iio_device *dds = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc");
    if (iio_channel_attr_write_bool(iio_device_find_channel(dds, "altvoltage0",
                                      true), "raw", 1)<0) return -1;

    if (iio_channel_attr_write_double(iio_device_find_channel(dds, "altvoltage0",
                                        true), "frequency", dds_freq_hz)<0) return -2;

    return iio_channel_attr_write_double(iio_device_find_channel(dds, "altvoltage0",
                                        true), "scale", 1);
}

int setfreq(double mhz, bool istx)
{
  struct iio_channel *chnn = NULL;
  if (istx)
  {  chnn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 1), true); }
  else {  chnn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 0), true); }
  if ( chnn == NULL ) return -2;

  return iio_channel_attr_write_longlong(chnn, "frequency", MHZ(mhz));
}

int setbw(long hz)
{
  return iio_channel_attr_write_longlong(iio_device_find_channel(get_ad9361_phy(ctx),
         get_ch_name("voltage", 0), false), "rf_bandwidth", (long long) hz);
}

int setsampel(long hz, bool istx)
{
  return iio_channel_attr_write_longlong(iio_device_find_channel(get_ad9361_phy(ctx),
         get_ch_name("voltage", 0), istx), "sampling_frequency", (long long) hz);  
}

int setport(const char* str, bool istx)        /* rx:"A_BALANCED"  tx:"A"*/
{
  return iio_channel_attr_write(iio_device_find_channel(get_ad9361_phy(ctx),
             get_ch_name("voltage", 0), istx), "rf_port_select", str);
} 

int receive(int16_t *iqbuf)
{
   int len;
   void *p_dat, *p_end;
   ptrdiff_t p_inc;

   len = iio_buffer_refill(rxbuf);
   if (len<0) return len;

   p_inc = iio_buffer_step(rxbuf);
   p_end = iio_buffer_end(rxbuf);
 
   for (p_dat = iio_buffer_first(rxbuf, rx0_i); p_dat < p_end; p_dat += p_inc) {
     *iqbuf++ = ((int16_t*)p_dat)[0]; // Real (I)
     *iqbuf++ = ((int16_t*)p_dat)[1]; // Imag (Q)
   }
   return len;
}

int transmit(int16_t *iqbuf)
{
   void *p_dat, *p_end;
   ptrdiff_t p_inc;

   p_inc = iio_buffer_step(txbuf);
   p_end = iio_buffer_end(txbuf);
   for (p_dat = (char *)iio_buffer_first(txbuf, tx0_i); p_dat < p_end; p_dat += p_inc) {
     ((int16_t*)p_dat)[0] = *iqbuf++; // Real (I)
     ((int16_t*)p_dat)[1] = *iqbuf++; // Imag (Q)
   }

   return iio_buffer_push(txbuf);
}
