cmake_minimum_required(VERSION 3.20)
project(temperature_alarm CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TEMPALARM_BUILD_TESTS "Build the doctest unit tests in tests/" OFF)
option(TEMPALARM_BUILD_BENCH "Build the benchmarks in bench/" OFF)

# The classes of the design. The same sources (all but hal_host.cpp) are compiled for the
# Arduino by arduino-cli, after tools/sync_firmware.py has copied them into the sketch.
add_library(tempalarm STATIC
    src/button.cpp
    src/buzzer.cpp
    src/command_handler.cpp
    src/command_parser.cpp
    src/hal_host.cpp
    src/interval_timer.cpp
    src/lcd_display.cpp
    src/message_formatter.cpp
    src/serial_link.cpp
    src/temperature_monitor.cpp
    src/temperature_sensor.cpp
    src/threshold.cpp
    src/threshold_editor.cpp
)
target_include_directories(tempalarm PUBLIC include)
if(MSVC)
    target_compile_options(tempalarm PRIVATE /W4)
else()
    target_compile_options(tempalarm PRIVATE -Wall -Wextra)
endif()

add_executable(demo src/demo.cpp)
target_link_libraries(demo PRIVATE tempalarm)

# One executable per sequence diagram: scenarios/<scenario>.cpp for sequence-<scenario>.drawio.
add_executable(scenario_measure_temperature scenarios/measure_temperature.cpp)
target_link_libraries(scenario_measure_temperature PRIVATE tempalarm)
add_executable(scenario_change_threshold scenarios/change_threshold.cpp)
target_link_libraries(scenario_change_threshold PRIVATE tempalarm)
add_executable(scenario_adjust_threshold scenarios/adjust_threshold.cpp)
target_link_libraries(scenario_adjust_threshold PRIVATE tempalarm)

# Built only on request, so the verifier's build and its compile_commands.json never see them.
if(TEMPALARM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
if(TEMPALARM_BUILD_BENCH)
    add_subdirectory(bench)
endif()
