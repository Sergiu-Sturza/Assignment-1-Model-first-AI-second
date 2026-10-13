#include "cabin/sensor.hpp"

#include <utility>

namespace cabin {

Sensor::Sensor(int id, std::string name) : id_(id), name_(std::move(name)) {}

int Sensor::getId() const { return id_; }

const std::string& Sensor::getName() const { return name_; }

const std::vector<Reading>& Sensor::getReadings() const { return readings_; }

}  // namespace cabin
