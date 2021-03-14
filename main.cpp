// The main file
#include "bazaar.h"
#include <thread>
#include <iostream>


// This is the code for milestone 1 -- 2 peers, one client and one seller.
// Need to run the tests:
//      One is seller of fish other buyer of fish
//      One is seller of boar other is buyer of something other than boar; nothing is ever sold.
//      Randomly assigned buyer/seller roles, items sold forever
void milestoneOne_1(){
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
    std::cout << "Milestone one, test 2, start\n";
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


int main(){

    //milestoneOne_1();
    milestoneOne_2();

    //testMultiCompile();
}