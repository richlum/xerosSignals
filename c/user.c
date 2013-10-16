/* user.c : User processes
 */

#include <xeroskernel.h>
#include <utility.h>
#include <xeroslib.h>
#include <sleep.h>

/* Your code goes here */

/*************************************************/
// top portion of this file is the common functions for test
// and assign 3 test deliverable cases
//
// middle portion is test cases when make test is issued
//
//
// bottom portion is the assignment 3 test solution
// that compiles with either debug or no option 
/*************************************************/

/****************Some common function for both halves *********************************/
#define buffsize 128
#define PAUSETIME 8
#define APPBUFSIZE 128

unsigned int idle_child_pid = 0;
unsigned int waiter_pid=0;
unsigned int step12_child_pid=0;

unsigned int root_pid =0;

// some general handlers that echo their names for general testing
void handler1(void* ctxt) {
	char msg[buffsize];
	sprintf(msg,"handler1 has been called, %x\n  ",(unsigned int)handler1);
	sysputs(msg);
}
void handler2(void* ctxt) {
	char msg[buffsize];
	sprintf(msg,"func address %x, handler2 has been called ", (unsigned int)handler2);
	sysputs(msg);
}

void handler3(void* ctxt) {
	char msg[buffsize];
	sprintf(msg,"func address %x, handler3 has been called ", (unsigned int)handler3);
	sysputs(msg);
}

void handler5(void* ctxt) {
	char msg[buffsize];
	sprintf(msg,"func address %x, handler5 has been called ", (unsigned int)handler5);
	sysputs(msg);
}
void handler7(void* ctxt) {
	char msg[buffsize];
	sprintf(msg,"func address %x, handler7 has been called ", (unsigned int)handler7);
	sysputs(msg);
}

// for step11 of assignment 3
void handler18(void* ctxt) {
	char msg[buffsize];
	sprintf(msg,"func address %x, handler18 has been called ", (unsigned int)handler18);
	sysputs(msg);
}

// for step 12 of assignment 3
void sleep_sig20_sig18(void){
	char msg[buffsize];
	sprintf(msg,"sleep_sig20_sig18 alive, sleeping 1 pid = %d\n", sysgetpid());
	sysputs(msg);
	syssleep(3000);
	int sigresult=sigkill ( root_pid, 20);//pid, signal number
	sprintf(msg,"child sleep_sig20_sig18 sent syskill(root_pid, 20) (nonexistant handler) rc = %d\n", sigresult);
	sysputs(msg);

	sigresult=sigkill ( root_pid, 18);//pid, signal number
	sprintf(msg,"child sleep_sig20_sig18 sent syskill(root_pid, 18)  rc = %d\n", sigresult);
	sysputs(msg);

}

//for step 15 of assignment 3
void sleep5_sig18(void){
	char msg[buffsize];
	syssleep(5000);
	int sigresult=sigkill ( root_pid, 18);
	sprintf(msg,"child sleep5_sig18 sends syskill(root_pid, 18) rc = %d\n", sigresult);
	sysputs(msg);

}

//for step 20 of assignment 3
void sleep5_sig20(void ){
	char msg[buffsize];
	syssleep(5000);
	int sigresult=sigkill ( root_pid, 20);
	sprintf(msg,"child sleep5_sig20 sends syskill(root_pid, 20) rc = %d\n", sigresult);
	sysputs(msg);

}

// create a child process to invoke a signal against for testing
void idlechild(void) {
	idle_child_pid = sysgetpid();
	char msg[buffsize];
	// install signal 5, function handler5

	void (*oldhandler)(void* );
	int result = syssighandler(7, handler7, &oldhandler);
	sprintf(msg,"idle child installed signalhandler1, rc = %d\n",result);
	sysputs(msg);

	result = syssighandler(5, handler5, &oldhandler);

	sprintf(msg,"idle child installed signalhandler5, rc = %d\n",result);
	sysputs(msg);

	result = syssighandler(3, handler3, &oldhandler);
	sprintf(msg,"idle child installed signalhandler3, rc = %d\n",result);
	sysputs(msg);

	kprintf("idle child  pid = %d\n", idle_child_pid);
	while(1) {
		sysyield();
	}
}

