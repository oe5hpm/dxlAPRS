#
# Makefile for dxlAPRS toolchain containing:
# - afskmodem	soundmodem for afsk/fsk up to 19200 bd using Linux OSS
# - gps2aprs	aprs-tracker using some serial gps, produces axudp frames which
#				may be sent with afskmodem
# - gps2digipos	generates a beaconfile for a (mobile) aprs-digi,
#				using some serial GPS
# - sondeudp	soundmodem for RS92 (and other) wx-ballons using Linux OSS,
#				the produced axudp frames are beeing post-processed by sondemod
#				to generate aprs-beacons
# - sondemod	RS92 (and other) wx-ballon decoder (takes data from sondeudp
#				and generates aprs-beacons)
# - udpbox		very powerful aprs-digi and axudp "distributor"
# - udpgate4	very powerful aprs-igate server (with http-interface)
# - udpflex		interfaces serial stuff like TNC2, TNC3, KISS, RMNC
#				to axudp frames
# - udphub		L2 AX25 switch to process many Packet Radio clients on one
#				single AXUDP interface from Flexnet, XNET, ...
# - aprsmap		powerful APRS viewer based on Openstreetmap
# - profile		makes png Image with Geo-Profile of 2 Positions using srtm-datafiles
# - radiorange		makes png Image/tiles with radio illumintion of 1 or 2 Positions
#                               using srtm-datafiles
# - panorama		makes png Image + csv-file from landscape with poi markers
#                               using srtm-datafiles
# - cmslogin   l2cat task for bidirectional TCP connect and password login in CMS server
# - srtmtag    modify srtm1 or srtm3 file with tags eg. wood/water
# - fmrepeater simple ctcss-squelch speach repeater audio processing + ptt + dtmf
#              + ctcss generator, maybe combined with afskmodem for multimode relay
# - downsample and shift frequecy in iq-file
# - waterfall iq file to png image with waterfall, waterfall3 same faster but with libfftw3
# - sdrtx modulate audio signal to iq file ssb/am/fm/stereo with audio processing
# - ra02  radio modul ra02 to axudp interface
# - loratx text line to lora IQ file with test features
# - lorarx IQ file with lora to axudp
# ---------------------- common section ---------------------------------------
HOSTARCH := $(shell uname -m | \
	sed -e s/i.86/x86/ \
	    -e s/sun4u/sparc64/ \
	    -e s/arm.*/arm/ \
	    -e s/sa110/arm/ \
	    -e s/ppc64/powerpc/ \
	    -e s/ppc/powerpc/ \
	    -e s/macppc/powerpc/\
	    -e s/sh.*/sh/)
export HOSTARCH

HOSTOS := $(shell uname -s)
export HOSTOS

ifeq ($(HOSTOS),Darwin)
# Using gcc as symlink to Mac's clan
GCCVERSION_GE4 := true
STRIPFLAGS =
else
GCCVERSION := $(shell $(CROSS_COMPILE)gcc --version | \
	grep gcc | sed 's/.*gcc (.*) //g' | cut -d '.' -f 1)

GCCVERSION_GE4 := $(shell [ $(GCCVERSION) -ge 4 ] && echo true)
STRIPFLAGS = -s
endif

.SUFFIXES:
	MAKEFLAGS += -r

CC		= $(CROSS_COMPILE)gcc
STRIP		= $(CROSS_COMPILE)strip
INCL		= .
ifeq ($(GCCVERSION_GE4),true)
CDEFS		= -Wall \
		  -Wno-unused-variable -Wno-parentheses -Wno-pointer-sign \
		  -Wno-format -Wno-return-type -Wno-char-subscripts \
		  -Wno-uninitialized \
		  -Werror=implicit-function-declaration
else
CDEFS		= -w
endif

