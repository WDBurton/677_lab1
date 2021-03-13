// The main file
#include "bazaar.h"
#include <thread>


// This is the code for milestone 1 -- 2 peers, one client and one seller.
// Need to run the tests:
//      One is seller of fish other buyer of fish
//      One is seller of boar other is buyer of boar
//      Randomly assigned buyer/seller roles, items sold forever
void milestoneOne(){

    struct peer peerDesc;
    peerDesc.showWork = true;
    peerDesc.port = 8080;
    peerDesc.neighborPort = 8081;
    //peerDesc.behavior = BEHAVE_TEST_X1;
    peerDesc.behavior = BEHAVE_M1_BUY_FISH;
    peerDesc.ID = 42;
    std::thread thread1(makePeer, peerDesc);

    peerDesc.port = 8081;
    peerDesc.neighborPort = 8080;
    //peerDesc.behavior = BEHAVE_TEST_X2;
    peerDesc.behavior = BEHAVE_M1_SELL_FISH;
    peerDesc.ID = 0;
    std::thread thread2(makePeer, peerDesc);

    thread1.join();
    thread2.join();

}


int main(){

    milestoneOne();

    //testMultiCompile();
}