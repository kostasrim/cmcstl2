// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Casey Carter 2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#ifndef STL2_DETAIL_ALGORITHM_IS_SORTED_HPP
#define STL2_DETAIL_ALGORITHM_IS_SORTED_HPP

#include <stl2/functional.hpp>
#include <stl2/iterator.hpp>
#include <stl2/detail/fwd.hpp>
#include <stl2/detail/algorithm/is_sorted_until.hpp>
#include <stl2/detail/concepts/callable.hpp>

///////////////////////////////////////////////////////////////////////////
// is_sorted [is.sorted]
//
STL2_OPEN_NAMESPACE {
  template <ForwardIterator I, Sentinel<I> S, class Proj = identity,
            IndirectCallableStrictWeakOrder<Projected<I, Proj>> Comp = less<>>
  bool is_sorted(I first, S last, Comp&& comp = Comp{}, Proj&& proj = Proj{}) {
    return last == __stl2::is_sorted_until(__stl2::move(first), last,
      __stl2::forward<Comp>(comp), __stl2::forward<Proj>(proj));
  }

  template<ForwardRange Rng, class Proj = identity,
           IndirectCallableStrictWeakOrder<Projected<IteratorType<Rng>, Proj>> Comp = less<>>
  bool is_sorted(Rng&& rng, Comp&& comp = Comp{}, Proj&& proj = Proj{}) {
    return __stl2::is_sorted(__stl2::begin(rng), __stl2::end(rng),
      __stl2::forward<Comp>(comp), __stl2::forward<Proj>(proj));
  }
} STL2_CLOSE_NAMESPACE

#endif