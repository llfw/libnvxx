/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or distribute
 * this software, either in source code form or as a compiled binary, for any
 * purpose, commercial or non-commercial, and by any means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors of
 * this software dedicate any and all copyright interest in the software to the
 * public domain. We make this dedication for the benefit of the public at
 * large and to the detriment of our heirs and successors. We intend this
 * dedication to be an overt act of relinquishment in perpetuity of all present
 * and future rights to this software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sys/cnv.h>

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

	default:
		std::abort();
	}
}

}
