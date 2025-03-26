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

#include <cerrno>
#include <cassert>

#include "nvxx.h"

namespace bsd::__detail {

/*
 * __nv_list_base
 */

__nv_list_base::__nv_list_base(int flags)
	: __m_nv(::nvlist_create(flags))
	, __m_owning(__nvlist_owning::__owning)
{
	if (__m_nv == nullptr)
		throw std::system_error(
			std::error_code(errno, std::generic_category()));
}

__nv_list_base::__nv_list_base(nvlist_t *nv, __nvlist_owning owning)
	: __m_nv(nv)
	, __m_owning(__nvlist_owning::__non_owning)
{
	assert(nv);
	__throw_if_error();
	__m_owning = owning;
}

__nv_list_base::~__nv_list_base()
{
	__free_nv();
}

void
__nv_list_base::__free_nv() noexcept
{
	if ((__m_nv != nullptr) &&
	    (__m_owning == __nvlist_owning::__owning))
		::nvlist_destroy(__m_nv);
}

void
__nv_list_base::__throw_if_error() const
{
	if (auto err = ::nvlist_error(__m_nv); err != 0)
		throw nv_error_state(std::error_code(err,
						     std::generic_category()));
}

} // namespace bsd::__detail
