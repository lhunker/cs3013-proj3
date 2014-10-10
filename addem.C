/*
Lukas Hunker
addem.C
Adds the numbers from 0 to an input number using threads
*/
#include <iostream>
using namespace std;
#include <cstdio>
#include "mailboxs.h"

mailboxs * box;

/*
worker
The worker thread that adds a specified range
Params:
	arg - the thread id
*/
void *worker(void* arg){
	int myNum = *(int *) arg;
	delete (int *)arg;
	struct msg myRange;
	box->RecvMsg(myNum, &myRange);
	int start = myRange.value1;
	int end = myRange.value2;
	int sum = 0;
	for(int i = start; i < end; i++){
		sum += i;
	}
	struct msg * send = new struct msg;
	send->iFrom = myNum;
	send->type = ALLDONE;
	send->value1 = sum;
	box->SendMsg(0, send);
}

int main (int argc, char ** argv){
	//Parse and validate input
	if (argc < 3){
		cerr << "<Usage> ./addem threads range\n";
		return 1;
	}

	int thread = -1, range = -1;
	sscanf( argv[1], "%d", &thread);
	sscanf(argv[2], "%d", &range);
	if( thread <= 0 || range <= 0){
		cerr << "Both inputs must be positive integers\n";
		return 1;
	}

	//Determing number of threads and range per thread
	if (thread > range){
		thread = range;
	}

	//Initialize mailboxs with mailboxs for workers and parent
	box = new mailboxs(thread + 1);

	//Initilaize thread ids
	pthread_t workers [thread];

	//calculate range and spawn threads
	int perthread = range/thread;
	int rem = range % thread;
	int next = 1;
	for (int i = 0; i < thread; i++){
		int start = next;
		int end = start + perthread;
		if(rem > 0){
			end++;
			rem--;
		}
		
		int * num = new int;
		*num = i+1;
		if(pthread_create(&workers[i], NULL, worker, (void *)num) != 0){
			cerr << "error creating thread\n";
			return 1;
		}
		struct msg * send = new struct msg;
		send->iFrom = 0;
		send->type = RANGE;
		send->value1 = start;
		send->value2 = end;
		box->SendMsg(i+1, send);

		next = end;
	}

	//recieve message
	int sum = 0;
	for(int i = 0; i < thread; i++){
		struct msg recv;
		box->RecvMsg(0, &recv);
		sum += recv.value1;
	}

	cout << "The result using " << thread << " worker threads is " << sum << endl;

	//rejoin worker threads
	for (int i = 0; i < thread; i++){
		pthread_join(workers[i], NULL);
	}
	delete box;

	return 0;
}
