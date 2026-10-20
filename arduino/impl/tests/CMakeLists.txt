# Unit tests (doctest). Built only with -DTEMPALARM_BUILD_TESTS=ON, so the verifier's build
# and its compile_commands.json never see them.
add_executable(tempalarm_tests
    test_main.cpp
    test_buzzer.cpp
    test_command_handler.cpp
    test_command_parser.cpp
    test_interval_timer.cpp
    test_lcd_display.cpp
    test_message_formatter.cpp
    test_serial_link.cpp
    test_temperature_monitor.cpp
    test_temperature_sensor.cpp
    test_threshold.cpp
)
target_link_libraries(tempalarm_tests PRIVATE tempalarm)
target_include_directories(tempalarm_tests SYSTEM PRIVATE third_party)
if(MSVC)
    target_compile_options(tempalarm_tests PRIVATE /W4 /WX)
else()
    target_compile_options(tempalarm_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_test(NAME tempalarm_tests COMMAND tempalarm_tests)