// child process that syssigwaits until signal is sent
void waiter(void){
	waiter_pid = sysgetpid();
	char msg[buffsize];

	sprintf(msg,"waiter pid = %d\n",waiter_pid);
	sysputs(msg);
	sprintf(msg,"installing sighandler %d\n",7);
	sysputs(msg);

	void (*oldhandler)(void* );
	int result = syssighandler(7, handler7, &oldhandler);
	sprintf(msg,"waiter installed sighandler %d",7);
	sysputs(msg);

	sprintf(msg,"waiter calling syssigwait",1);
	sysputs(msg);

	result = syssigwait();
	sprintf(msg,"waiter returned from syssigwait by signal %d",result);
	sysputs(msg);
	mypause(PAUSETIME);

}

//this portion is only compiled if 'make test' is executed
// this contains the non core tests , the other section contains
// the core assignment 6 test routine, just make or make beros

#ifdef TESTS

// method to exercise all the debug tracepoints in dispatcher
// for devices to verify argument passing is correct
void verify_dev_arg_passing(void){
	char msg[buffsize];
	//verify all device calls correctly pass parameters
	//	/****************************/
		sprintf(msg,"### call dii functions before assignment",1);
		sysputs(msg);

		int rc = sysopen(KEYBOARD_NOECHO);
		sprintf(msg, "sysopen(KEYBOARD_NOECHO) returned: %d\n", rc);
		sysputs(msg);

		rc = sysopen(KEYBOARD_ECHO);
		sprintf(msg, "sysopen(KEYBOARD_ECHO) returned: %d\n", rc);
		sysputs(msg);

		rc = sysclose(KEYBOARD_NOECHO);
		sprintf(msg, "sysclose(KEYBOARD_NOECHO) returned: %d\n", rc);
		sysputs(msg);

		rc = sysclose(KEYBOARD_ECHO);
		sprintf(msg, "sysclose(KEYBOARD_ECHO) returned: %d\n", rc);
		sysputs(msg);

		char buff[buffsize];
		rc = sysread(KEYBOARD_NOECHO, buff, sizeof(buff));
		sprintf(msg, "sysread(KEYBOARD_NOECHO) returned: %d\n", rc);
		sysputs(msg);

		rc = sysread(KEYBOARD_ECHO,buff, sizeof(buff));
		sprintf(msg, "sysread(KEYBOARD_ECHO) returned: %d\n", rc);
		sysputs(msg);

		rc = syswrite(KEYBOARD_NOECHO,buff, sizeof(buff));
		sprintf(msg, "syswrite(KEYBOARD_NOECHO) returned: %d\n", rc);
		sysputs(msg);

		rc = syswrite(KEYBOARD_ECHO,buff, sizeof(buff));
		sprintf(msg, "syswrite(KEYBOARD_ECHO) returned: %d\n", rc);
		sysputs(msg);

		rc = sysioctl(KEYBOARD_NOECHO,999 , 33, 44);
		sprintf(msg, "sysioctl(KEYBOARD_NOECHO) returned: %d\n", rc);
		sysputs(msg);

		rc = sysioctl(KEYBOARD_ECHO, 999, 33, 44);
		sprintf(msg, "sysioctl(KEYBOARD_ECHO) returned: %d\n", rc);
		sysputs(msg);
}


void sigwaittest(void){
	//test that ssysigwait will block until signal sent
	char msg[buffsize];
	sprintf(msg, "### starting sigwaittest",1);
	sysputs(msg);


	waiter_pid = syscreate( &waiter, 4096 );
	syssleep(9000);
	int sigrc=sigkill ( waiter_pid, 7);//pid, signal number
	sprintf(msg,"sigkill waiter with signal 7 returned %d\n",sigrc);
	sysputs(msg);
	mypause(PAUSETIME);
}

void sysopen_invalid(void){
	char msg[buffsize];
	int fd;
	sprintf(msg, "### test sysopen with invalid arg",1);
	sysputs(msg);

	fd = sysopen(99);
	sprintf(msg, "sysopen( 99 ) returned: %d\n", fd);
	sysputs(msg);

}

void syswrite_invalid(void){
	int rc;
	char msg[buffsize];
	sprintf(msg, "### test syswrite with invalid arg",1);
	sysputs(msg);

	char buff[buffsize];
	rc = syswrite(KEYBOARD_ECHO, buff, buffsize);
	sprintf(msg, "syswrite(KEYBOARD_ECHO, buff, buffsize) returned %d\n",
			rc);
	sysputs(msg);

}


