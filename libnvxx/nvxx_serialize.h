/*
 * SPDX-License-Identifier: Unlicense OR MIT
 * Refer to the file 'LICENSE' in the nvxx distribution for license terms.
 */

#ifndef _NVXX_SERIALIZE_H
#define _NVXX_SERIALIZE_H

#ifndef _NVXX_H_INCLUDED
# error include <nvxx.h> instead of including this header directly
#endif

#include <string_view>
#include <string>
#include <vector>
#include <span>
#include <optional>
#include <ranges>

namespace bsd {

namespace __detail {

template<typename _R, typename _T>
concept __range_of = std::ranges::range<_R> &&
		std::same_as<_T, 
			std::remove_cvref_t<std::ranges::range_value_t<_R>>>;

template<typename _Container, typename _T>
concept __from_range_container_of =
	__range_of<_Container, _T>
	&& (std::move_constructible<_Container>
	    || std::copy_constructible<_Container>)
	&& requires (std::span<_T> __values) {
		_Container(std::from_range, __values);
	};

template<typename _Container, typename _T>
concept __push_back_container_of =
	__range_of<_Container, _T>
	&& std::default_initializable<_Container> 
	&& (std::move_constructible<_Container>
	    || std::copy_constructible<_Container>)
	&& requires (_Container &__c, _T __value) {
		__c.push_back(__value);
	};

} // namespace __detail

/*
 * Encoders/decoders for basic types.
 */

template<typename T>
struct nv_encoder;
template<typename T>
struct nv_decoder;

/* bool */

template<>
struct nv_encoder<bool> {
	void encode(nv_list &, std::string_view, bool);
};

template<__detail::__range_of<bool> _R>
struct nv_encoder<_R> {
	template<typename _U>
	void encode(nv_list &__nvl, std::string_view __key, _U &&__range) {
		__nvl.add_bool_range(__key, std::forward<_U>(__range));
	}
};

template<>
struct nv_decoder<bool> {
	auto decode(const_nv_list const &, std::string_view) -> bool;
};

template<__detail::__from_range_container_of<bool> _C>
struct nv_decoder<_C> {
	auto decode(const_nv_list const &__nvl, std::string_view __key) -> _C {
		return (_C(std::from_range, __nvl.get_bool_array(__key)));
	}
};

/* uint64_t */

template<>
struct nv_encoder<std::uint64_t> {
	void encode(nv_list &, std::string_view, std::uint64_t);
};

template<__detail::__range_of<std::uint64_t> _R>
struct nv_encoder<_R> {
	template<typename _U>
	void encode(nv_list &__nvl, std::string_view __key, _U &&__range) {
		__nvl.add_number_range(__key, std::forward<_U>(__range));
	}
};

template<>
struct nv_decoder<std::uint64_t> {
	auto decode(const_nv_list const &, std::string_view) -> std::uint64_t;
};

template<__detail::__from_range_container_of<std::uint64_t> _C>
struct nv_decoder<_C> {
	auto decode(const_nv_list const &__nvl, std::string_view __key) -> _C {
		return (_C(std::from_range, __nvl.get_number_array(__key)));
	}
};

/* string */

template<>
struct nv_encoder<std::string> {
	void encode(nv_list &, std::string_view, std::string const &);
};

template<__detail::__range_of<std::string> _R>
struct nv_encoder<_R> {
	template<typename _U>
	void encode(nv_list &__nvl, std::string_view __key, _U &&__range) {
		__nvl.add_string_range(__key,
		       __range | std::views::transform([] (auto const &__s) {
			       return (std::string_view(__s));
		       }));
	}
};

template<>
struct nv_decoder<std::string> {
	auto decode(const_nv_list const &, std::string_view) -> std::string;
};

template<__detail::__from_range_container_of<std::string> _C>
struct nv_decoder<_C> {
	auto decode(const_nv_list const &__nvl, std::string_view __key) -> _C {
		return (_C(std::from_range,
			  __nvl.get_string_array(__key)
			  | std::views::transform([] (auto const &__s) {
				  return std::string(__s);
			  })));
	}
};

/* string_view */

template<>
struct nv_encoder<std::string_view> {
	void encode(nv_list &, std::string_view, std::string_view);
};

template<__detail::__range_of<std::string_view> _R>
struct nv_encoder<_R> {
	template<typename _U>
	void encode(nv_list &__nvl, std::string_view __key, _U &&__range) {
		__nvl.add_string_range(__key, std::forward<_U>(__range));
	}
};

template<>
struct nv_decoder<std::string_view> {
	auto decode(const_nv_list const &, std::string_view)
		-> std::string_view;
};

template<__detail::__from_range_container_of<std::string_view> _C>
struct nv_decoder<_C> {
	auto decode(const_nv_list const &__nvl, std::string_view __key) -> _C {
		return (_C(std::from_range, __nvl.get_string_array(__key)));
	}
};

/* nv_list */

template<>
struct nv_encoder<nv_list> {
	void encode(nv_list &, std::string_view, nv_list const &);
};

#ifdef notyet // XXX: implement add_nvlist_range()
template<__detail::__range_of<nv_list> _R>
struct nv_encoder<_R> {
	template<typename _U>
	void encode(nv_list &__nvl, std::string_view __key, _U &&__range) {
		__nvl.add_nvlist_range(__key, std::forward<_U>(__range));
	}
};
#endif

template<>
struct nv_decoder<nv_list> {
	auto decode(const_nv_list const &, std::string_view) -> nv_list;
};

template<__detail::__from_range_container_of<nv_list> _C>
struct nv_decoder<_C> {
	auto decode(const_nv_list const &__nvl, std::string_view __key) -> _C {
		return (_C(std::from_range,
			  __nvl.get_nvlist_array(__key)
			  | std::views::transform([] (auto const &__s) {
				  return nv_list(__s);
			  })));
	}
};

/* const_nv_list */

template<>
struct nv_encoder<const_nv_list> {
	void encode(nv_list &, std::string_view, const_nv_list const &);
};

#ifdef notyet // XXX: implement add_nvlist_range()
template<__detail::__range_of<const_nv_list> _R>
struct nv_encoder<_R> {
	template<typename _U>
	void encode(nv_list &__nvl, std::string_view __key, _U &&__range) {
		__nvl.add_nvlist_range(__key, std::forward<_U>(__range));
	}
};
#endif

template<>
struct nv_decoder<const_nv_list> {
	auto decode(const_nv_list const &, std::string_view) -> const_nv_list;
};

template<__detail::__from_range_container_of<const_nv_list> _C>
struct nv_decoder<_C> {
	auto decode(const_nv_list const &__nvl, std::string_view __key) -> _C {
		return (_C(std::from_range, __nvl.get_nvlist_array(__key)));
	}
};

/* optional<T> */

template<typename _T>
struct nv_encoder<std::optional<_T>> {
	auto encode(nv_list &__nvl,
		    std::string_view __key,
		    std::optional<_T> const &__value) {
		if (__value)
			nv_encoder<_T>{}.encode(__nvl, __key, *__value);
	}
};

template<typename _T>
struct nv_decoder<std::optional<_T>> {
	auto decode(const_nv_list const &__nvl,
		    std::string_view __key) -> std::optional<_T> {
		if (__nvl.exists(__key))
			return {nv_decoder<_T>{}.decode(__nvl, __key)};
		else
			return {};
	}
};

/*
 * object (de)serialization
 */

template<typename _Object, typename _Member>
struct nv_field {
	nv_field(std::string __name, _Member _Object::* __ptr)
		: __field_name(__name)
		, __field_ptr(__ptr)
	{
	}

