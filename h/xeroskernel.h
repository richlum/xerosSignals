/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */
#ifndef __XEROSKERNEL_H__
#define __XEROSKERNEL_H__

#include <stdarg.h>

/* Symbolic constants used throughout Xinu */

typedef char Bool; /* Boolean type			*/
#define	FALSE		0		/* Boolean constants		*/
#define	TRUE		1
#define	EMPTY		(-1)		/* an illegal gpq		*/
#define	NULL		0		/* Null pointer for linked lists*/
#define	NULLCH		'\0'		/* The null character		*/

/* Universal return constants */

#define	OK		 1		/* system call ok		*/
#define	SYSERR		-1		/* system call failed		*/
#define	EOF		-2		/* End-of-file (usu. from read)	*/
#define	TIMEOUT		-3		/* time out  (usu. recvtim)	*/
#define	INTRMSG		-4		/* keyboard "intr" key pressed	*/
/*  (usu. defined as ^B)	*/
#define	BLOCKERR	-5		/* non-blocking op would block	*/

/* Functions defined by startup code */

void bzero(void *base, int cnt);
void bcopy(const void *src, void *dest, unsigned int n);
int kprintf(char * fmt, ...);
void lidt(void);
void init8259(void);
void disable(void);
void outb(unsigned int, unsigned char);
unsigned char inb(unsigned int);

extern void kmeminit(void);
extern void *kmalloc(int size);
extern void kfree(void *ptr);

/* make sure interrupts are armed later on in the kernel development  */
#define STARTING_EFLAGS         0x00003000
// for enabling interrupts
#define INTR_ENA_EFLAGS			0x00003200
//static int      nextpid = 1;

#define MAX_PROC        64
#define KERNEL_INT      80
#define TIMER_INT       32
#define KEYBD_INT		33
#define PROC_STACK      (4096 * 5)
#define MAX_DEVICE		16

// processor states
#define STATE_STOPPED   0
#define STATE_READY     1
#define STATE_RUNNING 	2
#define STATE_DEAD    	3
#define STATE_BLOCKED	4
#define STATE_SLEEP		5

// request types
#define SYS_STOP        0
#define SYS_YIELD       1
#define SYS_CREATE      2
#define SYS_TIMER       3
#define SYS_GETPID      4
#define SYS_PUTS        5
#define SYS_SEND		6
#define SYS_RECV		7
#define SYS_SLEEP		8

#define SYS_SIGHANDLER  9
#define SYS_SIGRETURN	10
#define SYS_SIGKILL		11
#define SYS_SIGWAIT		12

#define SYS_OPEN		13
#define SYS_CLOSE		14
#define SYS_WRITE		15
#define SYS_READ		16
#define SYS_IOCTL		17

#define SYS_KEYBOARD	18

// return codes for send/recv messages
#define BAD_PID 		-1
#define SRC_DST_SAME 	-2
#define BAD_BUFFER_PARM -3
#define WOULD_DEADLOCK  -4

//1/TIMEDIVISOR seconds between SYS_TIMER events
#define TIMEDIVISOR    100
#define MAXSLEEPERS  	MAX_PROC

//return codes for syssighandler
#define SIGHDLR_OK		0
#define INVALID_SIGNAL -1
#define INVALID_ADDRESS -2
// return codes for signal
#define INVALID_PID 	-1
#define INVALID_SIG 	-2
#define INTR_BY_SIGNAL -128

#define MAXSIGNAL		31
#define MINSIGNAL		0
#define PID_NOT_EXIST 	-18
#define SIGNAL_INVALID  -3
#define EINTR			-128

#define DEVNAMESIZE		16
#define MAXFILES		4
#define MAXBUFFSIZE		256

//device status for file desc table
#define	DEV_CLOSED	0
#define DEV_OPEN	1

//keyboard return code for process_key
#define REGISTERED_KBDREAD 1
#define RECEIVED_EOF -2


typedef void (*funcptr)(void);
typedef void (**oldfuncptr)(void);

/**************devices major number *****************/
#define KEYBOARD_NOECHO	0
#define KEYBOARD_ECHO	1

