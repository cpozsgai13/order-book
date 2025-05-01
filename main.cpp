#include <iostream>
#include <functional>
#include <algorithm>
#include <numeric>
#include "ExchangeOrderBook.h"
#include "ExchangeDataProcessor.h"
#include "MarketDataFileReader.h"
#include "MarketDataLineParser.h"
#include "Order.h"
#include "rtdsc.h"
#include "MarketDataDefinitions.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <fstream>
#include <tuple>
#include <thread>
#include "TCPSender.h"
#include "TCPReceiver.h"

using namespace MarketData;
using namespace std;

std::vector<uint64_t> add_stats, update_stats, cancel_stats, print_stats;
auto getMinMaxMeanMedian=[](const std::vector<uint64_t>& stats) -> std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> {
    tuple<uint64_t, uint64_t, uint64_t, uint64_t> res;
    if(stats.empty()) {
        return res;
    }
    auto N = stats.size();

    //  Get min
    auto min_value = *std::min_element(begin(stats), end(stats));
    auto max_value = *std::max_element(begin(stats), end(stats));

    //  Get mean.
    auto mean = std::accumulate(begin(stats), end(stats), 0);
    mean = mean / N;

    auto median = 0;
    //  Median
    if(N % 2 == 1) {
        median = stats[N/2];
    } else {
        median = (int)(stats[N/2 - 1] + stats[N/2])/2;
    }
    get<0>(res) = min_value;
    get<1>(res) = max_value;
    get<2>(res) = mean;
    get<3>(res) = median;

    return res;
};

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

auto printStats = []() {
    std::string sep(50, '-');

    std::cout << "ADD ORDER STATS: " << std::endl;
    std::cout << "\tCOUNT: " << add_stats.size() << std::endl;
    auto res = getMinMaxMeanMedian(add_stats);
    std::cout << "\tMIN: " << get<0>(res) << std::endl;
    std::cout << "\tMAX: " << get<1>(res) << std::endl;
    std::cout << "\tMEAN: " << get<2>(res) << std::endl;
    std::cout << "\tMDN: " << get<3>(res) << std::endl;
    std::cout << sep << std::endl;


    std::cout << "MODIFY ORDER STATS: " << std::endl;
    std::cout << "\tCOUNT: " << update_stats.size() << std::endl;
    res = getMinMaxMeanMedian(update_stats);
    std::cout << "\tMIN: " << get<0>(res) << std::endl;
    std::cout << "\tMAX: " << get<1>(res) << std::endl;
    std::cout << "\tMEAN: " << get<2>(res) << std::endl;
    std::cout << "\tMDN: " << get<3>(res) << std::endl;
    std::cout << sep << std::endl;

    std::cout << "CANCEL ORDER STATS: " << std::endl;
    std::cout << "\tCOUNT: " << cancel_stats.size() << std::endl;
    res = getMinMaxMeanMedian(cancel_stats);
    std::cout << "\tMIN: " << get<0>(res) << std::endl;
    std::cout << "\tMAX: " << get<1>(res) << std::endl;
    std::cout << "\tMEAN: " << get<2>(res) << std::endl;
    std::cout << "\tMDN: " << get<3>(res) << std::endl;
    std::cout << sep << std::endl;

    std::cout << "PRINT STATS: " << std::endl;
    std::cout << "\tCOUNT: " << print_stats.size() << std::endl;
    res = getMinMaxMeanMedian(print_stats);
    std::cout << "\tMIN: " << get<0>(res) << std::endl;
    std::cout << "\tMAX: " << get<1>(res) << std::endl;
    std::cout << "\tMEAN: " << get<2>(res) << std::endl;
    std::cout << "\tMDN: " << get<3>(res) << std::endl;

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
            OrderID order_id;
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
    std::vector<Packet> symbol_packets, message_packets;
    symbol_packets.reserve(10);
    message_packets.reserve(100);
    std::string send_interface;
    uint16_t send_port = 0;
    std::string recv_interface, recv_multicast;
    uint16_t recv_port = 0;
    std::shared_ptr<TCPSender> producer;
    std::shared_ptr<TCPReceiver> consumer;
    std::shared_ptr<ExchangeDataProcessor> exchange_processor;
    std::string exchange_name;
    std::string consumer_ip;
    uint16_t consumer_port{0};
    uint16_t producer_port{0};
    std::string symbol_file;
    std::vector<std::string> data_files;

    if(argc > 1)
    {
        auto properties = parseIniFile(argv[1]);

        if(properties.empty()) {
            return 1;
        }

        symbol_file = properties.get<std::string>("symbol_file");
        //data_file = properties.get<std::string>("market_data_file");
        auto files_str = properties.get<std::string>("market_data_files");
        if(!files_str.empty()) {
            auto start_pos = 0;
            auto pos = files_str.find(',');
            data_files.push_back(files_str.substr(0, pos));
            while(pos != std::string::npos) {
                start_pos = pos + 1;
                pos = files_str.find(',', start_pos);
                data_files.push_back(files_str.substr(start_pos, pos));
            }
        }

        consumer_ip = properties.get<std::string>("consumer.ip");
        consumer_port = properties.get<uint16_t>("consumer.port");
        exchange_name = properties.get<std::string>("exchange_name");

        if(consumer_ip.empty() || consumer_port == 0) {
            return 1;
        }
        producer_port = properties.get<uint16_t>("producer.port");
        if(producer_port == 0) {
            return 1;
        }
    }

    MarketData::FileReader reader;
    if(reader.loadFile(symbol_file, symbol_packets)) {
        size_t symbol_msg_count = 0;
        for(const auto& packet: symbol_packets) {
            symbol_msg_count += packet.header.num_messages;
        }
        std::cout << "Loaded " << symbol_packets.size() << ", symbols: " << symbol_msg_count << endl;
    } else {
        std::cout << "Failed to load symbol data" << endl;
        return 1;
    }

    for(const auto& data_file: data_files) {
        if(reader.loadFile(data_file, message_packets)) {
            size_t md_msg_count = 0;
            for(const auto& packet: message_packets) {
                md_msg_count += packet.header.num_messages;
            }
        } else {
            std::cout << "Failed to load message data" << endl;
            return 1;
        }
    }
    std::cout << "Loaded " << message_packets.size() << "packets, messages: " << message_packets.size() << endl;

    ExchangeOrderBook exchange_order_engine{exchange_name};
    std::queue<Packet> tcp_message_queue;
    std::mutex tcp_queue_mutex;
    std::condition_variable tcp_queue_cond;

    exchange_processor = std::make_shared<ExchangeDataProcessor>(exchange_order_engine, tcp_message_queue, tcp_queue_mutex, tcp_queue_cond);
    producer = std::make_shared<TCPSender>(producer_port);
    consumer = std::make_shared<TCPReceiver>(consumer_ip, consumer_port, tcp_message_queue, tcp_queue_mutex, tcp_queue_cond);


    //  Start the exchange processor.
    std::thread processor_thread(&ExchangeDataProcessor::run, exchange_processor);
    exchange_processor->start();

    //  The consumer will populate a queue from which the processor will pull packets, and
    //  submit to the ExchangeOrderBook for processing.

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
            printStats();
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

}