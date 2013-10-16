#include <utility.h>
#include <xeroskernel.h>
#include <stdarg.h>
#include <sleep.h>
#include <xeroslib.h>

//libxc
extern char* strcat(register char *s1, register char *s2);

extern pcb     proctab[MAX_PROC];
unsigned int signal_priority_mask;

int validsignal(int sig_no, int pid){
	if ((pid<MINSIGNAL)||(pid>MAXSIGNAL))
		return INVALID_SIG;
	pcb* target_p = &proctab[pid%MAX_PROC];
	TRACE
	if ( !((1<<sig_no)&(target_p->sig_allow)) ){
		TRACE
			//process does not have a handler for this signal
		return SIGNAL_INVALID;
	}

	return 1;
	
}



//user space control of signal processing
// calls the appropriate signal handler

void sigtramp(void(*handler)(void*), void *cntx, void *osp){
	TRACE
	// call correct signal handler
	handler(cntx);
	TRACE
	sigreturn(osp);
	return; // we should never get here
}

// verify which bit are active
void printbits(unsigned int num){
	#ifdef DEBUG
        char b[32] ;
        b[0] = (char)'\0';

        int i = 1;
        i = i<<31;
        int j;
        for (j = 31; j>=0; j--)
        {   
                int z = 1;
                z=z<<j;
                strcat(b, ((num&z) == z) ? "1" : "0");
        }    
        kprintf("10987654321098765432109876543210\n");
        kprintf("%s\n",b);
	#endif
}

//input : priority mask with all leading 1 and trailing zeros
//output : the next signal number that should be dealt with
int pr_mask_to_signum(unsigned int mask){
	if (mask==0){
		return MAXSIGNAL;
	}
	if (mask==-1){
		return MINSIGNAL;
	}
	int sig;
	int i = 0;
	for (i=0;i<MAXSIGNAL;i++){
		if (mask&(1<<i)){
			sig=i;
			break;
		}
	}
	
	return (sig-1);
}


//create mask that accepts only signals of higher priority than the one
//supplied.   31 is highest, 0 lowest 
unsigned int get_priority_mask(int signalnumber){
	if (signalnumber==MAXSIGNAL)
		return 0; // nothing can signal higher
	int priority_mask = -1<<(signalnumber+1);
	TRACE
	printbits(priority_mask);
	return priority_mask;
}


//this is a kernel function called when signal sig_no 
//is to be registered for delivery to pid
//  we check for valid input, and set priority_mask if all ok
int signal (int pid, int sig_no){
	
	if (!validpid(pid)){
		TRACE
		return INVALID_PID;
	}
	if (validsignal(sig_no,pid)<0){
		TRACE
		return INVALID_SIGNAL;
	}
	pcb* target_p = &(proctab[(pid)%MAX_PROC]);
	target_p->sig_pending = (target_p->sig_pending)|(1<<sig_no);
	if (target_p->state==STATE_BLOCKED){
		ready(target_p);
		if (target_p->ret==SYS_SIGWAIT){
			//target pid called sigwait, return signal that unblocks him
			target_p->ret=sig_no;
		}else{
			target_p->ret=EINTR;
		}
	}else if(target_p->state==STATE_SLEEP){
		ready(target_p);
		target_p->ret=target_p->sleepticks;
	}
	TRACE
	return SIGHDLR_OK;
}

// turn off bit mask for given signal as indicated if we are sent a null signal handler
int unsignal (int pid, int sig_no){
	if (!validpid(pid)){
		return INVALID_PID;
	}
	if (!validsignal(sig_no,pid)){
		return INVALID_SIGNAL;
	}
	
	// update allowable signals mask to disallow this signal
	proctab[pid%MAX_PROC].sig_allow = proctab[pid%MAX_PROC].sig_allow|(0<<sig_no);
	#ifdef DEBUG
		kprintf("SIGALLOW BITS\n");
		printbits(proctab[pid%MAX_PROC].sig_allow );
		
	#endif
	
	return SIGHDLR_OK;
}


// precondition : process and signum have valid arguments to invoke 
// a signal handler from the process pcb
// this takes the stack pointed to by pcb->esp and places a frame on that
// will invoke sigtramp, which in turn invokes handler
void modify_process_stack(pcb* p,int signum){
	TRACE
	unsigned int stackptr = p->esp;
	//allocate space on process stack for sigtramp
	unsigned int newstackptr = stackptr-sizeof(sigtramp_setup_frame);
	sigtramp_setup_frame* ssf = (sigtramp_setup_frame*)newstackptr;
	memset(ssf, 0x81, sizeof( sigtramp_setup_frame ));
	
	ssf->esp = (int)(ssf + 1);
	ssf->ebp = p->esp;
	
	ssf->eip = (unsigned int) sigtramp;
	ssf->cs = getCS();
	ssf->flags = INTR_ENA_EFLAGS;
	ssf->ret_addr = ((context_frame*)p->esp)->ebp;  // value is irrelevant but just in case, use ebp from existing context
	ssf->handler = (unsigned int) p->sig_handler[signum];
	ssf->context_ptr = p->esp;
	DBGMSG("saving old esp %x\n", p->esp);
	ssf->old_sp = p->esp;
	DBGMSG("saving sig_priority = %x\n", p->sig_priority);
	ssf->saved_priority_mask = p->sig_priority; // save it before we overwrite it 
	DBGMSG("saving ret value %d\n", p->ret);
	ssf->saved_retval = p->ret;		// save it in case we overwrite 
	//mypause(9);
	//repoint esp to top of new sigtramp frame
	p->esp=newstackptr;
	
}
