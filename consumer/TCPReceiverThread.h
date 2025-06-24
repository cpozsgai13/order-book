#ifndef _TCP_RECEIVER_H_
#define _TCP_RECEIVER_H_

#include "CoreMessages.h"
#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "ring_buffer_spsc.hpp"
#include "TCPMemoryPool.h"
namespace MarketData
{
class TCPReceiverThread {
    static constexpr size_t BUFFER_SIZE = 1500;
public:
    TCPReceiverThread(const std::string& address, int port,
        RingBufferSPSC<MarketData::Packet, RING_BUFFER_SIZE>& output,
        int retry_count = -1, int retry_interval_sec = 1);
    ~TCPReceiverThread() = default;

    bool run();
    bool start();
    void stop();
	bool openSocket();
	bool closeSocket();
private:
    bool processMessage(Packet&& packet);

    std::string address;
    uint16_t port{0};
    RingBufferSPSC<Packet, RING_BUFFER_SIZE>& output_message_queue;
    int retry_count{-1};
    int retry_interval_sec{1};
    std::unique_ptr<TCPMemoryPool> tcp_memory_pool;
    int sockfd{-1};
    std::atomic_bool running{false};
};

}

#endif