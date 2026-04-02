/**
 * @file rdtsc_clock.cpp
 * @brief RdtscClock 类的实现
 */

#if defined(__x86_64__) || defined(_M_X64)

#include "chronometer/rdtsc_clock.hpp"

#include <thread>

namespace chronometer {

uint64_t RdtscClock::rdtsc() {
  unsigned int aux = 0;
  uint64_t tsc = __builtin_ia32_rdtscp(&aux);
  _mm_lfence();
  return tsc;
}

void RdtscClock::Calibrate() {
  constexpr int kSamples = 5;
  constexpr auto kSleepDuration = std::chrono::milliseconds(10);

  double total_ratio = 0.0;
  int valid_samples = 0;

  for (int i = 0; i < kSamples; ++i) {
    uint64_t tsc_start = rdtsc();
    auto chrono_start = std::chrono::steady_clock::now();

    std::this_thread::sleep_for(kSleepDuration);

    uint64_t tsc_end = rdtsc();
    auto chrono_end = std::chrono::steady_clock::now();

    uint64_t tsc_delta = tsc_end - tsc_start;
    auto chrono_delta = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            chrono_end - chrono_start)
                            .count();

    if (chrono_delta > 0) {
      total_ratio +=
          static_cast<double>(tsc_delta) / static_cast<double>(chrono_delta);
      ++valid_samples;
    }
  }

  if (valid_samples > 0) {
    ticks_per_ns_ = total_ratio / static_cast<double>(valid_samples);
    calibrated_ = true;
  }
}

uint64_t RdtscClock::Now() { return rdtsc(); }

std::chrono::nanoseconds RdtscClock::ToNanoseconds(uint64_t start,
                                                   uint64_t end) {
  uint64_t tsc_delta = end - start;
  double ns = static_cast<double>(tsc_delta) / ticks_per_ns_;
  return std::chrono::nanoseconds(static_cast<int64_t>(ns));
}

double RdtscClock::GetTicksPerNs() { return ticks_per_ns_; }

}  // namespace chronometer

#endif  // defined(__x86_64__) || defined(_M_X64)
