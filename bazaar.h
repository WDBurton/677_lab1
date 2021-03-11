// Bazaar's .h function
#ifndef BAZAAR_H
#define BAZAAR_H

// The 'peer' structure, used for constructing the p2p network.
struct peer{
    int type;           // The type of peer, whether buyer, seller, or both
    int port;           // The port of the peer, for binding
    int neighborPort;   // The port of the neighbor, will be replaced when having more than 1 neighbor
    int numFish;        // The number of fish available to sell
    int numBoar;        // '             boar                  '
    int numDuck;        // '             duck                  '
    int behavior;       // The behavior, if specified, for testing purposes.  Otherwise it will be 0.
};




// The functions in bazaar
void testMultiCompile();
int buyer(int peerId, int portNum, int otherPort);
int seller(int peerId, int portNum, int otherPort);
int makePeer(struct peer);



// Constants for useage everywhere

// The constants that determine what type of peer it is
#define BUY 0
#define SELL 1
#define BOTH 2

// The constants that determine what type of goods are avaiable for purchase or selling, as well as
// how many of them there are
#define FISH 0
#define BOAR 1
#define DUCK 2

// Behavior constants
#define BEHAVE_NULL 0;
#define BEHAVE_M1_SELL_FISH 1;
#define BEHAVE_M1_BUY_FISH 2;
#define BEHAVE_M1_SELL_BOAR 3;
#define BEHAVE_M1_BUY_BOAR 4;
#define BEHAVE_M1_SELL_ANY 5;
#define BEHAVE_M1_BUY_ANY 6;


#endif