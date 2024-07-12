#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>



void sendRideNames(int clientSocket, Ride *rides);
void sendTickets(int clientSocket, Ride *rides);





// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should wait for a client to send a request, process it, and then
// close the client connection and wait for another client.  The requests that may be
// handled are as follows:
//
//   SHUTDOWN - causes the fair server to go offline.  No response is returned.
//
//   ADMIT - contains guest's process ID as well. return a list of all rides and their
//			 ticketRequirements.
//
//	 GET_WAIT_ESTIMATE - takes a ride ID as well.   It then returns an estimate as to 
//						 how long of a wait (in seconds) the guest would have to wait
//						 in order to get on the ride.
//
//	 GET_IN_LINE - takes a ride ID and guest's process ID as well.  It then causes the
//				   guest to get in line for the specified ride ... assuming that the 
//				   ride ID was valid and that the line hasn't reached its maximum.
//				   An OK response should be returned if all went well, otherwise NO.
// 
//   LEAVE_FAIR - takes a guest's process ID.  It then causes the guest to leave the fair.
//				  No response is returned.





void *handleIncomingRequests(void *x) {
	Fair *f = (Fair *)x;

	

	

	//Server variables
	int                 serverSocket, clientSocket;
	struct sockaddr_in  serverAddress, clientAddr;
	int                 status, addrSize, bytesRcv;
	
    //Sending and reciving commands
	unsigned int buffer[5];
	unsigned int response[4];

	//Intialize semaphore

	
	//Fair Variables
	f -> numGuests = 0;
	unsigned short waitTime = 0; //= getWaitTime(&f->rides[0]);
	

	// Create the server socket
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket < 0) {
		printf("*** SERVER ERROR: Could not open socket.\n");
		exit(-1);
	}

	// Setup the server address
	memset(&serverAddress, 0, sizeof(serverAddress)); // zeros the struct
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons((unsigned short) SERVER_PORT);

	

	// Bind the server socket
	status = bind(serverSocket,  (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (status < 0) {
		//printf("*** SERVER ERROR: Could not bind socket.\n");
		perror("*** SERVER ERROR: Could not bind socket");
		exit(-1);
	}

	// Set up the line-up to handle up to 5 clients in line 
	status = listen(serverSocket, 5);
	if (status < 0) {
		printf("*** SERVER ERROR: Could not listen on socket.\n");
		exit(-1);
	}



    
  
    
	while(1) {

	  

		addrSize = sizeof(clientAddr);
		clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &addrSize);
		if (clientSocket < 0) {
			printf("*** SERVER ERROR: Could not accept incoming client connection.\n");
			exit(-1);
		}else{
			printf("Server accepted client connection ------------------------------------- \n");
		}
		

		//go into loop to talk to guest
		
		while (1) {
		
				recv(clientSocket, buffer, sizeof(buffer), 0); //get id from here
				printf("SERVER: Received client request \n");
			

				//Admitting guest
				
				if (buffer[0] == ADMIT){
					printf("Got admit request from guest %d ", buffer[1]);
					
					//printf("\n ---Admit recieved\n");
					if(f->numGuests < MAX_GUESTS){
						f->numGuests++;
						
						response[0] = YES;
						printf("Guest admitted sending respone and their id is %d \n", buffer[1]);
					
						send(clientSocket, response, sizeof(response), 0);
						//Add guest id to the fair array
					
						f ->guestIDs[f->numGuests] = buffer[1];

						//Send the list of rides and the tickets required
						sendRideNames(clientSocket, f -> rides);	
						sendTickets(clientSocket, f -> rides);
						//f->numGuests++;
						printf("Guest num %d ", f -> numGuests);
						
						//sem_post(&serverBusyIndicator);
						
					}
					
						

				}
				//sem_post(&serverBusyIndicator);
				//Get wait estimate command
				
				if(buffer[0] == GET_WAIT_ESTIMATE){
					sem_wait(&serverBusyIndicator);
					printf("Estimate req \n");
					printf("wants to ride %s \n ", f -> rides[buffer[1]].name);
					
					unsigned int waitEstimate;
				    waitEstimate = (f->rides[buffer[1]].lineupSize/f->rides[buffer[1]].capacity) * (f->rides[buffer[1]].rideTime + ((f->rides[buffer[1]].onOffTime*2)*f->rides[buffer[1]].capacity));
					response[0] = waitTime;
					send(clientSocket, response, sizeof(response), 0);
					sem_post(&serverBusyIndicator);
					printf("leaving get wait \n");
				}
		
				//Get in in line command
				if (buffer[0] == GET_IN_LINE){
					sem_wait(&serverBusyIndicator);
					printf("Guest wants to go on ride %d and the process id is %d \n", buffer[1],buffer[2]);
					//check if ride has reached max lineup
					//Then add guest to that lineup 
					//else respond with no
					if (f -> rides[buffer[1]].lineupSize < MAX_LINEUP){
						response[0] = YES;
						f->rides[buffer[1]].waitingLine[f->rides[buffer[1]].lineupSize] = buffer[2];
						f->rides[buffer[1]].lineupSize++;
					//	printf("The first guest in line is %d sending yes \n", f -> rides->waitingLine[0]);
					}else{
						response[0] = NO;
					}
					

					send(clientSocket,response,sizeof(response),0);
					sem_post(&serverBusyIndicator);
					
					
				}
				printf("Out of get line \n");

				//Leave fair
				if (buffer[0] == LEAVE_FAIR){
					sem_wait(&serverBusyIndicator);
					//Find the id of the guest and remove it from the array
					int found =0;
					int i,j;
                    			//Find the position to remove
					for (i =0; i< f ->numGuests; i++){
						if (f -> guestIDs[i] == buffer[1]){
							found =1;
							break;
						}
					}
					
					if (found){
						//shift elements to the left
						for (int j = i; j < f-> numGuests; j++){
							f->guestIDs[j] = f -> guestIDs[j+1];

						}
						
					}
				
					
					f -> numGuests--;
					sem_post(&serverBusyIndicator);
					
					break;


				}
				if(buffer[0] == SHUTDOWN){
				sem_wait(&serverBusyIndicator);
				for(int i = 0; i<NUM_RIDES; i++){
					f->rides[i].status = OFF_LINE;
				}
				sem_post(&serverBusyIndicator);
				break;
			}



	
		}
		if(buffer[0] == SHUTDOWN){
			break;
		}

		printf("--SERVER: Closing client connection.\n");
		close(clientSocket); // Close this client's socket

	

    
  }
    

	//close the sockets
    close(serverSocket);
    printf("Fair SERVER: Shutting down.\n");
	pthread_exit(NULL);

}


//Sending ride names
void sendRideNames(int clientSocket, Ride *rides) {
    unsigned char buffer[4 + NUM_RIDES * RIDE_NAME_MAX_CHARS];

    // Copy the ride numbers into the buffer
    for (int i = 0; i < NUM_RIDES; i++) {
        buffer[i] = i;
    }

    // Copy the ride names into the buffer
    int offset = 4;
    for (int i = 0; i < NUM_RIDES; i++) {
        strncpy((char *)(buffer + offset), rides[i].name, RIDE_NAME_MAX_CHARS);
        offset += RIDE_NAME_MAX_CHARS;
    }

    // Send the buffer to the client
    send(clientSocket, buffer, sizeof(buffer), 0);
}

//Sending client the tickets
void sendTickets(int clientSocket, Ride *rides){
	unsigned char sendTickets[NUM_RIDES];

	for (int i=0; i< NUM_RIDES; i++){
		//Fill up the array with tickets
		sendTickets[i] = rides[i].ticketsRequired;

	}
	send(clientSocket, sendTickets, sizeof(sendTickets), 0);
}

