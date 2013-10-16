/* syscall.c : syscalls
 */

#include <xeroskernel.h>
#include <stdarg.h>
#include <utility.h>
#include <i386.h>

/****************main syscall gateway **************************/
//user side originated system calls
// req = request type (sysgetpid, yield, sysputs etc..
//  	other parameters depend on type of request
int syscall( int req, ... ) {
/**********************************/

    va_list     ap;
    int         rc;

    va_start( ap, req );

	// req = request (SYS_STOP, SYS_SEND etc..
	// ap = arguments to request on stack in order, depends on req type
	//	eg SYS_SEND, ap = dest_pid, ap+1 = *buffer, ap+2 = buffer_len 
	//		- assuming all args are same size (ptr length = sizeof (int))
	//	kernel returns result in eax which is loaded into rc.
    __asm __volatile( " \
        movl %1, %%eax \n\
        movl %2, %%edx \n\
        int  %3 \n\
        movl %%eax, %0 \n\
        "
        : "=g" (rc)
        : "g" (req), "g" (ap), "i" (KERNEL_INT)
        : "%eax" 
    );
 
    va_end( ap );

    return( rc );
}

/****************state change calls **************************/
 int syscreate( funcptr fp, int stack ) {
/*********************************************/

    return( syscall( SYS_CREATE, fp, stack ) );
}

 int sysyield( void ) {
/***************************/

    return( syscall( SYS_YIELD ) );
}

 int sysstop( void ) {
/**************************/
    return( syscall( SYS_STOP ) );
}

/****************general system service calls **************************/

// get the pid of the current process
unsigned int sysgetpid(void){
    return (syscall (SYS_GETPID) );
}
// send a null terminated string to kernel to output
void sysputs(char* str){
    syscall (SYS_PUTS, str);
}

/****************send/recv messaging calls **************************/
// send a message to another pid.
// BLOCKING call
//	returns number of bytes acceptd by destination or 
//	returns -1 if dest process does not exist
//	returns -2 if process trying to send to self
//	returns -3 for other send error. 
extern int syssend(unsigned int dest_pid, void *buffer, int buffer_len){
	
	int result = (syscall (SYS_SEND,dest_pid, buffer, buffer_len));
	
	return result;
}


// receive a message from another pid
// BLOCKING call
//	if from_pid = 0, receive from any pid, will return pid 
//	returns number of bytes received or
//	returns -1 if pid to receive from is invalid
//	returns -2 if recv pid  = send pid
//	returns -3 if other recv error (eg neg buff size, invalid buff addr, etc
//	

extern int sysrecv(unsigned int *from_pid, void *buffer, int buffer_len){
	return (syscall (SYS_RECV, from_pid, buffer, buffer_len));
}

extern unsigned int syssleep(unsigned int milliseconds){
	return (syscall (SYS_SLEEP, milliseconds));
	
}

/****************signal  calls **************************/
/*********************************
 * signal 0..31, lowest to highest priority
 * function pointer to signal handler
 * syssighandler updates oldHanlder to contain address of previous signal handler
 * return 0 on successful installation of new handler
 * 		-1 for invalid signal request
 * 		-2 invalid address for signal handler function
 * 
 * */

// install signal handler for process at signalnumber = signal
// if there is an old handler being replaced, retrieve it in oldHandler
int syssighandler(int signal, void(*newhandler)(void*), void(**oldHandler)(void*)){
	TRACE
	// freemem is for user processes, all code resides below freemem
	if (((void*)newhandler > (void*)freemem) || ((void*)newhandler > (void*)maxaddr) ||
			(((long)newhandler>HOLESTART)&&((long)newhandler<HOLEEND))
			){
		TRACE
		return INVALID_ADDRESS;
	}
	if ((signal<0)||(signal>MAXSIGNAL)){
		TRACE
		return INVALID_SIG;
	}
	TRACE
	int rc = syscall(SYS_SIGHANDLER, signal, newhandler, oldHandler);

	return SIGHDLR_OK;
}


// user side access to send signals, returns immediate;y
// 0 on success, -ve on faiure
int sigkill(int pid, int signal_number){
	TRACE
	int rc = syscall(SYS_SIGKILL,pid, signal_number);
	return rc;
}

// to be used only by trampoline code 
// sp is the location on stack to return to after signal handling
// should be same value trampoline code gets as arg.
// 	kernel will replace stored stackpointer with this arg.
//  this call will not 'return'
void sigreturn(void *old_sp){
	TRACE
	syscall(SYS_SIGRETURN, old_sp);
	
}

// blocks waiting on signal. return signal number that unblocks
int syssigwait(void){
	TRACE
	int ret = syscall(SYS_SIGWAIT);
	return ret;
}

/****************device calls **************************/


int sysopen (int device_no){
	TRACE
	DBGMSG("device_no=%d\n",device_no);
	int rc = syscall(SYS_OPEN,device_no);
	return rc;
}

int sysclose(int fd){
	TRACE
	int rc = syscall(SYS_CLOSE,fd);
	return rc;
}

int syswrite(int fd, void* buff, int bufflen){
	TRACE
	int rc = syscall(SYS_WRITE, fd, buff, bufflen);
	return rc;
}

int sysread(int fd, void* buff, int bufflen){
	TRACE
	DBGMSG("fd = %d",fd);
	int rc = syscall(SYS_READ,fd, buff, bufflen);
	DBGMSG("syscall sys_read returning %d\n", rc);
	return rc; 
}

int sysioctl(int fd, unsigned long command, ...){
	TRACE
	va_list     ap;
    va_start( ap, command );
    int a= va_arg(ap,int);
    int b= va_arg(ap, int);
	int rc = syscall(SYS_IOCTL,fd,command, a, b);
	va_end(ap);
	return rc;
}
