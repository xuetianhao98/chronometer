/**
 * @file chronometer.cpp
 * @brief Chronometer 类的实现文件
 *
 * 实现线程安全的高精度计时器功能，使用 C++20 的 shared_mutex
 * 和 chrono 库实现纳秒级精度计时。
 */

#include "chronometer/chronometer.hpp"

#include <mutex>
#include <stdexcept>

namespace chronometer {

namespace {

/**
 * @brief 将纳秒级时长转换为指定时间单位
 * @param duration 以纳秒表示的时长
 * @param unit 目标时间单位
 * @return double 转换后的时长值
 *
 * 内部辅助函数，使用 C++20 的 std::chrono::duration_cast 进行单位转换。
 */
double convertDuration(std::chrono::nanoseconds duration, TimeUnit unit) {
  switch (unit) {
    case TimeUnit::kNanoseconds:
      return static_cast<double>(duration.count());
    case TimeUnit::kMicroseconds:
      return static_cast<double>(
          std::chrono::duration_cast<std::chrono::microseconds>(duration)
              .count());
    case TimeUnit::kMilliseconds:
      return static_cast<double>(
          std::chrono::duration_cast<std::chrono::milliseconds>(duration)
              .count());
    case TimeUnit::kSeconds:
      return static_cast<double>(
          std::chrono::duration_cast<std::chrono::seconds>(duration).count());
  }
  return 0.0;
}

}  // anonymous namespace

Chronometer& Chronometer::Instance() {
  // Meyer's Singleton: C++11 起保证线程安全
  static Chronometer instance;
#ifdef CHRONOMETER_USE_RDTSC
  static bool calibrated = [] {
    RdtscClock::Calibrate();
    return true;
  }();
  (void)calibrated;
#endif
  return instance;
}

uint64_t Chronometer::Start() {
  // 使用 relaxed 内存序生成唯一 ID（仅需原子性，无需同步）
  uint64_t id = next_id_.fetch_add(1, std::memory_order_relaxed);
  // 在获取锁之前采样时间（前置采样规范）
#ifdef CHRONOMETER_USE_RDTSC
  auto start_time = RdtscClock::Now();
#else
  auto start_time = std::chrono::steady_clock::now();
#endif
  // 独占锁保护 timers_ 的写入
  std::unique_lock lock(mutex_);
  timers_[id] = start_time;
  return id;
}

double Chronometer::Stop(uint64_t id, TimeUnit unit) {
  // 前置采样
#ifdef CHRONOMETER_USE_RDTSC
  auto end_time = RdtscClock::Now();
#else
  auto end_time = std::chrono::steady_clock::now();
#endif
  // 独占锁：stop 会修改 timers_（删除条目）
  std::unique_lock lock(mutex_);
  auto it = timers_.find(id);
  if (it == timers_.end()) {
    throw std::out_of_range("Timer id not found");
  }
  auto start_time = it->second;
  timers_.erase(it);

#ifdef CHRONOMETER_USE_RDTSC
  auto duration = RdtscClock::ToNanoseconds(start_time, end_time);
#else
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
      end_time - start_time);
#endif
  return convertDuration(duration, unit);
}

double Chronometer::Elapsed(uint64_t id, TimeUnit unit) const {
  // 前置采样
#ifdef CHRONOMETER_USE_RDTSC
  auto end_time = RdtscClock::Now();
#else
  auto end_time = std::chrono::steady_clock::now();
#endif
  // 共享锁：elapsed 只读，支持并发查询
  std::shared_lock lock(mutex_);
  auto it = timers_.find(id);
  if (it == timers_.end()) {
    throw std::out_of_range("Timer id not found");
  }
  auto start_time = it->second;

#ifdef CHRONOMETER_USE_RDTSC
  auto duration = RdtscClock::ToNanoseconds(start_time, end_time);
#else
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
      end_time - start_time);
#endif
  return convertDuration(duration, unit);
}

}  // namespace chronometer
