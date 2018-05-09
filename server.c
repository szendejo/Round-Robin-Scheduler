
/********************** server.c ***************************** 
* Programmer:  Stephanie Zendejo 
* 
* Course:  CSCI 4534
* 
* Date:  December 1, 2017
* 
* Assignment:  Program #4: Round Robin CPU Scheduler with I/O Queue 
* 
* Environment:  Command line, PuTTy, GCC 
* 
* Files Included: server.c, client.c
* 
* Purpose:  A short-term scheduler will run a round-robin scheduler. Each client will
* send a request to a server. The server will receive requests from multiple clients.
* The server will act as a CPU scheduler and I/O, and report a client's completion time
* to them privately. The server will print idle time at the end of the program.
*
* Input:  The clients will input a set number of bursts. These bursts will alternate between 
* CPU and I/O bursts. The server will input for
* the number of clients, and the timeQuant.
* 
* Preconditions: The program have at least 3 clients and use the round-robin CPU scheduler 
* algorithm. Exception handling must be created for invalid requests. A client may only
* contact the server once, through a privateFIFO. The server will use a common FIFO 
* that will only be opened once. The client program will be run multiple times.
* 
* Output: The server will send exception handling responses privately to the client. The
* server will periodically display the contents of a queue named Ready. The value of the
* timeQuant is calculated, the value in the clock is recorded, and the idle time 
* is calculated. The clock value is sent privately to the client.
* Once the clients have completed processing, the server will report the total idle time .
*
* Postconditions: The client will terminate after displaying response from server.
*  When all clients have completed, the server will shut down.
* 
* Time Estimations:
* Design Est: 5 hrs, Act: 7 hrs
* Implementation: 10 hrs, Act: 17 hrs
* Testing: 5 hrs, Act. 9 hrs
* 
* Algorithm: 
*	Create commonFIFO
*	Open commonFIFO
* 	Prompt for numberofClients, timeQuant
* 	Load client data
*   Create client[numberofClients]
*	Set client data
*	Load PCBs in Ready Queue
*	Print loaded PCBs in Ready Queue
*   while queues are not empty
*		if CPU queue is not empty
*			if burst is larger than timeQuant 
*               Update burst
*               if waitClock is larger than clock
*			        Find idle time
*				    Update clock with burst and idle time
*			    else
*					Update clock with burst
*				Dequeue from Ready and Enqueue to Ready again  
*			else if burst is less than timeQuant
*				if waitClock is larger than clock
*			        Find idle time
*				    Update clock with burst and idle time
*			    else
*					Update clock with burst
*				if not the last CPU request  
*               	Update index
*               	Dequeue from Ready and Enqueue to Wait
*				else if the last CPU request  
*               	Update completion 
*					Dequeue node from head of Ready queue
*               	Open privateFIFO
*               	Write to privateFIFO
*               	Close privateFIFO
*				else print exception handling
*		else if Wait queue is not empty
*			if waitClock in PCB is larger than current waitClock
*				Update waitClock with PCB waitClock
*			Add burst time to waitClock
*			Update index, waitClock
*			Dequeue from Wait and Enqueue to Ready
*		else print exception handling
*   Print idle time
*	Close commonFIFO
*	Unlink commonFIFO
***********************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
    
typedef struct pcb{
	char privateFIFO[14];
	int request[5];
	int id;
	int index;
	int burst;
	int completion;
	int numberofRequests;
	int waitClock;
} PCB;

typedef struct node{  /*Nodes stored in the linked list*/
	struct pcb elements;
	struct node *next;
} Node;

typedef struct queue{ /*A struct facilitates passing a queue as an argument*/
	Node *head;       /*Pointer to the first node holding the queue's data*/
	Node *tail;       /*Pointer to the last node holding the queue's data*/
	int sz;           /*Number of nodes in the queue*/
} Queue;

int size(Queue *Q){
	return Q->sz;
}

int isEmpty(Queue *Q){
	if( Q->sz == 0 ) return 1;
	return 0;
}

int getCompletion(Queue *Q){
	return Q->head->elements.completion;
}

void setCompletion(Queue *Q, int clock){
	Q->head->elements.completion = clock;
}

int getBurst(Queue *Q){
	PCB temp;
	if(isEmpty(Q)){
		printf("ERROR: Queue is empty\n");
	}
	return Q->head->elements.request[Q->head->elements.index];
}

void setBurst(Queue *Q, int timeQuant){
	Q->head->elements.request[Q->head->elements.index] -= timeQuant;
}

int getId(Queue *Q){
	PCB temp;
	if(isEmpty(Q)){
		printf("ERROR: Queue is empty\n");
	}
	return Q->head->elements.id;
}

