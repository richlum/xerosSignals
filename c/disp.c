/* disp.c : dispatcher
 */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <stdarg.h>
#include <utility.h>
#include <sleep.h>
#include <di_calls.h>
#include <kbd.h>


extern void end_of_intr(void);

// head and tail of the ready_q
static pcb      *head = NULL;
static pcb      *tail = NULL;
// keep track currently running process, blocked processes and dead processes 
pcb 	*running=NULL;
pcb	*blocked_q=NULL;
pcb	*dead_q=NULL;
// keep reference to idleproc to put it in running state when required
extern unsigned int idleproc_pid;

/**
 * If a process dies, this method finds processes that were waiting for 
 * the newly dead target.  Appropriate error message supplied and process
 * formerly blocked process is put back on ready q
 * 
 * pid is the pid of the newly dead process that might have other
 * processes waiting on it
 * */
void notify_waiters(unsigned int pid){
	TRACE
	pcb* q = blocked_q;
	pcb* prev = blocked_q;
	pcb* dest;
	
	unsigned int destpid;
	while (q){
		
		if (q->ret==SYS_SEND){
			va_list ap = (va_list)q->args;
			destpid = va_arg(ap, unsigned int);
			
		}else if (q->ret==SYS_RECV){
			va_list ap = (va_list)q->args;
			unsigned int* from_pid = (unsigned int*)va_arg(ap, unsigned int);
			destpid = *from_pid;
		}
		
		if (destpid == pid){
			DBGMSG("found pid %d waiting on our dying pid %d\n",q->pid, pid);
			dest = &proctab[pid%MAX_PROC];
			prev->next = q->next;
			q->ret=BAD_PID;
			ready(q);
		}
		prev=q;
		q=q->next;
	}
}

// convience function mostly for debugging output
// given a state, return the head of the q that holds pcb with that state
pcb** get_q(int state){
	if (state==STATE_READY){
		return &head;
	}else if (state==STATE_RUNNING){
		return &running;
	}else if ((state==STATE_DEAD)||(state==STATE_STOPPED)){
		return &dead_q;
	}else if (state==STATE_BLOCKED){
		return &blocked_q;
	}
	return NULL;
}
// for debugging
// show the current q contents
void print_q(pcb* q){
	#ifdef DEBUG
/********************************/
	pcb* p;
	p=q;
	kprintf("\n%dq - ",q);
	while(p){
		kprintf("%d:%d, ",p->pid, p->state);
		p=p->next;
	}
	kprintf("\n");
	#endif
}

//given head of q, search q for pid and delete its pcb from q
// return the pid number on success, -1 on failure to find
int remove(pcb** q, unsigned int pid){
	pcb* queue=*q;
	pcb** prev;
	prev = q;
	if (!queue)
		return -1;  // q is empty 
	while (queue){
		if (queue->pid == pid){
			//found our target
			*prev=queue->next;
			queue->next=NULL;
			return (int) pid;
		}
		*prev=queue;
		queue=queue->next;
	}
	return -1; //not found
}


int get_signaller_signum(pcb* p){
	va_list     ap;
	ap = (va_list)p->args;
	unsigned int new_pid = va_arg(ap, unsigned int);
	int new_signum = va_arg(ap,  int);
	DBGMSG("newpid = %d, signum = %d\n", new_pid, new_signum);
	return new_signum;
}


int get_signaller_targetpid(pcb* p){
	va_list     ap;
	ap = (va_list)p->args;
	unsigned int new_pid = va_arg(ap, unsigned int);
	int new_signum = va_arg(ap,  int);
	DBGMSG("newpid = %d, signum = %d\n", new_pid, new_signum);
	return new_pid;
}



// given a q of pcb, append pcb to the tail
// return the number of pcb in the q after the append operation
// precondition is that all links from other q to this have already been removed
int append(pcb** q, pcb* p){
/********************************/
	int count =0;
	
	//print_q(*q);
	
	if (p->next!=NULL){
		kprintf("WARNING,pcb %d next was not null: resetting to null", p->pid);
		p->next=NULL;
	}
	if (*q==NULL){
		count++;
		*q=p;
	}else{
		//get tail of q and append to it
		pcb* prev = *q;
		pcb* next = *q;
		while(next){
			if (next==p){
				DBGMSG("WARNING: trying to add to queue an existing member: ignoring\n",1);
				return count;
			}
			prev=next;
			next=next->next;
			count++;
		}
		//append our new value
		prev->next=p;
		count++;
	}
	//#ifdef DEBUG
	//print_q(*q);
	//#endif
	return count;
}

