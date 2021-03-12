// Bazaar's .h function
#ifndef BAZAAR_H
#define BAZAAR_H

// The 'peer' structure, used for constructing the p2p network.
struct peer{
    int ID;             // The ID of the peer in question
    int type;           // The type of peer, whether buyer, seller, or both
    int port;           // The port of the peer, for binding
    int neighborPort;   // The port of the neighbor, will be replaced when having more than 1 neighbor
    int numFish;        // The number of fish available to sell
    int numBoar;        // '             boar                  '
    int numDuck;        // '             duck                  '
    int behavior;       // The behavior, if specified, for testing purposes.  Otherwise it will be 0.
};


#define MAX_HOPS = 10

// The 'message' structure, used for messages between peers in the bazaar.  A basic struct/union/struct.
struct bazaarMessage{
    int type;                       // The type of message; this will be determined using preprocessor constants.
    union message{                      // A union of possible messages.
        struct sellerSeek{                   // A flood, looking for a seller of a good
            int buyerID;                        // The ID of the buyer
            int goodType;                       // The type of good being sought out.
            int hopNum;                         // The number of hops allowed.
            int prevHops[10];                   // Past hop IDs; Need to edit when figure out better method.
        } sellerSeek;
        struct buyerFound{                  // A return to the flood, letting a seller know the buyer has been found
            int buyerID;                        // The ID of the buyer
            int sellerID;                       // The ID of the seller
            int hopNum;                         // Used with prevHops to get back to buyer
            int prevHops[10];                   // The previous hop IDs;  Need to edit when figure out/need better method           
        } buyerFound;
        struct buy{                         // A command to buy 1 of a particular good.
            int goodType;                       // The good type -- defined with preproccesor constants.
        } buy;

    } message;
};

struct test{
    int a;
    union tu{
        struct{
            int a;
            int b;
        } ta;
        struct tb{
            char a;
            char b;
        };
    } tu;
};



// The functions in bazaar
void testMultiCompile();
int buyer(int peerId, int portNum, int otherPort);
int seller(int peerId, int portNum, int otherPort);
int mOne_sellFish( struct peer peerDesc, struct sockaddr_in address, int peerSocket );
int mOne_buyFish( struct peer peerDesc, struct sockaddr_in address, int peerSocket );
int makePeer(struct peer);



// Constants for useage everywhere

// The constants that determine what type of peer it is
#define BUY 0
#define SELL 1
#define BOTH 2


// The constants that define what type of command is being sent over the network.
#define MESSAGE_SELLER_SEEK 0
#define MESSAGE_SELLER_FOUND 1
#define MESSAGE_BUY 2



// The constants that determine what type of goods are avaiable for purchase or selling, as well as
// how many of them there are
#define FISH 0
#define BOAR 1
#define DUCK 2




// Behavior constants
#define BEHAVE_NULL 0
#define BEHAVE_M1_SELL_FISH 1
#define BEHAVE_M1_BUY_FISH 2
#define BEHAVE_M1_SELL_BOAR 3
#define BEHAVE_M1_BUY_BOAR 4
#define BEHAVE_M1_SELL_ANY 5
#define BEHAVE_M1_BUY_ANY 6

#define BEHAVE_TEST_X1 100     // Basic test to send a message
#define BEHAVE_TEST_X2 101     // Basic test to recieve a message


#endif