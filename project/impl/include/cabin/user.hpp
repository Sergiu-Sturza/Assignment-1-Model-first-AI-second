#pragma once

#include <memory>
#include <string>
#include <vector>

#include "cabin/alert.hpp"
#include "cabin/cabin.hpp"

namespace cabin {

/// The owner of cabins and the receiver of their alerts.
class User
{
public:
    User(int id, std::string name);

    int getId() const;
    const std::string& getName() const;

    /// Takes ownership of @p c.
    /// @throws std::invalid_argument if @p c is null or a cabin with its id is present
    void addCabin(std::unique_ptr<Cabin> c);
    /// Destroys the cabin with @p id, in whatever state it is. An unknown id does nothing.
    void removeCabin(int id);
    /// Stores @p a; typically each alert returned by Cabin::checkSensors().
    void addAlert(const Alert& a);

    /// The cabins, in the order they were added. The pointers stay owned by the user and
    /// dangle once the cabin is removed.
    std::vector<Cabin*> getCabins() const;
    /// The received alerts, oldest first.
    const std::vector<Alert>& getAlerts() const;

private:
    int id_;
    std::string name_;
    std::vector<std::unique_ptr<Cabin>> cabins_;   // User *-- "*" Cabin : cabins
    std::vector<Alert> alerts_;                    // User *-- "*" Alert : alerts
};

}  // namespace cabin
