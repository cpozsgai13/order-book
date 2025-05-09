#ifndef _MULTICAST_RECEIVER_H_
#define _MULTICAST_RECEIVER_H_

#include "CoreMessages.h"
#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace MarketData
{
class MulticastReceiver {
    static constexpr size_t BUFFER_SIZE = 1500;
public:
    MulticastReceiver(const std::string& interface, int port, 
        const std::string& multicast);
    ~MulticastReceiver() = default;

    bool run();
    void start();
    void stop();
	bool openSocket();
	bool closeSocket();
private:
    bool processMessage(Packet& packet);

    std::string interface_address;
    uint16_t port{0};
    std::string multicast_address;
    char buffer[BUFFER_SIZE];
    int sockfd{-1};

    std::atomic_bool running{false};
	std::mutex m;
	std::condition_variable cond;
    std::queue<Packet> message_queue;
};

}

#endif