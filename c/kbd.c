#include <kbd.h>
#include <xeroskernel.h>
#include <utility.h>
#include <xeroslib.h>
#include <di_calls.h>
#include <i386.h>
#include <scanToASCII.h>

extern pcb		*blocked_q;
extern int remove(pcb** q, unsigned int pid);

char* kbdbuffer;
int   kbdbufidx=0;
#define   KBDBUFSZ 4
char  kbuffer[KBDBUFSZ];
char* applbuffer;
int   applbufsize;
int   applbufidx;
pcb*  reader;
int	  device;
int   eof_received=0;

//default <ctrl> down and 'd' down signify EOF
//unsigned char EOF_startbyte=0x1d;
// use scanToASCII state tracking instead

//this is the scan code value of d in hex
unsigned char EOF_endbyte=0x20;

int kbd_open(pcb*p, int devno){
	TRACE
	keyboard_init();
	//keyboard irq maps to interrupt 33 = KEYBD_INT
	enable_irq(1,0);
	reader=p;
	device=devno;
	eof_received=0;
	return 1;
}

int kbd_close(pcb*p, int fd){
	TRACE
	//disable keyboard
	enable_irq(1,1);
	applbuffer=NULL;
	applbufsize=0;
	device=-1;
	reader=NULL;
	eof_received=0;

	return 0;
}

//no echo keyboard, application must echo if required
//return 1 : successful registration to read keyboard inputs

int kbd_read(pcb* p,int fd, void* buff, int bufflen){
	TRACE
	int i;
	unsigned char *buffer = (unsigned char*)buff;
	if (eof_received==1){
		TRACE
		for (i=0;i<bufflen;i++){
			buffer[i]='\0';
		}
		return EOF;
	}
	applbuffer=buff;
	applbufsize=bufflen;
	applbufidx=0;
	kbdbufidx=0;
	reader=p;
	DBGMSG("read process %d, fd = %d, buff addr %x, bufflen %d\n",
			p->pid, fd, buff, bufflen);
	//validate fd
	//validate kbd open
	return REGISTERED_KBDREAD;
}
//echo keyboard - lower half immediately echos chars as it receives them
int ekbd_read(pcb* p,int fd, void* buff, int bufflen){
	TRACE
	if (eof_received==1){
		TRACE
		return EOF;
	}
	applbuffer=buff;
	applbufsize=bufflen;
	applbufidx=0;
	kbdbufidx=0;
	reader=p;
	DBGMSG("read process %d, fd = %d, buff addr %x, bufflen %d\n",
			p->pid, fd, buff, bufflen);
	//validate fd
	//validate kbd open
	return REGISTERED_KBDREAD;
}

//http://cs.smith.edu/~thiebaut/ArtOfAssembly/CH20/CH20-2.html#MARKER-9-36
//ref for cmd table
int validcommand(int cmd){
	int result = 0;
	int cmds[]={ 0x20,0x60,0xa4,
			0xa5,0xa6,0xa7,0xa8,
			0xa9,0xaa,0xab,0xac,
			0xad,0xae,0xc0,0xc1,
			0xc2,0xd0,0xd1,0xd2,
			0xd3,0xe0,0xf0,0xf1,
			0xf2,0xf3,0xf4,0xf5,
			0xf6,0xf7,0xf8,0xf9,
			0xfa,0xfb,0xfc,0xfd,
			0xfe,0xff
			};
	int i;
	for (i=0;i<sizeof(cmds);i++){
		if (cmd==cmds[i])
			return 1;
	}
	return result;
}


int kbd_ioctl(pcb*p, int fd, int command, ...){
	TRACE
	if (!validcommand(command)&&(command!=49)){
		return -1;
	}
	if (command==49){
		//install replacement char for EOF
		va_list ap;
        va_list sap;
        va_start(ap,command);
        int a = va_arg(ap,int);
        DBGMSG("installing EOF char: %x %c\n", (unsigned char)a, kbtoa(a));
        EOF_endbyte=(unsigned char)a;
        DBGMSG(" EOF char: %x %c\n", EOF_endbyte, kbtoa(EOF_endbyte));

	}else{
		DBGMSG("sending command %x\n ", command);
		outb(i8042_control_port, command);
		DBGMSG("kbdioctl returning %d\n",1);
	}
	return 1;
}