int getIndex(Queue *Q){
	PCB temp;
	if(isEmpty(Q)){
		printf("ERROR: Queue is empty\n");
	}
	return Q->head->elements.index;
}

char* getPrivateFIFO(Queue *Q){
	PCB temp;
	if(isEmpty(Q)){
		printf("ERROR: Queue is empty\n");
	}
	return Q->head->elements.privateFIFO;
}

void setIndex(Queue *Q){
	Q->head->elements.index += 1;
}

void setWaitClock(Queue *Q, int waitTime){
	Q->head->elements.waitClock;
}

int getNumberOfRequests(Queue *Q){
	PCB temp;
	if(isEmpty(Q)){
		printf("ERROR: Queue is empty\n");
	}
	return Q->head->elements.numberofRequests;
}

int getWaitClock(Queue *Q){
	PCB temp;
	if(isEmpty(Q)){
		printf("ERROR: Queue is empty\n");
	}
	return Q->head->elements.waitClock;
}

void enqueue(Queue *Q, struct pcb elements){
	Node *v = (Node*)malloc(sizeof(Node));/*Allocate memory for the Node*/
	if(!v){
		printf("ERROR: Insufficient memory\n");
		return;
	}
	v->elements = elements;
	v->next = NULL;
	if( isEmpty(Q) ) Q->head = v;
	else Q->tail->next = v;
	Q->tail = v;
	Q->sz++;
}

PCB dequeue(Queue *Q){
	PCB temp;
	Node *oldHead;
	if(isEmpty(Q)){
		printf("ERROR: Queue is empty\n");
		return temp;
	}
	oldHead = Q->head;
	temp = Q->head->elements;
	Q->head = Q->head->next;
	free(oldHead);
	Q->sz--;
	return temp;
}

void destroyQueue( Queue *Q ){
	while( !isEmpty(Q) ) dequeue(Q);
}

/*A different visit function must be used for each different datatype.*/
/*The name of the appropriate visit function is passed as an argument */
/*to traverseQueue.                                                   */
void visitNode(PCB elements, int count){
	printf("\n**** Client: %s ****\n", elements.privateFIFO);
	int loop;
	for(loop = 0; loop < elements.numberofRequests; loop++){
		if(loop % 2 == 0){
			printf("CPU Burst: ");
		}
		else {
			printf("I/O Burst: ");
		}
		printf("%d\n", elements.request[loop]);
	}
}

/*The following function isn't part of the Queue ADT, however*/
/*it can be useful for debugging.                            */
void traverseQueue(Queue *Q){
	Node *current = Q->head;
	int count;
	while(current){
		visitNode(current->elements, count);
		current = current->next;
		count++;
	}
	printf("\n");
}

