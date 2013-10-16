#include <stdlib.h>
#include <stdio.h>


void func1 (void){
	printf("this is func1\n");
}

void func2 (void){
	printf("this is func2\n");
}

void func3 (void){
	printf("this is func3\n");
}







int main (int argc, char** argv){

void  (*f)(void);

f=&func1;


f();


void (*funcarray[3])(int);

funcarray[0] = (void (*)(int))func1;
funcarray[1] = (void (*)(int))func2;
funcarray[2] = (void (*)(int))func3;

int i;
for (i=0;i<3;i++){
	funcarray[i](i);
	printf("function address: %x\n", (unsigned int) funcarray[i]);
}


printf("------------------\n");
    //void  *((*oldfp)());  // same as
void  *(*oldfp)();

oldfp = (void*)funcarray[0];

(*oldfp)();  // same as 
// (*oldfp)();  // same as 
//*oldfp();

printf("**oldfp %x\n", (unsigned int)**oldfp);
printf("*oldfp %x\n", (unsigned int)*oldfp);
printf("oldfp %x\n", (unsigned int)oldfp);

printf("=======================\n");
void (**oldhandler)();

oldhandler = func2;

printf("**oldhandler %x\n", (unsigned int)**oldhandler);
printf("*oldhandler %x\n", (unsigned int)*oldhandler);
printf("oldhandler %x\n", (unsigned int)oldhandler);




}

