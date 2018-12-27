// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Eric Niebler 2014
//  Copyright Casey Carter 2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
//===-------------------------- algorithm ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
#ifndef STL2_DETAIL_ALGORITHM_PREV_PERMUTATION_HPP
#define STL2_DETAIL_ALGORITHM_PREV_PERMUTATION_HPP

#include <stl2/detail/algorithm/reverse.hpp>
#include <stl2/detail/range/primitives.hpp>

///////////////////////////////////////////////////////////////////////////
// prev_permutation [alg.permutation.generators]
//
STL2_OPEN_NAMESPACE {
	struct __prev_permutation_fn : private __niebloid {
		template<BidirectionalIterator I, Sentinel<I> S, class Comp = less,
			class Proj = identity>
		requires Sortable<I, Comp, Proj>
		constexpr bool
		operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
			if (first == last) return false;

			I end = next(first, std::move(last));
			I i = end;
			if (first == --i) return false;

			auto pred = [&](auto&& lhs, auto&& rhs) -> bool {
				return __stl2::invoke(comp,
					__stl2::invoke(proj, static_cast<decltype(lhs)>(lhs)),
					__stl2::invoke(proj, static_cast<decltype(rhs)>(rhs)));
			};

			while (true) {
				I ip1 = i;
				if (pred(*ip1, *--i)) {
					I j = end;
					while (!pred(*--j, *i)) {
						;
					}
					iter_swap(i, j);
					reverse(ip1, end);
					return true;
				}
				if (i == first) {
					reverse(first, end);
					return false;
				}
			}
		}

		template<BidirectionalRange Rng, class Comp = less, class Proj = identity>
		requires Sortable<iterator_t<Rng>, Comp, Proj>
		constexpr bool
		operator()(Rng&& rng, Comp comp = {}, Proj proj = {}) const {
			return (*this)(begin(rng), end(rng), __stl2::ref(comp),
				__stl2::ref(proj));
		}
	};

	inline constexpr __prev_permutation_fn prev_permutation {};
} STL2_CLOSE_NAMESPACE

#endif
