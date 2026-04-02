#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <chronometer/chronometer.hpp>
#include <thread>
#include <vector>

using namespace chronometer;

// 启动计时器，sleep一小段时间，stop，验证返回值 > 0
TEST(ChronometerTest, StartAndStop) {
  auto& chrono = Chronometer::Instance();
  uint64_t id = chrono.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  double elapsed = chrono.Stop(id);
  EXPECT_GT(elapsed, 0.0);
}

// start，调用 elapsed 后计时器仍在，可再次 elapsed 和 stop
TEST(ChronometerTest, ElapsedDoesNotRemoveTimer) {
  auto& chrono = Chronometer::Instance();
  uint64_t id = chrono.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  double e1 = chrono.Elapsed(id);
  EXPECT_GT(e1, 0.0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  double e2 = chrono.Elapsed(id);
  EXPECT_GT(e2, e1);

  double s = chrono.Stop(id);
  EXPECT_GT(s, e2);
}

// start，sleep，elapsed 拿到 t1，再 sleep，elapsed 拿到 t2，验证 t2 > t1
TEST(ChronometerTest, ElapsedValueIncreases) {
  auto& chrono = Chronometer::Instance();
  uint64_t id = chrono.Start();

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  double t1 = chrono.Elapsed(id);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  double t2 = chrono.Elapsed(id);

  EXPECT_GT(t2, t1);

  chrono.Stop(id);
}

// 分别用 Nanoseconds/Microseconds/Milliseconds/Seconds
// stop/elapsed，验证数量级关系合理
TEST(ChronometerTest, DifferentTimeUnits) {
  auto& chrono = Chronometer::Instance();

  // 测试 ns/us/ms 的转换关系（使用较短的 sleep）
  uint64_t id1 = chrono.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  double ns = chrono.Elapsed(id1, TimeUnit::kNanoseconds);
  double us = chrono.Elapsed(id1, TimeUnit::kMicroseconds);
  double ms = chrono.Elapsed(id1, TimeUnit::kMilliseconds);

  // 验证数量级关系: 1ms = 1000us = 1,000,000ns
  // 允许一定的误差范围（±20%）
  EXPECT_NEAR(us * 1000.0, ns, ns * 0.2);
  EXPECT_NEAR(ms * 1000.0, us, us * 0.2);

  // 验证基本范围（50ms 的 sleep）
  EXPECT_GT(ms, 40.0);   // 至少 40ms
  EXPECT_LT(ms, 200.0);  // 不超过 200ms（给足余量）

  chrono.Stop(id1);

  // 单独测试 Seconds 单位（需要更长的 sleep 才能产生非零值）
  uint64_t id2 = chrono.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(1100));  // 1.1秒

  double ms2 = chrono.Elapsed(id2, TimeUnit::kMilliseconds);
  double s = chrono.Stop(id2, TimeUnit::kSeconds);

  // Seconds 返回整数秒，1.1s 应该返回 1
  EXPECT_EQ(s, 1.0);
  // 验证 ms 和 s 的对应关系（1100ms ≈ 1s）
  EXPECT_NEAR(s * 1000.0, ms2, 200.0);  // 允许 ±200ms 误差
}

// 对不存在的 id 调用 stop 和 elapsed，验证抛出 std::out_of_range
TEST(ChronometerTest, InvalidIdThrows) {
  auto& chrono = Chronometer::Instance();
  uint64_t invalid_id = 999999;

  EXPECT_THROW(chrono.Stop(invalid_id), std::out_of_range);
  EXPECT_THROW(chrono.Elapsed(invalid_id), std::out_of_range);
  EXPECT_THROW(chrono.Stop(invalid_id, TimeUnit::kMilliseconds),
               std::out_of_range);
  EXPECT_THROW(chrono.Elapsed(invalid_id, TimeUnit::kSeconds),
               std::out_of_range);
}

// 多线程同时 start 和 stop，验证不崩溃、不死锁，所有操作正常完成
TEST(ChronometerTest, ConcurrentStartStop) {
  auto& chrono = Chronometer::Instance();
  const int num_threads = 8;
  const int iterations_per_thread = 100;

  std::vector<std::thread> threads;
  std::atomic<int> success_count{0};

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&]() {
      for (int j = 0; j < iterations_per_thread; ++j) {
        uint64_t id = chrono.Start();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        double elapsed = chrono.Stop(id);
        if (elapsed > 0.0) {
          success_count++;
        }
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(success_count.load(), num_threads * iterations_per_thread);
}

#ifdef CHRONOMETER_USE_RDTSC

#include <chronometer/rdtsc_clock.hpp>

// 验证校准后的频率合理
TEST(RdtscClockTest, RdtscCalibration) {
  RdtscClock::Calibrate();
  double ticks_per_ns = RdtscClock::GetTicksPerNs();

  // 断言频率 > 0
  EXPECT_GT(ticks_per_ns, 0.0);

  // 断言频率在合理范围内：0.5 ~ 6.0 ticks/ns（对应 0.5-6 GHz）
  EXPECT_GE(ticks_per_ns, 0.5);
  EXPECT_LE(ticks_per_ns, 6.0);
}

// 对比 RDTSC 和 steady_clock 计时结果
TEST(RdtscClockTest, RdtscPrecision) {
  RdtscClock::Calibrate();

  auto steady_start = std::chrono::steady_clock::now();
  uint64_t rdtsc_start = RdtscClock::Now();

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  auto steady_end = std::chrono::steady_clock::now();
  uint64_t rdtsc_end = RdtscClock::Now();

  // 计算 steady_clock 的耗时（纳秒）
  auto steady_duration =
      std::chrono::duration_cast<std::chrono::nanoseconds>(steady_end -
                                                           steady_start)
          .count();

  // 计算 RDTSC 的耗时（纳秒）
  auto rdtsc_duration =
      RdtscClock::ToNanoseconds(rdtsc_start, rdtsc_end).count();

  // 断言 RDTSC 结果与 steady_clock 结果的误差 < 5%
  double diff = std::abs(static_cast<double>(steady_duration) -
                         static_cast<double>(rdtsc_duration));
  double error_rate = diff / static_cast<double>(steady_duration);
  EXPECT_LT(error_rate, 0.05);
}

// 验证 RDTSC 调用开销极低
TEST(RdtscClockTest, RdtscLowOverhead) {
  RdtscClock::Calibrate();

  uint64_t tsc1 = RdtscClock::Now();
  uint64_t tsc2 = RdtscClock::Now();

  // 计算两次调用的时间差
  auto overhead = RdtscClock::ToNanoseconds(tsc1, tsc2).count();

  // 断言开销 < 1000ns（1微秒）
  EXPECT_LT(overhead, 1000);
}

// 多线程并发 RDTSC 计时
TEST(RdtscClockTest, RdtscConcurrent) {
  RdtscClock::Calibrate();

  const int num_threads = 8;
  const int iterations_per_thread = 100;

  std::vector<std::thread> threads;
  std::atomic<int> success_count{0};

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&]() {
      for (int j = 0; j < iterations_per_thread; ++j) {
        auto& chrono = Chronometer::Instance();
        uint64_t id = chrono.Start();
        // brief work
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        double elapsed = chrono.Stop(id);
        if (elapsed > 0.0) {
          success_count++;
        }
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  // 断言 success_count == 800
  EXPECT_EQ(success_count.load(), num_threads * iterations_per_thread);
}

#endif  // CHRONOMETER_USE_RDTSC
