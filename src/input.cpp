#include "input.hpp"

#include <SFML/Window/Keyboard.hpp>

#include <array>

namespace gem {
namespace {
const auto keyMapping = [] {
    std::array<sf::Keyboard::Key, 8> mapping;
    mapping[idx(Input::Button::Up)] = sf::Keyboard::Key::Up;
    mapping[idx(Input::Button::Down)] = sf::Keyboard::Key::Down;
    mapping[idx(Input::Button::Left)] = sf::Keyboard::Key::Left;
    mapping[idx(Input::Button::Right)] = sf::Keyboard::Key::Right;
    mapping[idx(Input::Button::Start)] = sf::Keyboard::Key::Enter;
    mapping[idx(Input::Button::Select)] = sf::Keyboard::Key::RShift;
    mapping[idx(Input::Button::A)] = sf::Keyboard::Key::Z;
    mapping[idx(Input::Button::B)] = sf::Keyboard::Key::X;
    return mapping;
}();
}
bool Input::isButtonPressed(const Input::Button b) {
    return sf::Keyboard::isKeyPressed(keyMapping[idx(b)]);
}

}  // namespace gem
