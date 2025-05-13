#include "TCPSender.h"
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
#include <memory>
#include <string.h>
#include <unistd.h>

namespace MarketData 
{

TCPSender::TCPSender(uint16_t send_port):
port(send_port)
{
  openSocket();
}

bool TCPSender::openSocket() {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		return false;
	}

  struct sockaddr_in server_addr;
  memset((void *)&server_addr, 0, sizeof(server_addr));
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    std::cout << "Bind to sender socket failed";
    return false;
  }

  if(listen(sockfd, 1) < 0) {
    return false;
  }
	return true;
}

bool TCPSender::closeSocket() {
	close(sockfd);
	return false;
}

bool TCPSender::enqueue(Packet& packet) {
  std::lock_guard<std::mutex> lock(m);
  message_queue.push(packet);
  cond.notify_one();
	return true;
}

bool TCPSender::run() {
  if(sockfd < 0) {
    return false;
  }

  //  Wait until started
  {
    std::unique_lock<std::mutex> lock(m);
    cond.wait(lock, [this]{return running.load();});
  }

  struct sockaddr_in client_addr;
  uint32_t clilen = sizeof(client_addr);
  listen(sockfd, 5);
  client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &clilen);

  if(client_sockfd < 0)
  {
    return false;
  }
  //std::cout << "Client connected to TCP sender" << std::endl;

  Packet packet;
  while(running.load()) {
    if(!running.load()) {
      //std::cout << "Exiting run loop" << std::endl;
      break;
    }

    if(message_queue.pop(packet)) {
      //  Send the packet.
      if(packet.header.num_messages > 0) {
        auto sz = packet.header.total_length;
        int sent = send(client_sockfd, (char *)&packet, sz, 0);
        if(sent <= 0) {
          std::cout << "Send error: " << errno << std::endl;
        } else {
          std::cout << "Sender sent: " << sent << std::endl;
        }
      }
    }
    if(!running) {
      break;
    }

  }
  std::cout << "Sender stopped" << std::endl;
  return true;
}

void TCPSender::start() {
  std::lock_guard<std::mutex> lock(m);
  running.store(true);
  cond.notify_one();
}

void TCPSender::stop() {
  std::lock_guard<std::mutex> lock(m);
  running.store(false);
  cond.notify_one();
  closeSocket();
}

}

