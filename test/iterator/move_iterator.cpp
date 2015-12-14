// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Casey Carter 2015
//  Copyright Eric Niebler 2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#include <stl2/iterator.hpp>
#include <stl2/functional.hpp>
#include <stl2/detail/algorithm/copy.hpp>
#include <vector>
#include "../simple_test.hpp"

namespace ranges = __stl2;

struct A {
  static std::size_t copy_count;
  static std::size_t move_count;

  static void clear() { copy_count = move_count = 0; }

  A() = default;
  A(const A&) { ++copy_count; }
  A(A&&) noexcept { ++move_count; }
  A& operator=(const A&) & { ++copy_count; return *this; }
  A& operator=(A&&) & noexcept { ++move_count; return *this; }
};

std::size_t A::copy_count;
std::size_t A::move_count;

void test_move_iterator() {
  static constexpr std::size_t N = 42;
  std::vector<A> vec(N);
  std::vector<A> vec2;
  vec2.reserve(ranges::size(vec));

  A::clear();

  {
    auto first = ranges::make_move_iterator(ranges::begin(vec)),
      last = ranges::make_move_iterator(ranges::end(vec));
    auto out = ranges::back_inserter(vec2);

    for (; first != last; ++first, ++out) {
      *out = *first;
    }
  }

  CHECK(ranges::size(vec2) == N);
  CHECK(A::copy_count == std::size_t{0});
  CHECK(A::move_count == N);
}

void test_iter_move() {
  static constexpr std::size_t N = 42;
  std::vector<A> vec(N);
  std::vector<A> vec2;
  vec2.reserve(ranges::size(vec));

  A::clear();

  {
    auto first = ranges::begin(vec),
      last = ranges::end(vec);
    auto out = ranges::back_inserter(vec2);

    for (; first != last; ++first, ++out) {
      *out = ranges::iter_move(first);
    }
  }

  CHECK(ranges::size(vec2) == N);
  CHECK(A::copy_count == std::size_t{0});
  CHECK(A::move_count == N);
}

void test_both() {
  static constexpr std::size_t N = 42;
  std::vector<A> vec(N);
  std::vector<A> vec2;
  vec2.reserve(ranges::size(vec));

  A::clear();

  {
    auto first = ranges::make_move_iterator(ranges::begin(vec)),
      last = ranges::make_move_iterator(ranges::end(vec));
    auto out = ranges::back_inserter(vec2);

    for (; first != last; ++first, ++out) {
      *out = ranges::iter_move(first);
    }
  }

  CHECK(ranges::size(vec2) == N);
  CHECK(A::copy_count == std::size_t{0});
  CHECK(A::move_count == N);
}

template <class T>
class proxy_iterator {
public:
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using iterator_category = ranges::input_iterator_tag;

  proxy_iterator() = default;
  explicit proxy_iterator(T* p) :
    ptr_{p} {}

  ranges::reference_wrapper<T> operator*() const {
    return ranges::reference_wrapper<T>{*ptr_};
  }

  bool operator==(const proxy_iterator& that) const {
    return ptr_ == that.ptr_;
  }
  bool operator!=(const proxy_iterator& that) const {
    return !(*this == that);
  }

  proxy_iterator& operator++() & {
    ++ptr_;
    return *this;
  }
  proxy_iterator operator++(int) & {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  friend T&& iter_move(const proxy_iterator& p) {
    return ranges::move(*p.ptr_);
  }

private:
  T* ptr_;
};

void test_proxy_iterator() {
  static constexpr std::size_t N = 42;
  std::vector<A> vec(N);
  std::vector<A> vec2;
  vec2.reserve(ranges::size(vec));

  static_assert(
    ranges::models::Same<
      ranges::reference_t<proxy_iterator<A>>,
      ranges::reference_wrapper<A>>);
  static_assert(
    ranges::models::Same<
      ranges::reference_t<const proxy_iterator<A>>,
      ranges::reference_wrapper<A>>);
  static_assert(
    ranges::models::Same<
      ranges::rvalue_reference_t<proxy_iterator<A>>,
      A&&>);

  {
    static_assert(
      ranges::models::Same<
        ranges::rvalue_reference_t<
          ranges::move_iterator<proxy_iterator<A>>>,
        A&&>);
    auto first = ranges::make_move_iterator(proxy_iterator<A>{ranges::data(vec)}),
      last = ranges::make_move_iterator(proxy_iterator<A>{ranges::data(vec) + ranges::size(vec)});
    auto out = ranges::back_inserter(vec2);

    vec2.clear();
    A::clear();
    std::copy(first, last, out);

    CHECK(ranges::size(vec2) == N);
    CHECK(A::copy_count == std::size_t{0});
    CHECK(A::move_count == N);
  }

  {
    static_assert(
      ranges::models::Same<
        ranges::rvalue_reference_t<
          ranges::counted_iterator<proxy_iterator<A>>>,
        A&&>);
    static_assert(
      ranges::models::Same<
        ranges::rvalue_reference_t<
          ranges::move_iterator<
            ranges::counted_iterator<proxy_iterator<A>>>>,
        A&&>);
    auto first = ranges::make_move_iterator(
      ranges::make_counted_iterator(
        proxy_iterator<A>{ranges::data(vec)}, ranges::size(vec)));
    auto out = ranges::back_inserter(vec2);

    vec2.clear();
    A::clear();
    ranges::copy(first, ranges::default_sentinel{}, out);

    CHECK(ranges::size(vec2) == N);
    CHECK(A::copy_count == std::size_t{0});
    CHECK(A::move_count == N);
  }
}

template <class T>
constexpr bool can_eq = false;
template <class T>
requires requires(const T a, const T b) { a == b; }
constexpr bool can_eq<T> = true;

template <class T>
constexpr bool can_neq = false;
template <class T>
requires requires(const T a, const T b) { a != b; }
constexpr bool can_neq<T> = true;

void test_incomparable() {
  struct I {
    using value_type = int;
    using difference_type = int;
    using iterator_category = ranges::input_iterator_tag;

    int operator*() const;
    I& operator++() &;
    I operator++(int) &;
  };
  static_assert(ranges::models::InputIterator<I>);

  static_assert(can_eq<ranges::move_iterator<int*>>);
  static_assert(can_neq<ranges::move_iterator<int*>>);
  static_assert(!can_eq<ranges::move_iterator<I>>);
  static_assert(!can_neq<ranges::move_iterator<I>>);
}

int main() {
  test_move_iterator();
  test_iter_move();
  test_both();
  test_proxy_iterator();
  test_incomparable();
  return ::test_result();
}
