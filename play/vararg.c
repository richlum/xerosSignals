#include <stdio.h>
#include <stdarg.h>


int varfunc(int a, int b, ...){
	int total = a + b;
	va_list ap;
	va_start(ap,b);
	int c = va_arg(ap, int);
	int d = va_arg(ap, int);
	va_end(ap);
	total += c + d;
	return total;
}



int main(int argc, char** argv){

printf("%d\n", varfunc(1,2,3,4));


}
