#ifndef _PP065MB_H_
#define _PP065MB_H_

#include <linux/errno.h>
#include <linux/ioctl.h>
// ----------------------------------------------------------------
typedef struct
{
	unsigned char		command;
	unsigned short		address;
	unsigned char		target;
	unsigned char		devnum;
	unsigned char		datasize;
	void				*pData;
} pp065mb_mtcxIoctlArg_typ;
// ----------------------------------------------------------------
// constants for GPCTRCFG
#define		GPCTR_RISING			0x00000001
#define		GPCTR_FALLING			0x00000002
#define		GPCTR_RISEFALL			0x00000003
#define		GPCTR_FILTER_1MS		0x00000004
#define		GPCTR_FILTER_10MS		0x00000008
#define		GPCTR_CLEAR				0x00008000
#define		GPCTR_CFGENABLE			0x80000000
// definition for GPCTRCFG
typedef struct
{
	unsigned int		ctrCfg1;
	unsigned int		ctrCfg2;
	unsigned int		ctrCfg3;
	unsigned int		ctrCfg4;
} ppCtrCfg_typ;
// definition for GPCTRVAL
typedef struct
{
	int					ctrVal1;	// GPIO15
	int					ctrVal2;	// GPIO16
	int					ctrVal3;	// GPIO17
	int					ctrVal4;	// GPIO18
} ppCtrVal_typ;
// ----------------------------------------------------------------
// definition for MTCX_READ_VERSIONS
typedef struct
{
	unsigned int		fpgaVersion;
	unsigned int		px32Version;
	unsigned int		fwVersion;
	unsigned int		reserved;
} ppVersionInfo_typ;
// ----------------------------------------------------------------
// ioctl commands
#define MTCX_READ_VERSIONS					_IOR('b', 0x01, ppVersionInfo_typ)
#define MTCX_READ_GENCMD					_IOR('b', 0x02, pp065mb_mtcxIoctlArg_typ)
#define MTCX_WRITE_GENCMD					_IOW('b', 0x03, pp065mb_mtcxIoctlArg_typ)
#define	 GPCTRCFG							_IOW('b', 0x04, ppCtrCfg_typ)
#define GPCTRVAL							_IOR('b', 0x05, ppCtrVal_typ)
#define PORTCFG								_IOW('b', 0x06, unsigned short)
#define PARWR								_IOW('b', 0x07, unsigned short)
#define PARRD								_IOR('b', 0x08, unsigned char)

#define PRESSKEY							_IOW('b', 0x08, unsigned short)
#endif //_PP065MB_H_