ifeq ($(HOSTOS),Darwin)
CFLAGS		= -I$(INCL) -c -O2 -fdata-sections -ffunction-sections -I/usr/X11/include -DMACOS
LFLAGS		= -L/usr/X11/lib -lX11
else
CFLAGS		= -I$(INCL) -c -O2 -fdata-sections -ffunction-sections
LFLAGS		= -Wl,--gc-sections
endif
# ---------------------- armv6 (raspberry pi 1) specific -----------------------
ifeq ($(PLATFORM), armv6tce)
OUT		:= $(if $(OUT),$(OUT),../out-armv6tce/)
EXTLIB  	= lib_armv6/
GFXLIBS		= $(EXTLIB)libpng15/libpng.a \
			  $(EXTLIB)libz.a \
			  $(EXTLIB)libjpeg.a
XLIBS		= $(EXTLIB)libxcb.so.1 $(EXTLIB)libXdmcp.so.6 $(EXTLIB)libXau.so.6 \
			  -L$(EXTLIB) -lXext -lX11
CFLAGS		+= -march=armv6zk -mfpu=vfp -mfloat-abi=hard -mcpu=arm1176jzf-s
CFLAGS		+= -Ilib_armv6/libpng15/ -I$(EXTLIB) -I.
LFLAGS		+=
TARGETS		+=	$(OUT)aprsmap
else ifeq ($(PLATFORM), armv6)
OUT		:= $(if $(OUT),$(OUT),../out-armv6/)
EXTLIB  	= lib_armv6/
GFXLIBS		= $(EXTLIB)libpng16/libpng.a \
			  $(EXTLIB)libz.a \
			  $(EXTLIB)libjpeg.a
XLIBS		= $(EXTLIB)libxcb.so.1 $(EXTLIB)libXdmcp.so.6 $(EXTLIB)libXau.so.6 \
			 -L$(EXTLIB) -lXext -lX11
CFLAGS		+= -march=armv6zk -mfpu=vfp -mfloat-abi=hard -mcpu=arm1176jzf-s
CFLAGS		+= -Ilib_armv6/libpng16/ -I$(EXTLIB) -I.
LFLAGS		+=
TARGETS		+=	$(OUT)aprsmap
# ---------------------- armv7hf (bur am335x pp) specific ---------------------
else ifeq ($(PLATFORM), armv7hf)
OUT		:= $(if $(OUT),$(OUT),../out-armv7hf/)
EXTLIB		= lib_armv7hf/
GFXLIBS		= $(EXTLIB)libpng.a \
			  $(EXTLIB)libz.a \
			  $(EXTLIB)libjpeg.a
XLIBS		= $(EXTLIB)libxcb.so $(EXTLIB)libXdmcp.so $(EXTLIB)libXau.so \
			 -L$(EXTLIB) -lXext -lX11
CFLAGS		+= -march=armv7-a -mfpu=neon -mfloat-abi=hard
CFLAGS		+= -I$(EXTLIB)
LFLAGS		+=
TARGETS		+=	$(OUT)aprsmap
# ---------------------- aarcg64 (rpi 4) specific -----------------------------
else ifeq ($(PLATFORM), aarch64)
OUT		:= $(if $(OUT),$(OUT),../out-aarch64/)
EXTLIB		= lib_aarch64/
GFXLIBS		=  $(EXTLIB)libpng.a \
			  $(EXTLIB)libz.a \
			  $(EXTLIB)libjpeg.a
XLIBS		= $(EXTLIB)libxcb.so $(EXTLIB)libXdmcp.so $(EXTLIB)libXau.so \
			 -L$(EXTLIB) -lXext -lX11
CFLAGS		+= -I$(EXTLIB)
LFLAGS		+=
# -------------------------- x86_64 specific -----------------------------------
else ifeq ($(HOSTARCH), x86_64)
GFXLIBS		= -lpng -ljpeg
XLIBS		= -lXext -lX11
CFLAGS		+= -I$(EXTLIB)
EXTLIB		= lib_x86_64/
TARGETS		+=	$(OUT)aprsmap
ifeq ($(PLATFORM), x86_32)
	EXTLIB	= lib_x86_32/
	OUT	:= $(if $(OUT),$(OUT),../out-x86_32/)
	CFLAGS	+= -m32
	LFLAGS	+= -m32
	TARGETS	+= $(OUT)aprsmap-x86_32
