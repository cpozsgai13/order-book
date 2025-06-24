#ifndef _PERFORMANCE_COUNTER_H_
#define _PERFORMANCE_COUNTER_H_

#include <vector>
#include <string>
#include <cstdint>

namespace MarketData
{
struct PerformanceMeta {
    bool enabled;
    std::string output_file;
    double processor_speed;
};

class PerformanceCounter {
public:
    PerformanceCounter(const std::string& out_file="", double proc_speed=0.0);
    ~PerformanceCounter() = default;
    void printStats();
    bool writeToFile(const std::string& path);
    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> getStats(const std::vector<uint64_t>& data);
    void addStat(uint64_t stat);
    void updateStat(uint64_t stat);
    void cancelStat(uint64_t stat);
    void save();    
    inline void setProcessorSpeed(double speed) {
        processor_speed = speed;
    }
private:
    std::vector<uint64_t> add_stats;
    std::vector<uint64_t> update_stats;
    std::vector<uint64_t> cancel_stats;
    std::string perf_file;
    double processor_speed{0.0};
};

}
#endif