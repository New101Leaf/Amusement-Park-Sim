#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "fair.h"

// This program stops the Fair server
void main() {
	// WRITE ALL THE NECESSARY CODE

	int                 clientSocket;
	struct sockaddr_in  clientAddress;
	int                 status, bytesRcv;


	unsigned int buffer[4];
	


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

	// Connect to server
	status = connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
	if (status < 0) {
		printf("*** CLIENT ERROR: Could not connect.\n");
		exit(-1);
	}

	buffer[0] = SHUTDOWN;
	send(clientSocket,buffer,sizeof(buffer),0);

	
	close(clientSocket);  // Don't forget to close the socket !
  	printf("Sending command from stop.c.\n");




}
