#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <xeroskernel.h>
#include <stdarg.h>


#define DEVICE_OPENED	1
#define DEVICE_CLOSED	2
#define UNSUPPORTED_DEV_OP -1
#define DEV_WRITE_FAIL	-2
#define DEV_READ_FAIL	-3
#define DEV_IOCTL_FAIL	-4
#define BAD_MAJOR_NUMBER -5



/**************devices major number *****************/
#define KEYBOARD_NOECHO	0
#define KEYBOARD_ECHO	1

extern struct devsw 	devtab[MAX_DEVICE];

void deviceinit(void);
int device_open(pcb* p, int devno);
int	device_close(pcb* p,int fd);
int	device_write(pcb *p, int fd,void* buff,int bufflen);
int	device_read(pcb* p, int fd, void* buff,int bufflen);
int	device_ioctl(pcb* p, int fd, int command, ...);
char* devno_to_name(int devno);
#endif
