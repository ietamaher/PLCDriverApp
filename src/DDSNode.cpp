#include "Query_msgPubSubTypes.h"
#include "Response_msgPubSubTypes.h"

#include <chrono>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>


#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <modbus/modbus.h>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <array>
#include <yaml-cpp/yaml.h>
#include <string>

using namespace eprosima::fastdds::dds;
 
class DDSNode {
private:
    DomainParticipant* participant;
    Publisher* publisher;
    Subscriber* subscriber;
    Topic* queryTopic;
    Topic* responseTopic;
    DataWriter* responseWriter;
    DataReader* queryReader;
    modbus_t* modbusContext;
    TypeSupport queryType;
    TypeSupport responseType;
    std::mutex modbusMutex; // Mutex for Modbus operations



    // Inner class definition
    class QueryListener : public DataReaderListener {
    public:
        QueryListener(DDSNode& node) : ddsNode(node) , samples_(0) {}

        void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override {
            if (info.current_count_change == 1) {
                std::cout << "Subscriber matched." << std::endl;
            } else if (info.current_count_change == -1) {
                std::cout << "Subscriber unmatched." << std::endl;
            }
        }

        void on_data_available(DataReader* reader) override {
            Query_msg queryMsg;
            SampleInfo info;
            if (reader->take_next_sample(&queryMsg, &info) == ReturnCode_t::RETCODE_OK && info.valid_data) {
                //std::cout << "Received Query: Slave ID: " << (int)queryMsg.slave_id() << ", Func Code: " << (int)queryMsg.func_code()  << ", Write Add : " << (int)queryMsg.write_addr() << std::endl;

                ddsNode.handleQuery(queryMsg);
            }

        }
        std::atomic_int samples_;
    
    private:
        DDSNode& ddsNode;


    } queryListener;

    void initializeModbus() {
        std::lock_guard<std::mutex> lock(modbusMutex);

        // Load configuration from YAML file
        YAML::Node config = YAML::LoadFile("config.yaml");
        std::string device = config["modbus"]["device"].as<std::string>();
        int baud_rate = config["modbus"]["baud_rate"].as<int>();
        char parity = config["modbus"]["parity"].as<char>();
        int data_bits = config["modbus"]["data_bits"].as<int>();
        int stop_bits = config["modbus"]["stop_bits"].as<int>();

        modbusContext = modbus_new_rtu(device.c_str(), baud_rate, parity, data_bits, stop_bits);
        if (modbusContext && modbus_connect(modbusContext) == -1) {
            modbus_free(modbusContext);
            modbusContext = nullptr;
            throw std::runtime_error("Failed to connect Modbus");
        }
    }

    void finalizeModbus() {
        std::lock_guard<std::mutex> lock(modbusMutex);
        if (modbusContext) {
            modbus_close(modbusContext);
            modbus_free(modbusContext);
            modbusContext = nullptr;
        }
    }


public:
    DDSNode() : participant(nullptr), publisher(nullptr), subscriber(nullptr),
                queryTopic(nullptr), responseTopic(nullptr), queryType(new Query_msgPubSubType()), responseType(new Response_msgPubSubType()),
                responseWriter(nullptr), queryReader(nullptr), modbusContext(nullptr), queryListener(*this) {

                initializeModbus();
            }   

    ~DDSNode() {
        finalizeModbus();
        // Additional cleanup for DDS entities
    }

    bool init() {

        DomainParticipantQos participantQos;
        participantQos.name("PLC_Query_Response");
        participant = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);
        if (participant == nullptr) {
            return false;
        }

        responseType.register_type(participant);
        queryType.register_type(participant);



        queryTopic = participant->create_topic("PLC_QueryTopic", "Query_msg", TOPIC_QOS_DEFAULT);

        responseTopic = participant->create_topic("PLC_ResponseTopic", "Response_msg", TOPIC_QOS_DEFAULT);

        publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        if (publisher == nullptr) {
            return false;
        }

        subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (subscriber == nullptr) {
            return false;
        }
        DataWriterQos dw_qos;
        publisher->get_default_datawriter_qos(dw_qos);

        // Enable asynchronous publishing
        dw_qos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
        responseWriter = publisher->create_datawriter(responseTopic, dw_qos, nullptr);

        //responseWriter = publisher->create_datawriter(responseTopic, DATAWRITER_QOS_DEFAULT, nullptr);

        //queryListener = QueryListener(*this);
        queryReader = subscriber->create_datareader(queryTopic, DATAREADER_QOS_DEFAULT, &queryListener);


