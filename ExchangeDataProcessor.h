#ifndef _EXCHANGE_DATA_PROCESSOR_H_
#define _EXCHANGE_DATA_PROCESSOR_H_

#include "ExchangeOrderBook.h"
#include "CoreMessages.h"
#include "PerformanceCounter.h"
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "ring_buffer_spsc.hpp"

namespace MarketData
{

    
class ExchangeDataProcessor {
public:    
    ExchangeDataProcessor(ExchangeOrderBook& exch_order_book, 
        RingBufferSPSC<MarketData::Packet, RING_BUFFER_SIZE>& msg_queue, std::mutex& mut, std::condition_variable& cnd);
    ~ExchangeDataProcessor() = default;

    void start();
    void stop();
    bool run();

private:
    ExchangeOrderBook& exchange_order_book;
    RingBufferSPSC<MarketData::Packet, RING_BUFFER_SIZE>& packet_queue;
    std::atomic_bool running{false};
    std::mutex& m;
    std::condition_variable& cond;
    bool measuring{true};
    PerformanceCounter perf_counter;

    void initHandlers();
    void processPacket(Packet&& packet);

    std::unordered_map<DataType, std::function<void(CoreMessage&)>> handler_map;
    void processSymbol(CoreMessage& cm);
    void processAddOrder(CoreMessage& cm);
    void processUpdateOrder(CoreMessage& cm);
    void processCancelOrder(CoreMessage& cm);
};

}

#endif