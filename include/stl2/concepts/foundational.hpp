#ifndef STL2_CONCEPTS_FOUNDATIONAL_HPP
#define STL2_CONCEPTS_FOUNDATIONAL_HPP

#include <stl2/detail/config.hpp>
#include <stl2/detail/fwd.hpp>
#include <stl2/concepts/core.hpp>

#include <type_traits>

////////////////////////
// Foundational Concepts
//
namespace stl2 { namespace v1 { namespace concepts {

template <class T>
concept bool Destructible =
  std::is_object<T>::value &&
  requires(T& t, T* p) {
    &t; requires Same<T*, decltype(&t)>;
    { t.~T() } noexcept;
    delete p;
  };

template <class T>
concept bool MoveConstructible =
  Destructible<T> &&
  Constructible<T, T&&> &&
  requires(T&& t) {
    new T(move(t)); requires Same<T*, decltype(new T(move(t)))>;
  };

template <class T>
concept bool CopyConstructible =
  MoveConstructible<T> &&
  Constructible<T, T&> &&
  Constructible<T, const T&> &&
  Constructible<T, const T&&> &&
  requires(T& a, const T& b, const T&& c) {
    new T(a); requires Same<T*, decltype(new T(a))>;
    new T(b); requires Same<T*, decltype(new T(b))>;
    new T(move(c)); requires Same<T*, decltype(new T(move(c)))>;
  };

template <class T>
concept bool Movable =
  MoveConstructible<T> &&
  Assignable<T, T&&>;

template <class T>
concept bool Copyable =
  Movable<T> &&
  CopyConstructible<T> &&
  Assignable<T, T&> &&
  Assignable<T, const T&> &&
  Assignable<T, const T&&>;

template <class T>
concept bool Semiregular =
  Copyable<T> &&
  Constructible<T> &&
  requires(T& t, T* p, std::size_t n) {
    new T; requires Same<T*, decltype(new T)>;
    new T[n]; requires Same<T*, decltype(new T[n])>;
    delete[] p;
  };

} // namespace concepts

Movable{T}
constexpr void swap(T& a, T& b)
  noexcept(std::is_nothrow_move_constructible<T>::value &&
           std::is_nothrow_move_assignable<T>::value);

namespace detail {

template <class, class>
struct swappable_array :
  std::false_type {};

template <class T, class U>
  requires requires(T& t, U&u) {
    swap(t, u);
    swap(u, t);
  }
struct swappable_array<T, U> : std::true_type {
  static constexpr bool nothrow =
    noexcept(swap(stl2::declval<T&>(),
                  stl2::declval<U&>()));
};

template <class T, class U, std::size_t N>
struct swappable_array<T[N], U[N]> :
  swappable_array<T, U> {};

} // namespace detail

template <class T, class U, std::size_t N>
  requires detail::swappable_array<T, U>::value
constexpr void swap(T (&t)[N], U (&u)[N])
  noexcept(detail::swappable_array<T, U>::nothrow);

#ifdef STL2_SWAPPABLE_POINTERS
namespace detail {

template <class T, class U>
concept bool SwappableLvalue =
  requires(T& t, U& u) {
    swap(t, u);
    swap(u, t);
  };

} // namespace detail

template <class T, class U>
  requires detail::SwappableLvalue<T, U>
constexpr void swap(T*&& a, U*&& b)
  noexcept(noexcept(swap(*a, *b)));

template <class T, class U>
  requires detail::SwappableLvalue<T, U>
constexpr void swap(T& a, U*&& b)
  noexcept(noexcept(swap(a, *b)));

template <class T, class U>
  requires detail::SwappableLvalue<T, U>
constexpr void swap(T*&& a, U& b)
  noexcept(noexcept(swap(*a, b)));
#endif // STL2_SWAPPABLE_POINTERS

namespace concepts {

template <class T, class U = T>
concept bool Swappable =
  requires (T&& t, U&& u) {
    swap(forward<T>(t), forward<U>(u));
    swap(forward<U>(u), forward<T>(t));
  };

template <class B>
concept bool Boolean =
  Convertible<B, bool> &&
  requires(B&& b1, B&& b2) {
    //{ !b1 } -> Boolean;
    !b1; requires Convertible<decltype(!b1),bool>;
    //{ b1 && b2 } -> Same<bool>;
    b1 && b2; requires Convertible<decltype(b1 && b2),bool>;
    //{ b1 || b2 } -> Same<bool>;
    b1 || b2; requires Convertible<decltype(b1 || b2),bool>;
  };
} // namespace concepts

namespace detail {

template <class T, class U>
concept bool EqualityComparable_ =
  requires(T&& t, U&& u) {
#if 0 // FIXME: ICE
    { forward<T>(t) == forward<U>(u) } -> Boolean;
    { forward<T>(t) != forward<U>(u) } -> Boolean;
#else
    { forward<T>(t) == forward<U>(u) } -> bool;
    { forward<T>(t) != forward<U>(u) } -> bool;
#endif
  };

} // namespace detail

namespace concepts {

template <class T, class U = T>
concept bool EqualityComparable =
  detail::EqualityComparable_<T, T> &&
  (Same<T, U> ||
    (detail::EqualityComparable_<T, U> &&
     detail::EqualityComparable_<U, T> &&
     detail::EqualityComparable_<U, U>));

template <class T, class U = T>
concept bool StronglyEqualityComparable =
  EqualityComparable<T, U> &&
  (Same<T, U> ||
    (Common<T, U> &&
     EqualityComparable<CommonType<T, U>>));

template <class T>
concept bool Regular =
  Semiregular<T> &&
  EqualityComparable<T>;

} // namespace concepts

namespace detail {

template <class T, class U>
concept bool TotallyOrdered_ =
  EqualityComparable<T, U> &&
  requires(T&& a, U&& b) {
#if 0 // FIXME: ICE
    //{ a < b } -> Boolean;
    //{ a > b } -> Boolean;
    //{ a <= b } -> Boolean;
    //{ a >= b } -> Boolean;
#else
    { a < b } -> bool;
    { a > b } -> bool;
    { a <= b } -> bool;
    { a >= b } -> bool;
#endif
  };

} // namespace detail

namespace concepts {

template <class T, class U = T>
concept bool TotallyOrdered =
  detail::TotallyOrdered_<T, T> &&
  (Same<T, U> ||
    (detail::TotallyOrdered_<T, U> &&
     detail::TotallyOrdered_<U, T> &&
     detail::TotallyOrdered_<U, U>));

template <class T, class U = T>
concept bool StronglyTotallyOrdered =
  TotallyOrdered<T, U> &&
  (Same<T, U> ||
    (Common<T, U> &&
     TotallyOrdered<CommonType<T, U>>));

#if 0
template <class T>
concept bool Scalar =
  std::is_scalar<T>::value && Regular<T>;

template <class T>
concept bool Arithmetic =
  std::is_arithmetic<T>::value && Scalar<T> && TotallyOrdered<T>;

template <class T>
concept bool Integral =
  std::is_integral<T>::value && Arithmetic<T>;

#else

template <class T>
concept bool Integral =
  std::is_integral<T>::value;
#endif

template <class T>
concept bool SignedIntegral =
  Integral<T> && std::is_signed<T>::value;

template <class T>
concept bool UnsignedIntegral =
  Integral<T> && !SignedIntegral<T>;

// Integral<T> subsumes SignedIntegral<T> and UnsignedIntegral<T>
// SignedIntegral<T> and UnsignedIntegral<T> are mutually exclusive

namespace models {

template <class>
constexpr bool destructible() { return false; }

Destructible{T}
constexpr bool destructible() { return true; }


template <class>
constexpr bool move_constructible() { return false; }

MoveConstructible{T}
constexpr bool move_constructible() { return true; }


template <class>
constexpr bool copy_constructible() { return false; }

CopyConstructible{T}
constexpr bool copy_constructible() { return true; }


template <class>
constexpr bool movable() { return false; }

Movable{T}
constexpr bool movable() { return true; }


template <class>
constexpr bool copyable() { return false; }

Copyable{T}
constexpr bool copyable() { return true; }


template <class>
constexpr bool swappable() { return false; }

template <class T>
  requires Swappable<T>
constexpr bool swappable() { return true; }

template <class, class>
constexpr bool swappable() { return false; }

Swappable{T, U}
constexpr bool swappable() { return true; }


template <class>
constexpr bool equality_comparable() { return false; }

template <class T>
  requires EqualityComparable<T>
constexpr bool equality_comparable() { return true; }

template <class, class>
constexpr bool equality_comparable() { return false; }

EqualityComparable{T, U}
constexpr bool equality_comparable() { return true; }


template <class>
constexpr bool semiregular() { return false; }

Semiregular{T}
constexpr bool semiregular() { return true; }


template <class>
constexpr bool regular() { return false; }

Regular{T}
constexpr bool regular() { return true; }


template <class>
constexpr bool totally_ordered() { return false; }

template <class T>
  requires TotallyOrdered<T>
constexpr bool totally_ordered() { return true; }

template <class, class>
constexpr bool totally_ordered() { return false; }

TotallyOrdered{T, U}
constexpr bool totally_ordered() { return true; }


template <class>
constexpr bool boolean() { return false; }

Boolean{T}
constexpr bool boolean() { return true; }


template <class>
constexpr bool integral() { return false; }

Integral{T}
constexpr bool integral() { return true; }

}}}} // namespace stl2::v1::concepts::models

#endif // STL2_CONCEPTS_FOUNDATIONAL_HPP