#pragma pack(1)
//context frame used to get/set values from syscall
typedef struct context_frame {
	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int esp;
	unsigned int ebx;
	unsigned int edx;  //20
	unsigned int ecx;
	unsigned int eax;  //28
	unsigned int iret_eip;
	unsigned int iret_cs;
	unsigned int eflags;
	unsigned int stackSlots[0];
} context_frame;

extern unsigned int signal_priority_mask;
typedef struct struct_pcb pcb;

typedef struct devsw {
	int devnum;
	int (*dvopen)(pcb* p,int device_no);
	int (*dvclose)(pcb*, int fd);
	int (*dvwrite)(pcb*, int fd, void* buff, int bufflen);
	int (*dvread)(pcb*, int fd, void* buff, int bufflen);
	int (*dvioctl)(pcb*, int fd, unsigned long command, ...);
} devsw;

typedef struct struct_filedesc fildes;
struct struct_filedesc {
	int major;
	devsw* dev_tab;
	void* control_data;
	const char* devname;
	int status;
};


struct struct_pcb {
	long esp;
	pcb *next;
	int state;
	unsigned int pid;
	int ret;
	long args;
	unsigned int sleepticks;
	//what signals are waiting to be serviced
	unsigned int sig_pending;
	//what signal handlers are installed
	unsigned int sig_allow;
	//what is the current min signal we will respond to
	unsigned int sig_priority;
	//array of signal handlers
	void (*sig_handler[MAXSIGNAL])(void);
	fildes fd[MAXFILES];
	int sigwaiter;
};

extern unsigned short getCS(void);
extern void kmeminit(void);
extern void *kmalloc(int size);
extern void dispatch(void);
extern void dispatchinit(void);
extern void ready(pcb *p);
extern pcb *next(void);
extern void contextinit(void);
extern int contextswitch(pcb *p);
extern int create(funcptr fp, int stack);
extern void set_evec(unsigned int xnum, unsigned long handler);
extern int syscreate(funcptr fp, int stack);
extern int sysyield(void);
extern int sysstop(void);

extern void root(void);

void printCF(void * stack);

int syscall(int call, ...);
/* int syscreate(void (*func)(), int stack); */
int sysyield(void);
int sysstop(void);

extern unsigned int sysgetpid(void);
extern void sysputs(char *str);
extern int syssend(unsigned int dest_pid, void *buffer, int buffer_len);
extern int sysrecv(unsigned int *from_pid, void *buffer, int buffer_len);
extern int send(unsigned int dest_pid, void* sbuffer, int sbuffer_len, pcb* p);
extern int recv(unsigned int* from_pid, void* dbuffer, int dbuffer_len, pcb* p);
extern unsigned int syssleep(unsigned int milliseconds);

extern int signal(int pid, int sig_no);
extern int unsignal(int pid, int sig_no);
extern int validpid(int pid);
extern void sigreturn(void *old_sp);
extern int syssighandler(int signal, void (*newhandler)(void*),
		void (**oldHandler)(void*));
extern int syssigwait(void);
extern int sigkill(int pid, int signal_number);
extern void printbits(unsigned int num);
extern void modify_process_stack(pcb* p, int signum);
extern unsigned int get_priority_mask(int signalnumber);
extern int pr_mask_to_signum(unsigned int mask);
//sigtramp setup frame  to be pushed onto process when a signal is
// to be delivered.
typedef struct sigtramp_setup_frame {
	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int esp;
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int eax;  // end of pusha registers
	unsigned int eip;		//sigtramp
	unsigned int cs;
	unsigned int flags;
	unsigned int ret_addr;
	unsigned int handler;	//assigned handler
	unsigned int context_ptr;
	unsigned int old_sp;
	unsigned int saved_priority_mask;
	unsigned int saved_retval;
	unsigned int stackSlots[0];
} sigtramp_setup_frame;

/********user side device syscalls ****************/

extern int sysopen(int device_no);
extern int sysclose(int fd);
extern int syswrite(int fd, void* buff, int bufflen);
extern int sysread(int fd, void* guff, int bufflen);
extern int sysioctl(int fd, unsigned long command, ...);

extern pcb proctab[MAX_PROC];
extern struct devsw devtab[MAX_DEVICE];

#endif

