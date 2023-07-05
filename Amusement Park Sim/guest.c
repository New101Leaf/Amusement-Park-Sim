


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "fair.h"

volatile sig_atomic_t quitFlag = 0;


void receiveRideNames(int clientSocket, char (*fairRides)[RIDE_NAME_MAX_CHARS]);
void receiveRideTickets(int clientSocket, unsigned char ticketReq[NUM_RIDES], size_t element);
void handleSig1(int); 
void handleSig2(int);

// This program takes 3 command line arguments.  
// 1 - The number of tickets that the guest has (e.g., 5 to 40)
// 2 - The maximum time (in seconds) that the guest is willing to wait in line for a ride (e.g., 600 - 1200)
// 3 - The first ride that this guest wants to go on (i.e., 0 to NUM_RIDES)






void main(int argc, char *argv[]) {
	
	
	
    // Guest tickets 5-40
	// wait time 600 - 1200
	// the ride 0-9
     int guestTickets;
	 int waitTime;
	 unsigned char desiredRide;
	 unsigned int guestID = getpid();
	 unsigned int estWaitTime =0;

	 //Buffer for storing the rides and required tickets
	 char fairRidesNames[NUM_RIDES][RIDE_NAME_MAX_CHARS];
	 unsigned  char ticketsRequired[NUM_RIDES];


	if (argc != 4) {
		printf("Incorrect number of command line arguments. Usage: ./guest <guestTickets> <waitTime> <desiredRide>\n");
		exit(-1);
	}

   
	// Set the random seed
	srand(time(NULL));
	
	// Get the number of tickets, willing wait time and first ride from the command line arguments
  	guestTickets = atoi(argv[1]);
	waitTime = atoi(argv[2]);
	desiredRide = atoi(argv[3]);
	

	//printf("Guest has %d tickets, is willing to wait %d seconds and wants to go on ride %d ride \n",guestTickets, waitTime, desiredRide);

	
	int                 clientSocket;
	struct sockaddr_in  clientAddress;
	int                 status, bytesRcv;


	unsigned int buffer[5];
	unsigned int response[4];


    // Create socket
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket < 0) {
		printf("*** CLIENT ERROR: Could open socket.\n");
		exit(-1);
	}

	// Setup address
	memset(&clientAddress, 0, sizeof(clientAddress));
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	clientAddress.sin_port = htons((unsigned short) SERVER_PORT);

	//Connect to server
	status = connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
	if (status < 0) {
		printf("*** CLIENT ERROR: Could not connect.\n");
		exit(-1);
	}

	unsigned char admitted = 0;

	


	// Now simulate the going on rides until no more tickets remain (you will want to change the "1" here)
	while (guestTickets > 0){

		

		//Admit the guest 
		if (admitted == 0){
			buffer[0] = ADMIT;
			buffer[1] = guestID;
			//printf("Sending request\n");
			printf("SENDING ADMIT REQThe process id is %d \n", guestID);
			send(clientSocket,buffer,sizeof(buffer),0);
			
			recv(clientSocket,response, sizeof(response),0);

			printf("Before entering the Fair \n");
			if (response[0] == YES){
				printf("GUEST %d admitted  \n", guestID);
		
				//Save ride names and tickets 
				receiveRideNames(clientSocket, fairRidesNames);
				receiveRideTickets(clientSocket, ticketsRequired, sizeof(ticketsRequired));
				//printf("Guest wants to ride %s \n", fairRidesNames[desiredRide]);
				admitted = 1;

			}else{
				printf("Could not admit guest \n ");
				break;
			}

		}

		
		while (1) {
				// Make sure that the guest has enough tickets for the desired ride
				// otherwise chose a different ride
				//if does not have tickets for desired ride break out of loop 
				if (guestTickets < ticketsRequired[desiredRide]){
					//printf("Guest has less tickets than required ride tickets \n");
					break;
					
				}

			//	printf("Sending get wait est %d \n", guestID);
				buffer[0] = GET_WAIT_ESTIMATE;
				buffer[1] = desiredRide;
				send(clientSocket,buffer,sizeof(buffer),0);
				//Get the ride wait time
               

				recv(clientSocket, response, sizeof(response), 0);
				//printf("Wait time is %d seconds and the guest is willing to wait %d seconds \n", response[0], waitTime);
				
				
				// If the guest is willing to wait, then get into line for that ride
				if (response[0] < waitTime){
					//printf("GETTING IN line Sending get in line command %d with a waitime of %d \n",guestID, response[0]);
					buffer[0] = GET_IN_LINE;
					buffer[1] = desiredRide;
					buffer[2] = guestID;
					send(clientSocket,buffer,sizeof(buffer),0);

				}else{
					//printf("Guest is not willing to wait for ride \n");
					break;
				}
				//check if guest can get in line 
				recv(clientSocket, response, sizeof(response), 0);
				if (response[0] == NO){
				
					break;
				}


				//Send signals for boarding and unboarding 
				sigset_t setUsr1, setUsr2;
				sigemptyset(&setUsr1);
				sigaddset(&setUsr1, SIGUSR1);
				sigemptyset(&setUsr2);
				sigaddset(&setUsr2, SIGUSR2);
				int signal_number_usr1, signal_number_usr2;

				signal(SIGUSR1, handleSig1);
				sigwait(&setUsr1, &signal_number_usr1);

				signal(SIGUSR2, handleSig2);
				sigwait(&setUsr2, &signal_number_usr2);

				

				//Remove tickets
				guestTickets = guestTickets - ticketsRequired[desiredRide];

				// Delay a bit (DO NOT CHANGE THIS LINE)
				usleep(100000);
				break;
				
			
				
			}
			

			//choose a new ride at random
			desiredRide = rand() % NUM_RIDES;

		
	
	}
	//Guest out of tickets so send a leaving fair command
	printf("Guest leaving \n");
	buffer[0] = LEAVE_FAIR;
	buffer[1] = guestID;
	send(clientSocket,buffer,sizeof(buffer),0);
	
	
	printf("Guest leaving \n");
	//Killing the guest process if guest does not get admitted or runs out of tickets
   int result = kill (guestID, SIGTERM);
	
	

   close(clientSocket);  
   printf("Guest: Left.\n");


	
  

	
}


void receiveRideNames(int clientSocket, char (*fairRides)[RIDE_NAME_MAX_CHARS]) {
    unsigned char buffer[4 + NUM_RIDES * RIDE_NAME_MAX_CHARS];
    recv(clientSocket, buffer, sizeof(buffer), 0);

    int offset = 4;
    for (int i = 0; i < NUM_RIDES; i++) {
        char rideName[RIDE_NAME_MAX_CHARS + 1];
        strncpy(rideName, (char *)(buffer + offset), RIDE_NAME_MAX_CHARS);
        rideName[RIDE_NAME_MAX_CHARS] = '\0';
        strcpy(fairRides[i], rideName);
        offset += RIDE_NAME_MAX_CHARS;
    }

}




void receiveRideTickets(int clientSocket, unsigned char ticketReq[NUM_RIDES], size_t element){

	recv(clientSocket, ticketReq, element, 0);


}



void handleSig1(int i) {
  printf("  Guest is boarding Continuing...\n");
}

void handleSig2(int i) {
  printf(" Guest is unloading \n");
  quitFlag = 1; // Set the flag to terminate the loop
}



