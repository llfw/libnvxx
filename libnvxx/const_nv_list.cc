/*
 * SPDX-License-Identifier: Unlicense OR MIT
 * Refer to the file 'LICENSE' in the nvxx distribution for license terms.
 */

#include <cerrno>
#include <cassert>

#include "nvxx.h"

namespace bsd {

/*
 * const_nv_list
 */

const_nv_list::const_nv_list() noexcept
	: __nv_list_base(nullptr, __detail::__nvlist_owning::__non_owning)
{
}

// const_cast is safe here since a non-owning nvlist is never modified.
const_nv_list::const_nv_list(::nvlist_t const *nvl)
	: __nv_list_base(const_cast<::nvlist_t *>(nvl),
			 __detail::__nvlist_owning::__non_owning)
{
}

const_nv_list::const_nv_list(const_nv_list const &other) noexcept
	: __nv_list_base(other.__m_nv,
			 __detail::__nvlist_owning::__non_owning)
{
}

const_nv_list &
const_nv_list::operator=(const_nv_list const &other) noexcept
{
	if (this != &other) {
		/* This is not a leak since we never own the nvlist_t. */
		__m_nv = other.__m_nv;
	}

	return *this;
}

const_nv_list &
const_nv_list::operator=(nv_list const &other) noexcept
{
	/* This is not a leak since we never own the nvlist_t. */
	__m_nv = other.__m_nv;
	return *this;
}

::nvlist_t const *
const_nv_list::ptr() const
{
	return __m_nv;
}

namespace __detail {

std::error_code
__const_nv_list::error() const noexcept
{
	if (auto const err = nvlist_error(__m_nv); err != 0)
		return std::make_error_code(std::errc(err));
	return {};
}

bool
__const_nv_list::exists(std::string_view key) const
{
	return ::nvlist_exists(__m_nv, std::string(key).c_str());
}

bool
__const_nv_list::exists_type(std::string_view key, int type) const
{
	return ::nvlist_exists_type(__m_nv, std::string(key).c_str(), type);
}

bool
__const_nv_list::empty() const noexcept
{
	return ::nvlist_empty(__m_nv);
}

int
__const_nv_list::flags() const noexcept
{
	return ::nvlist_flags(__m_nv);
}

bool
__const_nv_list::in_array() const noexcept
{
	return ::nvlist_in_array(__m_nv);
}

__const_nv_list::operator bool() const noexcept
{
	return ::nvlist_error(__m_nv) == 0;
}

void
__const_nv_list::send(int fd) const
{
	__throw_if_error();

	if (::nvlist_send(fd, __m_nv) == 0)
		return;

	throw std::system_error(error());
}

void
__const_nv_list::dump(int fd) const noexcept
{
	::nvlist_dump(__m_nv, fd);
}

void
__const_nv_list::fdump(std::FILE *__fp) const noexcept
{
	::nvlist_fdump(__m_nv, __fp);
}

std::size_t
__const_nv_list::packed_size() const noexcept
{
	return ::nvlist_size(__m_nv);
}

std::vector<std::byte>
__const_nv_list::pack() const
{
	auto size = std::size_t{};

	if (auto *data = nvlist_pack(__m_nv, &size); data != nullptr) {
		auto bytes = __ptr_guard(static_cast<std::byte *>(data));
		return {bytes.__ptr, bytes.__ptr + size};
	}

	throw std::system_error(error());
}

/*
 * null operations
 */

bool
__const_nv_list::exists_null(std::string_view key) const
{
	return ::nvlist_exists_null(__m_nv, std::string(key).c_str());
}

/*
 * bool operations
 */

bool
__const_nv_list::exists_bool(std::string_view key) const
{
	return ::nvlist_exists_bool(__m_nv, std::string(key).c_str());
}

bool
__const_nv_list::get_bool(std::string_view key) const
{
	auto skey = std::string(key);

	if (!::nvlist_exists_bool(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	return ::nvlist_get_bool(__m_nv, skey.c_str());
}

bool
__const_nv_list::exists_bool_array(std::string_view key) const
{
	return ::nvlist_exists_bool_array(__m_nv, std::string(key).c_str());
}

std::span<bool const>
__const_nv_list::get_bool_array(std::string_view key) const
{
	auto skey = std::string(key);

	if (!::nvlist_exists_bool_array(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	auto nitems = std::size_t{};
	auto *data = ::nvlist_get_bool_array(__m_nv, skey.c_str(), &nitems);
	return {data, nitems};
}

/*
 * number operations
 */

bool
__const_nv_list::exists_number(std::string_view key) const
{
	return ::nvlist_exists_number(__m_nv, std::string(key).c_str());
}

std::uint64_t
__const_nv_list::get_number(std::string_view key) const
{
	auto skey = std::string(key);

	if (!::nvlist_exists_number(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	return ::nvlist_get_number(__m_nv, skey.c_str());
}

bool
__const_nv_list::exists_number_array(std::string_view key) const
{
	return ::nvlist_exists_number_array(__m_nv, std::string(key).c_str());
}

std::span<std::uint64_t const>
__const_nv_list::get_number_array(std::string_view key) const
{
	auto skey = std::string(key);

	if (!::nvlist_exists_number_array(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	auto nitems = std::size_t{};
	auto *data = ::nvlist_get_number_array(__m_nv, skey.c_str(), &nitems);
	return {data, nitems};
}

/*
 * string operations
 */

bool
__const_nv_list::exists_string(std::string_view key) const
{
	return ::nvlist_exists_string(__m_nv, std::string(key).c_str());
}

std::string_view
__const_nv_list::get_string(std::string_view key) const
{
	auto skey = std::string(key);

	if (!::nvlist_exists_string(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	return nvlist_get_string(__m_nv, skey.c_str());
}

bool
__const_nv_list::exists_string_array(std::string_view key) const
{
	return nvlist_exists_string_array(__m_nv, std::string(key).c_str());
}

std::vector<std::string_view>
__const_nv_list::get_string_array(std::string_view key) const
{
	auto skey = std::string(key);

	if (!::nvlist_exists_string_array(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	auto nitems = std::size_t{};
	auto *data = nvlist_get_string_array(__m_nv, skey.c_str(), &nitems);
	return std::span(data, data + nitems)
		| construct<std::string_view>()
		| std::ranges::to<std::vector>();
}

/*
 * nv_list operations
 */

bool
__const_nv_list::exists_nvlist(std::string_view key) const
{
	return nvlist_exists_nvlist(__m_nv, std::string(key).c_str());
}

const_nv_list
__const_nv_list::get_nvlist(std::string_view key) const
{
	auto skey = std::string(key);

	if (!::nvlist_exists_nvlist(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	auto nvl = ::nvlist_get_nvlist(__m_nv, skey.c_str());
	return const_nv_list(nvl);
}

bool
__const_nv_list::exists_nvlist_array(std::string_view key) const
{
	return nvlist_exists_nvlist_array(__m_nv, std::string(key).c_str());
}

std::vector<const_nv_list>
__const_nv_list::get_nvlist_array(std::string_view key) const
{
	auto skey = std::string(key);

	if (!::nvlist_exists_nvlist_array(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	auto nitems = std::size_t{};
	auto *data = nvlist_get_nvlist_array(__m_nv, skey.c_str(), &nitems);
	return {std::from_range,
		std::span(data, nitems) | construct<const_nv_list>()};
}

/*
 * descriptor operations
 */

int
__const_nv_list::get_descriptor(std::string_view key) const
{
	auto skey = std::string(key);

	if (!::nvlist_exists_descriptor(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	return ::nvlist_get_descriptor(__m_nv, skey.c_str());
}

std::span<int const>
__const_nv_list::get_descriptor_array(std::string_view key) const
{
	auto skey = std::string(key);

	if (!::nvlist_exists_descriptor_array(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	auto nitems = std::size_t{};
	auto *data = ::nvlist_get_descriptor_array(
			__m_nv, std::string(key).c_str(), &nitems);
	return {data, nitems};
}

bool
__const_nv_list::exists_descriptor(std::string_view key) const
{
	return ::nvlist_exists_descriptor(__m_nv, std::string(key).c_str());
}

bool
__const_nv_list::exists_descriptor_array(std::string_view key) const
{
	return ::nvlist_exists_descriptor_array(__m_nv,
						std::string(key).c_str());
}

/*
 * binary operations
 */

bool
__const_nv_list::exists_binary(std::string_view key) const
{
	return ::nvlist_exists_binary(__m_nv, std::string(key).c_str());
}

std::span<std::byte const>
__const_nv_list::get_binary(std::string_view key) const
{
	auto skey = std::string(key);

	if (!::nvlist_exists_binary(__m_nv, skey.c_str()))
		throw nv_key_not_found(skey);

	auto size = std::size_t{};
	auto *data = nvlist_get_binary(__m_nv, skey.c_str(), &size);
	return {static_cast<std::byte const *>(data), size};
}

} // namespace bsd::__detail
} // namespace bsd
