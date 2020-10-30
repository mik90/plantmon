#pragma once

namespace mik {

/**
 *  @brief Super basic implementation of std::optional, limited by default Arduino toolchain
 * settings
 */
template <class T>
class Optional {
private:
  T value_;
  bool has_value_;

public:
  constexpr Optional() noexcept : has_value_(false) {}

  explicit Optional(const T& value) {
    value_ = value;
    has_value_ = true;
  }

  explicit Optional(T&& value) {
    value_ = value;
    has_value_ = true;
  }

  const T& value() const { return value_; }
  T& value() { return value_; }

  constexpr bool has_value() const noexcept { return has_value_; }

  T value_or(T other) const {
    if (has_value_) {
      return value_;
    } else {
      return other;
    }
  }
};

} // namespace mik