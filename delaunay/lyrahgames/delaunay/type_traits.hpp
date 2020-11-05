#pragma once
#include <type_traits>

namespace lyrahgames::delaunay {

namespace detail {

template <typename F, typename... Args,
          typename = decltype(std::declval<F>()(std::declval<Args&&>()...))>
std::true_type is_valid(void*);

template <typename F, typename... Args>
std::false_type is_valid(...);

}  // namespace detail

constexpr auto is_valid = [](auto f) constexpr {
  return [](auto&&... args) constexpr {
    return decltype(
        detail::is_valid<decltype(f), decltype(args)&&...>(nullptr)){};
  };
};

}  // namespace lyrahgames::delaunay