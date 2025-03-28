/*
 * SPDX-License-Identifier: Unlicense OR MIT
 * Refer to the file 'LICENSE' in the nvxx distribution for license terms.
 */

#ifndef _NVXX_SERIALIZE_H
#define _NVXX_SERIALIZE_H

#ifndef _NVXX_H_INCLUDED
# error include <nvxx.h> instead of including this header directly
#endif

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

struct __serializer_tag {
	using __serializer_tag_t = int;
};

template<typename _T>
concept __serializer =
	requires(_T __t) {
		typename _T::__serializer_tag_t;
	};

} // namespace __detail

/*
 * Encoders/decoders for basic types.
 */

template<typename T>
struct nv_encoder;

/* bool */

template<>
struct nv_encoder<bool> {
	void encode(nv_list &__nvl, std::string_view __key, bool __value) {
		__nvl.add_bool(__key, __value);
	}

	auto decode(const_nv_list const &__nvl, std::string_view __key) -> bool {
		return (__nvl.get_bool(__key));
	}
};

template<__detail::__from_range_container_of<bool> _C>
struct nv_encoder<_C> {
	void encode(nv_list &__nvl, std::string_view __key, auto &&__range) {
		__nvl.add_bool_range(__key,
				     std::forward<decltype(__range)>(__range));
	}

	auto decode(const_nv_list const &__nvl, std::string_view __key) -> _C {
		return (_C(std::from_range, __nvl.get_bool_array(__key)));
	}
};

/* uint64_t */

template<>
struct nv_encoder<std::uint64_t> {
	void encode(nv_list &__nvl,
		    std::string_view __key,
		    std::uint64_t __value) {
		__nvl.add_number(__key, __value);
	}

	std::uint64_t decode(const_nv_list const &__nvl,
			     std::string_view __key) {
		return __nvl.get_number(__key);
	}
};

template<__detail::__from_range_container_of<std::uint64_t> _C>
struct nv_encoder<_C> {
	void encode(nv_list &__nvl, std::string_view __key, auto &&__range) {
		__nvl.add_number_range(
			__key, std::forward<decltype(__range)>(__range));
	}

	auto decode(const_nv_list const &__nvl, std::string_view __key) -> _C {
		return (_C(std::from_range, __nvl.get_number_array(__key)));
	}
};

/* string */

template<>
struct nv_encoder<std::string> {
	void encode(nv_list &__nvl,
		    std::string_view __key,
		    std::string const &__value) {
		__nvl.add_string(__key, __value);
	}

	std::string decode(const_nv_list const &__nvl,
			   std::string_view __key) {
		return std::string(__nvl.get_string(__key));
	}
};

template<__detail::__from_range_container_of<std::string> _C>
struct nv_encoder<_C> {
	void encode(nv_list &__nvl, std::string_view __key, auto &&__range) {
		__nvl.add_string_range(__key,
		       __range | std::views::transform([] (auto const &__s) {
			       return (std::string_view(__s));
		       }));
	}

	_C decode(const_nv_list const &__nvl, std::string_view __key) {
		auto __strings =
			  __nvl.get_string_array(__key)
			  | std::views::transform([] (auto const &__s) {
				  return std::string(__s);
			  });
		return {std::from_range, __strings};
	}
};

/* string_view */

template<>
struct nv_encoder<std::string_view> {
	void encode(nv_list &__nvl,
		    std::string_view __key,
		    std::string_view __value) {
		__nvl.add_string(__key, __value);
	}

	std::string_view decode(const_nv_list const &__nvl,
				std::string_view __key) {
		return __nvl.get_string(__key);
	}
};

template<__detail::__from_range_container_of<std::string_view> _C>
struct nv_encoder<_C> {
	void encode(nv_list &__nvl, std::string_view __key, auto &&__range) {
		__nvl.add_string_range(
			__key, std::forward<decltype(__range)>(__range));
	}

	_C decode(const_nv_list const &__nvl, std::string_view __key) {
		return {std::from_range, __nvl.get_string_array(__key)};
	}
};

/* nv_list */

template<>
struct nv_encoder<nv_list> {
	void encode(nv_list &__nvl,
		    std::string_view __key,
		    nv_list const &__value) {
		__nvl.add_nvlist(__key, __value);
	}

	nv_list decode(const_nv_list const &__nvl, std::string_view __key) {
		return (nv_list(__nvl.get_nvlist(__key)));
	}
};

template<__detail::__from_range_container_of<nv_list> _C>
struct nv_encoder<_C> {
#ifdef notyet // XXX: implement add_nvlist_range()
	template<typename _U>
	void encode(nv_list &__nvl, std::string_view __key, _U &&__range) {
		__nvl.add_nvlist_range(__key, std::forward<_U>(__range));
	}
#endif

	_C decode(const_nv_list const &__nvl, std::string_view __key) {
		auto __nvls = 
			  __nvl.get_nvlist_array(__key)
			  | std::views::transform([] (auto const &__s) {
				  return nv_list(__s);
			  });
		return {std::from_range, __nvls};
	}
};

/* const_nv_list */

template<>
struct nv_encoder<const_nv_list> {
	void encode(nv_list &__nvl,
		    std::string_view __key,
		    const_nv_list const &__value) {
		__nvl.add_nvlist(__key, __value);
	}

	const_nv_list decode(const_nv_list const &__nvl,
			     std::string_view __key) {
		return __nvl.get_nvlist(__key);
	}
};

