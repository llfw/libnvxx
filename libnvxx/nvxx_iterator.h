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

#ifndef _NVXX_ITERATOR_H_INCLUDED
#define _NVXX_ITERATOR_H_INCLUDED

#ifndef _NVXX_H_INCLUDED
# error include <nvxx.h> instead of including this header directly
#endif

#include <variant>
#include <iterator>
#include <concepts>

/*
 * iterator support for libnvxx.  this exposes a const_nv_list as an iterable
 * container or range.
 */

namespace bsd {

// the key type of an nvlist value
using nv_list_key_t = std::string_view;

// the value type of an nvlist value
using nv_list_value_t = std::variant<
	nullptr_t,			/* null */
	bool,				/* bool */
	std::uint64_t,			/* number */
	std::string_view,		/* string */
	const_nv_list,			/* nvlist */
	int,				/* descriptor */
	std::span<std::byte const>,	/* binary */
	std::span<bool const>,		/* bool array */
	std::span<std::uint64_t const>,	/* number array */
	std::vector<std::string_view>,	/* string array */
	std::span<int const>,		/* descriptor array */
	std::vector<const_nv_list>	/* nvlist array */
>;

// the iterator value type
using nv_list_pair_t = std::pair<nv_list_key_t, nv_list_value_t>;

/*
 * The nvlist iterator type.  This is never an 'end' iterator, because we use
 * a sentinel for that.
 */
struct nv_list_iterator {
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using value_type = nv_list_pair_t;
	using pointer = value_type *;
	using const_pointer = value_type const *;
	using reference = value_type &;
	using const_reference = value_type const &;
	using sentinel = std::default_sentinel_t;

	nv_list_iterator();
	explicit nv_list_iterator(const_nv_list const &nvl);
	explicit nv_list_iterator(nv_list const &nvl);

	nv_list_iterator &operator++();
	nv_list_iterator operator++(int);

	bool operator==(nv_list_iterator const &other) const;
	bool operator==(std::default_sentinel_t) const;

	const_reference operator*() const;
	const_pointer operator->() const;

private:
	::nvlist_t const *__nvlist = nullptr;
	void *__cookie = nullptr;
	nv_list_pair_t __current;

	void __advance();
};

static_assert(std::forward_iterator<nv_list_iterator>);
static_assert(std::sentinel_for<std::default_sentinel_t, nv_list_iterator>);

nv_list_iterator begin(const_nv_list const &nvl);
nv_list_iterator begin(nv_list const &nvl);
std::default_sentinel_t end(const_nv_list const &);
std::default_sentinel_t end(nv_list const &);

};

#endif	/* !_NVXX_ITERATOR_H_INCLUDED */
