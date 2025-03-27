/*
 * SPDX-License-Identifier: Unlicense OR MIT
 * Refer to the file 'LICENSE' in the nvxx distribution for license terms.
 */

#ifndef	_NVXX_UTIL_H_INCLUDED
#define _NVXX_UTIL_H_INCLUDED

#ifndef _NVXX_H_INCLUDED
# error include <nvxx.h> instead of including this header directly
#endif

// Some useful helper types.

namespace bsd::__detail {

template<typename _T>
struct __ptr_guard {
	__ptr_guard(_T *__ptr_) : __ptr(__ptr_) {}

	~__ptr_guard() {
		std::free(__ptr);
	}

	_T *__ptr;
};

template<typename T>
auto construct = std::views::transform([] (auto &&value) {
	return (T(std::forward<decltype(value)>(value)));
});

} // namespace bsd::__detail

#endif	/* !_NVXX_UTIL_H_INCLUDED */
