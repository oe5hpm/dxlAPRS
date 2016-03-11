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
# - udprfnet
# - aprsmap		powerful APRS viewer based on Openstreetmap
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

.SUFFIXES:
	MAKEFLAGS += -r

CC			= $(CROSS_COMPILE)gcc
STRIP		= $(CROSS_COMPILE)strip
INCL		= .
CDEFS		= -Wall \
			  -Wno-unused-variable -Wno-parentheses -Wno-pointer-sign \
			  -Wno-format -Wno-return-type -Wno-char-subscripts \
			  -Wno-uninitialized \
			  -Werror=implicit-function-declaration
#CDEFS		= -w
CFLAGS		= -I$(INCL) -c -O2 -fdata-sections -ffunction-sections
LFLAGS		= -Wl,--gc-sections
# ---------------------- armv6 (raspberry pi 1) specific -----------------------
ifeq ($(PLATFORM), armv6tce)
EXTLIB  	= lib_armv6/
ADDLIBS		= lib_armv6/libpng15/libpng.a \
			  lib_armv6/libz.a \
			  lib_armv6/libjpeg.a
XLIBS		= $(EXTLIB)libxcb.so.1 $(EXTLIB)libXdmcp.so.6 $(EXTLIB)libXau.so.6 -L$(EXTLIB) -lXext -lX11
CFLAGS		+= -march=armv6zk -mfpu=vfp -mfloat-abi=hard -mcpu=arm1176jzf-s
CFLAGS		+= -Ilib_armv6/libpng15/ -I.
LFLAGS		+=
else ifeq ($(PLATFORM), armv6)
EXTLIB  	= lib_armv6/
ADDLIBS		= lib_armv6/libpng16/libpng.a \
			  lib_armv6/libz.a \
			  lib_armv6/libjpeg.a
XLIBS		= $(EXTLIB)libxcb.so.1 $(EXTLIB)libXdmcp.so.6 $(EXTLIB)libXau.so.6 -L$(EXTLIB) -lXext -lX11
CFLAGS		+= -march=armv6zk -mfpu=vfp -mfloat-abi=hard -mcpu=arm1176jzf-s
CFLAGS		+= -Ilib_armv6/libpng16/ -I.
LFLAGS		+=
# ---------------------- armv7hf (bur am335x pp) specific ---------------------
else ifeq ($(PLATFORM), armv7hf)
EXTLIB		= lib_armv7hf/
XLIBS		= $(EXTLIB)libxcb.so $(EXTLIB)libXdmcp.so $(EXTLIB)libXau.so -L$(EXTLIB) -lXext -lX11
ADDLIBS		= lib_armv7hf/libpng.a \
			  lib_armv7hf/libz.a \
			  lib_armv7hf/libjpeg.a
CFLAGS		+= -march=armv7-a -mfpu=neon -mfloat-abi=hard
CFLAGS		+= -Ilib_armv7hf
LFLAGS		+=
# -------------------------- x86_64 specific -----------------------------------
else ifeq ($(HOSTARCH), x86_64)
EXTLIB		= .
ADDLIBS		= lib_x86/libpng.a \
			  lib_x86/libz.a \
			  lib_x86/libjpeg.a
XLIBS		= -L/usr/X11R6/lib -lXext -lX11
CFLAGS		+= -m32
LFLAGS		+= -m32
# -------------------------- x86_32 (default) specific ------------------------
else
EXTLIB		= .
ADDLIBS		= lib_x86/libpng.a \
			  lib_x86/libz.a \
			  lib_x86/libjpeg.a
XLIBS		= -L/usr/X11R6/lib -lXext -lX11
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
OBJ_GPS2DIGIPOS= gps2digipos.o

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

OBJ_SONDEUDP	= sondeudp.o
OBJ_UDPBOX		= udpbox.o
OBJ_UDPGATE4	= udpgate4.o
OBJ_UDPFLEX	= udpflex.o
OBJ_UDPHUB		= udphub.o
OBJ_UDPRFNET	= udprfnet.o
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
	xosi.o

OBJ_SDRTST		= tcpb.o sdr.o sdrtest.o
OBJ_SDRRADIO	= tcpb.o sdr.o radio.o

TARGETS = \
	afskmodem \
	gps2aprs \
	gps2digipos \
	udpbox \
	udpgate4 \
	udpflex \
	udphub \
	udprfnet \
	sondemod \
	sondeudp \
	aprsmap \
	sdrtst \
	sdrradio

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
	$(OBJ_UDPRFNET) \
	$(OBJ_APRSMAP) \
	$(OBJ_SDRTST) \
	$(OBJ_SDRRADIO)
# ------------------- target make rules -------------------
#                     -----------------
%.o: %.c
	@echo [compiling] $@
	@$(CC) $(CDEFS) $(CFLAGS) -o $@ $<

all: $(TARGETS)

clean:
	rm -f $(OBJS) $(TARGETS)

afskmodem: $(OBJ_AFSKMODEM) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o afskmodem $(OBJ_COMMON) $(OBJ_AFSKMODEM) -lm
	$(STRIP) afskmodem -s

gps2aprs: $(OBJ_GPS2APRS) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o gps2aprs $(OBJ_COMMON) $(OBJ_GPS2APRS) -lm
	$(STRIP) gps2aprs -s

gps2digipos: $(OBJ_GPS2DIGIPOS) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o gps2digipos $(OBJ_COMMON) $(OBJ_GPS2DIGIPOS) -lm
	$(STRIP) gps2digipos -s

sondemod: $(OBJ_SONDEMOD) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o sondemod $(OBJ_COMMON) $(OBJ_SONDEMOD) -lm
	$(STRIP) sondemod -s

sondeudp: $(OBJ_SONDEUDP) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o sondeudp $(OBJ_COMMON) $(OBJ_SONDEUDP) -lm
	$(STRIP) sondeudp -s

udpbox: $(OBJ_UDPBOX) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o udpbox $(OBJ_COMMON) $(OBJ_UDPBOX) -lm
	$(STRIP) udpbox -s

udpgate4: $(OBJ_UDPGATE4) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o udpgate4 $(OBJ_COMMON) $(OBJ_UDPGATE4) -lm
	$(STRIP) udpgate4 -s

udpflex: $(OBJ_UDPFLEX) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o udpflex $(OBJ_COMMON) $(OBJ_UDPFLEX) -lm
	$(STRIP) udpflex -s

udphub: $(OBJ_UDPHUB) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o udphub $(OBJ_COMMON) $(OBJ_UDPHUB) -lm
	$(STRIP) udphub -s

udprfnet: $(OBJ_UDPRFNET) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o udprfnet $(OBJ_COMMON) $(OBJ_UDPRFNET) -lm -lrt
	$(STRIP) udprfnet -s

aprsmap : $(OBJ_APRSMAP) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o aprsmap $(OBJ_APRSMAP) $(OBJ_COMMON) $(ADDLIBS) $(XLIBS) -lm
	$(STRIP) aprsmap -s
aprsmap-cleanobj:
	rm -f $(OBJ_APRSMAP)

sdrtst : $(OBJ_SDRTST) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o sdrtst $(OBJ_SDRTST) $(OBJ_COMMON) -lm
	$(STRIP) sdrtst -s

sdrradio : $(OBJ_SDRRADIO) $(OBJ_COMMON)
	$(CC) $(LFLAGS) -o sdrradio $(OBJ_SDRRADIO) $(OBJ_COMMON) -lm
	$(STRIP) sdrradio -s

# end of makefile
