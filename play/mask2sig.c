#include <stdio.h>
#include <stdlib.h>

#define MAXSIGNAL 31

int main(int argc, char** argv){

int mask = 0xffc0;
if (argc!=1){
	mask=atoi(argv[1]);
}

int sig;
int i = 0;
for (i=0;i<MAXSIGNAL;i++){
	if (mask&(1<<i)){
		sig=i;
		break;
	}
}


printf("%d\n", sig);

}
	
