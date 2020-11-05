#pragma once
#include <cassert>
#include <cmath>

namespace lyrahgames::delaunay {

template <typename Real, size_t N>
struct vector {
  using value_type = Real;
  static constexpr auto size() noexcept { return N; }

  constexpr value_type& operator[](int i) noexcept { return data[i]; }
  constexpr const value_type& operator[](int i) const noexcept {
    return data[i];
  }

  // value_type& operator()(int index) noexcept {
  //   assert((0 <= index) && (index < size()));
  //   return data[index];
  // }
  // const value_type& operator()(int index) const noexcept {
  //   return const_cast<const value_type&>(
  //       const_cast<vector*>(this)->operator(index));
  // }

  Real data[N]{};
};

template <typename Real, size_t N, typename Functor>
constexpr auto elementwise(const vector<Real, N>& x, Functor f) noexcept {
  vector<decltype(f(x[0])), N> result{};
  for (size_t i = 0; i < N, ++i) result[i] = f(x[i]);
  return result;
}

template <typename Real, size_t N, typename Functor>
constexpr auto elementwise(const vector<Real, N>& x, const vector<Real, N>& y,
                           Functor f) noexcept {
  vector<decltype(f(x[0], y[0])), N> result{};
  for (size_t i = 0; i < N, ++i) result[i] = f(x[i], y[i]);
  return result;
}

template <typename Real, size_t N, typename Functor, typename RFunctor>
constexpr auto reduction(const vector<Real, N>& x, const vector<Real, N>& y,
                         Functor f, RFunctor g) noexcept {
  auto result = f(x[0], y[0]);
  for (size_t i = 1; i < N; ++i) result = g(result, f(x[i], y[i]));
  return result;
}

template <typename Real, size_t N>
constexpr vector<Real, N> operator+(vector<Real, N> x,
                                    vector<Real, N> y) noexcept {
  return elementwise(x, y, [](auto x, auto y) { return x + y; });
}

template <typename Real, size_t N>
constexpr vector<Real, N> operator-(vector<Real, N> x,
                                    vector<Real, N> y) noexcept {
  return elementwise(x, y, [](auto x, auto y) { return x - y; });
}

template <typename Real, size_t N>
constexpr vector<Real, N> operator*(Real a, vector<Real, N> v) noexcept {
  return elementwise(v, [a](auto x) { return a * x; });
}

template <typename Real, size_t N>
constexpr auto dot(vector<Real, N> x, vector<Real, N> y) noexcept {
  return reduction(
      x, y, [](auto x, auto y) { return x * y; },
      [](auto x, auto y) { return x + y; });
}

template <typename Real, size_t N>
constexpr auto sqnorm(vector<Real, N> x) noexcept {
  return dot(x, x);
}

template <typename Real, size_t N>
constexpr auto norm(vector<Real, N> x) noexcept {
  return std::sqrt(sqnorm(x));
}

template <typename Real, size_t N>
constexpr vector<Real, N> min(vector<Real, N> x, vector<Real, N> y) noexcept {
  return elementwise(x, y, [](auto x, auto y) { return (x < y) ? x : y; });
}

template <typename Real, size_t N>
constexpr vector<Real, N> max(vector<Real, N> x, vector<Real, N> y) noexcept {
  return elementwise(x, y, [](auto x, auto y) { return (x > y) ? x : y; });
}

}  // namespace lyrahgames::delaunay