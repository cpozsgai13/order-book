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
#include <memory.h>
#include <unistd.h>

#include "MulticastReceiver.h"

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

MulticastReceiver::MulticastReceiver(const std::string& interface, int listen_port, 
    const std::string& multicast):
interface_address(interface),
port(listen_port),
multicast_address(multicast){}

bool MulticastReceiver::openSocket() {
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0)
	{
        return false;
	}

  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
  {
      return false;
  }
  return true;
}

bool MulticastReceiver::run() {
  if(!openSocket()) {
      return false;
  }

  //  Join the multicast group and note the address
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);    
  // addr.sin_addr.s_addr = inet_addr(interface_address.c_str());

  if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
      return false;
  }
    
  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = inet_addr(multicast_address.c_str());
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  //mreq.imr_interface.s_addr = inet_addr(interface_address.c_str());

  if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
  {
      return false;
  }

  //  Wait until started
  {
    std::unique_lock<std::mutex> lock(m);
    cond.wait(lock, [this]{return running.load();});
  }
  std::cout << "Sender signaled to start" << std::endl;

  while(running.load()) {
    int bytes = recv(sockfd, buffer, sizeof(buffer), 0);
    if(bytes == 0) {
        continue;
    } else if(bytes < 0) {
        continue;
        //running.store(false);
        //break;            
    }


    if(!running) {
      break;
    }

    Packet packet;
    if(Packet::FromBuffer(buffer, bytes, packet)) {
        processMessage(packet);
    }
  }
  std::cout << "Receiver done running" << std::endl;
  return true;
}

void MulticastReceiver::start() {
  std::lock_guard<std::mutex> lock(m);
  running.store(true);
  cond.notify_one();
  std::cout << "Receiver started" << std::endl;
}

void MulticastReceiver::stop() {
  std::lock_guard<std::mutex> lock(m);
  running.store(false);
  cond.notify_one();
  std::cout << "Receiver stopped" << std::endl;
}

bool MulticastReceiver::closeSocket() {
  close(sockfd);
  return true;
}

bool MulticastReceiver::processMessage(Packet& packet) {
  std::lock_guard<std::mutex> lock(m);
  message_queue.emplace(packet);
  return false;
}

}

