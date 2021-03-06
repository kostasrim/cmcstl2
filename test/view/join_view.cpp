// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Casey Carter 2016
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#include <stl2/view/join.hpp>
#include <stl2/view/iota.hpp>
#include <stl2/view/transform.hpp>
#include <stl2/view/filter.hpp>
#include <stl2/detail/algorithm/count.hpp>
#include <stl2/detail/algorithm/transform.hpp>
#include <stl2/detail/iterator/insert_iterators.hpp>
#include <memory>
#include <vector>
#include <string>
#include "../simple_test.hpp"

namespace ranges = __stl2;

int main()
{
	using namespace ranges;

	{
		std::vector<std::string> vs{"this","is","his","face"};
		join_view jv{vs};
		CHECK_EQUAL(jv, {'t','h','i','s','i','s','h','i','s','f','a','c','e'});
		static_assert(BidirectionalRange<decltype(jv)>);
		static_assert(BidirectionalRange<const decltype(jv)>);
		static_assert(CommonRange<decltype(jv)>);
		static_assert(CommonRange<const decltype(jv)>);
	}

	{
		auto rng = view::iota(0,4)
			| view::transform([](int i) {return view::iota(0,i);})
			| view::join;
		CHECK_EQUAL(rng, {0,0,1,0,1,2});
		static_assert(InputRange<decltype(rng)>);
		static_assert(!Range<const decltype(rng)>);
		static_assert(!ForwardRange<decltype(rng)>);
		static_assert(!CommonRange<decltype(rng)>);
	}

	{
		auto rng = view::iota(0,4)
			| view::transform([](int i) {return view::iota(0,i);})
			| view::filter([](auto){ return true; })
			| view::join;
		CHECK_EQUAL(rng, {0,0,1,0,1,2});
		static_assert(InputRange<decltype(rng)>);
		static_assert(!Range<const decltype(rng)>);
		static_assert(!ForwardRange<decltype(rng)>);
		static_assert(!CommonRange<decltype(rng)>);
	}

	{
		// https://github.com/ericniebler/stl2/issues/604
		auto rng0 = view::iota(0, 4)
			| view::transform([](int i) { return view::iota(0, i); });
		auto rng1 = ref_view{rng0};
		static_assert(RandomAccessRange<decltype(rng1)>);
		static_assert(Range<const decltype(rng1)>);
		static_assert(CommonRange<decltype(rng1)>);
		static_assert(RandomAccessRange<ext::range_reference_t<decltype(rng1)>>);
		static_assert(ext::SimpleView<decltype(rng1)>);
		static_assert(!std::is_reference_v<ext::range_reference_t<decltype(rng1)>>);
		auto rng2 = rng1 | view::join;
		CHECK_EQUAL(rng2, {0,0,1,0,1,2});
		static_assert(InputRange<decltype(rng2)>);
		static_assert(!Range<const decltype(rng2)>);
		static_assert(!ForwardRange<decltype(rng2)>);
		static_assert(!CommonRange<decltype(rng2)>);
	}

	return ::test_result();
}
