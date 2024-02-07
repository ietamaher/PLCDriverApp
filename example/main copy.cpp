#include "ResponsePublisher.cpp"
#include "StatePublisher.cpp"
#include "QuerySubscriber.cpp"
// ... other necessary includes

int main() {
    // Create instances of publishers and subscriber
    ResponsePublisher responsePublisher;
    StatePublisher statePublisher;
    QuerySubscriber querySubscriber;

    // Initialize publishers and subscriber
    if (!responsePublisher.init() || !statePublisher.init() || !querySubscriber.init()) {
        // Handle initialization failure
    }
    std::cout << "Starting subscriber." << std::endl;

    // Create a Response_msg instance and populate it with data
    Response_msg responseMessage;
    responseMessage.func_code() = 2 ;
    responseMessage.slave_id() =  1;
    responseMessage.data()[0]=  444;
    responseMessage.data()[1] =  111;
    responseMessage.data()[2]=  222;
    responseMessage.data()[3]=  333;
uint32_t samples = 10 ;
        uint32_t samples_sent = 0;
        while (samples_sent < samples)
        {
 
                samples_sent++;
                 responsePublisher.publish(responseMessage);    

                std::cout << "Message func_code: " << responseMessage.func_code() 
          << ", Data index: " << responseMessage.data()[0] << " SENT" << std::endl;
 
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

    responsePublisher.publish(responseMessage);    
    // Main loop
    // - Publish responses and states
    // - Subscriber will automatically receive and handle queries
    // - Integrate with Modbus operations

    return 0;
}