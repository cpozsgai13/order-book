#include "PerformanceCounter.h"
#include "rtdsc.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <atomic>
#include <tuple>
#include <fstream>

namespace MarketData
{

enum EventType : int {
    ADD = 0,
    UPDATE = 1,
    CANCEL = 2
};

std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> PerformanceCounter::getStats(const std::vector<uint64_t>& data) {
    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> res;
    if(data.empty()) {
        return res;
    }
    auto N = data.size();

    //  Get min
    auto min_value = *std::min_element(begin(data), end(data));
    auto max_value = *std::max_element(begin(data), end(data));

    //  Get mean.
    auto mean = std::accumulate(begin(data), end(data), 0);
    mean = mean / N;

    auto median = 0;
    //  Median
    if(N % 2 == 1) {
        median = data[N/2];
    } else {
        median = (int)(data[N/2 - 1] + data[N/2])/2;
    }
    std::get<0>(res) = min_value;
    std::get<1>(res) = max_value;
    std::get<2>(res) = mean;
    std::get<3>(res) = median;

    return res;
}

void PerformanceCounter::addStat(uint64_t stat) {
    add_stats.push_back(stat);
}

void PerformanceCounter::updateStat(uint64_t stat) {
    update_stats.push_back(stat);
}

void PerformanceCounter::cancelStat(uint64_t stat) {
    cancel_stats.push_back(stat);
}

void PerformanceCounter::printStats() {
    std::string sep(50, '-');

    std::cout << "ADD STATS: " << std::endl;
    std::cout << "\tCOUNT: " << add_stats.size() << std::endl;
    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> res = getStats(add_stats);
    std::cout << "\tMIN: " << std::get<0>(res) << std::endl;
    std::cout << "\tMAX: " << std::get<1>(res) << std::endl;
    std::cout << "\tMEAN: " << std::get<2>(res) << std::endl;
    std::cout << "\tMDN: " << std::get<3>(res) << std::endl;
    std::cout << sep << std::endl;

    std::cout << "UPDATE STATS: " << std::endl;
    std::cout << "\tCOUNT: " << update_stats.size() << std::endl;
    res = getStats(update_stats);
    std::cout << "\tMIN: " << std::get<0>(res) << std::endl;
    std::cout << "\tMAX: " << std::get<1>(res) << std::endl;
    std::cout << "\tMEAN: " << std::get<2>(res) << std::endl;
    std::cout << "\tMDN: " << std::get<3>(res) << std::endl;
    std::cout << sep << std::endl;

    std::cout << "CANCEL STATS: " << std::endl;
    std::cout << "\tCOUNT: " << cancel_stats.size() << std::endl;
    res = getStats(cancel_stats);
    std::cout << "\tMIN: " << std::get<0>(res) << std::endl;
    std::cout << "\tMAX: " << std::get<1>(res) << std::endl;
    std::cout << "\tMEAN: " << std::get<2>(res) << std::endl;
    std::cout << "\tMDN: " << std::get<3>(res) << std::endl;
    std::cout << sep << std::endl;
}

bool PerformanceCounter::writeToFile(const std::string& path) {
    std::ofstream out_file{path};

    if(!out_file.is_open()) {
        return false;
    }

    int i = 0;
    for(const auto count: add_stats) {
        out_file << (int)EventType::ADD << "," << ++i << "," << count << std::endl;
    }
    i = 0;
    for(const auto count: update_stats) {
        out_file << (int)EventType::UPDATE << "," << ++i << "," << count << std::endl;
    }
    i = 0;
    for(const auto count: cancel_stats) {
        out_file << (int)EventType::CANCEL << "," << ++i << "," << count << std::endl;
    }

    return true;
}

}
