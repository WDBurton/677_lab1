// The main file
#include "bazaar.h"
#include <thread>
#include <iostream>
#include <stdlib.h>


// This is the code for milestone 1 -- 2 peers, one client and one seller.
// Need to run the tests:
//      One is seller of fish other buyer of fish
//      One is seller of boar other is buyer of something other than boar; nothing is ever sold.
//      Randomly assigned buyer/seller roles, items sold forever
void milestoneOne_1(){
    std::cout << "Milestone one, test 1, start\n\n";
    struct peer peerDesc;
    peerDesc.showWork = true;
    peerDesc.port = 8080;
    peerDesc.neighborPort = 8081;
    peerDesc.numFish = 0;
    peerDesc.numBoar = 0;
    peerDesc.numFish = 0;
    peerDesc.buyType = FISH;
    peerDesc.behavior = BEHAVE_M1_BUY_FISH;
    peerDesc.ID = 42;
    std::thread thread1(makePeer, &peerDesc);

    struct peer peerDesc2 = peerDesc;
    peerDesc2.port = 8081;
    peerDesc2.neighborPort = 8080;
    peerDesc2.buyType = NONE;
    peerDesc2.behavior = BEHAVE_M1_SELL_FISH;
    peerDesc2.ID = 0;
    std::thread thread2(makePeer, &peerDesc2);

    thread1.join();
    thread2.join();
}

void milestoneOne_2(){
    std::cout << "Milestone one, test 2, start\n\n";
    struct peer peerDesc;
    peerDesc.showWork = true;
    peerDesc.port = 8080;
    peerDesc.neighborPort = 8081;
    peerDesc.numFish = 0;
    peerDesc.numBoar = 5;
    peerDesc.numFish = 0;
    peerDesc.buyType = NONE;
    peerDesc.behavior = BEHAVE_M1_SELL_BOAR;
    peerDesc.ID = 42;
    std::thread thread1(makePeer, &peerDesc);

    struct peer peerDesc2 = peerDesc;
    peerDesc2.port = 8081;
    peerDesc2.neighborPort = 8080;
    peerDesc2.buyType = FISH;
    peerDesc2.behavior = BEHAVE_M1_NO_BUY_BOAR;
    peerDesc2.ID = 0;
    std::thread thread2(makePeer, &peerDesc2);

    thread1.join();
    thread2.join();
}

void milestoneOne_3(){
    std::cout << "Milestone one, test 3, start\n\n";

    int r = rand()%2;

    struct peer peerDesc;
    peerDesc.showWork = true;
    peerDesc.port = 8080;
    peerDesc.neighborPort = 8081;
    peerDesc.numFish = 0;
    peerDesc.numBoar = 0;
    peerDesc.numDuck = 0;
    peerDesc.buyType = NONE;
    peerDesc.behavior = BEHAVE_NULL;
    peerDesc.ID = 42;

    struct peer peerDesc2 = peerDesc;
    peerDesc2.port = 8081;
    peerDesc2.neighborPort = 8080;
    peerDesc.ID = 0;

    // Now for the random bit!
    if(r == 1){
        std::cout << "Peer ID 42 is the buyer\n";
        peerDesc.numFish = 5;
        peerDesc2.buyType = FISH;
    } else{
        std::cout << "Peer ID 0 is the buyer\n";
        peerDesc2.numFish = 5;
        peerDesc.buyType = FISH;
    }
    
    std::thread t1(makePeer, &peerDesc);
    std::thread t2(makePeer, &peerDesc2);

    t1.join();
    t2.join();

}


void test_2(){
    struct peer peerBase;
    peerBase.numBoar = 0;
    peerBase.numFish = 0;
    peerBase.numDuck = 0;
    peerBase.neighborPort = -1;
    peerBase.showWork = true;
    peerBase.buyType = NONE;
    peerBase.behavior = BEHAVE_NULL;

    // For purposes of testing, each peer has at most 2 neighbors.
    peerBase.numNeighbors = 1;

    struct peer peer1, peer2;
    peer1 = peerBase;
    peer2 = peerBase;

    peerBase.ID = 0;
    peerBase.buyType = BOAR;
    int n0[1] = {2};
    peerBase.neighbors = n0;

    /*peer1.ID = 1;
    int n1[2] = {0,2};
    peer1.neighbors = n1;
    peer1.numNeighbors = 2;*/

    peer2.ID = 2;
    int n2[1] = {1};
    peer2.neighbors = n2;
    peer2.numBoar = 5;

    std::thread t0(makePeer, &peerBase);
    //std::thread t1(makePeer, &peer1);
    std::thread t2(makePeer, &peer2);

    
    t0.join();
    //t1.join();
    t2.join();

}

int main(){

    //milestoneOne_1();
    //milestoneOne_2();
    milestoneOne_3();
    //test_2();

    //testMultiCompile();
}