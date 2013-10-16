#include <stdio.h>
#include <stdarg.h>


//to recover var arg list, get the ptr to the list first
//then treat the pointer to the list as a va_list and recover varargs

int subfunc(int type, ...){
	va_list ap;
	va_list sap;
	va_start(ap,type);
	sap = (va_list)va_arg(ap,unsigned int);
	int a = va_arg(sap,int);
	int b = va_arg(sap,int);
	printf("subfunc args %d, %d\n", a, b); 
	if (type==1){
		return a+b;	
	}else{
		return a*b;
	}

}

// ap is a pointer to list of arguments, we can pass this var arg list on
int varfunc(int a, int b, ...){
	int total = a + b;
	va_list ap;
	va_start(ap,b);
	int sf=subfunc(2,ap);
	va_end(ap);
	total += sf ;
	return total;
}



int main(int argc, char** argv){

printf("%d\n", varfunc(1,2,3,4));


}
