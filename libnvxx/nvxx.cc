/*
 * SPDX-License-Identifier: Unlicense OR MIT
 * Refer to the file 'LICENSE' in the nvxx distribution for license terms.
 */

#include <cerrno>
#include <cassert>

#include "nvxx.h"

namespace bsd::__detail {

/*
 * __nv_list_base
 */

__nv_list_base::__nv_list_base(nvlist_t *nv, __nvlist_owning owning)
	: __m_nv(nv)
	, __m_owning(owning)
{
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
	__m_nv = nullptr;
}

void
__nv_list_base::__throw_if_error() const
{
	__throw_if_null();

	if (auto err = ::nvlist_error(__m_nv); err != 0)
		throw nv_error_state(std::error_code(err, std::generic_category()));
}

void
__nv_list_base::__throw_if_null() const
{
	if (__m_nv == nullptr)
		throw std::logic_error("attempt to access a null nv_list");
}

void
__nv_list_base::__check_string_null(std::string_view str,
				    std::string_view error) const
{
	if (str.find('\0') == str.npos)
		return;
	throw std::runtime_error(std::string(error));
}

} // namespace bsd::__detail