void sysioctl_invalid(void){
	int rc;
	char msg[buffsize];
	sprintf(msg, "### test sysioctl with invalid arg",1);
	sysputs(msg);

	rc = sysioctl(99, 2,3,4);
	sprintf(msg, "sysioctl(99, 2,3,4) returned: %d\n", rc);
	sysputs(msg);
	
}
int signaltargetpid=0;

void signaltarget(void){
	signaltargetpid = sysgetpid();
	char msg[buffsize];
	void (*oldhandler)(void* );

	int result = syssighandler(1, handler1, &oldhandler);
	sprintf(msg,"installed signalhandler1, rc = %d\n",result);
	sysputs(msg);

	result = syssighandler(3, handler3, &oldhandler);
	sprintf(msg,"installed signalhandler3, rc = %d\n",result);
	sysputs(msg);

	result = syssighandler(5, handler5, &oldhandler);
	sprintf(msg,"installed signalhandler5, rc = %d\n",result);
	sysputs(msg);

	result = syssighandler(7, handler7, &oldhandler);
	sprintf(msg,"installed signalhandler7, rc = %d\n",result);
	sysputs(msg);


	while(1) {
		sysyield();
	}

}

void syssigkilltest(void ){
	int rc,fd;
	char msg[buffsize];
	sprintf(msg, "### test syskill arg",1);
	sysputs(msg);
	signaltargetpid = syscreate( &signaltarget, 4096 );
	sprintf(msg, "signaltarget is launched, pid = %d", signaltargetpid);
	sysputs(msg);
	int i,sigrc;
	mypause(PAUSETIME);
	for (i=0;i<8;i++){
		sigrc=sigkill ( signaltargetpid, i);//pid, signal number
		sprintf(msg,"sigkill signal %d ",i);
		sysputs(msg);
		sprintf(msg, "result = %d", sigrc);
		sysputs(msg);
	}

	mypause(PAUSETIME);

}
// this test produces the output for snap.bufferrolling.txt
// when given a bufer that is just larger than keyboard buffer
//  eg size 6 = 5 payload bytes, 1 null term byte.
void sysread_overflow(void){
	int rc,fd;
	char msg[buffsize];
	char buff[buffsize];
	sprintf(msg, "### test sysread overflow ",1);
	sysputs(msg);

	sprintf(msg,"%d. Opening echo keyboard",5);
	sysputs(msg);
	int fd3 = sysopen(KEYBOARD_ECHO);
	sprintf(msg, "sysopen(KEYBOARD_ECHO) returned: %d\n", fd3);
	sysputs(msg);

	rc = sysread(fd3,buff, 6);
	sprintf(msg, "sysread(KEYBOARD_ECHO) returned: %d\n", rc);
	sysputs(msg);
	sysputs(buff);

	rc = sysread(fd3,buff, 6);
	sprintf(msg, "sysread(KEYBOARD_ECHO) returned: %d\n", rc);
	sysputs(msg);
	sysputs("$");
	sysputs(buff);

}



void sysread_underflow(void){
	int rc,fd;
	char msg[buffsize];
	char buff[buffsize];
	sprintf(msg, "### test sysread underflow ",1);
	sysputs(msg);

	sprintf(msg," Opening echo keyboard",5);
	sysputs(msg);
	int fd3 = sysopen(KEYBOARD_ECHO);
	sprintf(msg, "sysopen(KEYBOARD_ECHO) returned: %d\n", fd3);
	sysputs(msg);

	rc = sysread(fd3,buff, 3);
	sprintf(msg, "sysread(KEYBOARD_ECHO) returned: %d\n", rc);
	sysputs(msg);
	sysputs("$");
	sysputs(buff);

	rc = sysread(fd3,buff, 3);
	sprintf(msg, "sysread(KEYBOARD_ECHO) returned: %d\n", rc);
	sysputs(msg);
	sysputs("$");
	sysputs(buff);

	rc = sysread(fd3,buff, 3);
	sprintf(msg, "sysread(KEYBOARD_ECHO) returned: %d\n", rc);
	sysputs(msg);
	sysputs("$");
	sysputs(buff);


}




