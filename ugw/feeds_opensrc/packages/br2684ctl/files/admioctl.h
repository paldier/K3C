//joelin
#include <linux/ioctl.h>

/* type definitions */
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;

typedef struct _REGRW_
{
  unsigned int addr;
  unsigned int value;
  unsigned int mode;
}REGRW, *PREGRW;

/* IOCTL keys */
#define KEY_IOCTL_ADM_REGRW		0x01
#define KEY_IOCTL_ADM_SW_REGRW		0x02
#define KEY_IOCTL_ADM_SW_PORTSTS	0x03
#define KEY_IOCTL_ADM_SW_INIT		0x04

#define KEY_IOCTL_MAX_KEY		0x05

/* IOCTL MAGIC */
static const unsigned char ADM_MAGIC = 'a'|'d'|'m'|'t'|'e'|'k';

/* IOCTL parameters */
#define ADM_IOCTL_REGRW			_IOWR(ADM_MAGIC, KEY_IOCTL_ADM_REGRW, REGRW)
#define ADM_SW_IOCTL_REGRW		_IOWR(ADM_MAGIC, KEY_IOCTL_ADM_SW_REGRW, REGRW)
#define ADM_SW_IOCTL_PORTSTS		_IOWR(ADM_MAGIC, KEY_IOCTL_ADM_SW_PORTSTS, NULL)
#define ADM_SW_IOCTL_INIT		_IOWR(ADM_MAGIC, KEY_IOCTL_ADM_SW_INIT, NULL)

#define REG_READ	0x0
#define REG_WRITE	0x1
