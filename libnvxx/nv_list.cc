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

namespace bsd {

/*
 * nv_list
 */

nv_list::nv_list(int flags)
	: __nv_list_base(flags)
{
	if (__m_nv == nullptr)
		throw std::system_error(
			std::error_code(errno, std::system_category()));
}

nv_list::nv_list(::nvlist_t *nvl)
	: __nv_list_base(nvl, __detail::__nvlist_owning::__owning)
{
}

nv_list::nv_list(const_nv_list const &other)
	: __nv_list_base(::nvlist_clone(other.ptr()),
			 __detail::__nvlist_owning::__owning)
{
	if (__m_nv == nullptr)
		throw std::system_error(
			std::error_code(errno, std::system_category()));
}

nv_list::nv_list(nv_list const &other)
	: __nv_list_base(::nvlist_clone(other.__m_nv),
			 __detail::__nvlist_owning::__owning)
{
	if (__m_nv == nullptr)
		throw std::system_error(
			std::error_code(errno, std::system_category()));
}

nv_list::nv_list(nv_list &&other) noexcept
	: __nv_list_base(std::exchange(other.__m_nv, nullptr),
			 __detail::__nvlist_owning::__owning)
{
}

nv_list &
nv_list::operator=(nv_list const &other)
{
	if (this != &other) {
		auto *clone = nvlist_clone(other.__m_nv);
		if (clone == nullptr)
			throw std::system_error(
				std::error_code(errno,
						std::system_category()));
		__free_nv();
		__m_nv = clone;
		__m_owning = __detail::__nvlist_owning::__owning;
	}

	return *this;
}

nv_list &
nv_list::operator=(nv_list &&other) noexcept
{
	if (this != &other) {
		__m_nv = std::exchange(other.__m_nv, nullptr);
		__m_owning = __detail::__nvlist_owning::__owning;
	}

	return *this;
}

::nvlist_t *
nv_list::ptr()
{
	return __m_nv;
}

::nvlist_t const *
nv_list::ptr() const
{
	return __m_nv;
}

::nvlist_t *
nv_list::release() &&
{
	return std::exchange(__m_nv, nullptr);
}

nv_list
nv_list::unpack(std::span<std::byte> data, int flags)
{
	if (auto nv = ::nvlist_unpack(std::ranges::data(data),
				      std::ranges::size(data),
				      flags);
	    nv != nullptr) {
		return nv_list(nv);
	}

	throw std::system_error(
		std::error_code(errno, std::system_category()));
}

nv_list
nv_list::recv(int fd, int flags)
{
	if (auto *nv = ::nvlist_recv(fd, flags); nv != nullptr)
		return nv_list(nv);

	throw std::system_error(
		std::error_code(errno, std::system_category()));
}

nv_list
nv_list::xfer(int fd, nv_list &&nvl, int flags)
{
	if (auto *nv = ::nvlist_xfer(fd, nvl.__m_nv, flags);
	    nv != nullptr) {
		// nvlist_xfer destroys the original list
		nvl.__m_nv = nullptr;
		return nv_list(nv);
	}

	throw std::system_error(
		std::error_code(errno, std::system_category()));
}

namespace __detail {

/*
 * __nv_list
 */

void
__nv_list::set_error(int error) noexcept
{
	::nvlist_set_error(__m_nv, error);
}

__nv_list::operator const_nv_list() const
{
	return const_nv_list(__m_nv);
}

void
__nv_list::free(std::string_view key)
{
	::nvlist_free(__m_nv, std::string(key).c_str());
}

/*
 * null operations
 */

void
__nv_list::add_null(std::string_view key)
{
	auto skey = std::string(key);

	::nvlist_add_null(__m_nv, skey.c_str());

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

void
__nv_list::free_null(std::string_view key)
{
	::nvlist_free_null(__m_nv, std::string(key).c_str());
}

/*
 * bool operations
 */

void
__nv_list::add_bool(std::string_view key, bool value)
{
	auto skey = std::string(key);

	::nvlist_add_bool(__m_nv, skey.c_str(), value);

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

bool
__nv_list::take_bool(std::string_view key)
{
	auto skey = std::string(key);

	if (!::nvlist_exists_bool(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	return ::nvlist_take_bool(__m_nv, skey.c_str());
}

void
__nv_list::free_bool(std::string_view key)
{
	::nvlist_free_bool(__m_nv, std::string(key).c_str());
}

std::vector<bool>
__nv_list::take_bool_array(std::string_view key)
{
	auto nitems = std::size_t{};
	auto ptr = ptr_guard(::nvlist_take_bool_array(__m_nv,
						      std::string(key).c_str(),
						      &nitems));
	return std::vector<bool>(ptr.ptr, ptr.ptr + nitems);
}

void
__nv_list::add_bool_array(std::string_view key,
			  std::span<bool const> value)
{
	auto skey = std::string(key);

	::nvlist_add_bool_array(__m_nv, skey.c_str(),
				std::ranges::data(value),
				std::ranges::size(value));

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

void
__nv_list::move_bool_array(std::string_view key, std::span<bool> value)
{
	::nvlist_move_bool_array(__m_nv, std::string(key).c_str(),
				 std::ranges::data(value),
				 std::ranges::size(value));
}

void
__nv_list::append_bool_array(std::string_view key, bool value)
{
	::nvlist_append_bool_array(__m_nv, std::string(key).c_str(), value);
}

void
__nv_list::free_bool_array(std::string_view key)
{
	::nvlist_free_bool_array(__m_nv, std::string(key).c_str());
}

/*
 * number operations
 */

void
__nv_list::add_number(std::string_view key, std::uint64_t value)
{
	auto skey = std::string(key);

	::nvlist_add_number(__m_nv, skey.c_str(), value);

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

std::uint64_t
__nv_list::take_number(std::string_view key)
{
	auto skey = std::string(key);

	if (!::nvlist_exists_number(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	return ::nvlist_take_number(__m_nv, skey.c_str());
}

void
__nv_list::free_number(std::string_view key)
{
	::nvlist_free_number(__m_nv, std::string(key).c_str());
}

std::vector<std::uint64_t>
__nv_list::take_number_array(std::string_view key)
{
	auto nitems = std::size_t{};
	auto ptr = ptr_guard(
		::nvlist_take_number_array(__m_nv,
					   std::string(key).c_str(),
					   &nitems));
	return {ptr.ptr, ptr.ptr + nitems};
}

void
__nv_list::add_number_array(std::string_view key,
			    std::span<std::uint64_t const> value)
{
	auto skey = std::string(key);

	::nvlist_add_number_array(__m_nv, skey.c_str(),
				  std::ranges::data(value),
				  std::ranges::size(value));

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

void
__nv_list::move_number_array(std::string_view key,
			     std::span<std::uint64_t> value)
{
	::nvlist_move_number_array(__m_nv, std::string(key).c_str(),
				   std::ranges::data(value),
				   std::ranges::size(value));
}

void
__nv_list::append_number_array(std::string_view key, std::uint64_t value)
{
	::nvlist_append_number_array(__m_nv, std::string(key).c_str(), value);
}

void
__nv_list::free_number_array(std::string_view key)
{
	::nvlist_free_number_array(__m_nv, std::string(key).c_str());
}

/*
 * string operations
 */

void
__nv_list::add_string(std::string_view key, std::string_view value)
{
	auto skey = std::string(key);

	::nvlist_add_string(__m_nv, skey.c_str(), std::string(value).c_str());

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

void
__nv_list::move_string(std::string_view key, char *value)
{
	::nvlist_move_string(__m_nv, std::string(key).c_str(), value);
}

std::string
__nv_list::take_string(std::string_view key)
{
	auto skey = std::string(key);

	if (!::nvlist_exists_string(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	return nvlist_take_string(__m_nv, skey.c_str());
}

void
__nv_list::free_string(std::string_view key)
{
	nvlist_free_string(__m_nv, std::string(key).c_str());
}

void
__nv_list::add_string_array(std::string_view key,
			    std::span<std::string_view const> value)
{
	auto skey = std::string(key);

	// nvlist_add_string_array expects an array of NUL-terminated
	// C strings.

	auto strings = value
		| construct<std::string>()
		| std::ranges::to<std::vector>();

	auto ptrs = strings
		| std::views::transform(&std::string::c_str)
		| std::ranges::to<std::vector>();

	::nvlist_add_string_array(__m_nv, skey.c_str(),
				  ptrs.data(), ptrs.size());

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

void
__nv_list::move_string_array(std::string_view key,
			     std::span<char *> value)
{
	::nvlist_move_string_array(__m_nv, std::string(key).c_str(),
				   std::ranges::data(value),
				   std::ranges::size(value));
}

void
__nv_list::append_string_array(std::string_view key, std::string_view value)
{
	::nvlist_append_string_array(__m_nv, 
				     std::string(key).c_str(), 
				     std::string(value).c_str());
}

std::vector<std::string>
__nv_list::take_string_array(std::string_view key)
{
	auto nitems = std::size_t{};
	auto *data = nvlist_take_string_array(__m_nv, std::string(key).c_str(),
					      &nitems);
	return std::span(data, data + nitems)
		| construct<std::string>()
		| std::ranges::to<std::vector>();
}

void
__nv_list::free_string_array(std::string_view key)
{
	::nvlist_free_string_array(__m_nv, std::string(key).c_str());
}

/*
 * nv_list operations
 */

void
__nv_list::add_nvlist(std::string_view key, const_nv_list const &other)
{
	auto skey = std::string(key);

	::nvlist_add_nvlist(__m_nv, skey.c_str(), other.__m_nv);

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

void
__nv_list::move_nvlist(std::string_view key, nv_list &&value)
{
	::nvlist_move_nvlist(__m_nv, std::string(key).c_str(),
			     std::exchange(value.__m_nv, nullptr));
}

void
__nv_list::move_nvlist(std::string_view key, ::nvlist_t *value)
{
	::nvlist_move_nvlist(__m_nv, std::string(key).c_str(), value);
}

nv_list
__nv_list::take_nvlist(std::string_view key)
{
	auto skey = std::string(key);

	if (!::nvlist_exists_nvlist(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	auto nvl = nvlist_take_nvlist(__m_nv, skey.c_str());
	return nv_list(nvl);
}

void
__nv_list::free_nvlist(std::string_view key)
{
	nvlist_free_nvlist(__m_nv, std::string(key).c_str());
}

void
__nv_list::add_nvlist_array(std::string_view key,
			    std::span<const_nv_list const> value)
{
	auto skey = std::string(key);

	auto ptrs = value
		| std::views::transform(&const_nv_list::__m_nv)
		| std::ranges::to<std::vector>();

	::nvlist_add_nvlist_array(__m_nv, skey.c_str(),
				  ptrs.data(), ptrs.size());

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

void
__nv_list::add_nvlist_array(std::string_view key,
			    std::span<nv_list const> value)
{
	auto skey = std::string(key);

	auto ptrs = value
		| std::views::transform(&nv_list::__m_nv)
		| std::ranges::to<std::vector>();

	nvlist_add_nvlist_array(__m_nv, skey.c_str(),
				ptrs.data(), ptrs.size());

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

void
__nv_list::move_nvlist_array(std::string_view key,
			     std::span<::nvlist_t *> value)
{
	::nvlist_move_nvlist_array(__m_nv, std::string(key).c_str(),
				   std::ranges::data(value),
				   std::ranges::size(value));
}

void
__nv_list::append_nvlist_array(std::string_view key,
			       const_nv_list const &value)
{
	::nvlist_append_nvlist_array(__m_nv, std::string(key).c_str(),
				     value.__m_nv);
}

std::vector<nv_list>
__nv_list::take_nvlist_array(std::string_view key)
{
	auto nitems = std::size_t{};
	auto ptr = ptr_guard(
		::nvlist_take_nvlist_array(__m_nv,
					   std::string(key).c_str(),
					   &nitems));
	return {std::from_range,
		std::span(ptr.ptr, nitems) | construct<nv_list>()};
}

void
__nv_list::free_nvlist_array(std::string_view key)
{
	::nvlist_free_nvlist_array(__m_nv, std::string(key).c_str());
}

/*
 * descriptor operations
 */

int
__nv_list::take_descriptor(std::string_view key)
{
	return ::nvlist_take_descriptor(__m_nv, std::string(key).c_str());
}

void
__nv_list::add_descriptor(std::string_view key, int value)
{
	auto skey = std::string(key);

	::nvlist_add_descriptor(__m_nv, skey.c_str(), value);

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

void
__nv_list::move_descriptor(std::string_view key, int value)
{
	::nvlist_move_descriptor(__m_nv, std::string(key).c_str(), value);
}

void
__nv_list::free_descriptor(std::string_view key)
{
	::nvlist_free_descriptor(__m_nv, std::string(key).c_str());
}

void
__nv_list::append_descriptor_array(std::string_view key, int value)
{
	::nvlist_append_descriptor_array(__m_nv,
					 std::string(key).c_str(),
					 value);
}

void
__nv_list::add_descriptor_array(std::string_view key,
				std::span<int const> value)
{
	auto skey = std::string(key);

	::nvlist_add_descriptor_array(__m_nv, skey.c_str(),
				      std::ranges::data(value),
				      std::ranges::size(value));

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

void
__nv_list::move_descriptor_array(std::string_view key, std::span<int> value)
{
	::nvlist_add_descriptor_array(__m_nv, std::string(key).c_str(),
				      std::ranges::data(value),
				      std::ranges::size(value));
}

void
__nv_list::free_descriptor_array(std::string_view key)
{
	::nvlist_free_descriptor_array(__m_nv, std::string(key).c_str());
}

std::vector<int>
__nv_list::take_descriptor_array(std::string_view key)
{
	auto nitems = std::size_t{};
	auto ptr = ptr_guard(
		::nvlist_take_descriptor_array(__m_nv,
					       std::string(key).c_str(),
					       &nitems));
	return {ptr.ptr, ptr.ptr + nitems};
}

/*
 * binary operations
 */

void
__nv_list::add_binary(std::string_view key, std::span<std::byte const> value)
{
	auto skey = std::string(key);

	::nvlist_add_binary(__m_nv, skey.c_str(),
			    std::ranges::data(value),
			    std::ranges::size(value));

	switch (auto err = ::nvlist_error(__m_nv)) {
	case 0:
		return;

	case EEXIST:
		throw nv_key_exists(skey);

	default:
		throw std::system_error(
			std::error_code(err, std::generic_category()));
	}
}

void
__nv_list::move_binary(std::string_view key, std::span<std::byte> value)
{
	::nvlist_move_binary(__m_nv, std::string(key).c_str(),
			     std::ranges::data(value),
			     std::ranges::size(value));
}

void
__nv_list::free_binary(std::string_view key)
{
	::nvlist_free_binary(__m_nv, std::string(key).c_str());
}

std::vector<std::byte>
__nv_list::take_binary(std::string_view key)
{
	auto skey = std::string(key);

	if (!::nvlist_exists_binary(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	auto size = std::size_t{};
	auto *data = ::nvlist_take_binary(__m_nv, skey.c_str(), &size);
	auto ptr = ptr_guard(static_cast<std::byte *>(data));
	return {ptr.ptr, ptr.ptr + size};
}

} // namespace bsd
} // namespace detail