/*Sample code that demonstrates the queue*/
int main( int argc, char *argv[] ){
	// Declare variables
	int fda;	//read from client
	int fdb;	//write to client
	int clock = 0;
	int exitFlag = 0;
	int idle = 0;
	int loop = 0;
	int numberofClients = 0;
	int subloop = 0;
	int timeQuant = 0;
	int waitClock = 0;
	int waitFlag = 0;
	Queue Ready;
	Ready.head = NULL;
	Ready.tail = NULL;
	Ready.sz = NULL;
	Queue Wait;
	Wait.head = NULL;
	Wait.tail = NULL;
	Wait.sz = NULL;	
	// Create, open commonFIFO
	printf("Creating commonFIFO...\n");
	if ((mkfifo("commonFIFO", 0666)<0 && errno !=EEXIST)){
		perror("ERROR: Cannot create commonFIFO");
		exit(-1);
	}
	printf("Opening commonFIFO...\n");
	if((fda=open("commonFIFO", O_RDONLY))<0){
		printf("ERROR: Cannot open commonFIFO");
	}
	// Prompt for input
	printf("\nPlease enter the number of clients: ");
	scanf("%d", &numberofClients);  
	printf("\nPlease enter the time quant: ");
	scanf("%d", &timeQuant);  
	// Load client data
	PCB client[numberofClients];
	printf("Loading client data...\n");
	for(loop = 0; loop < numberofClients; loop++){
		read(fda, &client[loop], sizeof(client[loop]));
	}
	// Set client data
	printf("Client data loaded...\n");
	for(loop = 0; loop < numberofClients; loop++){
		client[loop].id = loop;
	}
	//Load PCBs in Ready
	for(loop = 0; loop < numberofClients; loop++){
		enqueue(&Ready, client[loop]);
	}
	// Print loaded PCBs in Ready
	traverseQueue(&Ready);
	// Begin Processing
	while(exitFlag != 1 && (!isEmpty(&Ready) || !isEmpty(&Wait))){
		// Processing CPU Request
		if(!isEmpty(&Ready)){
			// If burst is larger than timeQuant
			if(getBurst(&Ready) > timeQuant){
				printf("\n*** Processing %s CPU burst ***\n", getPrivateFIFO(&Ready));
				// Subtract burst by timeQuant
				setBurst(&Ready, timeQuant);
				// Update CPU queue clock
				printf("Clock: %d\n", clock);
				printf("Updating CPU clock...\n");
				// If I/O queue clock is larger than regular clock
				if(waitClock > clock){
					// Determine total idle time
					idle += waitClock - clock;
					printf("Idle: %d\n", (waitClock - clock));
					// Add idle time and the CPU burst
					clock += (waitClock - clock) + timeQuant;
				}
				// If I/O clock and CPU clock are the same
				else {
					// Add CPU burst to clock
					clock += timeQuant;
				}
				// Place PCB at end of Ready queue
				printf("(Updated) CPU Burst: %d\n", getBurst(&Ready));
				printf("Dequeueing from Ready...\n");
				PCB temp = dequeue(&Ready);
				printf("Enqueueing to Ready...\n");
				waitFlag = 0;		
				enqueue(&Ready, temp);		
			}
			// If burst is less than timeQuant
			else if (getBurst(&Ready) <= timeQuant){
				printf("\n*** Processing %s CPU burst ***\n", getPrivateFIFO(&Ready));
				// Subtract burst by size of burst
				int burstTemp = getBurst(&Ready);
				setBurst(&Ready, getBurst(&Ready));
				// Update CPU queue clock
				printf("Clock: %d\n", clock);
				printf("Updating CPU clock...\n");
				// If I/O queue clock is larger than regular clock
				if(waitClock > clock){
					// Determine total idle time
					idle += waitClock - clock;
					// Add idle time and the CPU burst
					clock += (waitClock - clock) + burstTemp;
				}
				// If I/O clock and CPU clock are the same
				else {
					// Add CPU burst to clock
					clock += burstTemp;
				}
				// If not on last CPU request
				if(getIndex(&Ready) != (getNumberOfRequests(&Ready) - 1)){
					// Update index
					setIndex(&Ready);
					// Dequeue from Ready
					printf("Dequeueing from Ready...\n");
					PCB temp = dequeue(&Ready);
					// Enqueue to Wait
					printf("Enqueueing to Wait...\n");
					waitFlag = 1;	
					enqueue(&Wait, temp);					
				}
				// If on last CPU request
				else if(getIndex(&Ready) == (getNumberOfRequests(&Ready) - 1)){
					printf("\n*** %s has completed ***\n", getPrivateFIFO(&Ready));
					//Update completion time
					setCompletion(&Ready, clock);
					printf("Completion Time: %d\n", getCompletion(&Ready));
					PCB temp = dequeue(&Ready);
					printf("Opening privateFIFO...\n");
					if ((fdb=open(temp.privateFIFO,O_WRONLY))<0){
						printf("ERROR: Cannot open privateFIFO");
					}
					printf("Writing to client...\n");
					write(fdb, &temp, sizeof(temp));
					printf("Closing privateFIFO...\n");
					close(fdb);
					waitFlag = 1;	
				}
				else{
					printf("Error: Could not determine if CPU request is last or not.\n");
					exitFlag = 1;
				}
			}
			else {
				printf("Error: CPU Burst Request could not be processed.\n");
				exitFlag = 1;
			}		
			printf("Clock: %d\n", clock);
		}
		// Processing I/O Request
		else if(waitFlag == 1 && !isEmpty(&Wait)){
			printf("\n*** Processing %s I/O burst ***\n", getPrivateFIFO(&Wait));
			printf("Clock: %d\n", clock);
			printf("Updating I/O clock...\n");
			// Update I/O queue clock
			// If first instance of clock or if I/O clock larger than I/O clock
			if (waitClock == 0 || (getWaitClock(&Wait) > waitClock)){
				// Update waitClock time
				waitClock = getWaitClock(&Wait);
			}
			// Add burst time to waitClock
			waitClock += getBurst(&Wait);
			printf("Clock: %d\n", waitClock);		
			// Update index
			setIndex(&Wait);
			// Update waitClock
			setWaitClock(&Wait, waitClock);
			// Dequeue from Wait
			printf("Dequeueing from Wait...\n");
			PCB temp = dequeue(&Wait);
			printf("Enqueueing to Ready...\n");
			enqueue(&Ready, temp);	
			waitFlag = 0;
		}
		else{
			printf("Error: If cases inside while loop could not be processed.\n");
			exitFlag = 1;
		}
	}
	
	printf("\nIdle Time: %d\n", idle);  
	
	// Clean Up
	destroyQueue(&Ready);
	destroyQueue(&Wait);
	printf("\nClosing the server...\n");  
	close(fda);
	unlink("commonFIFO");
	return 0;
}