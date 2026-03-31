cmake_minimum_required(VERSION 3.14)

project(chronometer VERSION 1.0.0 LANGUAGES CXX)

# C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 创建静态库
add_library(chronometer src/chronometer.cpp)
add_library(chronometer::chronometer ALIAS chronometer)

target_include_directories(chronometer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# 兼容性处理：PROJECT_IS_TOP_LEVEL 在 CMake 3.21+ 中定义
if(CMAKE_VERSION VERSION_LESS "3.21")
    get_directory_property(has_parent PARENT_DIRECTORY)
    if(NOT has_parent)
        set(PROJECT_IS_TOP_LEVEL TRUE)
    else()
        set(PROJECT_IS_TOP_LEVEL FALSE)
    endif()
endif()

# 选项
option(CHRONOMETER_BUILD_TESTS "Build tests" ${PROJECT_IS_TOP_LEVEL})
option(CHRONOMETER_BUILD_EXAMPLES "Build examples" ${PROJECT_IS_TOP_LEVEL})

# 条件性子目录
if(CHRONOMETER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(CHRONOMETER_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

# 安装规则
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS chronometer
    EXPORT chronometer-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT chronometer-targets
    NAMESPACE chronometer::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/chronometer
)

# 生成配置文件
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/chronometer-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/chronometer-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/chronometer
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/chronometer-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/chronometer-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/chronometer-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/chronometer
)
