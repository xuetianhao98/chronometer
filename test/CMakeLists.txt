include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)

FetchContent_MakeAvailable(googletest)

add_executable(chronometer_test test_chronometer.cpp)

target_link_libraries(chronometer_test
    PRIVATE
        chronometer::chronometer
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(chronometer_test)
