
// given a signalq of pcb, insert in priority order the pcb
// requesting to send a signal to this process pcb.
// >0 on success, negative on no insert
// precondition is that all links from other q to this have already been removed
int insert_signalq(pcb** q, pcb* p){
/********************************/
	int count =0;
	//print_q(*q);
	
	if (p->next!=NULL){
		kprintf("WARNING,pcb %d next was not null: resetting to null", p->pid);
		p->next=NULL;
	}
	//empty q, just insert it
	if (*q==NULL){
		count++;
		*q=p;
		return count;
	}
		
	// the new entry to insert
	unsigned int new_pid = get_signaller_targetpid(p);
	kassert((*q)->pid == new_pid);  // we are target of new signal
	int new_signal_priority = get_signaller_signum(p);
	
	// the current signalq entry value
	unsigned int q_pid = get_signaller_targetpid(*q);
	kassert((*q)->pid == q_pid);  // we are target of signal in our signalq
	int q_priority = get_signaller_signum(*q);
	
	if (q_priority < new_signal_priority){
		//insert new entry at head of q
		p->next = *q;
		*q = p;
		return ++count;
	}else if (q_priority == new_signal_priority){
		// trying to enter two of same signal at same time, ignore second
		return -1;
	}else{
		// we need to find the right spot to insert the new_signaller
		pcb* prev = *q;
		pcb* next = *q;
		while((next)&& (q_priority < new_signal_priority)){
			prev=next;
			next=next->next;
			count++;
		}
		//insert our new signaller pcb
		p->next=next;
		prev->next=p;
		return count++;
	}
}

void print_signal_q(pcb* q){
	while(q){
		kprintf("signum: %d" ,  get_signaller_signum(q));
		kprintf("  pid: %d \n", get_signaller_targetpid(q));
		q=q->next;
	}
	
}
