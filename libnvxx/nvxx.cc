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

namespace {

/*
 * Some useful helper types.
 */

template<typename T>
struct ptr_guard {
	ptr_guard(T *ptr_) : ptr(ptr_) {}

	~ptr_guard() {
		std::free(ptr);
	}

	T *ptr;
};

template<typename T>
auto construct = std::views::transform([] (auto &&value) {
	return T(std::forward<decltype(value)>(value));
});

}

namespace bsd {

/*
 * const_nv_list
 */

const_nv_list::const_nv_list() noexcept
	: __nv_list_base(nullptr, __detail::__nvlist_owning::__non_owning)
{
}

// const_cast is safe here since a non-owning nvlist is never modified.
const_nv_list::const_nv_list(::nvlist_t const *nvl) noexcept
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

nv_list::nv_list(::nvlist_t *nvl) noexcept
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

} // namespace bsd

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
			std::error_code(errno, std::system_category()));
}

__nv_list_base::__nv_list_base(nvlist_t *nv, __nvlist_owning owning)
	: __m_nv(nv)
	, __m_owning(owning)
{
	assert(nv);
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

/*
 * __const_nv_list
 */

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
		auto bytes = ptr_guard(static_cast<std::byte *>(data));
		return {bytes.ptr, bytes.ptr + size};
	}

	throw std::system_error(error());
}

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

bool
__const_nv_list::exists_null(std::string_view key) const
{
	return ::nvlist_exists_null(__m_nv, std::string(key).c_str());
}

void
__nv_list::add_null(std::string_view key)
{
	::nvlist_add_null(__m_nv, std::string(key).c_str());
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
	::nvlist_add_bool(__m_nv, std::string(key).c_str(), value);
}

bool
__const_nv_list::exists_bool(std::string_view key) const
{
	return ::nvlist_exists_bool(__m_nv, std::string(key).c_str());
}

bool
__const_nv_list::get_bool(std::string_view key) const
{
	return ::nvlist_get_bool(__m_nv, std::string(key).c_str());
}

bool
__nv_list::take_bool(std::string_view key)
{
	return ::nvlist_take_bool(__m_nv, std::string(key).c_str());
}

void
__nv_list::free_bool(std::string_view key)
{
	::nvlist_free_bool(__m_nv, std::string(key).c_str());
}

bool
__const_nv_list::exists_bool_array(std::string_view key) const
{
	return ::nvlist_exists_bool_array(__m_nv, std::string(key).c_str());
}

std::span<bool const>
__const_nv_list::get_bool_array(std::string_view key) const
{
	auto nitems = std::size_t{};
	auto *data = ::nvlist_get_bool_array(__m_nv,
					     std::string(key).c_str(),
					     &nitems);
	return {data, nitems};
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
	::nvlist_add_bool_array(__m_nv,
				std::string(key).c_str(),
				std::ranges::data(value),
				std::ranges::size(value));
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
	::nvlist_add_number(__m_nv, std::string(key).c_str(), value);
}

bool
__const_nv_list::exists_number(std::string_view key) const
{
	return ::nvlist_exists_number(__m_nv, std::string(key).c_str());
}

std::uint64_t
__const_nv_list::get_number(std::string_view key) const
{
	return ::nvlist_get_number(__m_nv, std::string(key).c_str());
}

std::uint64_t
__nv_list::take_number(std::string_view key)
{
	return ::nvlist_take_number(__m_nv, std::string(key).c_str());
}

void
__nv_list::free_number(std::string_view key)
{
	::nvlist_free_number(__m_nv, std::string(key).c_str());
}

bool
__const_nv_list::exists_number_array(std::string_view key) const
{
	return ::nvlist_exists_number_array(__m_nv, std::string(key).c_str());
}

std::span<std::uint64_t const>
__const_nv_list::get_number_array(std::string_view key) const
{
	auto nitems = std::size_t{};
	auto *data = ::nvlist_get_number_array(
			__m_nv, std::string(key).c_str(), &nitems);
	return {data, nitems};
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
	::nvlist_add_number_array(__m_nv, std::string(key).c_str(),
				  std::ranges::data(value),
				  std::ranges::size(value));
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
	::nvlist_add_string(__m_nv,
			  std::string(key).c_str(),
			  std::string(value).c_str());
}

void
__nv_list::move_string(std::string_view key, char *value)
{
	::nvlist_move_string(__m_nv, std::string(key).c_str(), value);
}

bool
__const_nv_list::exists_string(std::string_view key) const
{
	return nvlist_exists_string(__m_nv, std::string(key).c_str());
}

std::string_view
__const_nv_list::get_string(std::string_view key) const
{
	return nvlist_get_string(__m_nv, std::string(key).c_str());
}

