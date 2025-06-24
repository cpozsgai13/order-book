#include <iostream>
#include <functional>
#include <algorithm>
#include <numeric>
#include "CoreMessages.h"
#include "ExchangeOrderBook.h"
#include "ExchangeDataProcessorThread.h"
#include "MarketDataFileReader.h"
#include "MarketDataLineParser.h"
#include "MarketDataMessageGenerator.h"
#include "Order.h"
#include "rtdsc.h"
#include "MarketDataDefinitions.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <fstream>
#include <tuple>
#include <thread>
#include <string>
#include "ring_buffer_spsc.hpp"
#include "TCPSenderThread.h"
#include "TCPReceiverThread.h"
#include "PerformanceCounter.h"

using namespace MarketData;
using namespace std;

const auto parseAction=[](std::string& line) -> MarketData::Actions
{
    auto pos = line.find(' ');
    if(pos == std::string::npos)
    {
        if(line == "PRINT")
        {
            return MarketData::Actions::PRINT;
        }
        return MarketData::Actions::INVALID;
    }
    std::string action = line.substr(0, pos);
    line = line.substr(pos + 1);
    if(action == "BUY")
    {
        return MarketData::Actions::BUY;
    }
    else if(action == "SELL")
    {
        return MarketData::Actions::SELL;
    }
    else if(action == "CANCEL")
    {
        return MarketData::Actions::CANCEL;
    }
    else if(action == "MODIFY")
    {
        return MarketData::Actions::MODIFY;
    }
    else if(action == "PRINT")
    {
        return MarketData::Actions::PRINT;
    }
    return MarketData::Actions::INVALID;
};


auto parseIniFile=[](const char *path) -> boost::property_tree::ptree {
    boost::property_tree::ptree res;

    boost::property_tree::ini_parser::read_ini(path, res);
    return res;
};