//pop the head of the given queue and return the popped pcb
pcb* deque(pcb** q){
/********************************/

	pcb* result;
	result = *q;  //return head of the q
	if(*q) {
		*q=(*q)->next;
	}else{
		return NULL;
	}
	result->next=NULL;
	return result;	
}

//input signal_pending mask, output the signal number
// of the highest priority bit
//  if no pending signals return -1
int get_next_signal(unsigned int pending_signals){
	int i;
	for (i=MAXSIGNAL; i>=0;i--){
		if ((pending_signals)&(1<<i)){
			return i;
		}
	}
	//no pending signals, return -1
	return -1;
}

//utility to show buffer contents
void printbuffer(void* buff, int len){
	char localbuff[MAXBUFFSIZE];
	blkcopy(buff, localbuff, (unsigned int) len);
	localbuff[len]='\0';
	kprintf("%s\n", localbuff);
}
	

//check for any pending signals, if exist
//  note if two diff process signal the same signal, only one registers
//put sigtramp on stack to process signal
void deliver_signals_ifany(pcb* p){

	
	if ((p->sig_pending > 0)&&
		(p->sig_pending&p->sig_allow&p->sig_priority)){
		TRACE

		int signum = get_next_signal(p->sig_pending);  
		int priority_mask = get_priority_mask(signum);
		modify_process_stack(p,signum);
		// update which signals are allowed to interrupt - 
		// block lower priority and ourselves from stacking ontop 
		DBGMSG("sig priorty mask before : %x\n",  proctab[(p->pid)%MAX_PROC].sig_priority );
		proctab[(p->pid)%MAX_PROC].sig_priority = priority_mask;
		#ifdef DEBUG
			kprintf("SIG_PRIORITY BITS\n");
			printbits(proctab[(p->pid)%MAX_PROC].sig_priority );
			DBGMSG ("we have pending signal mask : 0x%08x\n",p->sig_pending);
			DBGMSG(" highest priority signalnumber is %d\n", signum	);
		#endif
		
	}
	
	
}

