#include <iostream>
#include <functional>
#include <algorithm>
#include <numeric>
#include "CoreMessages.h"
#include "ExchangeOrderBook.h"
#include "ExchangeDataProcessor.h"
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
#include "TCPSender.h"
#include "TCPReceiver.h"

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


const auto processMarketDataInputLine=[](std::string& line, ExchangeOrderBook& exchange_book) -> void
{
    Actions action = parseAction(line);
    OrderPtr order_ptr;
    //ModifyOrder mo;
    switch(action)
    {
        case Actions::BUY:
        case Actions::SELL:
        {
            // if(fromString(line, action == Actions::BUY ? Side::BID : Side::ASK, order_ptr))
            // {
            //     add_stats.push_back(CallAndMeasure(&OrderBook::AddOrder, &engine, order_ptr));
            //     // engine.AddOrder(order_ptr);
            // } 
            break;
        }
        case Actions::MODIFY:
        {
            // if(fromString(line, mo))
            // {
            //     update_stats.push_back(CallAndMeasure(&OrderBook::UpdateOrder, &engine, mo));
            // } 
            break;
        }
        case Actions::CANCEL:
        {
            //OrderID order_id;
            // if(fromString(line, order_id))
            // {
            //     cancel_stats.push_back(CallAndMeasure(&OrderBook::CancelOrder, &engine, order_id));
            // } 
            break;
        }
        case Actions::PRINT:
        {
            std::string symbol = line;
            exchange_book.PrintBook(symbol);
            // print_stats.push_back(CallAndMeasure(&OrderBook::Print, &engine));
            //engine.Print();
            break;
        }
        default:
            break;
    }

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
    std::shared_ptr<TCPSender> producer;
    std::shared_ptr<TCPReceiver> consumer;
    std::shared_ptr<ExchangeDataProcessor> exchange_processor;
    std::string exchange_name;
    std::string consumer_ip;
    uint16_t consumer_port{0};
    uint16_t producer_port{0};
    std::string symbol_file;
    std::vector<std::string> data_files;
    std::string data_input_type;
    uint32_t num_random_objects = 0;
    if(argc > 1)
    {
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

        if(consumer_ip.empty() || consumer_port == 0) {
            return 1;
        }
        producer_port = properties.get<uint16_t>("producer.port");
        if(producer_port == 0) {
            return 1;
        }
    }


    std::vector<Packet> message_packets;

    MarketData::FileReader reader;
    auto symbol_packets = reader.loadDataFile(symbol_file);
    if(symbol_packets.empty()) {
        std::cout << "Failed to load symbol data" << endl;
        return 1;
    }
    size_t symbol_msg_count = 0;    
    for(const auto& packet: symbol_packets) {
        symbol_msg_count += packet.header.num_messages;
    }
    std::cout << "Loaded " << symbol_packets.size() << ", symbols: " << symbol_msg_count << endl;

    if(data_input_type == "file") {
        size_t md_msg_count = 0;
        for(const auto& data_file: data_files) {
            auto&& packets = reader.loadDataFile(data_file);
            for(const auto& packet: packets) {
                md_msg_count += packet.header.num_messages;
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
        reader.loadSymbolFile(symbol_file, symbol_set);
        generator.GenerateMessages(symbol_set, num_random_objects, message_packets);
    }

    ExchangeOrderBook exchange_order_engine{exchange_name};
    RingBufferSPSC<Packet, RING_BUFFER_SIZE> tcp_message_queue;
    std::mutex tcp_queue_mutex;
    std::condition_variable tcp_queue_cond;

    exchange_processor = std::make_shared<ExchangeDataProcessor>(exchange_order_engine, tcp_message_queue, tcp_queue_mutex, tcp_queue_cond);
    producer = std::make_shared<TCPSender>(producer_port);
    consumer = std::make_shared<TCPReceiver>(consumer_ip, consumer_port, tcp_message_queue, tcp_queue_mutex, tcp_queue_cond);

    //  Start the exchange processor.
    std::thread processor_thread(&ExchangeDataProcessor::run, exchange_processor);
    exchange_processor->start();

    std::thread producer_thread(&TCPSender::run, producer);

    while(!symbol_packets.empty()) {
        Packet& packet = symbol_packets.front();
        producer->enqueue(packet);
        symbol_packets.erase(begin(symbol_packets));
    }

    //  TCP producer will block until connected, then will run.
    producer->start();

    //  Start a consumer and a producer.
    std::thread consumer_thread(&TCPReceiver::run, consumer);
    consumer->start();

    //  Import the non-symbol data.
    while(!message_packets.empty()) {
        Packet& packet = message_packets.front();
        producer->enqueue(packet);
        message_packets.erase(begin(message_packets));
    }
    std::string line;

    //  Allow user to manually add content or load from files.
    while(true)
    {
        std::getline(std::cin, line);
        if(line == "q")
        {
            break;
        } else {
            //  Take commands
            processMarketDataInputLine(line, exchange_order_engine);
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