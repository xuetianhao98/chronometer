#include <chrono>
#include <chronometer/chronometer.hpp>
#include <iostream>
#include <thread>

using namespace chronometer;

int main() {
  // 获取 Chronometer 单例实例
  auto& chrono = Chronometer::instance();

  std::cout << "=== Chronometer 基本用法示例 ===" << std::endl;
  std::cout << std::endl;

  // 示例 1: 基本用法 - start / stop
  std::cout << "1. 基本用法 (start / stop):" << std::endl;
  uint64_t id1 = chrono.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  double elapsed_us = chrono.stop(id1);
  std::cout << "   耗时: " << elapsed_us << " 微秒" << std::endl;
  std::cout << std::endl;

  // 示例 2: 使用 elapsed 检查中间耗时（不会停止计时器）
  std::cout << "2. 使用 elapsed 检查中间耗时:" << std::endl;
  uint64_t id2 = chrono.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(30));
  double mid_elapsed = chrono.elapsed(id2);
  std::cout << "   中间耗时: " << mid_elapsed << " 微秒" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  double total_elapsed = chrono.stop(id2);
  std::cout << "   总耗时: " << total_elapsed << " 微秒" << std::endl;
  std::cout << std::endl;

  // 示例 3: 使用不同的时间单位
  std::cout << "3. 使用不同的时间单位:" << std::endl;
  uint64_t id3 = chrono.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  double ns = chrono.stop(id3, TimeUnit::Nanoseconds);
  std::cout << "   Nanoseconds:  " << ns << " ns" << std::endl;

  // 重新计时展示其他单位
  uint64_t id4 = chrono.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  double us = chrono.stop(id4, TimeUnit::Microseconds);
  std::cout << "   Microseconds: " << us << " us" << std::endl;

  uint64_t id5 = chrono.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  double ms = chrono.stop(id5, TimeUnit::Milliseconds);
  std::cout << "   Milliseconds: " << ms << " ms" << std::endl;

  uint64_t id6 = chrono.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  double s = chrono.stop(id6, TimeUnit::Seconds);
  std::cout << "   Seconds:      " << s << " s" << std::endl;
  std::cout << std::endl;

  // 示例 4: 模拟代码块性能测量
  std::cout << "4. 模拟代码块性能测量:" << std::endl;
  uint64_t block_id = chrono.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "   代码块执行中..." << std::endl;
  double block_time = chrono.stop(block_id, TimeUnit::Milliseconds);
  std::cout << "   代码块执行耗时: " << block_time << " ms" << std::endl;

  return 0;
}
