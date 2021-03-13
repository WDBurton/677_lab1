// The bazaar main file

#include "bazaar.h"
#include <iostream>
#include <thread>

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

    // Storing the socket fd in the peerDesc
    peerDesc.socket = peer_fd;

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


// The 'peerListen' function.  Listens to the socket in an infinite loop, and creates a detatched thread to
// deal with whtever message it recieves.
int peerListen( struct peer *peerDesc, struct sockaddr_in address ){
    bool debugThis = true;
    int tempSocket;
    struct bazaarMessage toRead;
    int addrLen = sizeof(address);

    if(debugThis) std::cout << "peerListen start, listening in on port " << peerDesc->port << "\n";
    while(true){
        
        if(debugThis) std::cout << "peerListen loop, new start\n";
        tempSocket = 0;
        // Listen!
        if( listen(peerDesc->socket, 5) < 0 ){
            perror("peerListen function failed");
            exit(EXIT_FAILURE);
        }

        if(debugThis) std::cout << "Something has been heard\n";
        // If something is heard, time to deal with it!  Get message and spin off thread!

        if( (tempSocket = accept(peerDesc->socket, (struct sockaddr *)&address, (socklen_t *)&addrLen)) < 0 ){
            perror("peerListen failed to accept connection!");
            exit(EXIT_FAILURE);
        }

        if(debugThis) std::cout << "Something has been accepted\n";

        if( read( tempSocket, &toRead, sizeof(toRead) ) < 0 ){
            perror("peerListen read has failed");
            exit(EXIT_FAILURE);
        };

        if(debugThis) std::cout << "Something has been read\n";

        std::thread t(peerReceive, peerDesc, address, toRead);
        t.detach();
        close(tempSocket);
    }
}


// The 'peerRecieve' function.  When something is heard on a socket, it deals with the message
// it was sent.
int peerReceive( struct peer *peerDesc, struct sockaddr_in address, struct bazaarMessage toRespond ){
    bool thisDebug = false;
    if(thisDebug) std::cout << "peerRecieive start\n";

    // This is basically just a massive switch statement.
    switch( toRespond.type ){
        case MESSAGE_BUY:                   // The case for the buy message
            std::cout << "TODO: Buy in peerReceive\n";
            break;
        case MESSAGE_SELLER_FOUND:          // The case for seller found message
            std::cout << "TODO: Seller found in peerReceive\n";
            break;
        case MESSAGE_SELLER_SEEK:           // The case for seller seek message
            // If the peer has a good to sell, then it responds with seller found.
            // Otherwise, it sends the message to all of its neighbors.
            if (thisDebug) std::cout << "Message found: Seller Seek\n";
            if(
                toRespond.message.sellerSeek.goodType == FISH && peerDesc->numFish > 0 ||
                toRespond.message.sellerSeek.goodType == BOAR && peerDesc->numBoar > 0 ||
                toRespond.message.sellerSeek.goodType == DUCK && peerDesc->numDuck > 0
            ){
                // It wants a good we have!  Time to send it back!
                sellerFound(*peerDesc, toRespond, address);
            } else{
                std::cout << "TODO: Seller seek else statement in peerReceive\n";
            }
            break;
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
    bool thisDebug = true;      // A simple debug variable used in functions that are not working for some reason.

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
                            << " | SellerSeek good type: " << toSend.message.sellerSeek.goodType << "\n"
                            << "To port: " << peerDesc.neighborPort <<"\n";

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

    // Now that we have the message, it's time to send it out.  We need to make the address.
    struct sockaddr_in neighbor;
    neighbor.sin_family = AF_INET;
    neighbor.sin_port = htons(peerDesc.neighborPort);  // TODO:  Fix for more than one neighbor.

    sendMessage( toSend, neighbor );
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
    peerDesc.numFish = 5;
    struct bazaarMessage buyerMessage, sellerMessage;

    std::thread sellerListen(peerListen, &peerDesc, address);
    sellerListen.detach();
    //peerListen(&peerDesc, address);
    while(true){
        sleep(1);
    }



}

// The milestone one buy fish function
int mOne_buyFish( struct peer peerDesc, struct sockaddr_in address, int peerSocket ){
    // This operates a socket that will buy fish.  It will start by seeking a neighbor to buy fish from with a
    // hop length of 1 -- when it gets a reply, it will proceed to buy fish until it gets rejected; at this point,
    // it will start from the beginning.

    // For the purposes of the milestone, it will buy out the buyer five times before finishing itself, and every
    // time it will print a message.


    // Now that I have the sellerSeek function, going to attempt to use that.
    peerDesc.buyType = FISH;
    std::thread buyerListen(peerListen, &peerDesc, address);
    buyerListen.detach();

    sleep(1);
    std::cout << "Time to start the buying of fish!\n";

    sellerSeek( peerDesc, address );
    while(true){
        sleep(1);
    };



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