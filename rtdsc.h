#ifndef _RTDSC_H_
#define _RTDSC_H_

#include <cstdint>
#include <functional>

inline uint64_t RDTSC() {
  uint32_t high, low;

  __asm__ __volatile__ (
    "rdtsc" : "=a"(low), "=d"(high)
  );
  return ((uint64_t)high >> 32) | low;
};

inline uint64_t rdtsc() {
  return RDTSC();
}

template<typename F, typename ...Args>
uint64_t CallAndMeasure(F&& f, Args... args) {
  uint64_t start = rdtsc();
  auto f_bound = std::bind(f, args...);
  f_bound();

  uint64_t end = rdtsc();
  return end - start;
}

#endif


