#include "MulticastSender.h"
#include <iostream>
#include <vector>
#include <functional>
#include <unordered_map>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory>
#include <string.h>
#include <unistd.h>


namespace MarketData 
{

auto getPort = [](const std::string& addr) -> int
{
        int port = 0;
        auto pos = addr.find(':');
        if(pos != std::string::npos)
        {
                port = stoi(addr.substr(pos + 1));
        }
        return port;
};

auto getIP = [](const std::string& addr) -> std::string
{
        std::string ip;
        auto pos = addr.find(':');
        if(pos != std::string::npos)
        {
                ip = addr.substr(0, pos);
        }
        return ip;
};


MulticastSender::MulticastSender(const std::string& ip, uint16_t send_port):
multicast_address(ip),
port(send_port){}

bool MulticastSender::openSocket() {
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0)
	{
		return false;
	}

  int ttl = 1;
  if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
  {
      return false;
  }

	return true;
}

bool MulticastSender::closeSocket() {
	close(sockfd);
	return false;
}

bool MulticastSender::enqueue(Packet& packet) {
  std::cout << "About to add msg to output queue" << std::endl;
  std::lock_guard<std::mutex> lock(m);
  message_queue.push(packet);
  std::cout << "Message added to multicast output queue" << std::endl;
  cond.notify_one();
	return true;
}

bool MulticastSender::run() {
  if(!openSocket()) {
    return false;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(multicast_address.c_str());
  addr.sin_port = htons(port);
  socklen_t addr_len = sizeof(addr);

  //  Wait until started
  {
    std::unique_lock<std::mutex> lock(m);
    cond.wait(lock, [this]{return running.load();});
  }
  std::cout << "Sender signaled to start " << running << std::endl;


  while(running.load()) {

    std::cout << "Sender waiting for message" << std::endl;

    std::unique_lock<std::mutex> lock(m);
    cond.wait(lock, [this]{return !running.load() || !message_queue.empty();});

    if(!running.load()) {
      std::cout << "Exiting run loop" << std::endl;
      break;
    }

    auto msg = message_queue.front();
    message_queue.pop();
    std::cout << "Message popped: " << msg.data.packet.header.total_length << std::endl;

    //  Send the packet.
    auto sz = msg.data.packet.header.total_length;
    int sent = sendto(sockfd, (char *)&msg, sz, 0, (struct sockaddr *)&addr, addr_len);
    if(sent <= 0) {
      std::cout << "Send error: " << errno << std::endl;
    }

    if(!running) {
      break;
    }

  }
  std::cout << "Sender stopped" << std::endl;
  return true;
}

void MulticastSender::start() {
  std::lock_guard<std::mutex> lock(m);
  running.store(true);
  cond.notify_one();
}
void MulticastSender::stop() {
  std::lock_guard<std::mutex> lock(m);
  running.store(false);
  cond.notify_one();
}

}

