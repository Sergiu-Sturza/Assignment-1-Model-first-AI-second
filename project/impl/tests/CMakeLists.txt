# Unit tests (doctest). Built only with -DCABIN_BUILD_TESTS=ON, so the verifier's build and
# its compile_commands.json never see them.
add_executable(cabin_tests
    test_main.cpp
    test_alert.cpp
    test_cabin.cpp
    test_cabin_state.cpp
    test_hardware.cpp
    test_power_meter.cpp
    test_reading.cpp
    test_security_sensor.cpp
    test_sensor.cpp
    test_temperature_sensor.cpp
    test_user.cpp
)
target_link_libraries(cabin_tests PRIVATE cabin)
target_include_directories(cabin_tests SYSTEM PRIVATE third_party)
if(MSVC)
    target_compile_options(cabin_tests PRIVATE /W4 /WX)
else()
    target_compile_options(cabin_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_test(NAME cabin_tests COMMAND cabin_tests)
