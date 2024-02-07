#include "ResponsePublisher.cpp"
#include "StatePublisher.cpp"
#include "QuerySubscriber.cpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>

bool running = true;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    running = false;
}

int main() {
    // Register signal handler for graceful shutdown
    signal(SIGINT, signalHandler);

    // Create instances of publishers and subscriber
    ResponsePublisher responsePublisher;
    StatePublisher statePublisher;
    QuerySubscriber querySubscriber;

    // Initialize publishers and subscriber
    if (!responsePublisher.init() || !statePublisher.init() || !querySubscriber.init()) {
        std::cerr << "Initialization failed!" << std::endl;
        return 1;  // Return non-zero value to indicate failure
    }

    std::cout << "Publishers and Subscriber initialized. Starting main loop." << std::endl;

    // Main loop
    while (running) {
        // In this loop, the program will continuously listen for Query_msg
        // The ResponsePublisher and StatePublisher can also be used to publish messages as needed

        // Example: periodically publish a state or response message
        // This part can be adjusted based on your application's logic

        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Adjust the sleep time as needed
    }

    std::cout << "Shutting down..." << std::endl;
    // Clean-up code if needed

    return 0;
}



// swig -c++ -python Query_msg.i
//g++ -shared -fPIC Query_msg_wrap.cxx Query_msg.cxx  Query_msgPubSubTypes.cxx -o _Query_msg.so -I/usr/include/python3.10 -lpython3.10 -lfastcdr  -lfastrtps
//ldd _Query_msg.so
//nm -C _Query_msg.so | grep Query_msg::slave_id
//
//
//
//
//
//