template<__detail::__from_range_container_of<const_nv_list> _C>
struct nv_encoder<_C> {
#ifdef notyet // XXX: implement add_nvlist_range()
	template<typename _U>
	void encode(nv_list &__nvl, std::string_view __key, _U &&__range) {
		__nvl.add_nvlist_range(__key, std::forward<_U>(__range));
	}
#endif

	_C decode(const_nv_list const &__nvl, std::string_view __key) {
		return {std::from_range, __nvl.get_nvlist_array(__key)};
	}
};

/* optional<T> */

template<typename _T>
struct nv_encoder<std::optional<_T>> {
	void encode(nv_list &__nvl,
		    std::string_view __key,
		    std::optional<_T> const &__value) {
		if (__value)
			nv_encoder<_T>{}.encode(__nvl, __key, *__value);
	}

	std::optional<_T> decode(const_nv_list const &__nvl,
				 std::string_view __key) {
		if (__nvl.exists(__key))
			return {nv_encoder<_T>{}.decode(__nvl, __key)};
		else
			return {};
	}
};

/*
 * object (de)serialization
 */

template<typename _T>
struct nv_schema;

template<typename _Object, typename _Member>
struct nv_field : __detail::__serializer_tag {
	nv_field(std::string __name, _Member _Object::* __ptr)
		: __field_name(__name)
		, __field_ptr(__ptr)
	{
	}

	auto serialize(nv_list &__nvl, _Object const &__object) const {
		nv_encoder<_Member>{}.encode(__nvl,
					     __field_name,
					     __object.*__field_ptr);
	}

	auto deserialize(const_nv_list const &__nvl, _Object &__object) const {
		__object.*__field_ptr = nv_encoder<_Member>{}
						.decode(__nvl, __field_name);
	}

private:
	std::string __field_name;
	_Member _Object::* __field_ptr;
};

template<typename _Object, typename _Member>
nv_field(std::string_view, _Member _Object::*)
	-> nv_field<std::decay_t<_Object>, _Member>;

template<typename _Object, typename _Member>
struct nv_object : __detail::__serializer_tag {
	nv_object(std::string __name, _Member _Object::* __ptr)
		: __field_name(__name)
		, __field_ptr(__ptr)
	{
	}

	auto serialize(nv_list &__nvl, _Object const &__object) const {
		using __schema_type = nv_schema<_Member>;
		auto __schema = __schema_type{}.get();
		__schema.serialize(__nvl, __object.*__field_ptr);
	}

	auto deserialize(const_nv_list const &__nvl, _Object &__object) const {
		using __schema_type = nv_schema<_Member>;
		auto __schema = __schema_type{}.get();
		__schema.deserialize(__nvl, __object.*__field_ptr);
	}

private:
	std::string __field_name;
	_Member _Object::* __field_ptr;
};

template<typename _Member>
struct nv_literal;

template<>
struct nv_literal<std::string_view> : __detail::__serializer_tag {
	nv_literal(std::string __name, std::string __value)
		: __field_name(__name)
		, __field_value(__value)
	{
	}

	auto serialize(nv_list &__nvl, auto const &) const {
		__nvl.add_string(__field_name, __field_value);
	}

	auto deserialize(const_nv_list const &__nvl, auto &) const {
		auto __value = __nvl.get_string(__field_name);
		// TODO: possibly we could have a specific exception for this
		if (__value != __field_value)
			throw nv_key_not_found(__field_name);
	}

private:
	std::string __field_name;
	std::string __field_value;
};

nv_literal(std::string, char const *) -> nv_literal<std::string_view>;
nv_literal(std::string, std::string) -> nv_literal<std::string_view>;
nv_literal(std::string, std::string_view) -> nv_literal<std::string_view>;

namespace __detail {

template<__serializer _First, __serializer _Second>
struct __field_sequence : __detail::__serializer_tag {
	__field_sequence(_First __first_, _Second __second_)
		: __first(__first_)
		, __second(__second_)
	{
	}

	auto serialize(nv_list &__nvl, auto const &__object) const {
		__first.serialize(__nvl, __object);
		__second.serialize(__nvl, __object);
	}

	auto deserialize(const_nv_list const &__nvl, auto &__object) const {
		__first.deserialize(__nvl, __object);
		__second.deserialize(__nvl, __object);
	}

private:
	_First __first;
	_Second __second;
};

} // namespace __detail

auto operator>> (__detail::__serializer auto const &__f1,
		 __detail::__serializer auto const &__f2)
{
	return (__detail::__field_sequence(__f1, __f2));
}

nv_list
nv_serialize(auto &&__o, __detail::__serializer auto const &__schema)
{
	auto __nvl = nv_list();
	__schema.serialize(__nvl, __o);
	return (__nvl);
}

nv_list
nv_serialize(auto &&__o)
{
	using __schema_type = nv_schema<std::remove_cvref_t<decltype(__o)>>;
	auto __schema = __schema_type{}.get();
	return nv_serialize(std::forward<decltype(__o)>(__o), __schema);
}

void
nv_deserialize(const_nv_list const &__nvl,
	       auto &__obj,
	       __detail::__serializer auto const &__schema)
{
	__schema.deserialize(__nvl, __obj);
}

void nv_deserialize(const_nv_list const &__nvl, auto &__obj)
{
	using __schema_type = nv_schema<std::remove_cvref_t<decltype(__obj)>>;
	auto __schema = __schema_type{}.get();
	nv_deserialize(__nvl, __obj, __schema);
}

} // namespace bsd

#endif	/* !_NVXX_SERIALIZE_H */
