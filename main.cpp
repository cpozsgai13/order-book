#include <iostream>
#include <functional>
#include <algorithm>
#include <numeric>
#include "MatchOrderEngine.h"
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
#include "MulticastSender.h"
#include "MulticastReceiver.h"
using namespace MarketData;
using namespace std;
OrderBook engine;

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

const auto processMarketDataInputLine=[](std::string& line) -> void
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
    std::string send_interface;
    uint16_t send_port = 0;
    std::string recv_interface, recv_multicast;
    uint16_t recv_port = 0;
    std::shared_ptr<MulticastSender> producer;
    std::shared_ptr<MulticastReceiver> consumer;
    if(argc > 1)
    {
        auto properties = parseIniFile(argv[1]);

        if(properties.empty()) {
            return 1;
        }

        std::string symbol_file = properties.get<std::string>("symbol_file");
        std::string data_file = properties.get<std::string>("market_data_file");

        auto consumer_interface = properties.get<std::string>("consumer.interface");
        auto consumer_port = properties.get<uint16_t>("consumer.port");
        auto consumer_multicast = properties.get<std::string>("consumer.multicast");

        if(consumer_interface.empty() || consumer_multicast.empty() || consumer_port  == 0) {
            return 1;
        }
        auto producer_interface = properties.get<std::string>("producer.interface");
        auto producer_port = properties.get<uint16_t>("producer.port");
        if(producer_interface.empty() || producer_port == 0) {
            return 1;
        }

        producer = std::make_shared<MulticastSender>(producer_interface, producer_port);
        consumer = std::make_shared<MulticastReceiver>(consumer_interface, consumer_port, consumer_multicast);

        MarketData::FileReader reader;
        if(reader.loadFile(symbol_file, symbol_packets)) {
            for(const auto& packet: message_packets) {
                auto num_messages = packet.header.num_messages;
            }

        }

        if(reader.loadFile(data_file, message_packets)) {
            for(const auto& packet: message_packets) {
                auto num_messages = packet.header.num_messages;
                
            }

        }
    }

    //  Start a consumer and a producer.
    std::thread consumer_thread(&MulticastReceiver::run, consumer);
    consumer->start();

    std::thread producer_thread(&MulticastSender::run, producer);

    while(!symbol_packets.empty()) {
        Packet& packet = symbol_packets.front();
        producer->enqueue(packet);
        std::cout << "Added symbol packet" << std::endl;
        symbol_packets.erase(begin(symbol_packets));
    }

    std::this_thread::sleep_for(chrono::milliseconds(2000));
    producer->start();

    std::string line;

    while(true)
    {
        std::getline(std::cin, line);
        if(line == "q")
        {
            printStats();
            break;
        } else if(line == "p") {
            engine.Print();
        } else {
            //  Take commands
            //processMarketDataInputLine(line);
        }
    }

    producer->stop();
    producer_thread.join();

    consumer->stop();
    consumer_thread.join();
}