void root( void ) {
	/****************************/

//test syssighandler installs correctly and oldhandler is retreived
	char msg[buffsize];
	sprintf(msg, "### install handler1, then handler2 into signal1",1);

	int signal1 = 30;
	void (*oldhandler)(void* );
	int result = syssighandler(signal1, handler1, &oldhandler);
	sprintf(msg,"installed signalhandler1, rc = %d\n",result);
	sysputs(msg);
	result = syssighandler(signal1, handler2, &oldhandler);
	sprintf(msg,"installed signalhandler2, rc = %d\n",result);
	sysputs(msg);
	mypause(PAUSETIME);
	int context = 8;
	sprintf(msg,"### call retrieved oldhandler-handler1(%d)\n",context);
	sysputs(msg);
	oldhandler(&context);
	mypause(PAUSETIME);
//
	sprintf(msg, "### launch child process with sighandlers", 1  );
	sysputs(msg);
//
	idle_child_pid = syscreate( &idlechild, 4096 );
	sprintf(msg, "IDLECHILD is launched, pid = %d", idle_child_pid);
	sysputs(msg);
//
//	//note this pause is required to allow idlechild time to complete
//	//installation of handlers before signalling them. due to
//	// multitasking, without this pause, quite often signal handlers
//	// are not yet installed by the time we signal them.
	mypause(PAUSETIME);
//
	sprintf(msg, "### root sigkills non existant handler in child", 1 );
	sysputs(msg);
//
	sprintf(msg,"root issuing syskill idlechild signal %d", 1);
	sysputs(msg);
//
//	//idlechild installs handler for 3 5 7
//	//should fail immediately
	int sigresult=sigkill ( idle_child_pid, 1);//pid, signal number
	sprintf(msg,"syskill idlechild signal 1 (nonexistant handler) rc = %d\n", sigresult);
	sysputs(msg);
	//mypause(PAUSETIME);
//
	sprintf(msg, "### root sigkills 7 in child while signal 5 processing",1 );
	sysputs(msg);
//
	int sigresult5=sigkill ( idle_child_pid, 5);//pid, signal number
	int sigresult7=sigkill ( idle_child_pid, 7);//pid, signal number
//
	sprintf(msg,"syskill idlechild signal 5 rc = %d\n", sigresult5);
	sysputs(msg);
//
	sprintf(msg,"syskill idlechild signal 7 rc = %d\n", sigresult7);
	sysputs(msg);


	sigwaittest();
	sprintf(msg,"### sysopen syswrite sysioctl tests ()\n",1);
	sysputs(msg);
	mypause(PAUSETIME);
	sysopen_invalid();
	mypause(PAUSETIME);
	syswrite_invalid();
	mypause(PAUSETIME);
	sysioctl_invalid();
	mypause(PAUSETIME);
//	mypause(PAUSETIME);
	syssigkilltest();
	sysread_overflow();
	mypause(PAUSETIME);
	sysread_underflow();
	/********************************/
	//  test keyboard echo returns on ctld, return or full appl buffer

	//char msg[buffsize];

	int fd = sysopen(KEYBOARD_ECHO);
	sprintf(msg, "sysopen(KEYBOARD_ECHO) returned: %d\n", fd);
	sysputs(msg);
//
	sprintf(msg, "sysreading(KEYBOARD_ECHO) until eof or error: %d\n", fd);
	sysputs(msg);
	char buffer[APPBUFSIZE];
	int rc;
	while ((rc = sysread(fd,&buffer,sizeof(buffer)))>0){
		sysputs(buffer);
	}

	// not required but simplified trouble shooting by isolating out
	// idleprocess swapping from signals and device testing


	//console keyboard untl ctl d

	//scan code for 'w' in hex
	unsigned int newchar = 0x11;
	unsigned long replace_eof_cmd = 49;

	sprintf(msg,"  opening echo keyboard",7);
	sysputs(msg);
	fd = sysopen(KEYBOARD_ECHO);
	sprintf(msg, "sysopen(KEYBOARD_ECHO) returned: %d\n",fd);
	sysputs(msg);


	result = sysioctl(fd, replace_eof_cmd, newchar);

	//char buffer[buffsize];
	sprintf(msg,"%d. read chars until eof",9);
	sysputs(msg);
	rc=1;
	int total =0;
	while (rc!=0){
		rc= sysread(fd,&buffer,10);
		total += rc;
		sprintf(msg,"$%s\n",(char*)buffer);
		sysputs(msg);
	}
	sprintf(msg,"total chars read = %d", total);
	sysputs(msg);


	while(1) {
		sysyield();
	}

}

