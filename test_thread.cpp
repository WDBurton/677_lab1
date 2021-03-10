// Test for threading

#include <iostream>
#include <thread>


// Basic function for threads to run
void foo_thread(int i){
    std::cout << "Thread " << i << " ID: " << std::this_thread::get_id() << "\n";
}

int main(){
    std::thread thread1(foo_thread, 1);
    std::thread thread2(foo_thread, 2);
    std::thread thread3(foo_thread, 3);
    std::thread thread4(foo_thread, 4);
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    std::thread thread5(foo_thread, 5);
    std::thread thread6(foo_thread, 6);
    thread5.join();
    thread6.join();
}