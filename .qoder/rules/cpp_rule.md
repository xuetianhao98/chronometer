---
trigger: manual
alwaysApply: false
---
# C++代码规范
- C++代码必须遵循Google C++ Style Guide
- 能够用C++20新特性实现的逻辑都要用C++20实现
- C++函数声明的注释必须使用C风格注释并遵循Doxygen格式
# 命名规范
| 类型                          | 命名风格                   | 示例               |
| --------------------------- | ---------------------- | ---------------- |
| 类 / struct / enum / typedef | UpperCamelCase         | `MyClass`        |
| 函数                          | UpperCamelCase         | `ComputeValue()` |
| 变量（局部/全局）                   | lower_snake_case       | `total_count`    |
| 成员变量                        | lower_snake_case + `_` | `total_count_`   |
| 常量（const/constexpr）         | kUpperCamelCase        | `kMaxSize`       |
| 枚举值                         | kUpperCamelCase        | `kIdle`          |
| 宏                           | ALL_CAPS               | `MAX_SIZE`       |
| 命名空间                        | lower_snake_case       | `my_namespace`   |
| 文件名                         | lower_snake_case       | `my_class.cc`    |
