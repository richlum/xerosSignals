#include <stdio.h>

#define MAXSIGNAL 31

int get_next_signal(unsigned int pending_signals){
	int i;
	for (i=MAXSIGNAL; i>=0;i--){
		if (pending_signals&(1<<i)){
			return i;
		}
	}
	//no pending signals, return -1
	return -1;
}

int main(int argc, char** argv){

int res = get_next_signal(-1);
printf("-1 pending signal = %d\n", res);

res = get_next_signal(0);
printf("0 pending signal = %d\n", res);
res = get_next_signal(1);
printf("1 pending signal = %d\n", res);
res = get_next_signal(2);
printf("2 pending signal = %d\n", res);
res = get_next_signal(4);
printf("4 pending signal = %d\n", res);
}
	
