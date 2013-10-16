#include <stdio.h>
#include <string.h>

#define maxlen 64
#define maxidx 12
int main(int argc, char** argv){
int idx;
const char* a2darray[]={ "one", "two", "three", "four", "five"};

printf("%s\n",a2darray[0]);
printf("%s\n",a2darray[1]);
printf("%s\n",a2darray[2]);
printf("%s\n",a2darray[3]);
printf("%s\n",a2darray[4]);

char real2darray[maxidx][maxlen];

memset( real2darray,'\0',(maxidx-2)*(maxlen-2));

sprintf (real2darray[1], "%s", "red"); 
sprintf (real2darray[2], "%s", "blue"); 
sprintf (real2darray[3], "%s", "green"); 
sprintf (real2darray[4], "%s", "alpha");
 
int i ;

for (i=0;i<5;i++){
	printf("%s\n", real2darray[i]);
}


}
