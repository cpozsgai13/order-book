#include "TCPReceiverThread.h"
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
#include <poll.h>
#include <thread>

namespace MarketData
{

TCPReceiverThread::TCPReceiverThread(const std::string& addr, int listen_port,
  RingBufferSPSC<MarketData::Packet, RING_BUFFER_SIZE>& output,
  int retry_count, int retry_interval_sec):
  address(addr),
  port(listen_port),
  output_message_queue(output),
  retry_count(retry_count),
  retry_interval_sec(retry_interval_sec)
{
}


bool TCPReceiverThread::openSocket() {
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
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
  {
    return false;
  }
  return true;
}

bool TCPReceiverThread::run() {
  //  Connect to server
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(address.c_str());

  bool connected = false;
  if(retry_count < 0) {

    while(!connected && running.load()) {
      if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0)
      {
        connected = true;
        break;
      }
    }
  } else {
    for(int i = 0; i < retry_count; ++i) {
        if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
          connected = true;
          break;
        } else {
          std::this_thread::sleep_for(std::chrono::seconds(retry_interval_sec));
        }
    }    
  }

  if(!connected) {
    closeSocket();
    return false;
  }

  //  Delay creation of the memory pool so it gets created in the correct
  //  thread.
  if(!tcp_memory_pool) {
    tcp_memory_pool = std::make_unique<TCPMemoryPool>();
  }
  struct pollfd fds{};
  fds.fd = sockfd;
  fds.events = POLLIN;
  int timeout_sec = 1;
  size_t packet_count = 0;
  size_t bad_packet_count = 0;
  while(running.load()) {
    int ret = poll(&fds, 1, timeout_sec*1000);
    if(ret > 0 && fds.revents & POLLIN) {
      std::unique_ptr<TCPMemoryPool::MemoryBuffer> buffer = tcp_memory_pool->allocate();
      uint8_t *cursor = buffer.get()->data();
      int bytes = recv(sockfd, cursor, sizeof(Header), 0);
      if(bytes <= 0) {
        tcp_memory_pool->deallocate(std::move(buffer));
        continue;
      }
      Header *header = reinterpret_cast<Header *>(cursor);
      if(header->num_messages == 0 || header->num_messages > Packet::MESSAGES_PER_PACKET || header->total_length == 0) {
        //std::cerr << "BAD HEADER: WTF " << std::endl;
        tcp_memory_pool->deallocate(std::move(buffer));
        ++bad_packet_count;
        continue;
      }
      cursor += sizeof(Header);
      int bytes_remaining = header->total_length - sizeof(Header);

      if(bytes_remaining + sizeof(Header) >= UDP_BUFFER_SIZE) {
        //std::cerr << "OUT OF RANGE bytes_remaining: " << bytes_remaining <<  " WTF " << std::endl;
        ++bad_packet_count;
        tcp_memory_pool->deallocate(std::move(buffer));
        continue;
      }

      int bytes_recvd_remaining = 0;
      while(bytes_recvd_remaining < bytes_remaining) {
        int recvd = recv(sockfd, cursor, bytes_remaining, 0);
        if(recvd > 0) {
          bytes_recvd_remaining += recvd;
          cursor += recvd;
          bytes_remaining -= recvd;
        }
      }
      bytes += bytes_recvd_remaining;
      if(bytes > static_cast<int>(sizeof(*buffer))) {
        tcp_memory_pool->deallocate(std::move(buffer));
        continue;
      }
      Packet packet;
      if(Packet::FromBuffer((char *)buffer.get()->data(), bytes, packet)) {
        ++packet_count;
        output_message_queue.push(packet);
      }
      tcp_memory_pool->deallocate(std::move(buffer));
    }
  }
  closeSocket();
  std::cout << "Packets: " << packet_count << ", bad: " << bad_packet_count << '\n';
  return true;
}

bool TCPReceiverThread::start() {
  if(!openSocket()) {
      return false;
  }

  running.store(true);
  return run();
}

void TCPReceiverThread::stop() {
  running.store(false);
}

bool TCPReceiverThread::closeSocket() {
  close(sockfd);
  return true;
}

}

