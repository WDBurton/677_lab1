// The main file
#include "bazaar.h"
#include <thread>


// This is the code for milestone 1 -- 2 peers, one client and one seller.
// Need to run the tests:
//      One is seller of fish other buyer of fish
//      One is seller of boar other is buyer of boar
//      Randomly assigned buyer/seller roles, items sold forever
void milestoneOne(){

    std::thread thread1(buyer, 0, 8080, 8081);
    std::thread thread2(seller, 0, 8081, 8080);

    thread1.join();
    thread2.join();

}


int main(){

    milestoneOne();

    //testMultiCompile();
}