//dispatcher policy engine for deciding who gets to run next
void     dispatch( void ) {
/********************************/

    pcb         *p;
    int          r, rc, fd;
    funcptr     fp;
	void 	(**old_fp)(void);
    int         stack;
    va_list     ap;
    // messaging parameters
    unsigned int	dest_pid; 
    unsigned int	*from_pid = NULL;
    void*		dbuffer;
    int			dbuffer_len;
    void*		sbuffer;
    int			sbuffer_len;
    int 		count;
    void* 		buff;
    int			bufflen;
    

    for( p = next(); p; ) {
		
		p->state=STATE_RUNNING;
		running=p;
		deliver_signals_ifany(p);
		
      r = contextswitch( p );
      switch( r ) {
      case( SYS_CREATE ):
        ap = (va_list)p->args;
        fp = (funcptr)(va_arg( ap, int ) );
        stack = va_arg( ap, int );
        // create adds process to tail of readyq
        p->ret = create( fp, stack );
        break;
      
      case( SYS_YIELD ):
        ready( p );
        p = next();
        p->state=STATE_RUNNING;
        running=p;
        break;
      
      case( SYS_STOP ):
        p->state = STATE_STOPPED;
        p->next=NULL;
        
        count = append(&dead_q, p);
        DBGMSG("SYS_STOP on pid %d, total dead %d\n",p->pid, count);
        notify_waiters(p->pid);
        
        p = next();
        p->state=STATE_RUNNING;
        running=p;
        break;
      
      case (SYS_GETPID):
        p->ret = p->pid;
        break;
      
      case (SYS_PUTS):
        kprintf("%s\n",*((char**)(p->args)));
        break;
      
      case (SYS_SEND):
		ap = (va_list)p->args;
		dest_pid = va_arg(ap, unsigned int);
		sbuffer = (void*)va_arg(ap,int);
		sbuffer_len = va_arg(ap, int);
		pcb* dest = &proctab[dest_pid%MAX_PROC];
		
		int sent = 0;
		sent = send(dest_pid, sbuffer, sbuffer_len, p);
		
		if (sent>0){
			// sent msg (or part of), 
			p->ret = sent;
			dest->ret = sent;
			ready(p);
			//we have pulled dest from proctable directly, we need to 
			//remove it from q (probably blocked_q)
			DBGMSG("Send found dest in state %d \n",dest->state);
			pcb** aQueueRef = get_q(dest->state);
			if(remove(aQueueRef, dest->pid)){
				DBGMSG("removed %d, state %d, from %x\n", dest->pid, dest->state, *aQueueRef);
			}else{
				DBGMSG("Warning, failed to find pid %d in q for state %d.  Placing on readyq anyway\n",
					dest->pid, dest->state);
			}
			ready(dest);
		} else if (sent <0) {
			// error condition in p->ret, put p back into ready_q to process error
			p->ret = sent;
			ready(p);
		} else {
			// receiver is not ready to receive yet, block sender, recvr will copy msg
			// allow other processes to run
			p->state=STATE_BLOCKED;
			p->ret=SYS_SEND;
			append(&blocked_q,p);
		}
		p = next();
		p->state = STATE_RUNNING;
		running=p;
		break;
		
	  case (SYS_RECV):
		ap = (va_list)p->args;
		from_pid = (unsigned int*)va_arg(ap, unsigned int);
		dbuffer = (void*)va_arg(ap,int);
		dbuffer_len = va_arg(ap, int);
		
		
		int recvd = 0;
		recvd = recv(from_pid, dbuffer, dbuffer_len, p);
		pcb* sndr = &(proctab[(*from_pid)%MAX_PROC]);
		if (recvd>0){
			// received msg (or part of)
			p->ret = recvd;
			sndr->ret = recvd;
			ready(p);
		// from_pid might be zero - universal recv from any
			if (sndr->pid!=0) {
				//we pulled this directly from proc table. clean up links from q
				DBGMSG("Recv found src in state %d \n",sndr->state);
				pcb** aQueueRef = get_q(sndr->state);
				if(remove(aQueueRef, sndr->pid)){
					DBGMSG("removed %d, state %d, from %x\n", sndr->pid, sndr->state, *aQueueRef);
				}else{
					DBGMSG("Warning, failed to find pid %d in q for state %d.  Placing on readyq anyway\n",
						sndr->pid, sndr->state);
						
				}
				
				ready(sndr);
			}			
		}else if (recvd<0){
			//error condition in p->ret, place back into ready_q to process error
			p->ret=recvd;
			ready(p);
		} else {
			// sender is not ready to send yet, block receiver, sender will copy msg
			// allow other processes to run
			p->state=STATE_BLOCKED;
			p->ret=SYS_RECV;
			append(&blocked_q,p);
		}
		p=next();
		p->state = STATE_RUNNING;
		running=p;
		break;
      
      case SYS_TIMER:
		ready( p );
        p = next();
        p->state=STATE_RUNNING;
        running=p;
        tick();
        end_of_intr();
		break;
	
	  case SYS_SLEEP:
		ap = (va_list)p->args;
		unsigned int ms = va_arg(ap, unsigned int);
		DBGMSG("pid %d going to sleep for %d ms\n", p->pid, ms);
		kassert(p->state==STATE_RUNNING);
		kassert(p->next==NULL);
		sleep(ms,p);
		p = next();
        p->state=STATE_RUNNING;
        running=p;
		break;
		
	case SYS_SIGHANDLER : 	// 9
		TRACE
		ap = (va_list)p->args;
		int signum = va_arg(ap, unsigned int);
		DBGMSG("installing sighandler %d \n", signum);
		fp = (funcptr)(va_arg( ap, funcptr ) );
        old_fp = (oldfuncptr)(va_arg( ap, oldfuncptr ));

		if ((signum<0)||(signum>MAXSIGNAL)){
			TRACE
			p->ret = INVALID_SIGNAL;
		}else{
			TRACE
			void (*prev)(void);
			// maybe NULL if no handler prev assiged
			prev = p->sig_handler[signum];
			*old_fp=prev;
			p->sig_handler[signum]=fp;
			if (fp==NULL){
				//clear the bit for this signal
				TRACE
				p->sig_allow = p->sig_allow & ~(1<<signum);
			}else{
				//set the bit for this signal
				TRACE
				p->sig_allow = p->sig_allow | (1<<signum);
			}
			printbits(p->sig_allow);
			p->ret = SIGHDLR_OK;
		}
		TRACE
		break;
	
	case SYS_SIGRETURN	:	// 10
		TRACE
		ap = (va_list)p->args;
		// what signal did we just process
		int completed_signo = pr_mask_to_signum(p->sig_priority);
		unsigned int old_stackptr = va_arg(ap, unsigned int);
		unsigned int saved_priority_mask = *((unsigned int*)(old_stackptr -8));
		unsigned int saved_return = *((unsigned int*)(old_stackptr-4));
		DBGMSG("saved old stadkptr = 0x%x\n", old_stackptr);
		DBGMSG("saved priority mask = 0x%x\n", saved_priority_mask);
		DBGMSG("saved return = %d\n", saved_return);

		p->sig_priority=saved_priority_mask;
		DBGMSG ("p->ret before restoring = %d\n",p->ret);
		p->ret=saved_return;
		//todo: is this only if process was STATE_BLOCKED or for all signalled processes
		//if ((p->state==STATE_BLOCKED)&&(p->sig_pending==0))

		p->esp = old_stackptr;
		//remove this completed signal from sig_pending
		p->sig_pending = p->sig_pending & ~(1<<completed_signo);
		if (p->sig_pending==0){
			if (p->sigwaiter == 1){
				// for sigwait, return signal number
				p->ret=completed_signo;
				p->sigwaiter=0;
			}else{
				// any other process intr by signal set to eintr
				DBGMSG("current ret code = %d\n", p->ret);
				p->ret = EINTR;
				DBGMSG("pid %d\n",p->pid);
				DBGMSG("reset ret code to %d\n", p->ret);
			}
		}
		
		break;
	
	case SYS_SIGKILL	:	// 11
		TRACE
		ap = (va_list)p->args;
		int pid = va_arg(ap, unsigned int);
		int signalnumber = va_arg(ap, unsigned int);
		DBGMSG("target pid %d, signal %d \n", pid, signalnumber);
		rc = signal(pid, signalnumber);
		DBGMSG("signal returned %d\n",rc);
		p->ret = rc;
		
		break;
	
	case  SYS_SIGWAIT	:	// 12	
		TRACE

		if (p->sig_allow==0){
			// we aren't accepting any signals, no handlers
			// we will never unblock
			p->ret = SIGNAL_INVALID;
		}else{
			p->sigwaiter=1;
			p->state=STATE_BLOCKED;
			p->next=NULL;
			p->ret=SYS_SIGWAIT;
			append(&blocked_q,p);
			// return signal number that unblocks this process
			p = next();
		    p->state=STATE_RUNNING;
		    running=p;
		}
		break;		
	case  SYS_OPEN  : //	13
		ap = (va_list)p->args;
		int device_no = va_arg(ap,int);
		TRACE
		DBGMSG("sys_open, device_no = %d\n",device_no);
		rc = di_open(p, device_no);
		p->ret=rc;
		break;
	case SYS_CLOSE	: //	14
		ap = (va_list)p->args;
		fd = va_arg(ap,int);
		TRACE
		DBGMSG("sys_close, fd = %d\n",fd);
		rc = di_close(p, fd);
		p->ret=rc;
		break;
	case SYS_WRITE	: //	15
		ap = (va_list)p->args;
		fd = va_arg(ap,int);
		buff = (void*)(va_arg(ap, unsigned int));
		bufflen = va_arg(ap, int);
		TRACE
		DBGMSG("sys write, fd = %d, bufflen = %d\n", fd, bufflen);
		printbuffer(buff, bufflen);
		rc = di_write(p, fd,buff,bufflen);
		p->ret=rc;
		break;
	case SYS_READ	: //	16	
		ap = (va_list)p->args;
		fd = va_arg(ap,int);
		buff = (void*)(va_arg(ap, unsigned int));
		bufflen = va_arg(ap, int);
		DBGMSG("sys read, fd = %d, bufflen = %d buffaddr = %x\n", fd, bufflen,buff);
		TRACE
		rc = di_read(p, fd,buff,bufflen);
		if (rc==EOF){
			TRACE
			DBGMSG("dispatcher read returning %d\n",0);
			//cant register for read, EOF encountered
			p->ret=0;
		}else if (rc==REGISTERED_KBDREAD){
			TRACE
			//buffer not full, block and wait for
			// notify to put us back into readyq
			p->state=STATE_BLOCKED;
			p->next=NULL;
			p->ret=SYS_READ;
			append(&blocked_q,p);

			p = next();
			p->state=STATE_RUNNING;
			running=p;
		}else if (rc<0){
			//something is wrong, cant register read, pass
			// along error code
			TRACE
			DBGMSG("dispatcher read returning %d\n",rc);
			p->ret=rc;
		}else{
			TRACE
			DBGMSG("dispatcher read returning %d\n",-1);
			//something is wrong, cant register read
			p->ret=-1;
		}
		break;
	case SYS_IOCTL	: //	17		
		ap = (va_list)p->args;
		int fd = va_arg(ap,int);
		long command = (va_arg(ap, long));
		//todo not sure if this is portable. passing along varargs
		//va_list sap = (va_list)va_arg(ap,unsigned int);
		int a = va_arg(ap,int);
		int b = va_arg(ap,int);
		TRACE
		DBGMSG("sys ioctl, fd = %d, command = %d\n", fd, command);
		DBGMSG("command args a = %d, b= %d\n", a, b);
		rc = di_ioctl(p, fd, command, a, b);
		p->ret = rc;
		// can we pass on varargs
		// sysioctl(command,ap);
		// this will work but reciver has to recover ptr to ap first
		// then recover args
		// eg 
		// va_start(ap,command);
		// sap = (va_list)va_arg(ap,unsigned int);
		// int arg  = va_arg(sap,int);
		break;

	case SYS_KEYBOARD:
		//received a keyboard interrupt
		if (kbd_data_ready()){
			//get_kbd_data();
			process_key();
		}
		end_of_intr();
		break;
			
     default:
        kprintf( "Bad Sys request %d, pid = %d\n", r, p->pid );
        break;
      }
    }

    kprintf( "Out of processes: dying\n" );
    
    for( ;; );
}


