#ifndef _LOG_UTILS_H_
#define _LOG_UTILS_H_

#include <sstream>
#include <iomanip>

const auto logHex=[](const char *buffer, size_t len) -> std::stringstream {
  auto bytes_per_line = 20;

  auto num_lines = len / bytes_per_line + (len % bytes_per_line > 0);

  std::stringstream ss;
  for(size_t i = 0; i < num_lines; ++i) {

    int bytes_this_line = i < num_lines - 1 ? bytes_per_line : len % bytes_per_line;
    for(int j = 0; j < bytes_this_line; ++j) {
      ss << std::hex << std::setfill('0') << buffer[j + i*bytes_per_line] << " ";
    }
    ss << std::endl;
    std::cout << ss.str();
  }
  return ss;
};

const auto logHexI=[](const char *buffer, size_t len) {
  auto bytes_per_line = 20;

  auto num_lines = len / bytes_per_line + (len % bytes_per_line > 0);

  for(size_t i = 0; i < num_lines; ++i) {

    int bytes_this_line = i < num_lines - 1 ? bytes_per_line : len % bytes_per_line;
    for(int j = 0; j < bytes_this_line; ++j) {
      std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[j + i*bytes_per_line] << " ";
    }
    std::cout << std::endl;
  }
};

const auto logHexC=[](const char *buffer, size_t len) {
  auto bytes_per_line = 20;

  auto num_lines = len / bytes_per_line + (len % bytes_per_line > 0);

  for(size_t i = 0; i < num_lines; ++i) {

    int bytes_this_line = i < num_lines - 1 ? bytes_per_line : len % bytes_per_line;
    for(int j = 0; j < bytes_this_line; ++j) {
      if(isascii(buffer[j + i*bytes_per_line])) {
        std::cout << std::setw(2) << std::setfill('0') << buffer[j + i*bytes_per_line] << " ";
      } else {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[j + i*bytes_per_line] << " ";
      }
    }
    std::cout << std::endl;
  }
};

#endif