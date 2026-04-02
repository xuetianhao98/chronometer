#include <chrono>
#include <chronometer/chronometer.hpp>
#ifdef CHRONOMETER_USE_RDTSC
#include <chronometer/rdtsc_clock.hpp>
#endif
#include <iostream>
#include <thread>

using namespace chronometer;

int main() {
#ifdef CHRONOMETER_USE_RDTSC
  chronometer::RdtscClock::Calibrate();
  std::cout << "=== Timing Backend: RDTSC (High Precision) ===" << std::endl;
  std::cout << "TSC frequency: " << chronometer::RdtscClock::GetTicksPerNs()
            << " ticks/ns" << std::endl;
#else
  std::cout << "=== Timing Backend: steady_clock ===" << std::endl;
#endif
  std::cout << std::endl;

  // 获取 Chronometer 单例实例
  auto& chrono = Chronometer::Instance();

  std::cout << "=== Chronometer 基本用法示例 ===" << std::endl;
  std::cout << std::endl;

  // 示例 1: 基本用法 - start / stop
  std::cout << "1. 基本用法 (start / stop):" << std::endl;
  uint64_t id1 = chrono.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  double elapsed_us = chrono.Stop(id1);
  std::cout << "   耗时: " << elapsed_us << " 微秒" << std::endl;
  std::cout << std::endl;

  // 示例 2: 使用 elapsed 检查中间耗时（不会停止计时器）
  std::cout << "2. 使用 elapsed 检查中间耗时:" << std::endl;
  uint64_t id2 = chrono.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(30));
  double mid_elapsed = chrono.Elapsed(id2);
  std::cout << "   中间耗时: " << mid_elapsed << " 微秒" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  double total_elapsed = chrono.Stop(id2);
  std::cout << "   总耗时: " << total_elapsed << " 微秒" << std::endl;
  std::cout << std::endl;

  // 示例 3: 使用不同的时间单位
  std::cout << "3. 使用不同的时间单位:" << std::endl;
  uint64_t id3 = chrono.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  double ns = chrono.Stop(id3, TimeUnit::kNanoseconds);
  std::cout << "   Nanoseconds:  " << ns << " ns" << std::endl;

  // 重新计时展示其他单位
  uint64_t id4 = chrono.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  double us = chrono.Stop(id4, TimeUnit::kMicroseconds);
  std::cout << "   Microseconds: " << us << " us" << std::endl;

  uint64_t id5 = chrono.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  double ms = chrono.Stop(id5, TimeUnit::kMilliseconds);
  std::cout << "   Milliseconds: " << ms << " ms" << std::endl;

  uint64_t id6 = chrono.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  double s = chrono.Stop(id6, TimeUnit::kSeconds);
  std::cout << "   Seconds:      " << s << " s" << std::endl;
  std::cout << std::endl;

  // 示例 4: 模拟代码块性能测量
  std::cout << "4. 模拟代码块性能测量:" << std::endl;
  uint64_t block_id = chrono.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "   代码块执行中..." << std::endl;
  double block_time = chrono.Stop(block_id, TimeUnit::kMilliseconds);
  std::cout << "   代码块执行耗时: " << block_time << " ms" << std::endl;

#ifdef CHRONOMETER_USE_RDTSC
  // High-precision RDTSC timing demo
  std::cout << "\n--- RDTSC High-Precision Timing ---" << std::endl;
  auto rdtsc_id = chrono.Start();
  // 模拟一个短暂的计算任务
  volatile int sum = 0;
  for (int i = 0; i < 1000; ++i) {
    sum += i;
  }
  auto rdtsc_ns = chrono.Stop(rdtsc_id, chronometer::TimeUnit::kNanoseconds);
  std::cout << "Short computation took: " << rdtsc_ns << " ns" << std::endl;
#endif

  return 0;
}
