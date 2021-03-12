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


// The 'makePeer' function.  This will be the basis of all future peers
int makePeer(struct peer peerDesc){
    // This function takes the passed in struct, and using it creates a peer.  It creates the socket,
    // binds the port to it, stocks the items that it has to sell, and then follows the behavior that's
    // specified.

    // Initial variable declaration
    int peer_fd, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Creates the initial socket file descriptor
    if ((peer_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Socket Creation Failure!");
        exit(EXIT_FAILURE);
    }
    // Surgically prepare for port attachment
    if( setsockopt( peer_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt) )){
        perror("setsockopt Failure!");
        exit(EXIT_FAILURE);
    }
    // Modify address for port attachment
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons(peerDesc.port);
    // ATTACH THE PORT!
    if( bind( peer_fd, (struct sockaddr *)&address, sizeof(address) ) < 0 ){
        perror("Port Bind Failure!");
        exit(EXIT_FAILURE);
    }

    // We should now have a basic socket.  Now, we send them off to their appropiate behavior.
    if( peerDesc.behavior == BEHAVE_M1_BUY_FISH ) {
        mOne_buyFish(peerDesc, address, peer_fd);
    }
    else if ( peerDesc.behavior == BEHAVE_M1_SELL_FISH ){
        mOne_sellFish(peerDesc, address, peer_fd);
    }
    else if( peerDesc.behavior == BEHAVE_TEST_X1 ){

        int x2 = 0;
        // Prepare neighbor address
        struct sockaddr_in testx2_addr;
        testx2_addr.sin_family = AF_INET;
        testx2_addr.sin_port = htons(peerDesc.neighborPort);
        char* x2_hello = "Hello from test x1!";

        // Attempt to connect to neighbor
        if( connect( peer_fd, (struct sockaddr *)&testx2_addr, sizeof(testx2_addr)) < 0){
            perror("Connect Failure!");
            exit(EXIT_FAILURE);
        }

        send(peer_fd, x2_hello, strlen(x2_hello), 0);

        std::cout << "x2_hello sent\n";
        return 0;


    }
    else if ( peerDesc.behavior == BEHAVE_TEST_X2 ){

        int x1 = 0;
        int valRead = 0;
        char buffer[1024] = {0};

        if( listen(peer_fd, 3) < 0){
            perror("Listen Failure!");
            exit(EXIT_FAILURE);
        }

        if( (x1 = accept(peer_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0 ){
            perror("Accept Failure!");
            exit(EXIT_FAILURE);
        }

        valRead = read( x1, buffer, 1024);
        printf("%s\n", buffer);
        printf("x1 hello message recieved\n");
        return 0;
    }


}


// The 'sendMessage' function.  Sends out a bazaar message; creates the socket and sends it out.
int sendMessage(struct bazaarMessage toSend, struct sockaddr_in targetAddr ){
    
    // Creates the socket
    int sendSocket = socket(AF_INET, SOCK_STREAM, 0);
    if( sendSocket < 0 ){
        perror( "SEND MESSAGE func failed to make socket" );
        exit(EXIT_FAILURE);
    }
    
    // Now for the connection!
    if( connect( sendSocket, (struct sockaddr *)&targetAddr, sizeof(targetAddr) ) < 0 ){
        perror("SEND MESSAGE func failed to connect to neighbor");
        exit(EXIT_FAILURE);
    }

    send( sendSocket, &toSend, sizeof(toSend), 0 );

    close( sendSocket );
}

// The 'sellerSeek' function.  Sends out a sellerSeek message.
int sellerSeek(struct peer peerDesc, struct sockaddr_in address){
    bool thisDebug = false;      // A simple debug variable used in functions that are not working for some reason.

    // This simply creates a message, and sends it out to all neighbors.
    // The buyerID and first prevHops are both the peer's ID, with the goodType being what the peer
    // wants to buy.
    struct bazaarMessage toSend;
    toSend.type = MESSAGE_SELLER_SEEK;
    toSend.message.sellerSeek.buyerID = peerDesc.ID;
    toSend.message.sellerSeek.goodType = peerDesc.buyType;
    toSend.message.sellerSeek.hopNum = 1;                       // TODO: Make this a variable that can be assigned at very start.
    toSend.message.sellerSeek.prevHops[0] = peerDesc.ID;

    if(thisDebug) std::cout << "SellerSeek message type: " << toSend.type
                            << " | SellerSeek good type: " << toSend.message.sellerSeek.goodType << "\n";

    // Now we need to actually make the connection to all of the neighbors, and send the message out.
    // TODO: This will be a for loop over all neighbors.

    // Make the neighbor address
    struct sockaddr_in neighbor;
    neighbor.sin_family = AF_INET;
    neighbor.sin_port = htons(peerDesc.neighborPort);  //TODO: Currently, this relies on there being only one neighbor.  Fix that.

    // Send the message!
    sendMessage( toSend, neighbor );


}


// The 'sellerFound' function.  Sends out a sellerFound message.
int sellerFound(struct peer peerDesc, struct bazaarMessage seekerMessage, struct sockaddr_in address){
    // This works similarly to the sellerSeek function.  It's send out from a seller once it's confirmed it has the goods to sell.
    struct bazaarMessage toSend;
    toSend.type = MESSAGE_SELLER_FOUND;
    toSend.message.sellerFound.hopNum = seekerMessage.message.sellerSeek.hopNum;
    toSend.message.sellerFound.buyerID = seekerMessage.message.sellerSeek.buyerID;
    toSend.message.sellerFound.sellerID = peerDesc.ID;

    // For the neighbors, I need to copy them over iteratively.
    for( int i = 0; i < MAX_NEIGHBORS; i++ ){
        toSend.message.sellerFound.prevHops[i] = seekerMessage.message.sellerSeek.prevHops[i];
    }

    // Now that we have the message, it's time to send it out.
}
// The 'buy'
// The 'buyAck'




// The milestone one sell fish function
int mOne_sellFish( struct peer peerDesc, struct sockaddr_in address, int peerSocket ){
    // This operates a socket that will sell fish.  The peer will start by listening for somebody that wants fish;
    // it will reply with its connection, and then it will proceed to sell fish.  When it is out of fish, it will
    // then reject the purchase, restock, and repeat.

    // For the purposes of the milestone, it will empty itself five times before finishing itself, and every time
    // it will print a message.


    // As this is very well defined, no threads will be used at this moment in time.  Just pinging back and forth
    // using the message structs.

    // Variable creation; the to-be recieved message, and the empty socket.
    int buyerSocket, valRead;
    int adderLen = sizeof(address);
    struct bazaarMessage buyerMessage, sellerMessage;

    // Now we start the loop.
    //for( int i = 1; i > 0; i ++ ){

        // First, we try to listen.
        if( listen(peerSocket, 3) < 0 ){
            perror("Milestone 1, fish test, seller can not listen to buyer!");
            exit(EXIT_FAILURE);
        }

        if(debugAll) std::cout << "Fish seller listened to buyer\n";

        // Then we accept.
        if( (buyerSocket = accept( peerSocket, (struct sockaddr *)&address, (socklen_t *)&adderLen) ) < 0 ){
            perror("Milestone 1, seller can not accept buyer!");
            exit(EXIT_FAILURE);
        }
        
        if(debugAll) std::cout << "Fish seller accepted buyer\n";

        // Now that we're connected, time to see if we can recieve the message
        valRead = read( buyerSocket, &buyerMessage, sizeof(buyerMessage) );

        // Now, we check to ensure that we recieved the right message, and that it's for the right type of
        // product.
        if( buyerMessage.type != MESSAGE_SELLER_SEEK || buyerMessage.message.sellerSeek.goodType != FISH ){
            perror("Milestone 1, incorrect seller seek message recieved!");
            std::cout << "Mesage type: " << buyerMessage.type << "\n";
            std::cout << "Good type: " << buyerMessage.message.sellerSeek.goodType << "\n";
            exit(EXIT_FAILURE);
        }
        /*
        // If we reach here, then we have obtained the correct message.  As such, it's time to return with
        // one of our own.
        sellerMessage.type = MESSAGE_SELLER_FOUND;
        sellerMessage.message.sellerFound.buyerID = buyerMessage.message.sellerSeek.buyerID;
        sellerMessage.message.sellerFound.hopNum = buyerMessage.message.sellerSeek.hopNum;
        sellerMessage.message.sellerFound.prevHops[0] = buyerMessage.message.sellerSeek.prevHops[0];
            // The line above will need to be adjusted for later iterations of this
        sellerMessage.message.sellerFound.sellerID = peerDesc.ID;*/

        // Now to connect back to the buyer
        //if( connect( peer))  NOTE: PAUSED HERE
    //}

}

// The milestone one buy fish function
int mOne_buyFish( struct peer peerDesc, struct sockaddr_in address, int peerSocket ){
    // This operates a socket that will buy fish.  It will start by seeking a neighbor to buy fish from with a
    // hop length of 1 -- when it gets a reply, it will proceed to buy fish until it gets rejected; at this point,
    // it will start from the beginning.

    // For the purposes of the milestone, it will buy out the buyer five times before finishing itself, and every
    // time it will print a message.


    // As we know how this will go, this will not use threads.  For now, the goal is a basic ping, using the message
    // structs.

    // Now that I have the sellerSeek function, going to attempt to use that.
    peerDesc.buyType = FISH;
    sellerSeek( peerDesc, address );


    /*struct bazaarMessage buyerMessage;
    buyerMessage.type = MESSAGE_SELLER_SEEK;

    buyerMessage.message.sellerSeek.buyerID = peerDesc.ID;
    buyerMessage.message.sellerSeek.goodType = FISH;
    buyerMessage.message.sellerSeek.hopNum = 0;
    buyerMessage.message.sellerSeek.prevHops[0] = peerDesc.ID;

    // Now, the structure for the neighbor's socket address.  We base it off of address, because a lot of the
    // details will be the same.
    struct sockaddr_in neighborAddr = address;
    neighborAddr.sin_port = htons(peerDesc.neighborPort);

    if(debugAll) std::cout << "Struct created; connecting to seller\n";

    
    for( int i = 1; i > 0; i ++ ){

        // Now I see if I can connect.
        if( connect( peerSocket, (struct sockaddr *)&neighborAddr, sizeof(neighborAddr) ) < 0 ){
            perror("Milestone 1, fish test, buyer can not connect to seller!");
            exit(EXIT_FAILURE);
        }

        // Presuming we do, then we're going to send a basic message to the seller!
        send( peerSocket, &buyerMessage, sizeof(buyerMessage), 0 );
        if(debugAll) std::cout << "Struct sent to buyer\n";
    }*/


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