#include <gtest/gtest.h>

#include <chrono>
#include <chronometer/chronometer.hpp>
#include <thread>
#include <vector>

using namespace chronometer;

// 启动计时器，sleep一小段时间，stop，验证返回值 > 0
TEST(ChronometerTest, StartAndStop) {
  auto& chrono = Chronometer::instance();
  uint64_t id = chrono.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  double elapsed = chrono.stop(id);
  EXPECT_GT(elapsed, 0.0);
}

// start，调用 elapsed 后计时器仍在，可再次 elapsed 和 stop
TEST(ChronometerTest, ElapsedDoesNotRemoveTimer) {
  auto& chrono = Chronometer::instance();
  uint64_t id = chrono.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  double e1 = chrono.elapsed(id);
  EXPECT_GT(e1, 0.0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  double e2 = chrono.elapsed(id);
  EXPECT_GT(e2, e1);

  double s = chrono.stop(id);
  EXPECT_GT(s, e2);
}

// start，sleep，elapsed 拿到 t1，再 sleep，elapsed 拿到 t2，验证 t2 > t1
TEST(ChronometerTest, ElapsedValueIncreases) {
  auto& chrono = Chronometer::instance();
  uint64_t id = chrono.start();

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  double t1 = chrono.elapsed(id);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  double t2 = chrono.elapsed(id);

  EXPECT_GT(t2, t1);

  chrono.stop(id);
}

// 分别用 Nanoseconds/Microseconds/Milliseconds/Seconds
// stop/elapsed，验证数量级关系合理
TEST(ChronometerTest, DifferentTimeUnits) {
  auto& chrono = Chronometer::instance();

  // 测试 ns/us/ms 的转换关系（使用较短的 sleep）
  uint64_t id1 = chrono.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  double ns = chrono.elapsed(id1, TimeUnit::Nanoseconds);
  double us = chrono.elapsed(id1, TimeUnit::Microseconds);
  double ms = chrono.elapsed(id1, TimeUnit::Milliseconds);

  // 验证数量级关系: 1ms = 1000us = 1,000,000ns
  // 允许一定的误差范围（±20%）
  EXPECT_NEAR(us * 1000.0, ns, ns * 0.2);
  EXPECT_NEAR(ms * 1000.0, us, us * 0.2);

  // 验证基本范围（50ms 的 sleep）
  EXPECT_GT(ms, 40.0);   // 至少 40ms
  EXPECT_LT(ms, 200.0);  // 不超过 200ms（给足余量）

  chrono.stop(id1);

  // 单独测试 Seconds 单位（需要更长的 sleep 才能产生非零值）
  uint64_t id2 = chrono.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(1100));  // 1.1秒

  double ms2 = chrono.elapsed(id2, TimeUnit::Milliseconds);
  double s = chrono.stop(id2, TimeUnit::Seconds);

  // Seconds 返回整数秒，1.1s 应该返回 1
  EXPECT_EQ(s, 1.0);
  // 验证 ms 和 s 的对应关系（1100ms ≈ 1s）
  EXPECT_NEAR(s * 1000.0, ms2, 200.0);  // 允许 ±200ms 误差
}

// 对不存在的 id 调用 stop 和 elapsed，验证抛出 std::out_of_range
TEST(ChronometerTest, InvalidIdThrows) {
  auto& chrono = Chronometer::instance();
  uint64_t invalid_id = 999999;

  EXPECT_THROW(chrono.stop(invalid_id), std::out_of_range);
  EXPECT_THROW(chrono.elapsed(invalid_id), std::out_of_range);
  EXPECT_THROW(chrono.stop(invalid_id, TimeUnit::Milliseconds),
               std::out_of_range);
  EXPECT_THROW(chrono.elapsed(invalid_id, TimeUnit::Seconds),
               std::out_of_range);
}

// 多线程同时 start 和 stop，验证不崩溃、不死锁，所有操作正常完成
TEST(ChronometerTest, ConcurrentStartStop) {
  auto& chrono = Chronometer::instance();
  const int num_threads = 8;
  const int iterations_per_thread = 100;

  std::vector<std::thread> threads;
  std::atomic<int> success_count{0};

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&]() {
      for (int j = 0; j < iterations_per_thread; ++j) {
        uint64_t id = chrono.start();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        double elapsed = chrono.stop(id);
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
