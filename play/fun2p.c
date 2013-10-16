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


void handleit(void (*f)(), void(**g)()){
	f();
	(*g)();
	(*g)=func3;

}




int main (int argc, char** argv){

void  (*f)(void);
void (*g)(void);

f=func1;
g=func2;

f();
g();

handleit(f,&g);
g();


}