void keyboard_init(void){
	devtab[KEYBOARD_NOECHO].devnum=KEYBOARD_NOECHO;
	devtab[KEYBOARD_NOECHO].dvopen=(int(*)(pcb*,int))kbd_open;
	devtab[KEYBOARD_NOECHO].dvclose=(int(*)(pcb*, int))kbd_close;
	devtab[KEYBOARD_NOECHO].dvwrite=(int(*)(pcb*, int,void*,int))unsupported_device_operation;
	devtab[KEYBOARD_NOECHO].dvread=(int(*)(pcb*, int,void*,int))kbd_read;
	devtab[KEYBOARD_NOECHO].dvioctl=(int(*)(pcb*, int,unsigned long, ...))kbd_ioctl;

	devtab[KEYBOARD_ECHO].devnum=KEYBOARD_ECHO;
	devtab[KEYBOARD_ECHO].dvopen=(int(*)(pcb*, int))kbd_open;
	devtab[KEYBOARD_ECHO].dvclose=(int(*)(pcb*, int))kbd_close;
	devtab[KEYBOARD_ECHO].dvwrite=(int(*)(pcb*, int,void*,int))unsupported_device_operation;
	devtab[KEYBOARD_ECHO].dvread=(int(*)(pcb*, int,void*,int))ekbd_read;
	devtab[KEYBOARD_ECHO].dvioctl=(int(*)(pcb*, int,unsigned long, ...))kbd_ioctl;
}

int kbd_data_ready(void){
	unsigned char c = inb(i8042_control_port);
	//lower order bit set indicates data ready
	return c&1;
}


int get_kbd_data(void){
	unsigned char c = inb(i8042_datasrc_port);
#ifdef DEBUG
	kprintf("\nkey=%x %d, hexof ascii = %x ascii= %c\n", c, c, (unsigned char) kbtoa(c),(unsigned char) kbtoa(c));
#endif
	kprintf("%c",(unsigned char)kbtoa(c));
	return c;
}

//unblock reader
void notify(pcb* p){
	TRACE
	DBGMSG("pid %d state %d \n", p->pid, p->state);
	kassert (p->state==STATE_BLOCKED);
	int mypid = remove(&blocked_q,p->pid);
	DBGMSG("removed pid %d from blocked \n", mypid);
	p->ret = applbufidx;

	ready(p);
}

//unsigned char lastbyte;
void print_kbd_buf(void){
#ifdef DEBUG
	int i ;
	kprintf("\nkbdbuffer:");
	for (i=0;i<KBDBUFSZ;i++){
			kprintf("%c ",kbdbuffer[i]);
	}
	kprintf("\n");
	mypause(6);
#endif
}

