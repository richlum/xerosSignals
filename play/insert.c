#include <stdio.h>

typedef struct nodestruct node;
struct nodestruct{
	int val;
	node* next;
} ;

node* head;
node nodes[20];


insert(node** q, node* p){

	if (*q==NULL){
		*q=p;
		return;
	}else if ((*q)->val < p->val){
		p->next = *q;
		*q = p;
	}else{
		node* prev = *q;
		node* next = *q;
		while((next)&&(p->val<next->val)){
			prev=next;
			next=next->next;
		}
		p->next = next;
		prev->next=p;
	}		

}


void print(node* q){
	while(q){
		printf("node: %d\n", q->val);
		q=q->next;
	}

}

int main(int argc, char**argv)
{

int i ;
for (i=0;i<20;i++){
	nodes[i].val=i;
}


insert(&head, &nodes[5]);
insert(&head, &nodes[4]);
insert(&head, &nodes[7]);
insert(&head, &nodes[9]);
insert(&head, &nodes[3]);
insert(&head, &nodes[1]);
insert(&head, &nodes[8]);
insert(&head, &nodes[2]);
insert(&head, &nodes[6]);


print(head);

}
