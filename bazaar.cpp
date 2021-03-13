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
    int sizeRead = 0;
    int tempSocket = 0;
    //struct bazaarMessage toRead;
    int addrLen = sizeof(address);

    if(debugThis) std::cout << "peerListen start, listening in on port " << peerDesc->port << "\n";

    
    // Listen!
    if( listen(peerDesc->socket, 50) < 0 ){
        perror("peerListen function failed");
        exit(EXIT_FAILURE);
    }

    // Listens for all eternity!
    while(true){
        //sleep(2);

        if( debugThis ) std::cout << "PEERLISTEN:  Peer " << peerDesc->ID << " listening on port "
                                    << peerDesc->port << " | " << htons(peerDesc->port) << "\n"
                                    << "CONT -- sin_family: " << address.sin_family << "\n"
                                    << "CONT -- s_addr: " << address.sin_addr.s_addr << "\n"; 
        
        tempSocket = 0;
        // Listen!
        /*if( listen(peerDesc->socket, 50) < 0 ){
            perror("peerListen function failed");
            exit(EXIT_FAILURE);
        }*/

        if( debugThis ) std::cout << "PEERLISTEN:  Peer " << peerDesc->ID << " accepting...\n";

        // If something is heard, time to deal with it!  Get message and spin off thread!
        if( (tempSocket = accept(peerDesc->socket, (struct sockaddr *)&address, (socklen_t *)&addrLen)) < 0 ){
            perror("peerListen failed to accept connection!");
            exit(EXIT_FAILURE);
        }

        if( debugThis ) std::cout << "PEERLISTEN:  Peer " << peerDesc->ID << " reading...\n";
        struct bazaarMessage toRead;

        // This is the reading...
        sizeRead = read( tempSocket, &toRead, sizeof(toRead) );
        if( sizeRead < 0 ){
            perror("peerListen read has failed");
            exit(EXIT_FAILURE);
        };
        close(tempSocket);

        if( debugThis ) std::cout << "PEERLISTEN:  Peer " << peerDesc->ID << " has read "
                                    << sizeRead << " Bytes, processing...\n";

        //... And then spin off the thread, detatch it, close the socket, and restart.
        std::thread t(peerReceive, peerDesc, address, toRead);
        t.detach();
        if( debugThis ) std::cout << "PEERLISTEN:  Peer " << peerDesc->ID << " finished; restarting loop.\n";
    }

    std::cout << "ERROR ERROR ERROR ERROR\n"
                << "peerListen should never reach here!\n";
}


