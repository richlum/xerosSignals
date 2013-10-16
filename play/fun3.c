#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

typedef void (*fptr)(void);
typedef void (**optr)(void);

void func1 (void){
	printf("this is func1\n");
	return;
}

void func2 (void){
	printf("this is func2\n");
	return;
}

void func3 (void){
	printf("this is func3\n");
	return;
}


void handleit(void (*f)(), void(**g)()){
	f();
	(*g)();
	(*g)=func3;

}

int varargs(int arg1, ...){
void (*myfunc)(void);
void (**oldfunc)(void);
int count;

va_list ap;
va_start(ap,arg1);
count = arg1;

if (myfunc!=NULL){
	*oldfunc=myfunc;
}
myfunc = (fptr) va_arg(ap, fptr);
oldfunc = (optr) va_arg(ap,optr);

va_end(ap);

printf("count = %d\n", count);
myfunc();


return;
}



int main (int argc, char** argv){

void  (*f)(void);
void (*g)(void);


varargs(1,func1, &g);
varargs(2,func2, &g);
printf("in Main, calling returned g\n");
g();

printf("in Main, after calling returned g\n");
return 0;
}

