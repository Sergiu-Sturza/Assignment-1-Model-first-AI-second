#pragma once

#include <memory>
#include <string>
#include <vector>

#include "cabin/alert.hpp"
#include "cabin/sensor.hpp"

namespace cabin {

/// A monitored cabin: owns its sensors and checks them once per measurement interval.
///
/// Its life cycle is the state machine of diagrams/input/state-cabin.drawio:
/// Unconfigured until it has a sensor, then Monitoring, and briefly Alerting user while
/// the alerts of one check are being sent.
class Cabin
{
public:
    // The states and events of diagrams/input/state-cabin.drawio.
    enum class State { Unconfigured, Monitoring, AlertingUser };
    enum class Event { addSensor, LastSensorRemoved, MeasurementIntervalElapsed, AlertSent };

    Cabin(int id, std::string name, std::string location);
    ~Cabin();

    Cabin(const Cabin&) = delete;              // alert ids are kept per cabin object
    Cabin& operator=(const Cabin&) = delete;

    int getId() const;
    const std::string& getName() const;
    const std::string& getLocation() const;

    /// Takes ownership of @p s and fires Event::addSensor.
    /// @throws std::invalid_argument if @p s is null or a sensor with its id is present
    void addSensor(std::unique_ptr<Sensor> s);
    /// Destroys the sensor with @p id, if there is one, and fires Event::LastSensorRemoved
    /// when no sensor is left. An unknown id does nothing.
    void removeSensor(int id);
    /// The sensors, in the order they were added. The pointers stay owned by the cabin and
    /// dangle once the sensor is removed.
    std::vector<Sensor*> getSensors() const;

    /// One measurement interval: reads every sensor in order, creates one Alert for each
    /// reading that must raise one, fires MeasurementIntervalElapsed and, if there are
    /// alerts, AlertSent. The caller delivers the alerts, e.g. with User::addAlert().
    std::vector<Alert> checkSensors();

    /// Fires @p event: takes the first transition of the table that leaves the current
    /// state on @p event and whose guard holds. An event with no transition is ignored.
    void handle(Event event);
    State state() const;

private:
    // Guards of the state machine.
    bool hasSensors() const;
    bool noSensors() const;
    bool outOfRange() const;   // the last check found at least one alert
    bool inRange() const;      // the last check found none

    int id_;
    std::string name_;
    std::string location_;
    std::vector<std::unique_ptr<Sensor>> sensors_;   // Cabin *-- "*" Sensor : sensors

    State state_ = State::Unconfigured;   // the initial state
    bool lastCheck_ = false;              // true: the last checkSensors() raised an alert

    // The transition table: one row per arrow in the state diagram.
    struct Transition
    {
        State from;
        Event event;
        State to;
        bool (Cabin::*guard)() const;   // nullptr: no guard
        void (Cabin::*action)();        // nullptr: no action
    };
    static constexpr Transition transitions[] = {
        {State::Unconfigured, Event::addSensor,                  State::Monitoring,   &Cabin::hasSensors, nullptr},
        {State::Monitoring,   Event::LastSensorRemoved,          State::Unconfigured, nullptr,            nullptr},
        {State::Monitoring,   Event::MeasurementIntervalElapsed, State::AlertingUser, &Cabin::outOfRange, nullptr},
        {State::Monitoring,   Event::MeasurementIntervalElapsed, State::Monitoring,   &Cabin::inRange,    nullptr},
        {State::AlertingUser, Event::AlertSent,                  State::Monitoring,   nullptr,            nullptr},
        {State::AlertingUser, Event::LastSensorRemoved,          State::Unconfigured, nullptr,            nullptr},
    };
};

}  // namespace cabin
