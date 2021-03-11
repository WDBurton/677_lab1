// The bazaar main file

#include "bazaar.h"
#include <iostream>

// Uncertain how many of these are neccessary, but including them anyway for now
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 


// The 'makePeer' function.  This will be the basis of all future p
int makePeer(struct peer){
    // This function takes the passed in struct, and using it creates a peer.  It creates the socket,
    // binds the port to it, stocks the items that it has to sell, and then follows the behavior that's
    // specified.


}





// This is the code for a buyer
int buyer(int peerId, int portNum, int otherPort){
    // The buyer function creates the socket, binds it to the given port number, and then stands by for testing.
    // For now, it's a lot of copy-pasting from other code, to ensure I can get it to work right.
    int server_fd, new_socket, valread; 
	struct sockaddr_in address;
    struct sockaddr_in neighbor;
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	char *hello = "Hello from buyer"; 
	
	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port number given
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
												&opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( portNum ); 
	
	// Forcefully attaching socket to the port number given
	if (bind(server_fd, (struct sockaddr *)&address, 
								sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

    // Added code to know who the neighbor is
    neighbor.sin_family = AF_INET;
    neighbor.sin_port = htons(otherPort);

    if (connect(server_fd, (struct sockaddr *)&neighbor, sizeof(neighbor)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 

	send(server_fd, hello , strlen(hello) , 0 ); 
	printf("Hello message sent from buyer\n"); 
	valread = read( server_fd , buffer, 1024); 
	printf("%s\n",buffer ); 
	return 0; 


}


// This is the code for a seller
int seller(int peerId, int portNum, int otherPort){
    // The seller function creates the socket, binds it to the given port number, and then stands by for testing.
    // For now, a lot of copying until I can make sure it works.
    int server_fd, new_socket, valread; 
	struct sockaddr_in address;
    struct sockaddr_in neighbor; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	char *hello = "Hello from seller"; 
	
	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port number given 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
												&opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( portNum ); 
	
	// Forcefully attaching socket to the port number given 
	if (bind(server_fd, (struct sockaddr *)&address, 
								sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 

    
    // Added code to know who the neighbor is
    neighbor.sin_family = AF_INET;
    neighbor.sin_port = htons(otherPort);

    
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen))<0) 
	{ 
		perror("accept"); 
		exit(EXIT_FAILURE); 
	} 
	valread = read( new_socket , buffer, 1024); 
	printf("%s\n",buffer ); 
	send(new_socket , hello , strlen(hello) , 0 ); 
	printf("Hello message sent from seller\n"); 
	return 0; 

}





void testMultiCompile(){
    std::cout << "Multi compile works!";
}