else ifneq ($(HOSTOS),Darwin)
TARGETS		+= $(OUT)aprsmap-x86_64
endif
OUT		:= $(if $(OUT),$(OUT),../out-x86_64/)
# -------------------------- x86_32 (default) specific ------------------------
else ifeq ($(HOSTARCH), x86)
OUT		:= $(if $(OUT),$(OUT),../out-x86_32/)
TARGETS		+= $(OUT)aprsmap-x86_32
GFXLIBS		=-lpng -ljpeg
XLIBS		= -lXext -lX11
else
OUT		:= $(if $(OUT),$(OUT),../out/)
GFXLIBS		+= -lpng -ljpeg
XLIBS		+=-lXext -lX11
TARGETS		+=	$(OUT)aprsmap
endif
# ------------------ objects enumeration ------------------
OBJ_COMMON = \
	$(OUT)Select.o \
	$(OUT)aprspos.o \
	$(OUT)aprsstr.o \
	$(OUT)osic.o \
	$(OUT)osi.o \
	$(OUT)soundctl.o \
	$(OUT)tcp.o \
	$(OUT)udp.o

OBJ_AFSKMODEM = \
	$(OUT)afskmodem.o \
	$(OUT)afskmodemptt.o

OBJ_GPS2APRS	= \
	$(OUT)gps2aprs.o
OBJ_GPS2DIGIPOS	= \
	$(OUT)gps2digipos.o

OBJ_SONDEMOD	= \
	$(OUT)geodesy.o \
	$(OUT)gps.o \
	$(OUT)gpspos.o \
	$(OUT)navigation.o \
	$(OUT)rinex.o \
	$(OUT)sem.o \
	$(OUT)sondeaprs.o \
	$(OUT)time_conversion.o \
	$(OUT)yuma.o \
	$(OUT)sondemod.o \
	$(OUT)libsrtm.o

OBJ_SONDEUDP	= \
	$(OUT)sondeudp.o \
	$(OUT)rsc.o \
	$(OUT)rsc_decode.o

OBJ_UDPBOX	= \
	$(OUT)udpbox.o
	
OBJ_UDPGATE4	= \
	$(OUT)udpgate4.o \
	$(OUT)libsrtm.o
	
OBJ_UDPFLEX	= \
	$(OUT)udpflex.o

OBJ_UDPHUB	= \
	$(OUT)udphub.o

OBJ_APRSMAP	= \
	$(OUT)aprsdecode.o \
	$(OUT)aprsmap.o \
	$(OUT)aprstat.o \
	$(OUT)aprstext.o \
	$(OUT)beep.o \
	$(OUT)maptool.o \
	$(OUT)pngread.o \
	$(OUT)pngwrite.o \
	$(OUT)jpgdec.o \
	$(OUT)useri.o \
	$(OUT)xosi.o \
	$(OUT)libsrtm.o \
	$(OUT)pastewrapper.o

OBJ_SDRTST	= \
	$(OUT)tcpb.o \
	$(OUT)sdr.o \
	$(OUT)sdrtest.o

OBJ_SDRRADIO	= \
	$(OUT)tcpb.o \
	$(OUT)sdr.o \
	$(OUT)radio.o

OBJ_L2CAT	= \
	$(OUT)l2.o \
	$(OUT)frameio.o \
	$(OUT)deflate.o \
	$(OUT)Execlogin.o \
	$(OUT)cleanup.o \
	$(OUT)l2cat.o

OBJ_ADSB2APRS	= \
	$(OUT)adsb2aprs.o \
	$(OUT)tcpb.o \
	$(OUT)libsrtm.o

OBJ_PROFILE	= \
	$(OUT)profile.o \
	$(OUT)imagetext.o \
	$(OUT)libsrtm.o \
	$(OUT)pngwrite.o

OBJ_RADIORANGE	= \
	$(OUT)radiorange.o \
	$(OUT)pngwritepalette.o \
	$(OUT)libsrtm.o

OBJ_PANORAMA = \
	$(OUT)panorama.o\
	$(OUT)imagetext.o \
	$(OUT)libsrtm.o \
	$(OUT)pngwrite.o \
	$(OUT)pngread.o