// initilize process table to all zeros
// includes null value for signal mask
// initialize proctab devtab to point to device table
extern void dispatchinit( void ) {
/********************************/
  // initial default priority mask = accept all priorities
  signal_priority_mask = -1;
  memset(proctab, 0, sizeof( pcb ) * MAX_PROC);
  //initialize all process filedescriptors to point at base of device table
  int i,j;
  for (i=0;i<MAX_PROC;i++){
	  for(j=0;j<MAXFILES;j++){
		  proctab[i].fd[j].dev_tab=devtab;
	  }
  }
}



// add the process to the tail of the read_q
// precondition: any ties to other queues are severed before entry to this q
// will not put idleproc on the readyq
extern void     ready( pcb *p ) {
/*******************************/
	
	unsigned int pid = p->pid;
	kassert(pid == proctab[pid%MAX_PROC].pid);
    p->next = NULL;
    p->state = STATE_READY;
    // we should not be using zeroth element in the array
	kassert(p != proctab);  
	kassert(pid == ((proctab[pid%MAX_PROC]).pid));
	// if its idle proc dont put it in readyq 
	if (p->pid == idleproc_pid) return;
    if( tail ) {
        tail->next = p;
    } else {
        head = p;
    }
    tail = p;
    
}

//extern unsigned int root_pid;

