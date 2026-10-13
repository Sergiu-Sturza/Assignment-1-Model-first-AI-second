#include "cabin/user.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace cabin {

User::User(int id, std::string name) : id_(id), name_(std::move(name)) {}

int User::getId() const { return id_; }

const std::string& User::getName() const { return name_; }

void User::addCabin(std::unique_ptr<Cabin> c)
{
    if (!c)
    {
        throw std::invalid_argument("User::addCabin: null cabin");
    }
    const int id = c->getId();
    if (std::ranges::any_of(cabins_, [id](const std::unique_ptr<Cabin>& p) { return p->getId() == id; }))
    {
        throw std::invalid_argument("User::addCabin: duplicate cabin id");
    }
    cabins_.push_back(std::move(c));
}

void User::removeCabin(int id)
{
    std::erase_if(cabins_, [id](const std::unique_ptr<Cabin>& p) { return p->getId() == id; });
}

void User::addAlert(const Alert& a) { alerts_.push_back(a); }

std::vector<Cabin*> User::getCabins() const
{
    std::vector<Cabin*> cabins;
    cabins.reserve(cabins_.size());
    for (const std::unique_ptr<Cabin>& c : cabins_)
    {
        cabins.push_back(c.get());
    }
    return cabins;
}

const std::vector<Alert>& User::getAlerts() const { return alerts_; }

}  // namespace cabin