/**************idleproc is a user space function used by kernel*******************************/
void idleproc (void) {

	char abuffer[buffsize];
	sprintf(abuffer,"idleproc is running as pid %d\n", sysgetpid());
	sysputs(abuffer);
	//park idle proc in blocking queue - optional, can just leave it out of any queues
	//but this is a convient way to get it off the run state and out of the ready q
	sysrecv(0,abuffer, buffsize);

	//sysputs(abuffer);
	TRACE
	while(1) {
		asm
		( " \
		hlt \n\
        "
				:
				:
				: "%eax"
		);

	}
}

#else
/*************************************************/
// here is the main assignment 3 test routine
// to run
// make clean; make; make beros;

void assign3_test_seq(void ){
	//idle_child_pid = syscreate( &idlechild, 4096 );

	char msg[buffsize];
	root_pid = sysgetpid();

	sprintf(msg,"%d. SEASONS GREETING EARTHLING", 1);
	sysputs(msg);
	int fd,i,rc;
	char buffer[APPBUFSIZE];
	sprintf(msg,"%d. Opening echo keyboard",2);
	fd = sysopen(KEYBOARD_ECHO);
	sprintf(msg, "sysopen(KEYBOARD_ECHO) returned: %d\n", fd);
	sysputs(msg);

	sprintf(msg, "%d. sysread 10 char one at a time\n", 3);
	sysputs(msg);

	// we need min 2 spaces - 1 for char and 1 for null term
	for (i=0;i<10;i++){
		rc = sysread(fd,&buffer,2) ;
		sysputs(buffer);
	}

	// design intent : should fail
	sprintf(msg,"%d. Opening noecho keyboard",4);
	sysputs(msg);
	int fd2 = sysopen(KEYBOARD_NOECHO);
	sprintf(msg, "sysopen(KEYBOARD_NOECHO) returned: %d\n", fd2);
	sysputs(msg);

	// design intent : disallow multiple opens on a device
	sprintf(msg,"%d. Opening echo keyboard",5);
	sysputs(msg);
	int fd3 = sysopen(KEYBOARD_ECHO);
	sprintf(msg, "sysopen(KEYBOARD_ECHO) returned: %d\n", fd3);
	sysputs(msg);

	sprintf(msg,"%d. closing keyboard",6);
	sysputs(msg);
	rc = sysclose(fd);
	sprintf(msg, "sysclose(KEYBOARD_ECHO) returned: %d\n",rc);
	sysputs(msg);


	sprintf(msg,"%d. Opening noecho keyboard",7);
	sysputs(msg);
	fd = sysopen(KEYBOARD_NOECHO);
	sprintf(msg, "sysopen(KEYBOARD_NOECHO) returned: %d\n", fd);
	sysputs(msg);


	sprintf(msg,"%d. Three reads of 10 chars",8);
	sysputs(msg);
	int total=0;
	for (i=0;i<=4;i++){
		rc= sysread(fd,&buffer,10);
		total += rc;
		sprintf(msg,"$%s\n",(char*)buffer);
		sysputs(msg);
	}
	sprintf(msg,"total chars read = %d", total);
	sysputs(msg);

	sprintf(msg,"%d. read chars until eof",9);
	sysputs(msg);
	rc=1;
	total =0;
	while (rc!=0){
		rc= sysread(fd,&buffer,10);
		total += rc;
		sprintf(msg,"$%s\n",(char*)buffer);
		sysputs(msg);
	}
	sprintf(msg,"total chars read = %d", total);
	sysputs(msg);

	sprintf(msg,"%d. closing keyboard, open echo keyboard",10);
	sysputs(msg);
	rc = sysclose(fd);
	sprintf(msg, "sysclose(fd) returned: %d\n",rc);
	sysputs(msg);

	sprintf(msg,"  opening echo keyboard",10);
	sysputs(msg);
	fd = sysopen(KEYBOARD_ECHO);
	sprintf(msg, "sysopen(KEYBOARD_ECHO) returned: %d\n",fd);
	sysputs(msg);


	sprintf(msg,"%d. Installing signalhandler for signal 18 into root\n ",11);
	sysputs(msg);
	void (*oldhandler)(void* );
	void (*remember)(void*);
	//for ease of recognition, use handler18 function
	int result = syssighandler(18, handler18, &oldhandler);
	sprintf(msg,"installed signalhandler18, rc = %d\n",result);
	sysputs(msg);
	//mypause(PAUSETIME);

	sprintf(msg,"%d. create child that sleeps and signals 20 and 18 to root\n ", 12);
	sysputs(msg);
	step12_child_pid = syscreate( &sleep_sig20_sig18, 4096 );
//	step12_child_pid = syscreate( &sleep5_sig18, 4096 );


	sprintf(msg,"%d. root will read\n ", 13);
	sysputs(msg);
	DBGMSG("fd = %d, buffer = %x, size = %d\n",fd,buffer,40);
	rc= sysread(fd,&buffer,40);


	sprintf(msg,"14. read returned rc = %d\n", rc);
	sysputs(msg);
	mypause(PAUSETIME);

	sprintf(msg,"%d. create child(sleep5,sig18)\n ", 15);
	sysputs(msg);
	int step15_child_pid = syscreate( &sleep5_sig18, 4096 );

	sprintf(msg,"%d. replace sighdr18 \n ", 16);
	sysputs(msg);
	result = syssighandler(18, handler2, &oldhandler);
	sprintf(msg,"installed signalhandler18, rc = %d\n",result);
	sysputs(msg);
	remember=oldhandler;// for step 19

	sprintf(msg,"%d. root will read\n ", 17);
	sysputs(msg);
	rc= sysread(fd,&buffer,40);

	sprintf(msg,"18. read returned rc = %d\n", rc);
	sysputs(msg);
	mypause(PAUSETIME);

	sprintf(msg,"19. install oldsig18 handler into sig20 rc = %d\n", rc);
	sysputs(msg);
	result = syssighandler(20, remember, &oldhandler);
	sprintf(msg,"installed signalhandler18, rc = %d\n",result);
	sysputs(msg);

	sprintf(msg,"%d. create new process: sleep5_sig20 %d\n", 20);
	sysputs(msg);
	int step20_child_pid = syscreate( &sleep5_sig20, 4096 );

	sprintf(msg,"%d. root will read\n ", 21);
	sysputs(msg);
	rc= sysread(fd,&buffer,40);

	sprintf(msg,"22. read returned rc = %d\n", rc);
	sysputs(msg);
	mypause(PAUSETIME);

	sprintf(msg,"%d. root will read until eof\n ", 23);
	sysputs(msg);
	total=0;
	while ((rc= sysread(fd,&buffer,80))>0){
		total += rc;
		sysputs(buffer);
	}
	sprintf(msg,"%d. total chars read\n ", total);
	sysputs(msg);


	sprintf(msg,"%d. root attempt to read again\n ", 24);
	sysputs(msg);
	rc= sysread(fd,&buffer,40);
	sprintf(msg,"read returned rc = %d\n", rc);
	sysputs(msg);

	rc = sysclose(fd);
	sprintf(msg, "sysclose(fd) returned: %d\n",rc);
	sysputs(msg);

	sprintf(msg,"%d, GoodBye Cruel World\n",25);
	sysputs(msg);


}

