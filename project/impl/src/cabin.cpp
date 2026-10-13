#include "cabin/cabin.hpp"

#include <algorithm>
#include <format>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "cabin/power_meter.hpp"
#include "cabin/security_sensor.hpp"
#include "cabin/temperature_sensor.hpp"

namespace cabin {

namespace {

// The last alert id each cabin has handed out. The design draws no attribute for it, so
// it lives here, keyed by the cabin; ~Cabin() erases the entry.
std::unordered_map<const Cabin*, int>& lastAlertIds()
{
    static std::unordered_map<const Cabin*, int> ids;
    return ids;
}

}  // namespace

Cabin::Cabin(int id, std::string name, std::string location)
    : id_(id), name_(std::move(name)), location_(std::move(location))
{
}

Cabin::~Cabin() { lastAlertIds().erase(this); }

int Cabin::getId() const { return id_; }

const std::string& Cabin::getName() const { return name_; }

const std::string& Cabin::getLocation() const { return location_; }

void Cabin::addSensor(std::unique_ptr<Sensor> s)
{
    if (!s)
    {
        throw std::invalid_argument("Cabin::addSensor: null sensor");
    }
    const int id = s->getId();
    if (std::ranges::any_of(sensors_, [id](const std::unique_ptr<Sensor>& p) { return p->getId() == id; }))
    {
        throw std::invalid_argument("Cabin::addSensor: duplicate sensor id");
    }
    sensors_.push_back(std::move(s));
    handle(Event::addSensor);
}

void Cabin::removeSensor(int id)
{
    const auto removed =
        std::erase_if(sensors_, [id](const std::unique_ptr<Sensor>& p) { return p->getId() == id; });
    if (removed > 0 && noSensors())
    {
        handle(Event::LastSensorRemoved);
    }
}

std::vector<Sensor*> Cabin::getSensors() const
{
    std::vector<Sensor*> sensors;
    sensors.reserve(sensors_.size());
    for (const std::unique_ptr<Sensor>& s : sensors_)
    {
        sensors.push_back(s.get());
    }
    return sensors;
}

std::vector<Alert> Cabin::checkSensors()
{
    // Severity and message of the alert for reading r of sensor s (assumptions A5, A7).
    // A lambda, not a helper method: it reads the fields through friendship, so the trace
    // shows only the drawn calls, and it adds no undrawn member.
    const auto describe = [](const Sensor& s, const Reading& r) -> std::pair<Severity, std::string> {
        if (const auto* t = dynamic_cast<const TemperatureSensor*>(&s))
        {
            const char* kind = r.value_ < t->minThreshold_ ? "low" : "high";
            return {r.value_ < 0.0 ? Severity::HIGH : Severity::MEDIUM,
                    std::format("{}: temperature {} ({} °C)", s.name_, kind, r.value_)};
        }
        if (dynamic_cast<const PowerMeter*>(&s) != nullptr)
        {
            return {Severity::LOW, std::format("{}: consumption ({} kWh)", s.name_, r.value_)};
        }
        if (dynamic_cast<const SecuritySensor*>(&s) != nullptr)
        {
            return {Severity::HIGH, std::format("{}: intrusion ({})", s.name_, r.value_)};
        }
        return {Severity::MEDIUM, std::format("{}: out of range ({})", s.name_, r.value_)};
    };

    int& lastAlertId = lastAlertIds()[this];
    std::vector<Alert> alerts;
    for (const std::unique_ptr<Sensor>& sensor : sensors_)
    {
        const Reading r = sensor->read();   // Cabin -> <sensor> : read()
        if (!sensor->shouldAlert(r))        // Cabin -> <sensor> : shouldAlert(r)
        {
            continue;
        }
        auto [severity, message] = describe(*sensor, r);
        alerts.emplace_back(++lastAlertId, r.time_, severity, std::move(message));
    }

    lastCheck_ = !alerts.empty();
    handle(Event::MeasurementIntervalElapsed);
    if (!alerts.empty())
    {
        handle(Event::AlertSent);   // the alerts leave the cabin with the return below
    }
    return alerts;
}

void Cabin::handle(Event event)
{
    for (const Transition& t : transitions)
    {
        if (t.from != state_ || t.event != event)
        {
            continue;
        }
        if (t.guard != nullptr && !(this->*t.guard)())
        {
            continue;
        }
        if (t.action != nullptr)
        {
            (this->*t.action)();
        }
        state_ = t.to;
        return;
    }
    // No transition for this event in this state: the event is ignored.
}

Cabin::State Cabin::state() const { return state_; }

bool Cabin::hasSensors() const { return !sensors_.empty(); }

bool Cabin::noSensors() const { return sensors_.empty(); }

bool Cabin::outOfRange() const { return lastCheck_; }

bool Cabin::inRange() const { return !lastCheck_; }

}  // namespace cabin
