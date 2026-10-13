cmake_minimum_required(VERSION 3.20)
project(cabin_monitor CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CABIN_BUILD_TESTS "Build the doctest unit tests in tests/" OFF)
option(CABIN_COVERAGE "Instrument the library and tests for gcov coverage" OFF)

add_library(cabin STATIC
    src/alert.cpp
    src/cabin.cpp
    src/hardware.cpp
    src/power_meter.cpp
    src/reading.cpp
    src/security_sensor.cpp
    src/sensor.cpp
    src/temperature_sensor.cpp
    src/user.cpp
)
target_include_directories(cabin PUBLIC include)
if(MSVC)
    target_compile_options(cabin PRIVATE /W4)
else()
    target_compile_options(cabin PRIVATE -Wall -Wextra)
endif()
if(CABIN_COVERAGE)
    target_compile_options(cabin PUBLIC --coverage -O0)
    target_link_options(cabin PUBLIC --coverage)
endif()

add_executable(demo src/demo.cpp)
target_link_libraries(demo PRIVATE cabin)

# One executable per sequence diagram: scenarios/<scenario>.cpp for sequence-<scenario>.drawio.
add_executable(scenario_check_sensors scenarios/check_sensors.cpp)
target_link_libraries(scenario_check_sensors PRIVATE cabin)

if(CABIN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
