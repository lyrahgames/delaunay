#pragma once
#include <cassert>
#include <cmath>
//
#include <lyrahgames/delaunay/type_traits.hpp>

namespace lyrahgames::delaunay {

template <typename Real, size_t N>
struct vector {
  using value_type = Real;
  static constexpr auto size() noexcept { return N; }

  constexpr value_type& operator[](int i) noexcept { return data[i]; }
  constexpr const value_type& operator[](int i) const noexcept {
    return data[i];
  }

  constexpr value_type& operator()(int index) noexcept {
    assert((0 <= index) && (index < size()));
    return data[index];
  }
  constexpr const value_type& operator()(int index) const noexcept {
    return const_cast<const value_type&>(
        const_cast<vector*>(this)->operator()(index));
  }

  Real data[N]{};
};

template <typename Real, size_t N>
constexpr auto size(vector<Real, N> x) noexcept {
  return N;
}

template <typename Real, size_t N, typename Functor>
constexpr auto elementwise(const vector<Real, N>& x, Functor f) noexcept {
  vector<decltype(f(x[0])), N> result{};
  for (size_t i = 0; i < N; ++i) result[i] = f(x[i]);
  return result;
}

template <typename Real, size_t N, typename Functor>
constexpr auto elementwise(const vector<Real, N>& x, const vector<Real, N>& y,
                           Functor f) noexcept {
  vector<decltype(f(x[0], y[0])), N> result{};
  for (size_t i = 0; i < N; ++i) result[i] = f(x[i], y[i]);
  return result;
}

template <typename Real, size_t N, typename Functor>
constexpr void apply_elementwise(vector<Real, N>& x, Functor f) noexcept {
  for (size_t i = 0; i < N; ++i) x[i] = f(x[i]);
}

template <typename Real, size_t N, typename Functor, typename RFunctor>
constexpr auto reduction(const vector<Real, N>& x, const vector<Real, N>& y,
                         Functor f, RFunctor g) noexcept {
  auto result = f(x[0], y[0]);
  for (size_t i = 1; i < N; ++i) result = g(result, f(x[i], y[i]));
  return result;
}

template <typename Real, size_t N>
constexpr auto operator+(const vector<Real, N>& x) noexcept {
  return elementwise(x, [](auto x) { return +x; });
}

template <typename Real, size_t N>
constexpr auto operator+(const vector<Real, N>& x,
                         const vector<Real, N>& y) noexcept {
  return elementwise(x, y, [](auto x, auto y) { return x + y; });
}

template <typename Real, size_t N>
constexpr auto operator-(const vector<Real, N>& x) noexcept {
  return elementwise(x, [](auto x) { return -x; });
}

template <typename Real, size_t N>
constexpr auto operator-(const vector<Real, N>& x,
                         const vector<Real, N>& y) noexcept {
  return elementwise(x, y, [](auto x, auto y) { return x - y; });
}

template <typename Real, size_t N>
constexpr auto operator*(Real a, const vector<Real, N>& v) noexcept {
  return elementwise(v, [a](auto x) { return a * x; });
}

template <typename Real, size_t N>
constexpr auto operator*(const vector<Real, N>& v, Real a) noexcept {
  return elementwise(v, [a](auto x) { return x * a; });
}

template <typename Real, size_t N>
constexpr auto operator*(const vector<Real, N>& x,
                         const vector<Real, N>& y) noexcept {
  return elementwise(x, y, [](auto x, auto y) { return x * y; });
}

template <typename Real, size_t N>
constexpr auto operator/(const vector<Real, N>& x,
                         const vector<Real, N>& y) noexcept {
  return elementwise(x, y, [](auto x, auto y) { return x / y; });
}

template <typename Real, size_t N>
constexpr auto operator/(const vector<Real, N>& v, Real a) noexcept {
  return (1 / a) * v;
}

template <typename Real, size_t N>
constexpr auto operator/(Real a, const vector<Real, N>& v) noexcept {
  return elementwise(v, [a](auto x) { return a / x; });
}

template <typename Real, size_t N>
constexpr auto dot(const vector<Real, N>& x,
                   const vector<Real, N>& y) noexcept {
  return reduction(
      x, y, [](auto x, auto y) { return x * y; },
      [](auto x, auto y) { return x + y; });
}

template <typename Real, size_t N>
constexpr auto sqnorm(const vector<Real, N>& x) noexcept {
  return dot(x, x);
}

template <typename Real, size_t N>
constexpr auto norm(const vector<Real, N>& x) noexcept {
  return std::sqrt(sqnorm(x));
}

template <typename Real, size_t N>
constexpr auto min(const vector<Real, N>& x,
                   const vector<Real, N>& y) noexcept {
  return elementwise(x, y, [](auto x, auto y) { return (x < y) ? x : y; });
}

template <typename Real, size_t N>
constexpr auto max(const vector<Real, N>& x,
                   const vector<Real, N>& y) noexcept {
  return elementwise(x, y, [](auto x, auto y) { return (x > y) ? x : y; });
}

constexpr auto has_public_xy = is_valid([](auto&& v) -> decltype(v.x * v.y) {});
constexpr auto has_public_xyz =
    is_valid([](auto&& v) -> decltype(v.x * v.y * v.z) {});
constexpr auto has_function_xy =
    is_valid([](auto&& v) -> decltype(v.x() * v.y()) {});
constexpr auto has_function_xyz =
    is_valid([](auto&& v) -> decltype(v.x() * v.y() * v.z()) {});
constexpr auto has_accesss_operator =
    is_valid([](auto&& v) -> decltype(v[0] * v[1]) {});

template <typename Vector, typename T>
constexpr auto vector_cast(T&& t) noexcept
    -> std::enable_if_t<decltype(has_accesss_operator(t))::value, Vector> {
  Vector result{};
  for (size_t i = 0; i < Vector::size(); ++i) result[i] = t[i];
  return result;
}

template <typename Vector, typename T>
constexpr auto vector_cast(T&& t) noexcept
    -> std::enable_if_t<!decltype(has_accesss_operator(t))::value &&
                            (Vector::size() == 2) &&
                            decltype(has_public_xy(t))::value,
                        Vector> {
  return {t.x, t.y};
}

template <typename Vector, typename T>
constexpr auto vector_cast(T&& t) noexcept
    -> std::enable_if_t<!decltype(has_accesss_operator(t))::value &&
                            (Vector::size() == 3) &&
                            decltype(has_public_xyz(t))::value,
                        Vector> {
  return {t.x, t.y, t.z};
}

using float32x2 = vector<float, 2>;
using float64x2 = vector<double, 2>;
using float32x3 = vector<float, 3>;
using float64x3 = vector<double, 3>;

}  // namespace lyrahgames::delaunay