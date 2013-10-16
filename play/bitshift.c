#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv){


int i = 0;

for (i=0;i<32;i++){
	printf("%8x  %8x\n", (-1)<<i,  (-1)>>i);
}
printf("-------------------\n");

for (i=0;i<32;i++){
	printf("%8x  %8x  %8x\n", 1<<i,  1>>i,  ~(1<<i));
}


// we want to accept signal 2 and 5
unsigned int accepting = 0;
printf("%x\n",accepting|(1<<2));
accepting = 0;
accepting = (accepting | (1<<2) | (1<<5));
//should be 0x21
printf("%x\n",accepting);

// is bit 2 set
printf("%x\n", (1<<2)& accepting);

}
