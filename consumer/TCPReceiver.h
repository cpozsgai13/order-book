#ifndef _TCP_RECEIVER_H_
#define _TCP_RECEIVER_H_

#include "CoreMessages.h"
#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace MarketData
{
class TCPReceiver {
    static constexpr size_t BUFFER_SIZE = 1500;
public:
    TCPReceiver(const std::string& address, int port, std::queue<Packet>& output, 
        std::mutex& mut, std::condition_variable& cnd);
    ~TCPReceiver() = default;

    bool run();
    void start();
    void stop();
	bool openSocket();
	bool closeSocket();
private:
    bool processMessage(Packet&& packet);

    std::string address;
    uint16_t port{0};
    char buffer[BUFFER_SIZE];
    int sockfd{-1};

    std::atomic_bool running{false};
	std::mutex& m;
	std::condition_variable& cond;
    std::queue<Packet>& output_message_queue;
};

}

#endif