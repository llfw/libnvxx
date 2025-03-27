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

namespace bsd {

/*
 * An RAII guard for a file descriptor.
 */
struct nv_fd {
	explicit nv_fd(int __fd) noexcept
		: __m_fd(__fd)
	{
	}

	nv_fd(nv_fd const &) = delete;

	nv_fd(nv_fd &&__other) noexcept
		: __m_fd(std::exchange(__other.__m_fd, -1))
	{
	}

	nv_fd& operator=(nv_fd const &) = delete;

	nv_fd& operator=(nv_fd &&__other) noexcept {
		if (this != &__other) {
			if (__m_fd != -1)
				(void)::close(__m_fd);
			__m_fd = std::exchange(__other.__m_fd, -1);
		}
		return (*this);
	}

	~nv_fd() {
		if (__m_fd != -1)
			(void)::close(__m_fd);
	}

	int get() const {
		if (__m_fd == -1)
			throw std::logic_error("attempt to access a "
					       "closed nv_fd");
		return (__m_fd);
	}

	int release() && {
		if (__m_fd == -1)
			throw std::logic_error("attempt to access a "
					       "closed nv_fd");
		return (std::exchange(__m_fd, -1));
	}

private:
	int __m_fd = -1;
};

namespace __detail {

/*
 * An RAII guard for a C pointer.
 */
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

} // namespace __detail
} // namespace bsd

#endif	/* !_NVXX_UTIL_H_INCLUDED */
