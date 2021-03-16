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
int makePeer(struct peer *peerDesc){
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
    if(peerDesc->neighborPort == -1){
        peerDesc->port = 8080+peerDesc->ID;
    }
	address.sin_port = htons(peerDesc->port);
    // ATTACH THE PORT!
    if( bind( peer_fd, (struct sockaddr *)&address, sizeof(address) ) < 0 ){
        perror("Port Bind Failure!");
        exit(EXIT_FAILURE);
    }

    // Storing the socket fd in the peerDesc
    peerDesc->socket = peer_fd;

    // We should now have a basic socket.  Now, we send them off to their appropiate behavior.
    if( peerDesc->behavior == BEHAVE_NULL ){
        // This is a general purpose portion, thus why it's right on top.
        // If we're buying stuff, then it will spin off a thread for a delayed seller seek and
        // detatch it.  And regardless, it shall start a peerListen for the peer.
        if(peerDesc->buyType != NONE){
            std::thread t1(delayedSellerSeek, *peerDesc);
            t1.detach();
        }
        peerListen(peerDesc, address);
    }
    else if( peerDesc->behavior == BEHAVE_M1_BUY_FISH ) {
        // This is very similar to normal behavior.  Just set it off to go.
        std::thread t1(delayedSellerSeek, *peerDesc);
        t1.detach();
        peerListen(peerDesc, address);
    }
    else if ( peerDesc->behavior == BEHAVE_M1_SELL_FISH ){
        // This is very similar to normal behavior.  Just set up a listen, nothing else.
        peerListen(peerDesc, address);
    }
    else if(peerDesc->behavior == BEHAVE_M1_NO_BUY_BOAR){
        // Though it does technically set up a listen, the listen will never recieve anything.
        // It also goes through m1_noBuyBoar; Which does 'seeks' every five seconds.
        // Thus a fork, to keep it all attached, so that when I kill the function it all dies.
        
        std::thread t1(peerListen, peerDesc, address);
        while(true){
            sleep(5);
            address.sin_port = htons(peerDesc->neighborPort);
            sellerSeek(*peerDesc, address);
        }
        t1.join();
        
    }
    else if(peerDesc->behavior == BEHAVE_M1_SELL_BOAR){
        peerListen(peerDesc, address);
    }
    else if( peerDesc->behavior == BEHAVE_TEST_X1 ){

        int x2 = 0;
        // Prepare neighbor address
        struct sockaddr_in testx2_addr;
        testx2_addr.sin_family = AF_INET;
        testx2_addr.sin_port = htons(peerDesc->neighborPort);
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
    else if ( peerDesc->behavior == BEHAVE_TEST_X2 ){

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
    bool debugThis = false;
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
    // This entire function is basically a massive switch statement.  Depending on the type of message
    // it recieves, it behaves accordingly.  There are multiple outputs to help show what's going on
    // if the user wants to see, and it can slow it down for demonstration purposes.

    // An initial set of debug variables and printing.
    bool debugThis = false || peerDesc->showWork;
    bool thisDebugMax = false;
    if(peerDesc->showWork) sleep(1);
    if(thisDebugMax || debugThis) std::cout << "REC: START\n";
    if(peerDesc->showWork) std::cout << "REC: PEER sender ID: " << peerDesc->ID << "\n";
    if(thisDebugMax) printPeerDesc(*peerDesc);

    // This is basically just a massive switch statement.
    switch( toRespond.type ){
        case MESSAGE_BUY:                   // The case for the buy message
            if(debugThis) std::cout << "REC: Message received: Buy, starting Buy Ack\n";
            buyAck(peerDesc, toRespond.message.buy.goodType);
            break;
        case MESSAGE_BUY_ACK:
            if(debugThis) std::cout << "REC: Message received: Buy Ack\n";
            if(toRespond.message.buyAck.numBought > 0){
                // If buyTotal was not 0, buy more!
                if(debugThis) std::cout << "REC: Starting Buy\n";
                buy(*peerDesc);
            }else{      
                // If buyTotal was 0, wait one second, and send out a message for a new seller!
                if(debugThis) std::cout << "REC: The seller ran out of goods; starting sellerSeek\n";
                sellerSeek(*peerDesc, address);          
            }
            break;
        case MESSAGE_SELLER_FOUND:          // The case for seller found message
            if(debugThis) std::cout << "REC: Message received: Seller Found\n";
            buy(*peerDesc);
            //sellerSeek(*peerDesc, address);
            break;
        case MESSAGE_SELLER_SEEK:           // The case for seller seek message
            // If the peer has a good to sell, then it responds with seller found.
            // Otherwise, it sends the message to all of its neighbors.
            if (debugThis) std::cout << "REC: Message received: Seller Seek\n";
            if( 
                (toRespond.message.sellerSeek.goodType == FISH && peerDesc->numFish > 0) ||
                (toRespond.message.sellerSeek.goodType == BOAR && peerDesc->numBoar > 0) ||
                (toRespond.message.sellerSeek.goodType == DUCK && peerDesc->numDuck > 0)
            ){
                // It wants a good we have!  Time to send it back!
                if (debugThis) std::cout << "REC: Starting sellerFound\n";
                sellerFound(*peerDesc, toRespond, address);
            } else{
                // If we get here, first, we need to confirm that it's not on its last hop.
                // If it is, well, too bad, so sad, no go.
                if(toRespond.message.sellerSeek.hopNum > 0){
                    if (debugThis) std::cout << "REC: SellerSeek message to be continued\n";
                    contSellerSeek(*peerDesc, toRespond);
                }else{
                    if(debugThis) std::cout << "REC: SellerSeek message out of hops, do nothing\n";
                }
            }
            break;
        default:
            std::cout << "ERROR: UNKNOWN MESSAGE TYPE\n";
    }
    if(peerDesc->showWork) std::cout << "\n---------------------\n\n";
}



// The 'sendMessage' function.  Sends out a bazaar message; creates the socket and sends it out.
int sendMessage(struct bazaarMessage toSend, struct sockaddr_in targetAddr ){
    bool debugThis = false;
    bool debugThisMax = false;
    
    // Creates the socket
    int sendSocket = socket(AF_INET, SOCK_STREAM, 0);
    if( sendSocket < 0 ){
        perror( "SEND MESSAGE func failed to make socket" );
        exit(EXIT_FAILURE);
    }

    // Debug printing
    if(debugThis) std::cout << "SEND MESSAGE:  Attempting to send message...\n";
    if(debugThisMax) printBazaarMessage(toSend);
    if(debugThisMax) std::cout << "SEND MESSAGE:  Sending message to: " << targetAddr.sin_port << "\n";

    // Ensures the address works.
    targetAddr.sin_family = AF_INET;
    // BEWARE NEXT LINE, is desturbingly important!
    targetAddr.sin_addr.s_addr = INADDR_ANY;
    
    // Now for the connection!
    if( connect( sendSocket, (struct sockaddr *)&targetAddr, sizeof(targetAddr) ) < 0 ){
        perror("SEND MESSAGE func failed to connect to neighbor");
        exit(EXIT_FAILURE);
    }

    if(debugThis) std::cout << "SEND MESSAGE:  Sending message of type: "
                            << toSend.type << "\n";

    // Sending the message
    send( sendSocket, &toSend, sizeof(toSend), 0 );

    if(debugThis) std::cout << "SEND MESSAGE:  Message sent\n";

    // Closing the socket, as the message is sent.
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
    toSend.message.sellerSeek.hopNum = 0;                       // TODO: Make this a variable that can be assigned at very start.
    toSend.message.sellerSeek.prevHops[0] = peerDesc.ID;

    //if(thisDebug) std::cout << "SELLERSEEK good type: " << toSend.message.sellerSeek.goodType << "\n"
    //                        << "To port: " << peerDesc.neighborPort <<"\n";

    // Now we need to actually make the connection to all of the neighbors, and send the message out.
    // TODO: This will be a for loop over all neighbors.

    // Make the neighbor address
    struct sockaddr_in curNeighbor;
    if(peerDesc.neighborPort == -1){
        if(thisDebug) std::cout << "S_SEEK: Start sending\n";
        // If using proper implementation, then send it out to all neighbors.
        for(int i = 0; i < peerDesc.numNeighbors; i ++){
            if(thisDebug) std::cout << "S_SEEK:  To neighbor " << i << ", ID: " << peerDesc.neighbors[i] << "\n";
            curNeighbor.sin_port = htons(8080+peerDesc.neighbors[i]);
            sendMessage(toSend, curNeighbor);        
        }
    } else{
        // Otherwise use old implemention.
        curNeighbor.sin_port = htons(peerDesc.neighborPort);
        sendMessage(toSend, curNeighbor);
    }
}

// The 'contSellerSeek' function.  Spreads out a sellerSeek message across its neighbors.
int contSellerSeek(struct peer peerDesc, struct bazaarMessage seekerMessage){
    //std::cout << "TODO: CONT. SELLER-SEEK\n";

    // First, adust the seeker message.
    seekerMessage.message.sellerSeek.hopNum --;

    //For all neighbors, check to see if it's not been sent to in the past -- if it has, don't send it.
    bool found = false;
    struct sockaddr_in neighbor;
    for(int i = 0; i < peerDesc.numNeighbors; i ++){
        for(int j = 0; j < MAX_HOPS; j ++ ){
            if(seekerMessage.message.sellerSeek.prevHops[j] == peerDesc.neighbors[i]){
                found = true;
            }
        }
        // Now we know if we've found it, if we haven't...
        if( !found ){
            neighbor.sin_port = htons(8080+peerDesc.neighbors[i]);
            seekerMessage.message.sellerSeek.prevHops[seekerMessage.message.sellerSeek.hopNum] = peerDesc.neighbors[i];
            sendMessage(seekerMessage, neighbor);
        }
        // Either way, reset bool.
        found = false;
    }
}

int contSellerFound(struct peer peerDesc, struct bazaarMessage foundMessage){
    // Decrement the foundMessage.hopNum by 1, use the new ID for the port num, and then send off to 'send'.
    std::cout << "???\n";
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
    bool thisDebug = false;

    bool buyGood = true;
    struct bazaarMessage toSend;
    toSend.type = MESSAGE_BUY_ACK;

    struct sockaddr_in buyer;
    buyer.sin_port = htons(peerDesc->neighborPort);

    if (thisDebug) std::cout << "BUYACK:  Starting purchase\n";
    
    // A switch case depending on the good type found.  Could have used an array, but this implementation is
    // more human-readable.
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
            if(thisDebug) std::cout << "BUYACK:  BOAR from " << peerDesc->numFish << " to " << peerDesc->numFish-1 << "\n";
            peerDesc->numBoar -= 1;
            break;
        case DUCK:
            if(peerDesc->numDuck < 1){
                if(thisDebug) std::cout << "BUYACK:  DUCK out\n";
                buyGood = false;
                break;
            }
            if(thisDebug) std::cout << "BUYACK:  DUCK from " << peerDesc->numFish << " to " << peerDesc->numFish-1 << "\n";
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
        peerDesc->numFish += 5;
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
    peerDesc.numFish = 1;
    peerDesc.numBoar = 0;
    peerDesc.numDuck = 0;
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
    peerDesc.numDuck = 0;
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

void delayedSellerSeek( struct peer peerDesc ){
    sleep(2);
    struct sockaddr_in address;
    sellerSeek(peerDesc, address);
}


/***********************************************************************************/
// Milestone specific functions
void m1_noBuyBoar(struct peer peerDesc){
    // This will simply send out a seller_seek every five seconds.
    struct sockaddr_in address;
    while(true){
        sleep(5);
        sellerSeek(peerDesc, address);
    }
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
                << "Fish/Boar/Ducks: " << toPrint.numFish << "/" << toPrint.numBoar << "/" << toPrint.numDuck << "\n";
    for( int i = 0; i < toPrint.numNeighbors; i++ ){
        std::cout << "Neighbor " << i+1 << ": " << toPrint.neighbors[i] << "\n";
    }

    std::cout << "\n--------------------------------------\n";


}
