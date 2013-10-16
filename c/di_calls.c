#include <di_calls.h>
#include <xeroslib.h>
#include <utility.h>
#include <kbd.h>

#define DEVLEN 64
// if we allow multiple simultanous opens set to 1, no counting implemented
// for close, we just close on first request.
#define MULTIPLE_OPEN_ALLOWED 0

struct devsw 	devtab[MAX_DEVICE];
//this should reflects device constants in xeroskernel.h
char* devnames[] = { "KEYBOARD_NOECHO", "KEYBOARD_ECHO" };
char controldata[MAX_DEVICE][DEVLEN];

//default function for all device methods
int unsupported_device_operation(void){
	kprintf("unsupported operation for device\n");
	return UNSUPPORTED_DEV_OP;
}

//initialize all functions to output unsupported error
void deviceinit(void){
	int i ;
	for (i=0;i<MAX_DEVICE;i++){
		devtab[i].devnum=i;
		devtab[i].dvopen=(int(*)(pcb*, int))unsupported_device_operation;
		devtab[i].dvclose=(int(*)(pcb*, int))unsupported_device_operation;
		devtab[i].dvwrite=(int(*)(pcb*, int,void*,int))unsupported_device_operation;
		devtab[i].dvread=(int(*)(pcb*, int,void*,int))unsupported_device_operation;
		devtab[i].dvioctl=(int(*)(pcb*, int,unsigned long, ...))unsupported_device_operation;
	}

	keyboard_init();

}

//Searches for empty slot in processes file descriptor
//table and fills out the descriptor
//precondition, all args have been validated before entry
//here, no checking is done here, it just fills in data
//as given
// input:
//	fd = ptr to this processes filedesctable
//	devno = device major number
//	devtab = ptr to device table
//	cdata = ptr to device control data buffer
//	devnamestr = string to identify device name
//	status = device status
void createFDTentry(	fildes* fd, int devno, devsw* devtab,
		char* cdata, const char* devnamestr, int status){
	TRACE
	DBGMSG("devno=%d, ", devno);
	DBGMSG("devtab=%x, ",devtab);
	DBGMSG("devname=%s, ",devnamestr);
	DBGMSG("status=%d \n",status);
	fd->major = devno;
	fd->dev_tab = devtab;
	fd->control_data = cdata;
	fd->devname= devnamestr;
	fd->status = status;
}
// verify fd valid
// id device
// fdt of process -> index no, id function
// call funciton
// return value

//on success return filedescriptor
//  negative number on failure

int validate_fd(pcb* p, int fd){
	if ((fd<0)||(fd>=MAXFILES)){
		return 0;
	}
	if (p->fd[fd].status==DEV_OPEN)
		return 1;
	return 0;
}

//device independent open
//create fd, point to device table, return index into this
//  process fd
//return negative value on failure
int di_open(pcb* p, int devno){
	TRACE
	int result;
	if (devno<=MAX_DEVICE){
		TRACE
		// create entry for pcb.fd, start by assigning ptr to device table
		// all entries point to base of devtab anyway, ok to refresh if already exists
		// required to datafill, if doesnt exist.
		fildes* fdt = (proctab[p->pid%MAX_PROC].fd);
		fdt->dev_tab=devtab;
		int i=0;
		while ((i<MAXFILES)&&(fdt[i].status == DEV_OPEN)){
			TRACE
			//special case only allow either keyboard with or without echo
			// cannot open
			if( ((devno==KEYBOARD_NOECHO)&&(fdt[i].major==KEYBOARD_ECHO))||
				((devno==KEYBOARD_ECHO)&&(fdt[i].major==KEYBOARD_NOECHO)) )
			{
				//disallow opening of 'other' type of keyboard if this one is
				//already open
				return -1;
			}
			//multiple opens on same device, just ignore
			if (fdt[i].major==devno){
				if (MULTIPLE_OPEN_ALLOWED)
					//its already open, return filedescriptor index
					return i;
				else
					return -1;
			}
			i++;
		}
		DBGMSG("index into fdt is %d\n", i);
		if (i>=MAXFILES){
			TRACE
			return -1;
		}else{
			TRACE
			//device index is < maxfiles and status is not dev_open
			// use i as the new index into fdt and initialize entry
			memset(controldata[i],'\0',DEVLEN);
			createFDTentry(&(proctab[p->pid%MAX_PROC].fd[i]),
					devno,devtab,
					controldata[i],
					devnames[devno],DEV_OPEN);
			fildes* fd= &(p->fd[i]);
			int rc = (*(fd->dev_tab)->dvopen)(p, devno);

			if (rc>0)
				result = i;
			else
				result = rc;
			// todo check that rc error is meaningful and negative
		}

		return result;
	}else{
		return -1;
	}
	
}

// device independent close,
// release fd by changing state to closed
// return negative value on failure.
int	di_close(pcb* p,int fdindex){
	int result=-1;
	struct devsw* devptr;
	if (validate_fd(p,  fdindex)){
		if ((p->fd[fdindex]).status == DEV_OPEN){
			devptr = ((p->fd[fdindex]).dev_tab);
			result = (*devptr->dvclose)(p,fdindex);
			if (result==0)
				(p->fd[fdindex]).status = DEV_CLOSED;
			//state will allow overwrite of fd when next device requires it
		}
	}
	return result;
}

// device independant write
// return negative value on failure.
// return number of bytes written on success
int	di_write(pcb *p, int fdindex,void* buff,int bufflen){
	int result=-1;
	struct devsw* devptr;
	if (validate_fd(p,  fdindex)){
		if ((p->fd[fdindex]).status == DEV_OPEN){
			devptr = ((p->fd[fdindex]).dev_tab);
			result = (*devptr->dvwrite)(p,fdindex,buff,bufflen);
		}
	}
	return result;
}

// device independent read
// return negative value on failure
// return number of bytes read on success,
//	can be < bufflen
//	0 = endoffile

int	di_read(pcb* p, int fdindex, void* buff,int bufflen){
	int result=-1;
	struct devsw* devptr;
	TRACE
	DBGMSG("di_read pid = %d, state = %d\n", p->pid, p->state);
	DBGMSG("fdindex = %d, buffaddr = %x, bufflen = %d\n",
				fdindex, buff, bufflen)
	if (validate_fd(p,  fdindex)){
		if ((p->fd[fdindex]).status == DEV_OPEN){
			TRACE
			devptr = ((p->fd[fdindex]).dev_tab);
			result = (*devptr->dvread)(p,fdindex,buff, bufflen);
			DBGMSG("dvread returned %d", result);
		}
	}
	//if (result==EOF) return 0;
	TRACE
	DBGMSG("di_read returning  %d\n",result);
	return result;
}



//device independent io control commands
// return negative value on failure
// return zero otherwise
int	di_ioctl(pcb* p, int fdindex, int command, ...){
	TRACE
	va_list ap;
	//va_list sap;
	va_start(ap,command);
	//sap = (va_list)va_arg(ap, unsigned int);
	int a = va_arg(ap, unsigned int);
	int b = va_arg(ap, unsigned int);
	DBGMSG("di command = %d\n", command);
	DBGMSG("command args a=%d b=%d\n", a, b);
	int result=-1;

	struct devsw* devptr;
	if (validate_fd(p,  fdindex)){
		if ((p->fd[fdindex]).status == DEV_OPEN){
			devptr = ((p->fd[fdindex]).dev_tab);
			result = (*devptr->dvioctl)(p,fdindex,command,a,b);
		}
	}
	va_end(ap);
	return result;
}






