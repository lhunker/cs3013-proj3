#include <iostream>
using namespace std;
#include <cstdio>
#include <string.h>
#include <fstream>
#include <sstream>
#include "mailboxs.h"

#define MAXGRID 40
#define EVEN 0
#define ODD 1
int ** even;
int ** odd;
int rowcount, colcount;
mailboxs * box;
int totalgen;

int getNeighbors (int row, int col, int eo){
	int count = 0;
	for(int i = row-1; i <= row + 1; i++){
		for (int j = col -1; j <= col +1; j++){
			if (i >= 0 && j >= 0 && i < rowcount && j < colcount && !(i == row && col == j)){
				if (eo == EVEN ){
					count += even[i][j];
					//cout << "counted " << even [i][j] << endl;
				} else if (eo == ODD){
					count += odd [i][j];
				}
			} else{
				//cout << "ignoreing " << i << " " << j << endl;
			}
		}
	}
	//cout << endl;
	return count;
}

void * worker(void* arg){
	int myid = (int) arg;
	struct msg myRange;
	box->RecvMsg(myid, &myRange);
	int start = myRange.value1;
	int end = myRange.value2;

	for (int i = 1; i <= totalgen; i++){
		//Wait for go message
		int t;
		do{
			box->RecvMsg(myid, &myRange);
			t = myRange.type;
		}while (t != GO);

		//Process Generation
		for (int j = start; j < end; j++){
			for(int k = 0; k < colcount; k++){
				int n = getNeighbors(j, k, (i-1)%2);
				int newval = -1;
				if (n == 3)
					newval = 1;
				else if ( n < 2 || n > 3)
					newval = 0;

				if (i%2){	//odd generation
					if (newval == -1)
						odd[j][k] = even[j][k];
					else
						odd[j][k] = newval;
				} else {	//even generation
					if (newval == -1)
						even[j][k] = odd[j][k];
					else
						even[j][k] = newval;
				}
			}
		}

		//Send genDone
		struct msg *send = new struct msg;
		send->iFrom = myid;
		send->type = GENDONE;
		box->SendMsg(0, send);

	}
	//Send genDone
	struct msg *send = new struct msg;
	send->iFrom = myid;
	send->type = ALLDONE;
	box->SendMsg(0, send);

}

void printgrid(int eo){
	for (int i = 0; i < rowcount; i++){
		for (int j = 0; j < colcount; j++){
			if (eo == ODD){	//odd ending
				cout << odd[i][j] << " ";
			}else{	//even ending
				cout << even[i][j] << " ";
			}
		}
		cout << endl;
		cout.flush();
	}
}

int main (int argc, char** argv){
	if(argc < 4){
		cerr << "Usage: ./life threads filename generations (print input)\n";
		return 1;
	}

	int thread = -1, maxgen = -1;
	sscanf( argv[1], "%d", &thread);
	sscanf(argv[3], "%d", &maxgen);
	if( thread <= 0 || maxgen <= 0){
		cerr << "Both numbers must be positive integers\n";
		return 1;
	}
	totalgen = maxgen;
	int print = 0, input = 0;
	if(argc > 4){
		print = strcmp(argv[4], "y") == 0;
	}
	if (argc > 5){
		input = strcmp(argv[5], "y") == 0;
	}
	const char* filename = argv[2];

	ifstream infile;

	//get grid size and validate input
	string line;
	infile.open(filename);
	rowcount = 0;
	colcount = 0;
	if(!infile){
		cerr << "can't open file" << endl;
		return -1;
	}
	while(getline(infile, line)){
		istringstream ss (line);
		int count = 0, a = 0;
		while (ss >> a){
			count++;
		}
		if(colcount == 0)
			colcount = count;
		else if (colcount != count && count != 0){	//Check for an even grid ignoreing whitespace
			cerr << "Invalid grid" << endl;
			return -1;
		}
		if (count != 0)
			rowcount++;
	}
	infile.close();

	if (rowcount ==0 || colcount == 0 || rowcount > MAXGRID || colcount > MAXGRID){
		cerr << "Invalid grid\n";
		return -1;
	}

	//allocate grids
	even = new int* [rowcount];
	odd = new int* [rowcount];
	for (int i = 0; i< rowcount; i++){
		even[i] = new int [colcount];
		odd[i] = new int [colcount];
	}

	//fill grids
	infile.open (filename);
	for (int i = 0; i < rowcount; i++){
		for (int j = 0; j < colcount; j++){
			int a;
			infile >> a;
			even[i][j] = a;
		}
	}
	infile.close();


	//setup threads
	if (thread > rowcount){
		thread = rowcount;
	}
	box = new mailboxs(thread + 1);
	int perthread = rowcount/thread;
	int rem = rowcount % thread;
	int next = 0;

	//Initilaize thread ids
	pthread_t workers [thread];

	for (int i = 0; i < thread; i++){
		int start = next;
		int end = start + perthread;
		if(rem > 0){
			end++;
			rem--;
		}
		
		if(pthread_create(&workers[i], NULL, worker, (void *)(i + 1)) != 0){
			cerr << "error creating thread\n";
			return 1;
		}
		struct msg * send = new msg;
		send->iFrom = 0;
		send->type = RANGE;
		send->value1 = start;
		send->value2 = end;
		box->SendMsg(i+1, send);

		next = end;
	}

	//run threads
	int donecount = 0;
	int lastgen = 0;
	for (int i = 1; i <= totalgen; i++){
		if (print){
			cout << "Generation " << i-1 << endl;
			printgrid((i-1)%2);
		}

		if(input){
			cin.get();
		}

		for(int j = 0; j < thread; j++){
			struct msg * send = new msg;
			send->iFrom = 0;
			send->type = GO;
			box->SendMsg(j+1, send);
		}

		for(int j = 0; j < thread; j++){
			struct msg recv;
			int t;
			do{
				box->RecvMsg(0, &recv);
				t = recv.type;
				if (t == ALLDONE){
					donecount++;
				}

			} while (t != GENDONE);
		}
		lastgen = i;
	}

	//recieve alldone message
	int sum = 0;
	for(int i = 0; i < thread-donecount; i++){
		struct msg recv;
		int t;
		do{
			box->RecvMsg(0, &recv);
			t = recv.type;
		} while (t != ALLDONE);
	}

	//rejoin worker threads
	for (int i = 0; i < thread; i++){
		pthread_join(workers[i], NULL);
	}
	delete box;

	//print final grid
	cout << "The game ended after " << lastgen << " generations with\n";
	printgrid(totalgen % 2);

	return 0;
}