// The 'peerRecieve' function.  When something is heard on a socket, it deals with the message
// it was sent.
int peerReceive( struct peer *peerDesc, struct sockaddr_in address, struct bazaarMessage toRespond ){
    bool debugThis = false;
    bool thisDebugMax = false;
    //bool wubba = true;
    sleep(1);
    /*if(thisDebugMax) std::cout << "REC: START\n";
    if(thisDebugMax) printPeerDesc(*peerDesc);*/
    if(debugThis){
        std::cout << "\n";//"REC: START\n";
    }

    // This is basically just a massive switch statement.
    switch( toRespond.type ){
        case MESSAGE_BUY:                   // The case for the buy message
            //if(debugThis) std::cout << "REC: Message received: Buy, starting Buy Ack\n";
            buyAck(peerDesc, toRespond.message.buy.goodType);
            break;
        case MESSAGE_BUY_ACK:
            //if(debugThis) std::cout << "REC: Message received: Buy Ack\n";
            if(toRespond.message.buyAck.numBought > 0){
                // If buyTotal was not 0, buy more!
                //if(debugThis) std::cout << "REC: Starting Buy\n";
                buy(*peerDesc);
            }else{      
                // If buyTotal was 0, wait one second, and send out a message for a new seller!
                /*std::cout   << "REC: The seller ran out of goods...\n"
                            << "REC: Waiting two seconds before looking for new seller.\n";
                //sleep(2);
                std::cout << "REC: Wait done!  Setting value to look for new seller!\n";*/
                //peerDesc->sellerOut = true;
                //buy(*peerDesc);
                //if(debugThis) std::cout << "REC: Starting Seller Seek\n";
                sellerSeek(*peerDesc, address);          
            }
            break;
        case MESSAGE_SELLER_FOUND:          // The case for seller found message
            //if(debugThis) std::cout << "REC:  Message received: Seller Found\n";
            buy(*peerDesc);
            //sellerSeek(*peerDesc, address);
            break;
        case MESSAGE_SELLER_SEEK:           // The case for seller seek message
            // If the peer has a good to sell, then it responds with seller found.
            // Otherwise, it sends the message to all of its neighbors.
            //if (debugThis) std::cout << "REC:  Message received: Seller Seek\n";
            if( true
                /*toRespond.message.sellerSeek.goodType == FISH && peerDesc->numFish > 0 ||
                toRespond.message.sellerSeek.goodType == BOAR && peerDesc->numBoar > 0 ||
                toRespond.message.sellerSeek.goodType == DUCK && peerDesc->numDuck > 0*/
            ){
                // It wants a good we have!  Time to send it back!
                //if (debugThis) std::cout << "REC:  Starting sellerFound\n";
                sellerFound(*peerDesc, toRespond, address);
            } else{
                std::cout << "TODO: Seller seek else statement in peerReceive\n";
            }
            break;
        default:
            std::cout << "---------------------------------\n"
                      << "ERROR: UNKNOWN MESSAGE TYPE\n";
    }
}



// The 'sendMessage' function.  Sends out a bazaar message; creates the socket and sends it out.
int sendMessage(struct bazaarMessage toSend, struct sockaddr_in targetAddr ){
    bool debugThis = false;
    bool debugThisMax = true;
    
    // Creates the socket
    int sendSocket = socket(AF_INET, SOCK_STREAM, 0);
    if( sendSocket < 0 ){
        perror( "SEND MESSAGE func failed to make socket" );
        exit(EXIT_FAILURE);
    }

    if(debugThis) std::cout << "SEND MESSAGE:  Attempting to send message...\n";
    if(debugThisMax) printBazaarMessage(toSend);
    if(debugThisMax) std::cout << "SEND MESSAGE:  Sending message to: " << targetAddr.sin_port << "\n";

    targetAddr.sin_family = AF_INET;
    targetAddr.sin_addr.s_addr = INADDR_ANY;
    int len = sizeof(targetAddr);
    
    // Now for the connection!
    if( connect( sendSocket, (struct sockaddr *)&targetAddr, /*sizeof(targetAddr)*/len ) < 0 ){
        perror("SEND MESSAGE func failed to connect to neighbor");
        exit(EXIT_FAILURE);
    }

    if(debugThis) std::cout << "SEND MESSAGE:  Sending message of type: "
                            << toSend.type << "\n";

    send( sendSocket, &toSend, sizeof(toSend), 0 );

    if(debugThis) std::cout << "SEND MESSAGE:  Message sent\n";

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

    if(thisDebug) std::cout << "SELLERSEEK good type: " << toSend.message.sellerSeek.goodType << "\n"
                            << "To port: " << peerDesc.neighborPort <<"\n";

    // Now we need to actually make the connection to all of the neighbors, and send the message out.
    // TODO: This will be a for loop over all neighbors.

    // Make the neighbor address
    struct sockaddr_in neighbor;
    //neighbor.sin_family = AF_INET;
	//neighbor.sin_addr.s_addr = INADDR_ANY; 
    neighbor.sin_port = htons(peerDesc.neighborPort);  //TODO: Currently, this relies on there being only one neighbor.  Fix that.

    // Send the message!
    sendMessage( toSend, neighbor );
}


