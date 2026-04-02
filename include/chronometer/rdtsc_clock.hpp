/**
 * @file rdtsc_clock.hpp
 * @brief 基于 x86 RDTSC 指令的高精度时钟类
 *
 * 提供基于 x86 R64 TSC (Time Stamp Counter) 的纳秒级精度计时功能。
 * 使用 RDTSCP 指令读取 TSC，配合 LFENCE 序列化确保准确性。
 * 仅支持 x86_64 架构。
 */

#pragma once

#if defined(__x86_64__) || defined(_M_X64)

#include <x86intrin.h>

#include <chrono>
#include <cstdint>

namespace chronometer {

/**
 * @brief 基于 x86 RDTSC 指令的高精度时钟类
 *
 * 使用 CPU 的 Time Stamp Counter 提供纳秒级精度的计时功能。
 * 需要在首次使用前调用 Calibrate() 进行校准。
 * 仅支持 x86_64 架构。
 */
class RdtscClock {
 public:
  /// @brief 读取当前 TSC 计数
  /// @return uint64_t 当前 TSC 计数值
  ///
  /// 使用 RDTSCP 指令读取 TSC，配合 LFENCE 序列化指令确保读取准确性。
  /// RDTSCP 相比 RDTSC 具有更好的序列化特性，能提供更稳定的读数。
  static uint64_t rdtsc();

  /**
   * @brief 校准 TSC 与纳秒的转换比率
   *
   * 使用 std::chrono::steady_clock 作为参考时钟，通过多次采样计算
   * TSC ticks 与纳秒的比率。采样 10ms 的 sleep 间隔，进行 3-5 次
   * 采样取平均值以提高精度。
   *
   * 校准精度目标：误差 < 1%
   *
   * @note 此方法不是线程安全的，应在程序初始化阶段单线程调用。
   */
  static void Calibrate();

  /**
   * @brief 获取当前 TSC 计数
   * @return uint64_t 当前 TSC 计数值
   *
   * 调用 rdtsc() 获取当前 CPU 的 Time Stamp Counter 值。
   */
  static uint64_t Now();

  /**
   * @brief 将 TSC 差值转换为纳秒
   * @param start 起始 TSC 计数
   * @param end 结束 TSC 计数
   * @return std::chrono::nanoseconds 转换后的纳秒时间
   *
   * 使用校准后的 ticks_per_ns_ 比率将 TSC 差值转换为纳秒。
   * 如果未进行校准，返回的结果将不准确。
   */
  static std::chrono::nanoseconds ToNanoseconds(uint64_t start, uint64_t end);

  /**
   * @brief 获取校准后的 TSC ticks 每纳秒比率
   * @return double TSC ticks 每纳秒的比率
   *
   * 返回 Calibrate() 方法计算出的 ticks_per_ns_ 值，供测试使用。
   * 如果未进行校准，返回 0.0。
   */
  static double GetTicksPerNs();

 private:
  inline static double ticks_per_ns_{0.0};  ///< TSC ticks 每纳秒的比率
  inline static bool calibrated_{false};    ///< 校准状态标志
};

}  // namespace chronometer

#endif  // defined(__x86_64__) || defined(_M_X64)
