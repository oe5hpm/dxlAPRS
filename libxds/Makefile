#
# Makefile for libxds needed by dxlaprs toolchain
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

CC			= $(CROSS_COMPILE)gcc
STRIP		= $(CROSS_COMPILE)strip
INCL		= include
CDEFS		= -D_unix -D_linux -DX2C_no_spawn -DX2C_strcmp
CFLAGS		= -I$(INCL) -c -O2 -fgnu89-inline -w -fdata-sections -ffunction-sections
# ---------------------- armv6 (raspberry pi 1) specific -----------------------
ifeq ($(PLATFORM), armv6)
CFLAGS		+= -march=armv6zk -mfpu=vfp -mfloat-abi=hard -mcpu=arm1176jzf-s
# ---------------------- armv7hf (bur am335x pp) specific ---------------------
else ifeq ($(PLATFORM), armv7hf)
CFLAGS		+= -march=armv7-a -mfpu=neon -mfloat-abi=hard
# -------------------------- x86_64 specific -----------------------------------
else ifeq ($(HOSTARCH), x86_64)
CFLAGS		+= -m32
# -------------------------- x86_32 (default) specific ------------------------
else
endif
# ------------------ objects enumeration ------------------
#                    -------------------
OBJ= \
	COROUTINES.o \
	ChanConsts.o \
	CharClass.o \
	ComplexMath.o \
	ConvTypes.o \
	DStrings.o \
	EXCEPTIONS.o \
	FileName.o \
	FilePath.o \
	FileSys.o \
	FormOut.o \
	FormStr.o \
	GeneralUserExceptions.o \
	IOChan.o \
	IOConsts.o \
	IOLink.o \
	IOResult.o \
	In.o \
	InOut.o \
	LocalHeap.o \
	LongComplexMath.o \
	LongConv.o \
	LongIO.o \
	LongInOut.o \
	LongMath.o \
	LongStr.o \
	LowLong.o \
	LowReal.o \
	M2EXCEPTION.o \
	MathC.o \
	MathCL.o \
	MathL.o \
	MathLib0.o \
	MathR.o \
	O2Strings.o \
	Out.o \
	POSIXIOChan.o \
	Printf.o \
	Processes.o \
	ProgEnv.o \
	ProgExec.o \
	ProgramArgs.o \
	RawIO.o \
	RealConv.o \
	RealIO.o \
	RealInOut.o \
	RealMath.o \
	RealStr.o \
	RegComp.o \
	RndFile.o \
	SIOResult.o \
	SLongIO.o \
	SRawIO.o \
	SRealIO.o \
	STextIO.o \
	SWholeIO.o \
	Semaphores.o \
	SeqFile.o \
	StdChans.o \
	Storage.o \
	StreamFile.o \
	Strings.o \
	SysClock.o \
	TERMINATION.o \
	TermFile.o \
	TextIO.o \
	TimeConv.o \
	WholeConv.o \
	WholeIO.o \
	WholeStr.o \
	X2C.o \
	XDSRTL.o \
	XEXCEPTIONS.o \
	XIOChan.o \
	XPrs.o \
	XRaise.o \
	XReal.o \
	oberonRTS.o \
	platform.o \
	x2cLib.o \
	xDevData.o \
	xFilePos.o \
	xMathAds.o \
	xmRTS.o \
	xosBalls.o \
	xosDirs.o \
	xosEnv.o \
	xosExec.o \
	xosExit.o \
	xosFS.o \
	xosFileIO.o \
	xosFiles.o \
	xosFmtIO.o \
	xosFmtNL.o \
	xosIpts.o \
	xosMalloc.o \
	xosMem.o \
	xosTTY.o \
	xosTime.o \
	xosTimeOps.o \
	xrAReal.o \
	xrBlockManager.o \
	xrDTree.o \
	xrExceptions.o \
	xrExponents.o \
	xrFName.o \
	xrHistory.o \
	xrInt64.o \
	xrLSETs.o \
	xrM2MM.o \
	xrMM.o \
	xrO2MM.o \
	xrSETs.o \
	xrStrCmp.o \
	xrStrings.o \
	xrTCov.o \
	xrThreads.o \
	xrcACMPLX.o \
	xrcArythmetics.o \
	xrcChecks.o \
	xrcCrtnStart.o \
	xrcIncDec.o \
	xrcO2MM.o \
	xrcRTS.o \
	xrcStart.o
# ------------------- target make rules -------------------
#                     -----------------
%.o: %.c
	$(CC) $(CDEFS) $(CFLAGS) -o $@ $<

libxds : $(OBJ)
	ar q libxds.a $(OBJ)
	ranlib libxds.a
clean:
	rm -f libxds.a $(OBJ)

