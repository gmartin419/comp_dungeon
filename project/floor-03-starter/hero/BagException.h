// COMP 2450 — Floor 3 starter
// hero/BagException.h — YOU implement the constructor body and what()
// this week.
//
// A user-defined exception. Inherits from std::exception so the standard
//     try { … } catch (const std::exception& e) { std::cout << e.what(); }
// pattern works out of the box. ZyBook §5.4 is the worked example; this
// is the version we use in the game.
//
// We keep the entire class in this header (constructor inline, what()
// inline) — it is small enough that a separate .cpp would be overhead
// with no gain. Not a template, though; template is the next file over.

#pragma once

#include <cstddef>
#include <exception>
#include <sstream>
#include <string>

namespace dungeon {

class BagException : public std::exception {
public:
    // Built when someone indexes past the end of a Bag. Carries BOTH
    // the bad index AND the actual size, so the error message can say
    // exactly what went wrong ("index 99 out of bounds for size 5")
    // instead of something vague like "bad index."
    BagException(std::size_t bad_index, std::size_t bag_size) {
        std::ostringstream oss;
        oss << "index " << bad_index
            << "out of bounds for size " << bag_size;
        msg_ = oss.str();
    }

    // Override the std::exception "what's wrong" virtual. MUST be
    // noexcept to match the base class contract — a what() that throws
    // inside a catch block is a particularly cursed kind of bug.
    const char* what() const noexcept override {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

}  // namespace dungeon
