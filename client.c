
/********************** client.c ***************************** 
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
* 	privateFIFO = client id
*	Prompt for arrival time
*	Prompt for burst requests
*	Create privateFIFO
* 	Open commonFIFO
*	write(struct values to server)
*	Open privateFIFO
* 	read(struct values from server)
* 	Print received values
*	Close commonFIFO
*	Close privateFIFO
*	Unlink privateFIFO
***********************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

struct pcb{
	char privateFIFO[14];
	int request[5];
	int id;
	int index;
	int burst;
	int completion;
	int numberofRequests;
	int waitClock;
} PCB;


int main (){
  int fda; //write to server
  int fdb; //read from server
  int clientID;
  int loop = 0;
  int numberofRequests;
  char temp[14];
  // Get unique client id, store in privateFIFO  
  clientID = getpid();
  strcpy(PCB.privateFIFO, "FIFO_");
  sprintf(temp, "%d", clientID);
  strcat(PCB.privateFIFO, temp);
  printf("privateFIFO: %s\n", PCB.privateFIFO);
  // Prompt for values  
  PCB.waitClock = 0;
  printf("Please enter an odd number of requests: ");
  scanf("%d", &PCB.numberofRequests);
  for(loop = 0; loop < PCB.numberofRequests; loop++){
	if(loop % 2 == 0){
		printf("Please enter length of CPU burst request: ");
	} else{
		printf("Please enter length of I/O burst request: ");
	}
	scanf("%d", &PCB.request[loop]);
  }
  for(loop = 0; loop < PCB.numberofRequests; loop++){
	if(loop % 2 == 0){
		printf("CPU burst request: ");
	} else{
		printf("I/O burst request: ");
	}
	printf("%d\n", PCB.request[loop]);
  }  
  // Create privateFIFO   
  if ((mkfifo(PCB.privateFIFO,0666)<0 && errno != EEXIST)){
	perror("ERROR: Cannot create privateFIFO\n");
	exit(-1);
  }
  // Open commonFIFO   
  if((fda=open("commonFIFO", O_WRONLY))<0){
    printf("ERROR: Cannot open commonFIFO\n");
  }
  printf("Writing to server...\n");
  write(fda, &PCB, sizeof(PCB)); //write to server
  printf("Waiting for server response...\n");
  // Open privateFIFO   
  if((fdb=open(PCB.privateFIFO, O_RDONLY))<0)
    printf("ERROR: Cannot open privateFIFO\n");
  printf("\nReading from server...\n");
  read(fdb, &PCB, sizeof(PCB)); //read from server
  printf("Completion time: %d\n", PCB.completion);
  printf("\nClosing the client...\n");  
  close(fda);
  close(fdb);
  unlink(PCB.privateFIFO);
  return 0;
}
