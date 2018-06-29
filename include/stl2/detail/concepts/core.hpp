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
#ifndef STL2_DETAIL_CONCEPTS_CORE_HPP
#define STL2_DETAIL_CONCEPTS_CORE_HPP

#include <stl2/detail/fwd.hpp>
#include <stl2/detail/meta.hpp>

////////////////////////////////////////
// Core Concepts [concepts.lib.corelang]
//
STL2_OPEN_NAMESPACE {
	template <bool B>
	inline constexpr bool __bool = B;

	template <template <class...> class T, class... U>
	concept bool _Valid = requires { typename T<U...>; };

	template <class U, template <class...> class T, class... V>
	concept bool _Is = _Valid<T, U, V...> && __bool<T<U, V...>::value>;

	template <class U, template <class...> class T, class... V>
	concept bool _IsNot = _Valid<T, U, V...> && __bool<!T<U, V...>::value>;

	// U is a cv/ref-qualified specialization of class template T.
	template <class U, template <class...> class T>
	concept bool _SpecializationOf = __bool<meta::is<__uncvref<U>, T>::value>;

	///////////////////////////////////////////////////////////////////////////
	// Same [concepts.lib.corelang.same]
	//
	template <class T, class U>
	concept bool _SameImpl = __is_same_as(T, U);
	template <class T, class U>
	concept bool Same = _SameImpl<T, U> && _SameImpl<U, T>;

	namespace models {
		template <class T, class U>
		constexpr bool Same = __is_same_as(T, U);
	}

	template <class T>
	concept bool _Decayed = Same<T, decay_t<T>>;

	template <class T>
	concept bool _Unqual = Same<T, __uncvref<T>>;

	namespace models {
		template <class T, class... Args>
		constexpr bool _OneOf = (Same<T, Args> || ...);
	}

	template <class T, class... Args>
	concept bool _OneOf = models::_OneOf<T, Args...>;

	template <class T, class U>
	concept bool _NotSameAs = !__is_same_as(__uncvref<T>, __uncvref<U>);

	///////////////////////////////////////////////////////////////////////////
	// DerivedFrom [concepts.lib.corelang.derived]
	//
	template <class T, class U>
	concept bool DerivedFrom =
#if defined(__GNUC__)
		__is_base_of(U, T) &&
#else
		_Is<U, is_base_of, T> &&
#endif
			std::is_convertible<std::remove_cv_t<T>*, std::remove_cv_t<U>*>::value;

	namespace models {
		template <class, class>
		constexpr bool DerivedFrom = false;
		__stl2::DerivedFrom{T, U}
		constexpr bool DerivedFrom<T, U> = true;
	}

	///////////////////////////////////////////////////////////////////////////
	// ConvertibleTo [concepts.lib.corelang.convertibleto]
	//
	template <class T, class U>
	concept bool ConvertibleTo =
		_Is<T, is_convertible, U> && requires(T (&t)()) {
			static_cast<U>(t());
		};
		// Axiom: implicit and explicit conversion have equal results.

	namespace models {
		template <class, class>
		constexpr bool ConvertibleTo = false;
		__stl2::ConvertibleTo{T, U}
		constexpr bool ConvertibleTo<T, U> = true;
	}
} STL2_CLOSE_NAMESPACE

#endif
