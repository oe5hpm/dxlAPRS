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
EXTLIB  	= lib_armv6/
XLIBS		= $(EXTLIB)libpng15/libpng.a \
		  $(EXTLIB)libz.a \
		  $(EXTLIB)libjpeg.a \
		  $(EXTLIB)libxcb.so.1 $(EXTLIB)libXdmcp.so.6 $(EXTLIB)libXau.so.6 \
		  -L$(EXTLIB) -lXext -lX11
CFLAGS		+= -march=armv6zk -mfpu=vfp -mfloat-abi=hard -mcpu=arm1176jzf-s
CFLAGS		+= -Ilib_armv6/libpng15/ -I$(EXTLIB) -I.
LFLAGS		+=
else ifeq ($(PLATFORM), armv6)
EXTLIB  	= lib_armv6/
XLIBS		= $(EXTLIB)libpng16/libpng.a \
		  $(EXTLIB)libz.a \
		  $(EXTLIB)libjpeg.a \
		  $(EXTLIB)libxcb.so.1 $(EXTLIB)libXdmcp.so.6 $(EXTLIB)libXau.so.6 \
		  -L$(EXTLIB) -lXext -lX11
CFLAGS		+= -march=armv6zk -mfpu=vfp -mfloat-abi=hard -mcpu=arm1176jzf-s
CFLAGS		+= -Ilib_armv6/libpng16/ -I$(EXTLIB) -I.
LFLAGS		+=
# ---------------------- armv7hf (bur am335x pp) specific ---------------------
else ifeq ($(PLATFORM), armv7hf)
EXTLIB		= lib_armv7hf/
XLIBS		= $(EXTLIB)libpng.a \
		  $(EXTLIB)libz.a \
		  $(EXTLIB)libjpeg.a \
		  $(EXTLIB)libxcb.so $(EXTLIB)libXdmcp.so $(EXTLIB)libXau.so \
		  -L$(EXTLIB) -lXext -lX11
CFLAGS		+= -march=armv7-a -mfpu=neon -mfloat-abi=hard
CFLAGS		+= -I$(EXTLIB)
LFLAGS		+=
# -------------------------- x86_64 specific -----------------------------------
else ifeq ($(HOSTARCH), x86_64)
ifneq ($(HOSTOS),Darwin)
TARGETS		= aprsmap-static
endif
XLIBS		= -lpng -ljpeg -lXext -lX11
APRSMAP_STATIC	= aprsmap-$(HOSTARCH)
ifeq ($(PLATFORM), x86_32)
	CFLAGS	+= -m32
	LFLAGS	+= -m32
	APRSMAP_STATIC	= aprsmap-x86
endif
# -------------------------- x86_32 (default) specific ------------------------
else ifeq ($(HOSTARCH), x86)
TARGETS		= aprsmap-static
XLIBS		= -lpng -ljpeg -lXext -lX11
APRSMAP_STATIC	= aprsmap-$(HOSTARCH)
else
XLIBS		+= -lpng -ljpeg -lXext -lX11
endif
# ------------------ objects enumeration ------------------
OBJ_COMMON = \
	Select.o \
	aprspos.o \
	aprsstr.o \
	osic.o \
	osi.o \
	soundctl.o \
	tcp.o \
	udp.o

OBJ_AFSKMODEM = \
	afskmodem.o \
	afskmodemptt.o

OBJ_GPS2APRS	= gps2aprs.o
OBJ_GPS2DIGIPOS	= gps2digipos.o

OBJ_SONDEMOD	= \
	geodesy.o \
	gps.o \
	gpspos.o \
	navigation.o \
	rinex.o \
	sem.o \
	sondeaprs.o \
	time_conversion.o \
	yuma.o \
	sondemod.o

OBJ_SONDEUDP	= sondeudp.o \
	rsc.o \
	rsc_decode.o

OBJ_UDPBOX	= udpbox.o
OBJ_UDPGATE4	= udpgate4.o
OBJ_UDPFLEX	= udpflex.o
OBJ_UDPHUB	= udphub.o
OBJ_APRSMAP	= \
	aprsdecode.o \
	aprsmap.o \
	aprstat.o \
	aprstext.o \
	beep.o \
	maptool.o \
	pngread.o \
	pngwrite.o \
	jpgdec.o \
	useri.o \
	xosi.o \
        libsrtm.o

OBJ_SDRTST	= tcpb.o sdr.o sdrtest.o
OBJ_SDRRADIO	= tcpb.o sdr.o radio.o
OBJ_L2CAT	= l2.o frameio.o deflate.o Execlogin.o cleanup.o l2cat.o
OBJ_ADSB2APRS	= adsb2aprs.o tcpb.o
OBJ_PROFILE	= profile.o imagetext.o libsrtm.o pngwrite.o

