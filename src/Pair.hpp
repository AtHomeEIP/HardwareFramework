#ifndef PAIR_HPP
#define PAIR_HPP

namespace athome {
namespace utility {
namespace container {
template <typename T, typename U>
class Pair {
 public:
  Pair() {}

  Pair(const T &a, const &U b) : first(a), second(b) {}

  Pair(const &Pair other) : first(other.first), second(other.second) {}

  Pair &operator=(const Pair &other) {
    first = other.first;
    second = other.second;
    return *this;
  }

  ~Pair() {}

 public:
  T first;
  U second;
};
}  // namespace container
}  // namespace utility
}  // namespace athome

#endif /* PAIR_HPP */
