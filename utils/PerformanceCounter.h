#ifndef _PERFORMANCE_COUNTER_H_
#define _PERFORMANCE_COUNTER_H_

#include <vector>
#include <cstdint>
namespace MarketData
{
class PerformanceCounter {
public:
    PerformanceCounter() = default;
    ~PerformanceCounter() = default;
    void printStats();
    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> getStats(const std::vector<uint64_t>& data);
    void addStat(uint64_t stat);
    void updateStat(uint64_t stat);
    void cancelStat(uint64_t stat);
private:
    std::vector<uint64_t> add_stats;
    std::vector<uint64_t> update_stats;
    std::vector<uint64_t> cancel_stats;
    //print_stats;

};

}
#endif