        return true;
    }

    void handleQuery(const Query_msg& queryMsg) {
        //std::cout << "Handling Query" << std::endl;
        std::cout << queryMsg.read_addr() << queryMsg.read_num() << queryMsg.slave_id() << queryMsg.func_code() << std::endl;

        if (queryMsg.func_code() == 2) {
            // Handle Modbus read
            modbus_read_function(queryMsg.slave_id(), queryMsg.read_addr(), queryMsg.read_num());
        } else if (queryMsg.func_code() == 1) {
            // Handle Modbus write
            modbus_write_function(queryMsg.slave_id(), queryMsg.write_addr(), queryMsg.write_num(), queryMsg.data());
        } else if (queryMsg.func_code() == 3) {
            // Handle Modbus write coils 
            modbus_write_coils_function(queryMsg.slave_id(), queryMsg.write_addr(), queryMsg.data());
        } else if (queryMsg.func_code() == 4) {
            // Handle Modbus read inputs 
            modbus_read_discrete_inputs(queryMsg.slave_id(), queryMsg.read_addr(), queryMsg.read_num());
        } 
    }

    void modbus_write_coils_function(int8_t slave_id, int32_t write_addr, const std::array<unsigned int, 64>& coil_values) {
        modbus_set_slave(modbusContext, slave_id);
        std::vector<uint8_t> modbus_data(coil_values.size());
        
        for (size_t i = 0; i < 7; ++i) {
            modbus_data[i] = coil_values[i] ? 1 : 0;  // Convert bool to Modbus coil value
        }

        int rc = modbus_write_bits(modbusContext, write_addr, 7, modbus_data.data());
        if (rc == -1) {
            fprintf(stderr, "Modbus write coils failed: %s\n", modbus_strerror(errno));
        }
    }

    void modbus_read_discrete_inputs(int8_t slave_id, int32_t read_addr, int8_t read_num) {
        modbus_set_slave(modbusContext, slave_id);
        std::vector<uint8_t> modbus_data(read_num);
        std::cout << read_addr << read_num << std::endl;
        int j = 0;
        std::array<uint32_t, 64> value;          
        int rc = modbus_read_input_bits(modbusContext, read_addr, read_num, modbus_data.data());
        if (rc == -1) {
            fprintf(stderr, "Modbus read discrete inputs failed: %s\n", modbus_strerror(errno));
        } else {
            // Process the received data
 
            for (int i = 0; i < rc; i += 1) {
                value[j] = modbus_data[i] ;
                j++;
                // Handle or store the value as needed
                // Example: printf("Read value: %d\n", value);
            }
        }

        Response_msg response_;
        response_.func_code(4);
        response_.slave_id(slave_id);
        response_.data(value);
        responseWriter->write(&response_);
    }    
    void modbus_write_function(int8_t slave_id, int32_t write_addr, int8_t write_num, const std::array<unsigned int, 64>& data){

        modbus_set_slave(modbusContext, slave_id);
 

        // Convert data to the format suitable for Modbus (e.g., split into int16)
        uint16_t modbus_data[write_num]; // Since we are splitting each int32 into two int16
        for (int i = 0; i < write_num; i++) {
            modbus_data[i] = data[i];   // High word
        }
        std::cout << modbus_data[0]  << std::endl;
       
        // Write data to Modbus
        int rc = modbus_write_registers(modbusContext, write_addr, write_num , modbus_data);

        //int modbus_write_registers(modbus_t *modbusContext, int addr, int nb, const uint16_t *src);

        if (rc == -1) {
            fprintf(stderr, "Modbus write failed: %s\n", modbus_strerror(errno));
        }
 
    }

    void modbus_read_function(int8_t slave_id, int32_t read_addr, int8_t read_num) {

        modbus_set_slave(modbusContext, slave_id);
 

        uint16_t modbus_data[128]; // Adjust size based on the maximum expected read_num
        int addr = 0x0F8;
        int j = 0;
        std::array<uint32_t, 64> value;        
        int rc = modbus_read_registers(modbusContext, read_addr, read_num * 2, modbus_data);
        if (rc == -1) {
            fprintf(stderr, "Modbus read failed: %s\n", modbus_strerror(errno));
        } else {
            // Process the received data
            //int j = 0;
            //int32_t value[64];
            for (int i = 0; i < rc; i += 2) {
                value[j] = (modbus_data[i] << 16) | modbus_data[i + 1];
                j++;
                // Handle or store the value as needed
                // Example: printf("Read value: %d\n", value);
            }
        }

        Response_msg response_;
        response_.func_code(2);
        response_.slave_id(slave_id);
        response_.data(value);
        responseWriter->write(&response_);
 
    }
};

int main() {
    DDSNode ddsNode;

    if (!ddsNode.init()) {
        std::cerr << "Initialization failed!" << std::endl;
        return 1;
    }

    // Use a condition variable to wait indefinitely instead of a busy loop
    std::mutex mtx;
    std::unique_lock<std::mutex> lck(mtx);
    std::condition_variable cv;
    cv.wait(lck);

    return 0;
}
