# Benchmarks. Built only with -DTEMPALARM_BUILD_BENCH=ON; meaningful in a Release build.
add_executable(tempalarm_bench bench_main.cpp)
target_link_libraries(tempalarm_bench PRIVATE tempalarm)
if(NOT MSVC)
    target_compile_options(tempalarm_bench PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
