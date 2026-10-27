// The host implementation of hal.hpp: a simulated board, controlled through hal_sim.hpp.
// The Arduino sketch has its own implementation (temperature_alarm/hal_arduino.cpp); this
// file is never copied there. Global state, not thread-safe.
#include <deque>
#include <map>
#include <string>
#include <vector>

#include "tempalarm/hal.hpp"
#include "tempalarm/hal_sim.hpp"

namespace tempalarm {
namespace hal {

namespace {

struct Board
{
    float celsius = 20.0f;
    std::deque<char> received;
    std::map<int, bool> pins;
    std::map<int, bool> buttons;
    std::map<int, std::string> lcd;
    std::vector<std::string> sent;
    bool keepSent = true;
};

Board& board()
{
    static Board instance;
    return instance;
}

}  // namespace

float readCelsius(int /*pin*/) { return board().celsius; }

void writePin(int pin, bool high) { board().pins[pin] = high; }

bool isButtonDown(int pin)
{
    const auto found = board().buttons.find(pin);
    return found != board().buttons.end() && found->second;
}

void lcdWrite(int row, Text text) { board().lcd[row] = text; }

bool serialAvailable() { return !board().received.empty(); }

char serialRead()
{
    const char c = board().received.front();
    board().received.pop_front();
    return c;
}

void serialWriteLine(Text text)
{
    if (board().keepSent)
    {
        board().sent.emplace_back(text);
    }
}

namespace sim {

void setCelsius(float celsius) { board().celsius = celsius; }

void receive(const std::string& characters)
{
    board().received.insert(board().received.end(), characters.begin(), characters.end());
}

void setButton(int pin, bool down) { board().buttons[pin] = down; }

bool pin(int pin)
{
    const auto found = board().pins.find(pin);
    return found != board().pins.end() && found->second;
}

std::string lcdRow(int row)
{
    const auto found = board().lcd.find(row);
    return found == board().lcd.end() ? std::string() : found->second;
}

const std::vector<std::string>& sent() { return board().sent; }

void keepSent(bool keep) { board().keepSent = keep; }

void reset() { board() = Board(); }

}  // namespace sim
}  // namespace hal
}  // namespace tempalarm
