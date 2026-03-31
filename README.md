# Chronometer

线程安全的高精度计时器管理库，基于 C++20，提供纳秒级精度计时。

## 项目信息

- **项目名**: Chronometer
- **版本**: 1.0.0
- **类型**: C++20 静态库
- **设计模式**: 基于 Meyer's Singleton 模式的线程安全单例实现

## 功能特性

- Meyer's Singleton 线程安全单例
- 原子操作生成计时器 ID
- 读写锁（std::shared_mutex）实现并发安全
- 基于 std::chrono::steady_clock 的高精度计时
- 支持纳秒/微秒/毫秒/秒四种时间单位

## 快速开始

### 使用示例

```cpp
#include <chronometer/chronometer.hpp>
#include <iostream>
#include <thread>

using namespace chronometer;

int main() {
    auto& chrono = Chronometer::instance();

    // 基本计时
    uint64_t id = chrono.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    double elapsed_us = chrono.stop(id); // 默认返回微秒
    std::cout << "耗时: " << elapsed_us << " μs" << std::endl;

    // 中间检查（不停止计时器）
    uint64_t id2 = chrono.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    double mid = chrono.elapsed(id2); // 检查中间耗时
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    double total = chrono.stop(id2);  // 停止并获取总耗时

    // 指定时间单位
    uint64_t id3 = chrono.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    double ms = chrono.stop(id3, TimeUnit::Milliseconds);
    std::cout << "耗时: " << ms << " ms" << std::endl;

    return 0;
}
```

## API 概览

| 方法 | 说明 |
|------|------|
| `static Chronometer& instance()` | 获取单例实例 |
| `uint64_t start()` | 启动计时器，返回唯一 ID |
| `double stop(uint64_t id, TimeUnit unit = TimeUnit::Microseconds)` | 停止计时器并返回耗时，ID 不存在则抛出 `std::out_of_range` |
| `double elapsed(uint64_t id, TimeUnit unit = TimeUnit::Microseconds) const` | 获取已运行时间（不停止计时器），ID 不存在则抛出 `std::out_of_range` |

### TimeUnit 枚举

- `Nanoseconds` — 纳秒
- `Microseconds` — 微秒（默认）
- `Milliseconds` — 毫秒
- `Seconds` — 秒

## 通过 FetchContent 集成

```cmake
include(FetchContent)
FetchContent_Declare(chronometer
    GIT_REPOSITORY https://github.com/user/chronometer.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(chronometer)

target_link_libraries(your_target PRIVATE chronometer::chronometer)
```

## 构建项目

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### CMake 选项

| 选项 | 说明 | 默认值 |
|------|------|--------|
| `CHRONOMETER_BUILD_TESTS` | 构建单元测试 | `ON` |
| `CHRONOMETER_BUILD_EXAMPLES` | 构建示例程序 | `ON` |

## 许可证

详见 [LICENSE](LICENSE) 文件。
