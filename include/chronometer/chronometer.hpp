/**
 * @file chronometer.hpp
 * @brief 线程安全的高精度计时器管理类头文件
 *
 * 提供基于 C++20 的线程安全单例计时器，支持纳秒级精度的计时功能。
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <shared_mutex>
#include <unordered_map>

namespace chronometer {

/**
 * @brief 时间单位枚举
 *
 * 定义计时结果可转换的目标时间单位。
 */
enum class TimeUnit {
  Nanoseconds,   ///< 纳秒 (10^-9 秒)
  Microseconds,  ///< 微秒 (10^-6 秒)
  Milliseconds,  ///< 毫秒 (10^-3 秒)
  Seconds        ///< 秒
};

/**
 * @brief 线程安全的单例计时器管理类
 *
 * 使用 C++20 特性实现的高精度计时器，基于 std::chrono::steady_clock
 * 提供纳秒级精度的计时功能。采用单例模式确保全局唯一实例，
 * 使用 shared_mutex 实现读写锁分离，保证线程安全。
 */
class Chronometer {
 public:
  /**
   * @brief 获取 Chronometer 单例实例
   * @return Chronometer& 单例对象的引用
   *
   * 使用 Meyer's Singleton 模式实现，C++11 起保证线程安全。
   */
  static Chronometer& instance();

  Chronometer(const Chronometer&) = delete;
  Chronometer& operator=(const Chronometer&) = delete;
  Chronometer(Chronometer&&) = delete;
  Chronometer& operator=(Chronometer&&) = delete;

  /**
   * @brief 启动一个新的计时器
   * @return uint64_t 新计时器的唯一标识符
   *
   * 线程安全：使用原子操作生成 ID，独占锁保护计时器映射。
   */
  uint64_t start();

  /**
   * @brief 停止指定计时器并返回经过的时间
   * @param id 计时器标识符
   * @param unit 返回结果的时间单位，默认为微秒
   * @return double 经过的时间（以指定单位表示）
   * @throws std::out_of_range 如果计时器 ID 不存在
   *
   * 线程安全：使用独占锁保护计时器映射。
   * 调用后该计时器将被移除。
   */
  double stop(uint64_t id, TimeUnit unit = TimeUnit::Microseconds);

  /**
   * @brief 获取指定计时器当前已运行的时间（不停止计时器）
   * @param id 计时器标识符
   * @param unit 返回结果的时间单位，默认为微秒
   * @return double 已运行的时间（以指定单位表示）
   * @throws std::out_of_range 如果计时器 ID 不存在
   *
   * 线程安全：使用共享锁实现并发读取。
   */
  double elapsed(uint64_t id, TimeUnit unit = TimeUnit::Microseconds) const;

 private:
  Chronometer() = default;

  std::atomic<uint64_t> next_id_{0};  ///< 下一个计时器 ID 的原子计数器
  mutable std::shared_mutex mutex_;  ///< 保护 timers_ 的读写锁
  std::unordered_map<uint64_t, std::chrono::steady_clock::time_point>
      timers_;  ///< 计时器 ID 到起始时间点的映射
};

}  // namespace chronometer