// remove the head of the ready_q and return it
// this is where the decision of when to put idleproc into running state is made
// if no more process available to run, return idleproc
// if idleproc gets put onto readyq, remove it
// if idleproc is running and there is an available process in ready state,
//		stop running idleproc and return new available process
extern pcb  *next( void ) {
/*******************************/
    pcb *p;
	p = head;

    if( p ) {
		unsigned int pid = p->pid;
		kassert(pid == proctab[pid%MAX_PROC].pid);
		// we should be drawing only from READY Q
		kassert(p->state==STATE_READY);
        head = p->next;
        p->next=NULL;
        if( !head ) {
            tail = NULL;
        }
	}

	//no more processes in ready q and nothing is running
//	if ((!p)&&((running==NULL)||(running->pid==idleproc_pid))){
	if (!p){
		p=&proctab[idleproc_pid%MAX_PROC];
		return p;
	}else if ((p->pid == idleproc_pid)&&(head)) {
		// idleproc in readyq - remove idleproc if anything else is ready
		DBGMSG("removing idleproc (%d) from readyq\n", idleproc_pid);
		pcb* idlepcb = p;
		p=head;
		DBGMSG("new running process (%d) from readyq\n", p->pid);
		head=head->next;
		if( !head ) {
            tail = NULL;
        }
		p->next=NULL;
		idlepcb->next=NULL;
	} else if ((running->pid==idleproc_pid)&&((p)&&(p->pid!=idleproc_pid))){
		// we've already retrieved a valid process into p in first part of block
		DBGMSG("removing idleproc (%d) from running, other process available\n", idleproc_pid);
		DBGMSG("process p = %d\n",p->pid);
		DBGMSG("running pid = %d\n", running->pid);
		running->state=STATE_READY;
	}
	
	return( p );
}
