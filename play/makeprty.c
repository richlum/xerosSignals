#include <stdio.h>

int main(int argc, char**argv)
{
unsigned int signalnumber;
if (argc==1){
	signalnumber = 5;
}else{
	signalnumber= atoi(argv[1]);
}
if (signalnumber=31){
	printf ("%x",0);
}else {

	unsigned int priority_mask = -1;
	int i=0;
	priority_mask=priority_mask<<(signalnumber+1);
	printf("%x\n", priority_mask);
}


}
