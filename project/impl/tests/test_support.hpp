#pragma once

// Shared test helpers: a fixture that isolates each test from the global hardware seam.

#include <chrono>
#include <map>

#include "cabin/hardware.hpp"

namespace test {

/// The default simulated clock's first time stamp.
inline cabin::DateTime epoch()
{
    using namespace std::chrono;
    return cabin::DateTime{sys_days{year{2026} / January / 1}};
}

/// Restores the default sampler and clock before and after each test that uses it.
struct HardwareFixture
{
    HardwareFixture() { cabin::hardware::reset(); }
    ~HardwareFixture() { cabin::hardware::reset(); }

    HardwareFixture(const HardwareFixture&) = delete;
    HardwareFixture& operator=(const HardwareFixture&) = delete;

    /// Every sensor id in @p values samples its value; any other id samples 0.
    static void setSamples(std::map<int, double> values)
    {
        cabin::hardware::setSampler([values](int id) {
            const auto it = values.find(id);
            return it == values.end() ? 0.0 : it->second;
        });
    }
};

}  // namespace test
