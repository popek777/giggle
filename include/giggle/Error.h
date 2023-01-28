#pragma once

#include <stdexcept>

namespace giggle {

class Error : public std::runtime_error {

  using runtime_error::runtime_error;
};

} // namespace giggle