int main(int argc, char *argv[])
{
    std::string send_interface;
    std::string recv_interface, recv_multicast;
    std::shared_ptr<TCPSenderThread> producer;
    std::shared_ptr<TCPReceiverThread> consumer;
    std::shared_ptr<ExchangeDataProcessorThread> exchange_processor;
    std::string exchange_name;
    std::string consumer_ip;
    uint16_t consumer_port{0};
    uint16_t producer_port{0};
    int consumer_core = -1;
    int producer_core = -1;
    int producer_send_rate = -1;
    std::string symbol_file;
    std::vector<std::string> data_files;
    std::string data_input_type;
    uint32_t num_random_objects = 0;
    PerformanceMeta perf_meta;
    if(argc > 1)
    {
        std::cout.sync_with_stdio(false);
        std::cerr.sync_with_stdio(false);

        auto properties = parseIniFile(argv[1]);

        if(properties.empty()) {
            return 1;
        }

        data_input_type = properties.get_optional<std::string>("data.type").get_value_or("file");
        if(data_input_type == "file") {
            symbol_file = properties.get_optional<std::string>("data.symbol_file").get_value_or("symbols.txt");
            auto files_str = properties.get_optional<std::string>("data.market_data_files").get_value_or("");
            if(!files_str.empty()) {
                auto start_pos = 0;
                auto pos = files_str.find(',');
                data_files.push_back(files_str.substr(0, pos));
                while(pos != std::string::npos) {
                    start_pos = pos + 1;
                    pos = files_str.find(',', start_pos);
                    if(pos != std::string::npos) {
                        data_files.push_back(files_str.substr(start_pos, pos - start_pos));
                    } else {
                        data_files.push_back(files_str.substr(start_pos));
                    }
                }
            }
        } else {
            symbol_file = properties.get_optional<std::string>("data.symbol_file").get_value_or("symbols.txt");
            num_random_objects = properties.get_optional<uint32_t>("data.N").get_value_or(1000);
        }

        consumer_ip = properties.get<std::string>("consumer.ip");
        consumer_port = properties.get<uint16_t>("consumer.port");
        consumer_core = properties.get_optional<int>("consumer.core").get_value_or(-1);

        if(consumer_ip.empty() || consumer_port == 0) {
            return 1;
        }
        producer_port = properties.get<uint16_t>("producer.port");
        if(producer_port == 0) {
            return 1;
        }
        producer_core = properties.get_optional<int>("producer.core").get_value_or(-1);
        producer_send_rate = properties.get_optional<int>("producer.rate").get_value_or(-1);

        bool perf_enabled = properties.get_optional<bool>("perf.enabled").get_value_or(false);
        perf_meta.enabled = perf_enabled;
                
        if(perf_enabled) {
            perf_meta.output_file = properties.get_optional<std::string>("perf.outfile").get_value_or("./perf.csv");
            perf_meta.processor_speed = properties.get_optional<double>("perf.procspeed").get_value_or(0.0);
        }        
    }

    std::vector<Packet> message_packets;
    std::vector<MarketData::Packet> symbol_packets;
    if(data_input_type == "file") {
        MarketData::FileReader reader;
        symbol_packets = reader.loadDataFile(symbol_file);
        if(symbol_packets.empty()) {
            std::cout << "Failed to load symbol data" << endl;
            return 1;
        }
        size_t symbol_msg_count = 0;    
        for(const auto& packet: symbol_packets) {
            symbol_msg_count += packet.data.packet.header.num_messages;
        }
        std::cout << "Loaded " << symbol_packets.size() << ", symbols: " << symbol_msg_count << endl;
        size_t md_msg_count = 0;
        for(const auto& data_file: data_files) {
            auto&& packets = reader.loadDataFile(data_file);
            for(const auto& packet: packets) {
                md_msg_count += packet.data.packet.header.num_messages;
                message_packets.push_back(packet);
            }
        }

        for(const auto& data_file: data_files) {
            auto&& packets = reader.loadDataFile(data_file);
            message_packets.reserve(message_packets.size() + packets.size());
            std::copy(packets.begin(), packets.end(), std::back_inserter(message_packets));
        }
        std::cout << "Loaded " << message_packets.size() << "packets, messages: " << md_msg_count << endl;
    } else if(data_input_type == "random") {
        MarketDataMessageGenerator generator;
        std::vector<Symbol> symbol_set;
        MarketData::FileReader reader;
        reader.loadSymbolFile(symbol_file, symbol_set);
        generator.GenerateMessages(symbol_set, num_random_objects, message_packets);
    }

    ExchangeOrderBook exchange_order_engine{exchange_name};
    RingBufferSPSC<Packet, RING_BUFFER_SIZE> tcp_message_queue;
    std::mutex tcp_queue_mutex;
    std::condition_variable tcp_queue_cond;

    producer = std::make_shared<TCPSenderThread>(producer_port, producer_send_rate);
    consumer = std::make_shared<TCPReceiverThread>(consumer_ip, consumer_port, tcp_message_queue);

    //  Start the exchange processor.
    exchange_processor = std::make_shared<ExchangeDataProcessorThread>(exchange_order_engine, tcp_message_queue, perf_meta);
    std::thread processor_thread(&ExchangeDataProcessorThread::start, exchange_processor);


    size_t num_packets = message_packets.size();
    if(data_input_type == "file") {
        while(!symbol_packets.empty()) {
            //Packet& packet = symbol_packets.front();
            auto&& packet = symbol_packets.front();
            producer->enqueue(std::move(packet));
            symbol_packets.erase(begin(symbol_packets));
        }

    }

    //  Start a consumer.
    std::thread consumer_thread(&TCPReceiverThread::start, consumer);
    if(consumer_core >= 0) {
        cpu_set_t consumer_core_set;
        CPU_ZERO(&consumer_core_set);
        CPU_SET(consumer_core, &consumer_core_set);
        int rc = pthread_setaffinity_np(consumer_thread.native_handle(), sizeof(cpu_set_t), &consumer_core_set);
        if(rc != 0) {
            std::cout << "Failed to pin consumer core " << consumer_core << std::endl;
        }
    }

    //  Import the non-symbol data.
    while(!message_packets.empty()) {
        Packet& packet = message_packets.front();
        producer->enqueue(std::move(packet));
        message_packets.erase(begin(message_packets));
    }
    std::cout << "Done enqueuing input data packets: " << num_packets << '\n';

    //  Start producer after it has loaded the data packets.
    std::thread producer_thread(&TCPSenderThread::start, producer);
    if(producer_core >= 0) {
        cpu_set_t producer_core_set;
        CPU_ZERO(&producer_core_set);
        CPU_SET(producer_core, &producer_core_set);
        int rc = pthread_setaffinity_np(producer_thread.native_handle(), sizeof(cpu_set_t), &producer_core_set);
        if(rc != 0) {
            std::cout << "Failed to pin producer core " << producer_core << std::endl;
        }
    }

    std::string line;
    while(true)
    {
        std::getline(std::cin, line);
        if(line == "q")
        {
            break;
        }
    }
    producer->stop();
    producer_thread.join();

    consumer->stop();
    consumer_thread.join();

    exchange_processor->stop();
    processor_thread.join();
    return 0;
}