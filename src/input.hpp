#ifndef GEM_INPUT_HPP
#define GEM_INPUT_HPP

#include "fwd.hpp"

namespace gem {

namespace Input {

enum class Button : usize {
    Up,
    Down,
    Left,
    Right,
    Start,
    Select,
    A,
    B,
};

bool isButtonPressed(Button b);

}  // namespace Input

}  // namespace gem

#endif
