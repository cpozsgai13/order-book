#include "TCPReceiver.h"
#include "LogUtils.h"
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
#include <sstream>
#include <iomanip>

namespace MarketData
{

TCPReceiver::TCPReceiver(const std::string& addr, int listen_port, std::queue<Packet>& output, 
  std::mutex& mut, std::condition_variable& cnd):
  address(addr),
  port(listen_port),
  output_message_queue(output),
  m(mut),
  cond(cnd)
{
}

bool TCPReceiver::openSocket() {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
        return false;
	}

  uint32_t reuse = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
  {
      return false;
  }
  socklen_t socklen;
  int timeout_ms = 1000;
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
  {
      return false;
  }

  return true;
}

bool TCPReceiver::run() {
    if(!openSocket()) {
        return false;
    }

    //  Connect to server
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address.c_str());

    if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        return false;
    }
    //std::cout << "TCP receiver connected - waiting for start signal" << std::endl;
    

    //  Wait until started
    {
      std::unique_lock<std::mutex> lock(m);
      cond.wait(lock, [this]{return running.load();});
    }
    //std::cout << "TCP receiver signaled to start" << std::endl;

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
          // auto ss = logHex(buffer, bytes);
          //logHexI(buffer, bytes);

          // std::cout << ss.str();
          std::lock_guard<std::mutex> lock(m);
          output_message_queue.push(packet);
          cond.notify_all();
          //std::cout << "TCP enqueued packet " << packet.GetSize() << std::endl;

          //processMessage(packet));
      } else {
        std::cout << "Failed to create packet from buffer" << std::endl;
      }
    }
    //std::cout << "Receiver done running" << std::endl;
    closeSocket();
    return true;
}

void TCPReceiver::start() {
  std::lock_guard<std::mutex> lock(m);
  running.store(true);
  cond.notify_one();
  //std::cout << "Receiver started" << std::endl;
}

void TCPReceiver::stop() {
  std::lock_guard<std::mutex> lock(m);
  running.store(false);
  cond.notify_one();
}

bool TCPReceiver::closeSocket() {
  close(sockfd);
  return true;
}

// bool TCPReceiver::processMessage(Packet&& packet) {
//   std::cout << "TCP receiver enqueing packet: " << packet.header.total_length << std::endl;
//   logHexI(packet.data.buffer, packet.header.total_length);

//   std::lock_guard<std::mutex> lock(m);
//   output_message_queue.emplace(packet);
//   cond.notify_one();
//   //std::cout << "TCP enqueued packet " << packet.GetSize() << std::endl;
//   return true;
// }

}

