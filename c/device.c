#include <device.h>
#define DEVLEN 64

struct devsw 	devtab[MAX_DEVICE];
//this should reflects device constants in xeroskernel.h
const char devnames[] = { "KEYBOARD_NOECHO", "KEYBOARD_ECHO" };
char controldata[MAX_DEVICE][DEVLEN];


int unsupported_device_operation(void){
	kprintf("unsupported operation for device\n");
	return UNSUPPORTED_DEV_OP;
}

void deviceinit(void){
	int i ;
	for (i=0;i<MAX_DEVICE;i++){
		devtab[i].devnum=i;
		devtab[i].di_open=(int(*)(int))unsupported_device_operation;
		devtab[i].di_close=(int(*)(int))unsupported_device_operation;
		devtab[i].di_write=(int(*)(int,void*,int))unsupported_device_operation;
		devtab[i].di_read=(int(*)(int,void*,int))unsupported_device_operation;
		devtab[i].di_ioctl=(int(*)(int,unsigned long, ...))unsupported_device_operation;
	}

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
		char* cdata, char* devnamestr, int status){
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


int device_open(pcb* p, int devno){
	int result;
	if (devno<=MAX_DEVICE){
		devsw* devptr = p->fd[devno].dev_tab; 
		result = (*devptr->di_open)(devno);
		// we need to create entry for pcb.fd
		fildes fdt = (proctab[p->pid%MAX_PROC].fd);
		int i;
		while ((strlen(i<MAXFILES)&&(fdt[i].devname) != 0)){
			if (fdt[i].major==devno){
				//its already open, return filedescriptor index
				return i;
			}
			i++;
		}
		if (i>=MAXFILES){
			return -1;
		}else{
			memset(controldata[i],'\0',DEVLEN);
			createFDTentry(&(proctab[p->pid%MAX_PROC].fd[i]),
					devno,devtab,
					controldata[i],
					devnames[devno],DEV_OPEN);
			fildes* fd= &(p->fd[i]);
			int rc = (*(fd->dev_tab)->di_open)(devno);
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

	
int	device_close(pcb* p,int fd){
	int result;
	if (validate_fd(p,  fd)){

	}
}
	
int	device_write(pcb *p, int fd,void* buff,int bufflen){
	
}

int	device_read(pcb* p, int fd, void* buff,int bufflen){
	
}

int	device_ioctl(pcb* p, int fd, int command, ...){
	
}


char* devno_to_name(int devno){

}
}
