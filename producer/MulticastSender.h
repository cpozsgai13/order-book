#ifndef _MULTICAST_SENDER_H_
#define _MULTICAST_SENDER_H_

#include "BaseProducer.h"
#include <cstdint>
#include <cstddef>
#include <string>
#include <atomic>
#include <queue>
#include <memory>
#include "CoreMessages.h"
#include <mutex>
#include <condition_variable>

namespace MarketData 
{

class MulticastSender : public BaseProducer<MulticastSender> {
    static constexpr size_t BUFFER_SIZE = 1500;
public:
	MulticastSender(const std::string& ip, uint16_t send_port);
	~MulticastSender() = default;

    bool run();
	void start();
    void stop();
	bool openSocket();
	bool closeSocket();
	bool enqueue(Packet& packet);
private:
	std::queue<Packet> message_queue;

	uint16_t port;
	std::string multicast_address;
    char buffer[BUFFER_SIZE];
    int sockfd{-1};
    std::atomic_bool running{false};

	std::mutex m;
	std::condition_variable cond;
};

}

#endif