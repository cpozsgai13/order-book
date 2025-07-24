#include "TCPSenderThread.h"
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
#include "rtdsc.h"
#include <thread>
#include <chrono>

namespace MarketData 
{

static constexpr size_t NANOS_PER_SEC = 1'000'000'000;

TCPSenderThread::TCPSenderThread(uint16_t send_port, int send_rate):
port(send_port),
packets_per_second(send_rate)
{
}

bool TCPSenderThread::openSocket() {
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

bool TCPSenderThread::closeSocket() {
	close(sockfd);
	return true;
}

bool TCPSenderThread::enqueue(Packet&& packet) {
  message_queue.push(packet);
	return true;
}

bool TCPSenderThread::run() {
  if(sockfd < 0) {
    return false;
  }

  struct sockaddr_in client_addr;
  uint32_t clilen = sizeof(client_addr);
  if(listen(sockfd, 5) != 0) {
    closeSocket();
    return false;
  }
  client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &clilen);

  if(client_sockfd < 0)
  {
    closeSocket();
    return false;
  }

  auto thread_start_time = std::chrono::steady_clock::now().time_since_epoch().count();
  int64_t delay_ns = NANOS_PER_SEC/packets_per_second.load();
  while(running.load()) {
    if(!running.load()) {
      //std::cout << "Exiting run loop" << std::endl;
      break;
    }
    if(message_queue.pop(packet)) {
      //  Send the packet.
      if(packet.data.packet.header.num_messages > 0) {
        std::this_thread::sleep_for(std::chrono::nanoseconds(delay_ns));
        auto sz = packet.data.packet.header.total_length;
        int sent = send(client_sockfd, (char *)&packet, sz, 0);
        if(sent <= 0) {
          std::cout << "Send error: " << errno << std::endl;
        } else {
          //std::cout << "Sender sent: " << sent << std::endl;
        }
      }
    } else {
      auto thread_end_time = std::chrono::steady_clock::now().time_since_epoch().count();
      double delta_t = (thread_end_time - thread_start_time)/1'000'000;
      std::cout << "Message queue took " << delta_t << " ms to send packets " << std::endl;
      running.store(false);
      break;
    }
    if(!running) {
      break;
    }

  }
  //std::cout << "Sender stopped" << std::endl;
  return closeSocket();
}

bool TCPSenderThread::start(int core) {
  if(core != -1) {
    this->core = core;
    cpu_set_t producer_core_set;
    CPU_ZERO(&producer_core_set);
    CPU_SET(core, &producer_core_set);
    pthread_t self = pthread_self();
    int rc = pthread_setaffinity_np(self, sizeof(cpu_set_t), &producer_core_set);
    if(rc != 0) {
      std::cout << "Failed to pin producer core " << core << std::endl;
      return false;
    }

  }

  if(!openSocket()) {
    return false;
  }

  running.store(true);
  return run();
}

void TCPSenderThread::stop() {
  running.store(false);
}

}