std::string
__nv_list::take_string(std::string_view key)
{
	return nvlist_take_string(__m_nv, std::string(key).c_str());
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
	// nvlist_add_string_array expects an array of NUL-terminated
	// C strings.

	auto strings = value
		| construct<std::string>()
		| std::ranges::to<std::vector>();

	auto ptrs = strings
		| std::views::transform(&std::string::c_str)
		| std::ranges::to<std::vector>();

	nvlist_add_string_array(__m_nv, std::string(key).c_str(),
				ptrs.data(), ptrs.size());
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

bool
__const_nv_list::exists_string_array(std::string_view key) const
{
	return nvlist_exists_string_array(__m_nv, std::string(key).c_str());
}

std::vector<std::string_view>
__const_nv_list::get_string_array(std::string_view key) const
{
	auto nitems = std::size_t{};
	auto *data = nvlist_get_string_array(__m_nv, std::string(key).c_str(),
					     &nitems);
	return std::span(data, data + nitems)
		| construct<std::string_view>()
		| std::ranges::to<std::vector>();
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
	nvlist_add_nvlist(__m_nv, std::string(key).c_str(), other.__m_nv);
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

bool
__const_nv_list::exists_nvlist(std::string_view key) const
{
	return nvlist_exists_nvlist(__m_nv, std::string(key).c_str());
}

const_nv_list
__const_nv_list::get_nvlist(std::string_view key) const
{
	auto nvl = nvlist_get_nvlist(__m_nv, std::string(key).c_str());
	return const_nv_list(nvl);
}

nv_list
__nv_list::take_nvlist(std::string_view key)
{
	auto nvl = nvlist_take_nvlist(__m_nv, std::string(key).c_str());
	return nv_list(nvl);
}

void
__nv_list::free_nvlist(std::string_view key)
{
	nvlist_free_nvlist(__m_nv, std::string(key).c_str());
}

bool
__const_nv_list::exists_nvlist_array(std::string_view key) const
{
	return nvlist_exists_nvlist_array(__m_nv, std::string(key).c_str());
}

void
__nv_list::add_nvlist_array(std::string_view key,
			    std::span<const_nv_list const> value)
{
	auto ptrs = value
		| std::views::transform(&const_nv_list::__m_nv)
		| std::ranges::to<std::vector>();

	::nvlist_add_nvlist_array(__m_nv, std::string(key).c_str(),
				  ptrs.data(), ptrs.size());
}

void
__nv_list::add_nvlist_array(std::string_view key,
			    std::span<nv_list const> value)
{
	auto ptrs = value
		| std::views::transform(&nv_list::__m_nv)
		| std::ranges::to<std::vector>();

	nvlist_add_nvlist_array(__m_nv, std::string(key).c_str(),
				ptrs.data(), ptrs.size());
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

std::vector<const_nv_list>
__const_nv_list::get_nvlist_array(std::string_view key) const
{
	auto nitems = std::size_t{};
	auto *data = nvlist_get_nvlist_array(__m_nv, std::string(key).c_str(),
					     &nitems);
	return {std::from_range,
		std::span(data, nitems) | construct<const_nv_list>()};
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
__const_nv_list::get_descriptor(std::string_view key) const
{
	return ::nvlist_get_descriptor(__m_nv, std::string(key).c_str());
}

int
__nv_list::take_descriptor(std::string_view key)
{
	return ::nvlist_take_descriptor(__m_nv, std::string(key).c_str());
}

void
__nv_list::add_descriptor(std::string_view key, int value)
{
	::nvlist_add_descriptor(__m_nv, std::string(key).c_str(), value);
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
	::nvlist_add_descriptor_array(__m_nv, std::string(key).c_str(),
				      std::ranges::data(value),
				      std::ranges::size(value));
}

void
__nv_list::move_descriptor_array(std::string_view key, std::span<int> value)
{
	::nvlist_add_descriptor_array(__m_nv, std::string(key).c_str(),
				      std::ranges::data(value),
				      std::ranges::size(value));
}

std::span<int const>
__const_nv_list::get_descriptor_array(std::string_view key) const
{
	auto nitems = std::size_t{};
	auto *data = ::nvlist_get_descriptor_array(
			__m_nv, std::string(key).c_str(), &nitems);
	return {data, nitems};
}

void
__nv_list::free_descriptor_array(std::string_view key)
{
	::nvlist_free_descriptor_array(__m_nv, std::string(key).c_str());
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

bool
__const_nv_list::exists_binary(std::string_view key) const
{
	return ::nvlist_exists_binary(__m_nv, std::string(key).c_str());
}

void
__nv_list::add_binary(std::string_view key, std::span<std::byte const> value)
{
	::nvlist_add_binary(__m_nv, std::string(key).c_str(),
			    std::ranges::data(value),
			    std::ranges::size(value));
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

std::span<std::byte const>
__const_nv_list::get_binary(std::string_view key) const
{
	auto size = std::size_t{};
	auto *data = nvlist_get_binary(__m_nv, std::string(key).c_str(),
				       &size);
	return {static_cast<std::byte const *>(data), size};
}

std::vector<std::byte>
__nv_list::take_binary(std::string_view key)
{
	auto size = std::size_t{};
	auto *data = ::nvlist_take_binary(__m_nv,
					  std::string(key).c_str(),
					  &size);
	auto ptr = ptr_guard(static_cast<std::byte *>(data));
	return {ptr.ptr, ptr.ptr + size};
}

} // namespace bsd::__detail
