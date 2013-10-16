#ifndef __SIGNAL_H__
#define __SIGNAL_H__

int get_signaller_signum(pcb* p);{
	va_list     ap;
	ap = (va_list)p->args;
	unsigned int new_pid = va_arg(ap, unsigned int);
	int new_signum = va_arg(ap,  int);
	return new_signum;
}


int get_signaller_targetpid(pcb* p){
	va_list     ap;
	ap = (va_list)p->args;
	unsigned int new_pid = va_arg(ap, unsigned int);
	int new_signum = va_arg(ap,  int);
	return new_pid;
}


#endif