void signal_reader(void ){
	char msg[buffsize];
	char buffer[buffsize];
	int result,fd;
	void (*oldhandler)(void* );

	sprintf(msg,"%d. create child(sleep5,sig18)\n ", 15);
	sysputs(msg);
	int step15_child_pid = syscreate( &sleep5_sig18, 4096 );

	sprintf(msg,"install sig handler%d into sig 18 rc = %d\n",18);
	sysputs(msg);
	result = syssighandler(18, handler18, &oldhandler);
	sprintf(msg,"installed signalhandler18 into signal 18, rc = %d\n",result);
	sysputs(msg);




	sprintf(msg,"  opening echo keyboard",7);
	sysputs(msg);
	fd = sysopen(KEYBOARD_ECHO);
	sprintf(msg, "sysopen(KEYBOARD_ECHO) returned: %d\n",fd);
	sysputs(msg);


	sprintf(msg,"root will read \n ", 1);
	sysputs(msg);
	result = sysread(fd,&buffer,40);
	sprintf(msg, "sysread(fd) returned: %d\n",result);
	sysputs(msg);

	result = sysclose(fd);
	sprintf(msg, "sysclose(fd) returned: %d\n",result);
	sysputs(msg);

}

void root(void) {
	root_pid=sysgetpid();
	assign3_test_seq();
	//signal_reader();

	while(1){
		sysyield();
	}

}




// system idle proc in user space that is put onto stack whenever
// no other processes are available to run
void idleproc(void) {
//	TRACE
	while (1) {
//#ifdef DEBUG
//		//show when idleproc is running
//		kprintf(".");
//#endif
		asm
		( " \
		hlt \n\
        	"
				:
				:
				: "%eax"
		);

	}
}

#endif
