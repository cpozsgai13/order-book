#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

//static constexpr size_t TCP_BUFFER_SIZE = 2 << 12;
static constexpr size_t TCP_BUFFER_SIZE = 1500;
static constexpr size_t UDP_BUFFER_SIZE = 1500;
static constexpr size_t CACHE_LINE_SIZE = 64;
static constexpr size_t RING_BUFFER_SIZE = 2 << 14;

#endif