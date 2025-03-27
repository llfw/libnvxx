/*
 * SPDX-License-Identifier Unlicense OR MIT
 * Refer to the file 'LICENSE' in the nvxx distribution for license terms.
 */

#include "nvxx.h"

namespace bsd {

nv_list_iterator
begin(const_nv_list const &nvl)
{
	return nv_list_iterator(nvl);
}

nv_list_iterator
begin(nv_list const &nvl)
{
	return nv_list_iterator(nvl);
}

std::default_sentinel_t
end(const_nv_list const &)
{
	return {};
}

std::default_sentinel_t
end(nv_list const &)
{
	return {};
}

nv_list_iterator::nv_list_iterator()
{
}

nv_list_iterator::nv_list_iterator(const_nv_list const &nvl)
	: __nvlist(nvl.ptr())
{
	__advance();
}

nv_list_iterator::nv_list_iterator(nv_list const &nvl)
	: __nvlist(nvl.ptr())
{
	__advance();
}

nv_list_iterator &
nv_list_iterator::operator++()
{
	__advance();
	return *this;
}

nv_list_iterator
nv_list_iterator::operator++(int)
{
	nv_list_iterator tmp = *this;
	++(*this);
	return tmp;
}

bool
nv_list_iterator::operator==(nv_list_iterator const &other) const
{
	return (__nvlist == other.__nvlist)
		&& (__cookie == other.__cookie);
}

bool
nv_list_iterator::operator==(std::default_sentinel_t) const 
{
	return __cookie == nullptr;
}

nv_list_iterator::const_reference
nv_list_iterator::operator*() const
{
	return __current;
}

nv_list_iterator::const_pointer
nv_list_iterator::operator->() const
{
	return &__current;
}

void
nv_list_iterator::__advance()
{
	auto type = int{};
	auto const *namep = ::nvlist_next(__nvlist, &type, &__cookie);

	if (namep == nullptr) {
		__cookie = nullptr;
		return;
	}

	auto name = std::string_view(namep);

	switch (type) {
	case NV_TYPE_NULL:
		__current = std::make_pair(name, nullptr);
		break;

	case NV_TYPE_BOOL:
		__current = std::make_pair(name, cnvlist_get_bool(__cookie));
		break;

	case NV_TYPE_NUMBER:
		__current = std::make_pair(name, cnvlist_get_number(__cookie));
		break;

	case NV_TYPE_STRING:
		__current = std::make_pair(name,
				std::string_view(
					cnvlist_get_string(__cookie)));
		break;

	case NV_TYPE_NVLIST:
		__current = std::make_pair(name,
				const_nv_list(cnvlist_get_nvlist(__cookie)));
		break;

	case NV_TYPE_DESCRIPTOR:
		__current = std::make_pair(name,
					   cnvlist_get_descriptor(__cookie));
		break;

	case NV_TYPE_BINARY: {
		auto nitems = std::size_t{};
		auto ptr = cnvlist_get_binary(__cookie, &nitems);
		auto span = std::span{static_cast<std::byte const *>(ptr), nitems};
		__current = std::make_pair(name, span);
		break;
	}

	case NV_TYPE_BOOL_ARRAY: {
		auto nitems = std::size_t{};
		auto ptr = cnvlist_get_bool_array(__cookie, &nitems);
		auto span = std::span{ptr, nitems};
		__current = std::make_pair(name, span);
		break;
	}

	case NV_TYPE_NUMBER_ARRAY: {
		auto nitems = std::size_t{};
		auto ptr = cnvlist_get_number_array(__cookie, &nitems);
		auto span = std::span{ptr, nitems};
		__current = std::make_pair(name, span);
		break;
	}

	case NV_TYPE_STRING_ARRAY: {
		auto nitems = std::size_t{};
		auto ptr = cnvlist_get_string_array(__cookie, &nitems);
		auto span = std::span{ptr, nitems};
		auto vector =
			span
			| std::views::transform([] (char const *ptr) {
				return std::string_view(ptr);
			})
			| std::ranges::to<std::vector>();
		__current = std::make_pair(name, vector);
		break;
	}

	case NV_TYPE_DESCRIPTOR_ARRAY: {
		auto nitems = std::size_t{};
		auto ptr = cnvlist_get_descriptor_array(__cookie, &nitems);
		auto span = std::span{ptr, nitems};
		__current = std::make_pair(name, span);
		break;
	}

	case NV_TYPE_NVLIST_ARRAY: {
		auto nitems = std::size_t{};
		auto ptr = cnvlist_get_nvlist_array(__cookie, &nitems);
		auto span = std::span{ptr, nitems};
		auto vector =
			span
			| std::views::transform([] (::nvlist_t const *ptr) {
				return const_nv_list(ptr);
			})
			| std::ranges::to<std::vector>();
		__current = std::make_pair(name, vector);
		break;
	}

	default:
		std::abort();
	}
}

}