int process_key(void){
	unsigned char c = inb(i8042_datasrc_port);
	unsigned char ascii = kbtoa(c);
	//DBGMSG("-%c-\n",kbtoa(c));
	//ignore key up sequences

	//keyboardbuffer is full just drop chars as they come in
	if ((kbdbufidx>=KBDBUFSZ)&&(applbuffer==NULL)){
		return 0;
	}
	if ((kbdbufidx<KBDBUFSZ)&&(applbuffer==NULL)){
		//we have room in keyboard buffer but there is no appl buffer  
		//keep copying to keyboard buffer in case a reader shows up until
		// its full.
		kbdbuffer[kbdbufidx++]=kbtoa(c);
		return kbtoa(c);
	}

	//check if we should return to sysreader
	if ( ((kbdbufidx>0)&&(state&INCTL)&&(c==EOF_endbyte))||
			( ascii=='\n') ||(applbufidx+kbdbufidx+1 >= applbufsize)){
				//zerobased kbdbufidx +1,   room for null +1
		int returnvalue=0;
		//kbdbuffer[kbdbufidx]='\0';
		if ((state&INCTL)&&(c==EOF_endbyte)){
			//set state for subsequent read requests
			DBGMSG(" CTL state = %d\n",(state&INCTL) );
			DBGMSG(" EOF char: %x %c\n", EOF_endbyte, kbtoa(EOF_endbyte));
			DBGMSG(" c   char: %x %c\n", c, kbtoa(c)	);
			mypause(5);
			eof_received=1;
			applbuffer[applbufidx+kbdbufidx+1]='\0';
			returnvalue= RECEIVED_EOF;
		}
		//copy what we have into application buffer so we can return it
		int i;
		for (i=0;i<(kbdbufidx);i++){
			applbuffer[applbufidx]=kbdbuffer[i];
			applbufidx++;
		}
		kbdbufidx=0;
		if (applbuffer!=NULL) applbuffer[applbufidx]='\0';

		//TRACE
		if((reader)&&(reader->state==STATE_BLOCKED)){
			//TRACE
			notify(reader);
			applbuffer=NULL;
			applbufsize=0;
			applbufidx=0;
			reader=NULL;
			return returnvalue;
		}//else no waiting reader process
	}

	//lastbyte=c;
	if ((ascii==NOCHAR)||(c&KEY_UP)||(is_control_code(c)))
		return 0;
	//okay to store key as long as we dont exceed buffer size
	if (kbdbufidx < KBDBUFSZ)  
		kbdbuffer[kbdbufidx++]=kbtoa(c);
	//only echo things we are going to send back, might confuse
	//user if we echo things that we drop as well??
	if ((device==KEYBOARD_ECHO)&&(applbuffer!=NULL))
		kprintf("%c",kbdbuffer[kbdbufidx-1]);
		//stop echoing if we dont have an applbuffer to copy to

	int i;
	//full keyboard buffer and we have an appl buffer to copy to
	if ((kbdbufidx>=(KBDBUFSZ))&&(applbuffer!=NULL)){
		DBGMSG("\nkbdbufidx=%d\n",kbdbufidx);
		for (i=0;i<KBDBUFSZ;i++){
			applbuffer[applbufidx]=kbdbuffer[i];
			DBGMSG("copy: i=%d,applbufidx=%d, c=%c \n",i,applbufidx,applbuffer[applbufidx]);
			applbufidx++;
			//if kbd sends newline or if appl buffer full return to reader
			if ((applbufidx>=(applbufsize-1))||
					(kbdbuffer[i]=='\n')){
				DBGMSG("applbufidx = %d, applbufsize = %d\n",
						applbufidx, applbufsize);
				DBGMSG("i= %d, kdbbuffer[i] = %x\n",i,kbdbuffer[i])	;
				// stop filling appl buffer, return it to reader

				//the next kbdbuffer index is unprocessed
				//shift them all to the left and adjust size
				//to preserve untransfered bytes but allow more bytes
				//to be appended (if room)
				int elements_to_shift=(KBDBUFSZ-1)-(i);

				int distance_to_shift=KBDBUFSZ-elements_to_shift;
				if (elements_to_shift<=0){
					kbdbufidx=0;
					//we emptied kbdbuffer, start over
				}else{
					int j;
					//TRACE
					print_kbd_buf();
					for (j=0;j<elements_to_shift;j++){
						kbdbuffer[j]=kbdbuffer[j+distance_to_shift];
					}
					kbdbufidx=KBDBUFSZ-elements_to_shift;
					DBGMSG("reset keyboard buffer size to %d\n", kbdbufidx);
					print_kbd_buf();
					if((reader)&&(reader->state==STATE_BLOCKED)){
						//TRACE						
						notify(reader);
						applbuffer=NULL;
						applbufsize=0;
						applbufidx=0;
						reader=NULL;
					}
					return applbufidx;
				}//else elemto shift>0
			}//applbufidx>bufsize-1
		}//for i -> kbdbufsz
		kbdbufidx=0;
	}//if kbd buffer full and we have applbuffer to copy to
	return c;
}