	auto serialize(nv_list &__nvl, _Object const &__object) {
		nv_encoder<_Member>{}.encode(__nvl,
					     __field_name,
					     __object.*__field_ptr);
	}

	auto deserialize(const_nv_list const &__nvl, _Object &__object) {
		__object.*__field_ptr = nv_decoder<_Member>{}
						.decode(__nvl, __field_name);
	}

private:
	std::string __field_name;
	_Member _Object::* __field_ptr;
};

namespace __detail {

template<typename _First, typename _Second>
struct __field_sequence {
	__field_sequence(_First __first_, _Second __second_)
		: __first(__first_)
		, __second(__second_)
	{
	}

	template<typename _Object>
	auto serialize(nv_list &__nvl, _Object const &__object) {
		__first.serialize(__nvl, __object);
		__second.serialize(__nvl, __object);
	}

	template<typename _Object>
	auto deserialize(const_nv_list const &__nvl, _Object &__object) {
		__first.deserialize(__nvl, __object);
		__second.deserialize(__nvl, __object);
	}

private:
	_First __first;
	_Second __second;
};

} // namespace __detail

template<typename _Object, typename _M1, typename _M2>
auto operator>> (nv_field<_Object, _M1> const &__f1,
		 nv_field<_Object, _M2> const &__f2)
{
	return (__detail::__field_sequence(__f1, __f2));
}

template<typename _Object, typename _M, typename __first, typename __second>
auto operator>> (__detail::__field_sequence<__first, __second> const &__f1,
		 nv_field<_Object, _M> const &__f2)
{
	return (__detail::__field_sequence(__f1, __f2));
}

template<typename _Object, typename _Member>
nv_field(std::string_view, _Member _Object::*)
	-> nv_field<std::decay_t<_Object>, _Member>;

template<typename _T>
struct nv_schema;

template<typename _T>
auto nv_serialize(_T &&__o) -> nv_list
{
	using __schema_type = nv_schema<std::remove_cvref_t<_T>>;
	auto __schema = __schema_type{}.get();
	auto __nvl = nv_list();

	__schema.serialize(__nvl, __o);
	return (__nvl);
}

template<typename _Object>
void nv_deserialize(const_nv_list const &__nvl, _Object &__obj)
{
	using __schema_type = nv_schema<std::remove_cvref_t<_Object>>;
	auto __schema = __schema_type{}.get();

	__schema.deserialize(__nvl, __obj);
}

} // namespace bsd

#endif	/* !_NVXX_SERIALIZE_H */
