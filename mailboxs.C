#include "mailboxs.h"


	//constructor
mailboxs::mailboxs(int n){
	boxs = new struct msg [n];
	//initialize semaphores
	sendsem = new sem_t [n];
	recvsem = new sem_t [n];
	for(int i = 0; i < n; i++){
		 sem_init(&sendsem[i], 0, 1);
		 sem_init(&recvsem[i], 0, 0);
	}
	size = n;
}

void mailboxs::SendMsg(int iTo, struct msg &Msg){
	sem_wait(&sendsem[iTo]);
	boxs[iTo] = Msg;
	sem_post(&recvsem[iTo]);
}

void mailboxs::RecvMsg(int iFrom, struct msg &msg){
	sem_wait(&recvsem[iFrom]);
	msg = boxs[iFrom];
	sem_post(&sendsem[iFrom]);
}

mailboxs::~mailboxs(){
	for(int i = 0; i < size; i++){
		sem_destroy(&sendsem[i]);
		sem_destroy(&recvsem[i]);
	}
	delete sendsem, recvsem, boxs;
}