OBJ_CMSLOGIN = \
	$(OUT)cmslogin.o

OBJ_SRTMTAG = \
	$(OUT)srtmtag.o \
	$(OUT)pngread.o

OBJ_FMREPEATER = \
	$(OUT)fmrepeater.o \
	$(OUT)afskmodemptt.o

OBJ_WATERFALL = \
	$(OUT)waterfall.o \
	$(OUT)pngwritepalette.o \
	$(OUT)fft.o \
	$(OUT)complex.o

OBJ_WATERFALL3 = \
	$(OUT)waterfall3.o \
	$(OUT)pngwritepalette.o \
	$(OUT)fftw.o

OBJ_DOWNSAMPLE = \
	$(OUT)downsample.o

OBJ_SDRTX = \
	$(OUT)sdrtx.o

OBJ_RA02 = \
	$(OUT)ra02.o

OBJ_LORATX	= \
	$(OUT)loratx.o

OBJ_LORARX = \
	$(OUT)lorarx.o \
	$(OUT)fft.o \
	$(OUT)complex.o

OBJ_HOSTINT = \
	$(OUT)hostint.o \
	$(OUT)frameio.o \
	$(OUT)l2.o

TARGETS += \
	$(OUT)afskmodem \
	$(OUT)gps2aprs \
	$(OUT)gps2digipos \
	$(OUT)udpbox \
	$(OUT)udpgate4 \
	$(OUT)udpflex \
	$(OUT)udphub \
	$(OUT)sondemod \
	$(OUT)sondeudp \
	$(OUT)sdrtst \
	$(OUT)sdrradio \
	$(OUT)adsb2aprs \
	$(OUT)profile \
	$(OUT)radiorange \
	$(OUT)panorama \
	$(OUT)cmslogin \
	$(OUT)srtmtag \
	$(OUT)fmrepeater \
	$(OUT)waterfall \
	$(OUT)waterfall3 \
	$(OUT)downsample \
	$(OUT)sdrtx \
	$(OUT)ra02 \
	$(OUT)loratx \
	$(OUT)lorarx \
	$(OUT)hostint

ifneq ($(HOSTOS), Darwin)
	TARGETS += $(OUT)l2cat
endif

OBJS = \
	$(OBJ_COMMON) \
	$(OBJ_AFSKMODEM) \
	$(OBJ_GPS2APRS) \
	$(OBJ_GPS2DIGIPOS) \
	$(OBJ_SONDEMOD) \
	$(OBJ_SONDEUDP) \
	$(OBJ_UDPBOX) \
	$(OBJ_UDPGATE4) \
	$(OBJ_UDPFLEX) \
	$(OBJ_UDPHUB) \
	$(OBJ_APRSMAP) \
	$(OBJ_SDRTST) \
	$(OBJ_SDRRADIO) \
	$(OBJ_L2CAT)	\
	$(OBJ_ADSB2APRS) \
	$(OBJ_PROFILE) \
	$(OBJ_RADIORANGE) \
	$(OBJ_PANORAMA) \
	$(OBJ_CMSLOGIN) \
	$(OBJ_SRTMTAG) \
	$(OBJ_FMREPEATER) \
	$(OBJ_WATERFALL) \
	$(OBJ_DOWNSAMPLE) \
	$(OBJ_SDRTX) \
	$(OBJ_RA02) \
	$(OBJ_LORATX) \
	$(OBJ_LORARX)

# ------------------- target make rules -------------------
#                     -----------------
$(OUT)%.o: %.c
	@echo [compiling] $@
	@$(CC) $(CDEFS) $(CFLAGS) -o $@ $<

.PHONY: all

all: prepare $(TARGETS)

clean:
	@rm -f $(OBJS) $(TARGETS)
	@rm -r -f $(OUT)

prepare:
	@test -d $(OUT) || mkdir -p $(OUT)

