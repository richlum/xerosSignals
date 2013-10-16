#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char** argv)
{
	char b[32] ;
	b[0] = (char)'\0';
	int x;	
	if (argc==1){
		x=-1;
	}else{
		x=atoi(argv[1]);
	}

	int i = 1;
	i = i<<31;
	int j;
	for (j = 31; j>=0; j--)
	{
		int z = 1;
		z=z<<j;
		strcat(b, ((x&z) == z) ? "1" : "0");
	}	
	printf("10987654321098765432109876543210\n");
	printf("%s\n",b);

}