TARGETS += \
	afskmodem \
	gps2aprs \
	gps2digipos \
	udpbox \
	udpgate4 \
	udpflex \
	udphub \
	sondemod \
	sondeudp \
	aprsmap \
	sdrtst \
	sdrradio \
	adsb2aprs \
	profile

ifneq ($(HOSTOS), Darwin)
	TARGETS += l2cat
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
        $(OBJ_PROFILE)

# ------------------- target make rules -------------------
#                     -----------------
%.o: %.c
	@echo [compiling] $@
	@$(CC) $(CDEFS) $(CFLAGS) -o $@ $<

all: $(TARGETS)

clean:
	rm -f $(OBJS) $(TARGETS) aprsmap-x86 aprsmap-x86_64

afskmodem: $(OBJ_AFSKMODEM) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o afskmodem $(OBJ_COMMON) $(OBJ_AFSKMODEM) -lm
	$(STRIP) afskmodem $(STRIPFLAGS)

gps2aprs: $(OBJ_GPS2APRS) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o gps2aprs $(OBJ_COMMON) $(OBJ_GPS2APRS) -lm
	$(STRIP) gps2aprs $(STRIPFLAGS)

gps2digipos: $(OBJ_GPS2DIGIPOS) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o gps2digipos $(OBJ_COMMON) $(OBJ_GPS2DIGIPOS) -lm
	$(STRIP) gps2digipos $(STRIPFLAGS)

sondemod: $(OBJ_SONDEMOD) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o sondemod $(OBJ_COMMON) $(OBJ_SONDEMOD) -lm
	$(STRIP) sondemod $(STRIPFLAGS)

sondeudp: $(OBJ_SONDEUDP) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o sondeudp $(OBJ_COMMON) $(OBJ_SONDEUDP) -lm
	$(STRIP) sondeudp $(STRIPFLAGS)

udpbox: $(OBJ_UDPBOX) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o udpbox $(OBJ_COMMON) $(OBJ_UDPBOX) -lm
	$(STRIP) udpbox $(STRIPFLAGS)

udpgate4: $(OBJ_UDPGATE4) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o udpgate4 $(OBJ_COMMON) $(OBJ_UDPGATE4) -lm
	$(STRIP) udpgate4 $(STRIPFLAGS)

udpflex: $(OBJ_UDPFLEX) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o udpflex $(OBJ_COMMON) $(OBJ_UDPFLEX) -lm
	$(STRIP) udpflex $(STRIPFLAGS)

udphub: $(OBJ_UDPHUB) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o udphub $(OBJ_COMMON) $(OBJ_UDPHUB) -lm
	$(STRIP) udphub $(STRIPFLAGS)

aprsmap-static: $(OBJ_APRSMAP) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o $(APRSMAP_STATIC) $(OBJ_APRSMAP) $(OBJ_COMMON) \
	$(XLIBS) -lpthread -lxcb -lXau -lXdmcp -lm -lpthread -ldl -lz \
	-static
	$(STRIP) $(APRSMAP_STATIC) $(STRIPFLAGS)
aprsmap : $(OBJ_APRSMAP) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o aprsmap $(OBJ_APRSMAP) $(OBJ_COMMON) $(XLIBS) -lm
	$(STRIP) aprsmap $(STRIPFLAGS)
aprsmap-cleanobj:
	rm -f $(OBJ_APRSMAP) $(OBJ_COMMON)

sdrtst : $(OBJ_SDRTST) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o sdrtst $(OBJ_SDRTST) $(OBJ_COMMON) -lm
	$(STRIP) sdrtst $(STRIPFLAGS)

sdrradio : $(OBJ_SDRRADIO) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o sdrradio $(OBJ_SDRRADIO) $(OBJ_COMMON) -lm
	$(STRIP) sdrradio $(STRIPFLAGS)

l2cat : $(OBJ_L2CAT) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o l2cat $(OBJ_L2CAT) $(OBJ_COMMON) -lm
	$(STRIP) l2cat $(STRIPFLAGS)

adsb2aprs: $(OBJ_ADSB2APRS) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o adsb2aprs $(OBJ_ADSB2APRS) $(OBJ_COMMON) -lm
	$(STRIP) adsb2aprs $(STRIPFLAGS)

profile : $(OBJ_PROFILE) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o profile $(OBJ_PROFILE) $(OBJ_COMMON) $(XLIBS) -lm
	$(STRIP) profile $(STRIPFLAGS)

diag:
	echo $(HOSTARCH)

# end of makefile
