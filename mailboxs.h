/*
Lukas Hunker
mailboxs.h
Header file for mailbox class
*/
#define RANGE 1
#define ALLDONE 2
#define GO 3
#define GENDONE 4
#define STOP 5
#include <semaphore.h>

struct msg
{
	int iFrom;
	int type;
	int value1;
	int value2;
	
};

class mailboxs
{
public:
	mailboxs(int n);
	~mailboxs();
	void SendMsg(int iTo, struct msg *Msg);
	void RecvMsg(int iFrom, struct msg *msg);

private:
	int size; 	//the number of mailboxs being stored
	struct msg ** boxs;
	sem_t * sendsem;
	sem_t * recvsem;
};