$(OUT)afskmodem: $(OBJ_AFSKMODEM) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_AFSKMODEM) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)gps2aprs: $(OBJ_GPS2APRS) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_GPS2APRS) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)gps2digipos: $(OBJ_GPS2DIGIPOS) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_GPS2DIGIPOS) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)sondemod: $(OBJ_SONDEMOD) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_SONDEMOD) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)sondeudp: $(OBJ_SONDEUDP) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_SONDEUDP) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)udpbox: $(OBJ_UDPBOX) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_UDPBOX) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)udpgate4: $(OBJ_UDPGATE4) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_UDPGATE4) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)udpflex: $(OBJ_UDPFLEX) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_UDPFLEX) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)udphub: $(OBJ_UDPHUB) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_UDPHUB) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)aprsmap-x86_32: $(OBJ_APRSMAP) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_APRSMAP) \
	$(OBJ_COMMON) $(GFXLIBS) $(XLIBS) \
	-lpthread -lxcb -lXau -lXdmcp -lm -lpthread -ldl -lz \
	-static
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)aprsmap-x86_64: $(OBJ_APRSMAP) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_APRSMAP) \
	$(OBJ_COMMON) $(GFXLIBS) $(XLIBS) \
	-lpthread -lxcb -lXau -lXdmcp -lm -lpthread -ldl -lz \
	-static
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)aprsmap : $(OBJ_APRSMAP) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_APRSMAP) \
	$(OBJ_COMMON) $(GFXLIBS) $(XLIBS) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)sdrtst : $(OBJ_SDRTST) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_SDRTST) $(OBJ_COMMON) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)sdrradio : $(OBJ_SDRRADIO) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_SDRRADIO) $(OBJ_COMMON) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)l2cat : $(OBJ_L2CAT) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_L2CAT) $(OBJ_COMMON) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)adsb2aprs: $(OBJ_ADSB2APRS) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_ADSB2APRS) $(OBJ_COMMON) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)profile : $(OBJ_PROFILE) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_PROFILE) \
	$(OBJ_COMMON) $(GFXLIBS) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)radiorange: $(OBJ_RADIORANGE) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_RADIORANGE) $(OBJ_COMMON) $(GFXLIBS) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)panorama: $(OBJ_PANORAMA) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_PANORAMA) $(OBJ_COMMON) $(GFXLIBS) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)cmslogin: $(OBJ_CMSLOGIN) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_CMSLOGIN) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)srtmtag: $(OBJ_SRTMTAG) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_SRTMTAG) $(GFXLIBS) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)fmrepeater: $(OBJ_FMREPEATER) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_COMMON) $(OBJ_FMREPEATER) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)waterfall: $(OBJ_WATERFALL) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@  $(OBJ_WATERFALL) $(OBJ_COMMON) $(GFXLIBS) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)waterfall3: $(OBJ_WATERFALL3) $(OBJ_COMMON)
	@echo [ linking ] $@
	$(CC) $(LFLAGS) -o $@ $(OBJ_WATERFALL3) $(OBJ_COMMON) $(GFXLIBS) $(EXTLIB)libfftw3.a -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)downsample: $(OBJ_DOWNSAMPLE) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_DOWNSAMPLE) $(OBJ_COMMON) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)sdrtx: $(OBJ_SDRTX) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_SDRTX) $(OBJ_COMMON) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)ra02: $(OBJ_RA02) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_RA02) $(OBJ_COMMON) -lrt
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)loratx: $(OBJ_LORATX) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_LORATX) $(OBJ_COMMON) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)lorarx: $(OBJ_LORARX) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_LORARX) $(OBJ_COMMON) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

$(OUT)hostint: $(OBJ_HOSTINT) $(OBJ_COMMON)
	@echo [ linking ] $@
	@$(CC) $(LFLAGS) -o $@ $(OBJ_HOSTINT) $(OBJ_COMMON) -lm
	@$(STRIP) $@ $(STRIPFLAGS)

%:
	make prepare
	make $(OUT)$@

diag:
	@echo "Hostarch    : $(HOSTARCH)"
	@echo "BuildOutput : $(OUT)"
	@echo "Platform    : $(PLATFORM)"
# end of makefile