// The 'sellerFound' function.  Sends out a sellerFound message.
int sellerFound(struct peer peerDesc, struct bazaarMessage seekerMessage, struct sockaddr_in address){
    //bool debugThis = true;
    // This works similarly to the sellerSeek function.  It's send out from a seller once it's confirmed it has the goods to sell.
    struct bazaarMessage toSend;
    toSend.type = MESSAGE_SELLER_FOUND;
    toSend.message.sellerFound.hopNum = seekerMessage.message.sellerSeek.hopNum;
    toSend.message.sellerFound.buyerID = seekerMessage.message.sellerSeek.buyerID;
    toSend.message.sellerFound.sellerID = peerDesc.ID;

    // For the neighbors, I need to copy them over iteratively.
    /*for( int i = 0; i < MAX_NEIGHBORS; i++ ){
        toSend.message.sellerFound.prevHops[i] = seekerMessage.message.sellerSeek.prevHops[i];
    }*/

    // Now that we have the message, it's time to send it out.  We need to make the address.
    struct sockaddr_in neighbor;
    //neighbor.sin_family = AF_INET;
	//address.sin_addr.s_addr = INADDR_ANY;
    neighbor.sin_port = htons(peerDesc.neighborPort);  // TODO:  Fix for more than one neighbor.

    sendMessage( toSend, neighbor );
}

// The 'buy' function.  Purchases a thing!  Currently relies on the fact that there's only one neibhro.
int buy(struct peer peerDesc){
    // We need to make the message, and the nsend the message.  It's pretty simple.
    struct bazaarMessage toSend;
    toSend.type = MESSAGE_BUY;
    toSend.message.buy.goodType = peerDesc.buyType;

    // Message now made, time to construct neighbor address
    struct sockaddr_in neighbor;
    neighbor.sin_port = htons(peerDesc.neighborPort);
    neighbor.sin_family = AF_INET;

    // Send the message
    sendMessage( toSend, neighbor );
}

// The 'buyAck' function.  Gives away a thing!  Currently relies on the fact there's only one neighbor.
int buyAck(struct peer *peerDesc, int goodType){
    bool thisDebug = true;

    bool buyGood = true;
    struct bazaarMessage toSend;
    toSend.type = MESSAGE_BUY_ACK;

    struct sockaddr_in buyer;
    buyer.sin_port = htons(peerDesc->neighborPort);
    buyer.sin_family = AF_INET;

    if (thisDebug) std::cout << "BUYACK:  Starting purchase\n";
    // First off, decriment the type we're selling.
    switch(goodType){
        // For each, if we have enough, we decrement; if we don't, then we set 'buyGood' to false, instead.
        case FISH:
            if(peerDesc->numFish < 1){
                if(thisDebug) std::cout << "BUYACK:  FISH out\n";
                buyGood = false;
                break;
            }
                if(thisDebug) std::cout << "BUYACK:  FISH from " << peerDesc->numFish << " to " << peerDesc->numFish-1 << "\n";
            peerDesc->numFish -= 1;
            break;
        case BOAR:
            if(peerDesc->numBoar < 1){
                if(thisDebug) std::cout << "BUYACK:  BOAR out\n";
                buyGood = false;
                break;
            }
            peerDesc->numBoar -= 1;
            break;
        case DUCK:
            if(peerDesc->numDuck < 1){
                if(thisDebug) std::cout << "BUYACK:  DUCK out\n";
                buyGood = false;
                break;
            }
            peerDesc->numDuck -= 1;
            break;
    }

    // Check to make sure the purchase went through!
    if(buyGood){
        if(thisDebug) std::cout << "BUYACK:  Buy is good!\n";
        toSend.message.buyAck.numBought = 1;
    } else{
        if(thisDebug) std::cout << "BUYACK:  Buy is bad...\n";
        toSend.message.buyAck.numBought = 0;
    }

    // Send the message!
    sendMessage( toSend, buyer );

    // Finally, a behavior check for some of the milestone requirements
    if( !buyGood && peerDesc->behavior == BEHAVE_M1_SELL_FISH){
        // If we're doing milestone one, and we're empty, we need to restock!
        std::cout << "BUYACK:  Restocking fish for milestone 1.  Restocking.\n";
        peerDesc->numFish += 6;
        std::cout << "BUYACK:  Restocked.  Fish num is now: " << peerDesc->numFish <<"\n";
    };
}




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
    peerDesc.numFish = 0;
    peerDesc.numBoar = 0;
    peerDesc.numFish = 0;
    struct peer peerCopy = peerDesc;
    struct bazaarMessage buyerMessage, sellerMessage;

    std::thread sellerListen(peerListen, &peerDesc, address);
    //sellerListen.detach();
    //peerListen(&peerDesc, address);
    while(true){
        sleep(7);
        //std::cout << "Seller peer check:";
        //printPeerDesc(peerDesc);
        //printPeerDesc(peerCopy);
        //std::cout << "Is the seller awake...?  " << sellerListen.joinable() << "\n";
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
    peerDesc.sellerOut = false;
    peerDesc.numFish = 0;
    peerDesc.numBoar = 0;
    peerDesc.numFish = 0;
    struct peer peerCopy = peerDesc;
    std::thread buyerListen(peerListen, &peerDesc, address);
    //buyerListen.detach();

    sleep(1);
    std::cout << "Time to start the buying of fish!\n";

    sellerSeek( peerDesc, address );
    //std::cout << "Is listening?  " << buyerListen.joinable() << "\n";
    while(true){
        sleep(7);
        if(peerDesc.sellerOut){
            peerDesc.sellerOut = false;
            std::cout << "MILESTONE 1, BUY FISH:  Starting new sellerSeek\n";
            sellerSeek( peerCopy, address );
        } else {
            // I'm going to assume something has gone wrong if we get here, for testing purposes.
            //buyerListen.std::terminate();
            //std::terminate() buyerListen;
            //std::cout << "Is the buyer still awake...?  "  << buyerListen.joinable() << "\n";
            //std::cout << "Buyer check:";
            //printPeerDesc(peerDesc);
            //printPeerDesc(peerCopy);
        }
    };



}

/***********************************************************************************/
// Testing functions

void printBazaarMessage(struct bazaarMessage toPrint){
    
    std::cout << "\n--------------------------------------\n";
    std::cout << "Message Type:  ";
    switch(toPrint.type){
        case MESSAGE_BUY:
            std::cout   << "Buy\n"
                        << "Buy type: " << toPrint.message.buy.goodType;
            break;
        case MESSAGE_BUY_ACK:
            std::cout   << "Buy Ack\n"
                        << "Buy count: " << toPrint.message.buyAck.numBought;
            break;
        case MESSAGE_SELLER_FOUND:
            std::cout   << "Seller Found\n"
                        << "Seller ID: " << toPrint.message.sellerFound.sellerID << "\n"
                        << "Buyer ID: " << toPrint.message.sellerFound.buyerID << "\n"
                        << "Hop Number: "  << toPrint.message.sellerFound.hopNum;
            break;
        case MESSAGE_SELLER_SEEK:
            std::cout   << "Seller Seek\n"
                        << "Seller ID: " << toPrint.message.sellerSeek.buyerID << "\n"
                        << "Buy type: " << toPrint.message.sellerSeek.goodType << "\n"
                        << "Hop Number: " << toPrint.message.sellerSeek.hopNum;
            break;
    }
    std::cout << "\n";
}

void printPeerDesc( struct peer toPrint ){
    std::cout << "\n--------------------------------------\n"
                << "PEER PEER PEER PEER PEER PEER PEER\n"
                << "ID: " << toPrint.ID << "\n"
                << "Neighbor Port: " << toPrint.neighborPort << "\n"
                << "Socket: "  << toPrint.socket << "\n"
                << "BuyType: " << toPrint.buyType << "\n"
                << "Behavior: " << toPrint.behavior << "\n"
                << "Seller Out: " << toPrint.sellerOut << "\n"
                << "Type: " << toPrint.type << "\n"
                << "Fish/Boar/Ducks: " << toPrint.numFish << "/" << toPrint.numBoar << "/" << toPrint.numDuck
                << "\n--------------------------------------\n";


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