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

#ifndef	_NVXX_BASE_H_INCLUDED
#define _NVXX_BASE_H_INCLUDED

#include <sys/nv.h>

#include <expected>
#include <ranges>
#include <span>
#include <system_error>
#include <vector>
#include <stdexcept>

namespace bsd {

struct nv_list;
struct const_nv_list;

enum struct nvlist_owning {
	owning,
	non_owning
};

namespace __detail {

struct __nv_list_base {
protected:
	friend struct bsd::const_nv_list;

	__nv_list_base(int __flags = 0);
	__nv_list_base(::nvlist_t *,
		       nvlist_owning = nvlist_owning::owning);

	__nv_list_base(__nv_list_base const &) = delete;
	__nv_list_base(__nv_list_base &&) noexcept = delete;

	__nv_list_base &operator=(__nv_list_base const &) = delete;
	__nv_list_base &operator=(__nv_list_base &&) noexcept = delete;

	~__nv_list_base();
	void __free_nv() noexcept;

	::nvlist_t *__m_nv{};
	nvlist_owning __m_owning;
};

struct __const_nv_list : virtual __nv_list_base {
	friend struct __nv_list;

	/*
	 * Write the contents of this nvlist to __fd or __fp in a
	 * human-readable format suitable for debugging.
	 */
	void dump(int __fd) const noexcept;
	void fdump(std::FILE *__fp) const noexcept;

	/*
	 * Return the number of bytes that would be required to call pack() on
	 * this nvlist.  This is equivalent to nvlist_size(), but we rename it
	 * to packed_size() to avoid confusion with the usual container size()
	 * function (which returns the number of elements).
	 */
	[[nodiscard]] std::size_t packed_size() const noexcept;

	/*
	 * Return a byte array representing the contents of this nvlist; the
	 * array can later be passed to nv_list::unpack() to reproduce the
	 * nvlist.
	 */
	[[nodiscard]] std::vector<std::byte> pack() const;

	/*
	 * Return the error code associated with this nvlist, if any, by
	 * calling nvlist_error().
	 */
	[[nodiscard]] std::error_code error() const noexcept;

	/*
	 * Return true if this nvlist is in a non-error state, otherwise false.
	 */
	[[nodiscard]] explicit operator bool() const noexcept;

	/*
	 * Returns true if this nvlist is empty (contains no values).
	 */
	[[nodiscard]] bool empty() const noexcept;

	/*
	 * Return the flags used to create this nv_list.
	 */
	[[nodiscard]] int flags() const noexcept;

	/*
	 * Returns true if this nv_list is part of an array contained inside
	 * another nv_list.
	 */
	[[nodiscard]] bool in_array() const noexcept;

	/*
	 * Pack this nvlist and write it to the given file descriptor. On
	 * error, throws std::system_error.
	 */
	void send(int __fd) const;

	/*
	 * if a key of any type with the given name exists, return true.
	 */
	[[nodiscard]] bool exists(std::string_view __key) const;

	// TODO: exists_type()

	/* exists */

	[[nodiscard]] bool exists_null(std::string_view __key) const;
	[[nodiscard]] bool exists_bool(std::string_view __key) const;
	[[nodiscard]] bool exists_number(std::string_view __key) const;
	[[nodiscard]] bool exists_string(std::string_view __key) const;
	[[nodiscard]] bool exists_nvlist(std::string_view __key) const;
	[[nodiscard]] bool exists_descriptor(std::string_view __key) const;
	[[nodiscard]] bool exists_binary(std::string_view __key) const;

	[[nodiscard]] bool exists_bool_array(std::string_view __key) const;
	[[nodiscard]] bool exists_number_array(std::string_view __key) const;
	[[nodiscard]] bool exists_string_array(std::string_view __key) const;
	[[nodiscard]] bool exists_nvlist_array(std::string_view __key) const;
	[[nodiscard]] bool exists_descriptor_array(
				std::string_view __key) const;

	/* get */

	[[nodiscard]] auto
	get_bool(std::string_view __key) const -> bool;

	[[nodiscard]] auto
	get_number(std::string_view __key) const -> std::uint64_t;

	[[nodiscard]] auto
	get_string(std::string_view __key) const -> std::string_view;

	[[nodiscard]] auto
	get_nvlist(std::string_view __key) const -> const_nv_list;

	[[nodiscard]] auto
	get_descriptor(std::string_view __key) const -> int;

	[[nodiscard]] auto
	get_binary(std::string_view key) const -> std::span<std::byte const>;


	[[nodiscard]] auto
	get_bool_array(std::string_view __key) const -> std::span<bool const>;

	[[nodiscard]] auto
	get_number_array(std::string_view __key) const 
		-> std::span<std::uint64_t const>;

	[[nodiscard]] auto
	get_string_array(std::string_view __key) const
		-> std::vector<std::string_view>;

	[[nodiscard]] auto
	get_nvlist_array(std::string_view name) const
		-> std::vector<const_nv_list>;

	[[nodiscard]] auto
	get_descriptor_array(std::string_view __key) const
		-> std::span<int const>;
};

struct __nv_list : virtual __nv_list_base {
	friend struct const_nv_list;

	/*
	 * Set the error code on this nvlist to the given value.
	 */
	void set_error(int __error) noexcept;

	/*
	 * Convert this nv_list into a const_nv_list.  This is a shallow copy
	 * which does not clone the underlying nvlist_t; therefore, the
	 * lifetime of the const_nv_list ends when this nv_list is destroyed.
	 */
	operator const_nv_list() const;

	/* add */

	void add_null(std::string_view __key);
	void add_bool(std::string_view __key, bool);
	void add_number(std::string_view __key, std::uint64_t);
	void add_string(std::string_view __key, std::string_view);
	void add_nvlist(std::string_view __key, const_nv_list const &);
	void add_descriptor(std::string_view __key, int);
	void add_binary(std::string_view __key, std::span<std::byte const>);

	void add_bool_array(std::string_view __key,
			    std::span<bool const>);
	void add_number_array(std::string_view __key,
			      std::span<std::uint64_t const>);
	void add_string_array(std::string_view __key,
			      std::span<std::string_view const>);
	void add_nvlist_array(std::string_view __key,
			      std::span<const_nv_list const>);
	void add_nvlist_array(std::string_view __key,
			      std::span<nv_list const>);
	void add_descriptor_array(std::string_view __key,
				  std::span<int const>);

	/* free */

	void free(std::string_view __key);
	// TODO: free_type()
	void free_null(std::string_view __key);
	void free_bool(std::string_view __key);
	void free_number(std::string_view __key);
	void free_string(std::string_view __key);
	void free_nvlist(std::string_view __key);
	void free_descriptor(std::string_view __key);
	void free_binary(std::string_view __key);

	void free_bool_array(std::string_view __key);
	void free_number_array(std::string_view __key);
	void free_string_array(std::string_view __key);
	void free_nvlist_array(std::string_view __key);
	void free_descriptor_array(std::string_view __key);

	/* take */

	[[nodiscard]] auto take_bool(std::string_view __key) -> bool;
	[[nodiscard]] auto take_number(std::string_view __key)
		-> std::uint64_t;
	[[nodiscard]] auto take_string(std::string_view __key) -> std::string;
	[[nodiscard]] auto take_nvlist(std::string_view __key) -> nv_list;
	[[nodiscard]] auto take_descriptor(std::string_view __key) -> int;
	[[nodiscard]] auto take_binary(std::string_view __key)
		-> std::vector<std::byte>;

	[[nodiscard]] auto
	take_bool_array(std::string_view __key) -> std::vector<bool>;

	[[nodiscard]] auto
	take_number_array(std::string_view __key)
		-> std::vector<std::uint64_t>;

	[[nodiscard]] auto
	take_string_array(std::string_view __key) -> std::vector<std::string>;

	[[nodiscard]] auto
	take_nvlist_array(std::string_view __key) -> std::vector<nv_list>;

	[[nodiscard]] auto
	take_descriptor_array(std::string_view __key) -> std::vector<int>;

	/* move */

	void move_string(std::string_view __key, char *);
	void move_nvlist(std::string_view __key, nv_list &&);
	void move_nvlist(std::string_view __key, ::nvlist_t *);
	void move_descriptor(std::string_view __key, int);
	void move_binary(std::string_view __key, std::span<std::byte>);

	void move_bool_array(std::string_view __key, std::span<bool>);
	void move_number_array(std::string_view __key, std::span<std::uint64_t>);
	void move_string_array(std::string_view __key, std::span<char *>);
	void move_nvlist_array(std::string_view __key, std::span<::nvlist_t *>);
	void move_descriptor_array(std::string_view __key, std::span<int>);

	/* append */

	void append_bool_array(std::string_view __key, bool);
	void append_number_array(std::string_view __key, std::uint64_t);
	void append_string_array(std::string_view __key, std::string_view);
	void append_nvlist_array(std::string_view __key, const_nv_list const &);
	void append_descriptor_array(std::string_view __key, int);

	/* add_bool_range */
	template<std::ranges::contiguous_range _Range>
		requires std::is_same_v<bool,
			std::remove_cvref_t<
				std::ranges::range_value_t<_Range>>>
	void add_bool_range(std::string_view __key, _Range &&__value)
	{
		add_bool_array(__key, std::span(__value));
	}

	template<std::ranges::range _Range>
		requires (!std::ranges::contiguous_range<_Range>
			&& std::is_same_v<bool,
				std::remove_cvref_t<
					std::ranges::range_value_t<_Range>>>)
	void add_bool_range(std::string_view __key, _Range &&__value)
	{
		/*
		 * since vector<bool> is not a contiguous_range,
		 * we need to do two copies here.
		 */

		auto __v = std::vector(std::from_range, __value);
		auto __p = std::make_unique<bool[]>(__v.size());
		std::ranges::copy(__v, __p.get());
		add_bool_array(__key, std::span(__p.get(), __v.size()));
	}

	/* add_number_range */

	template<std::ranges::contiguous_range _Range>
		requires std::is_same_v<std::uint64_t,
			std::remove_cvref_t<
				std::ranges::range_value_t<_Range>>>
	void add_number_range(std::string_view __key, _Range &&__value)
	{
		add_number_array(__key, std::span(__value));
	}

	template<std::ranges::range _Range>
		requires (
			!std::ranges::contiguous_range<_Range>
			&& std::is_same_v<std::uint64_t,
				std::remove_cvref_t<
					std::ranges::range_value_t<_Range>>>)
	void add_number_range(std::string_view __key, _Range &&__value)
	{
		auto __arr = std::vector(std::from_range, __value);
		add_number_range(__key, __arr);
	}


	/* add_string_range */

	template<std::ranges::contiguous_range _Range>
		requires std::is_same_v<std::string_view,
			std::remove_cvref_t<
				std::ranges::range_value_t<_Range>>>
	void add_string_range(std::string_view __key, _Range &&__value)
	{
		add_string_array(__key, std::span(__value));
	}

	template<std::ranges::range _Range>
		requires (!std::ranges::contiguous_range<_Range>
			&& std::is_same_v<std::string_view,
				std::remove_cvref_t<
					std::ranges::range_value_t<_Range>>>)
	void add_string_range(std::string_view __key, _Range &&__value)
	{
		auto __arr = std::vector(std::from_range, __value);
		add_string_range(__key, __arr);
	}

	/* add_binary_range */

	template<std::ranges::contiguous_range _Range>
		requires std::is_same_v<std::byte,
			std::remove_cvref_t<
				std::ranges::range_value_t<_Range>>>
	void add_binary_range(std::string const &__name, _Range &&__value)
	{
		auto __span = std::span(__value);
		add_binary(__name, __span);
	}

	template<std::ranges::range _Range>
		requires (
			!std::ranges::contiguous_range<_Range>
			&& std::is_same_v<std::byte,
				std::remove_cvref_t<
					std::ranges::range_value_t<_Range>>>)
	void add_binary_range(std::string_view __key, _Range &&__value)
	{
		auto __arr = std::vector(std::from_range, __value);
		add_number_range(__key, __arr);
	}
};

} // namespace bsd::__detail

/*
 * const_nv_list is an immutable, non-owning reference to an nvlist.
 * it will not free the nvlist_t on destruction.
 */
struct const_nv_list final
	: virtual __detail::__nv_list_base
	, __detail::__const_nv_list
{
	/*
	 * Default constructing a const_nv_list leaves it in the empty state;
	 * it can be assigned to or destructed but no other operations are
	 * valid.
	 */
	const_nv_list() noexcept;

	/*
	 * Create an nv_list object that refers to an existing nvlist_t.  The
	 * const_nv_list is non-owning, i.e. it will not free the nvlist_t on
	 * destruction.  If the nvlist_t is null, the const_nv_list will be
	 * empty.
	 */
	explicit const_nv_list(::nvlist_t const *) noexcept;

	/*
	 * Copy the nvlist pointer from an existing const_nv_list.  This does
	 * not clone the nvlist itself; both nvlists will have the same
	 * lifetime.  If the other nvlist is empty, this nvlist will also be
	 * empty.
	 */
	const_nv_list(const_nv_list const &) noexcept;

	/*
	 * Cause this const_nv_list to refer to the same nvlist as the RHS.
	 * This does not clone the nvlist; both nvlists will have the same
	 * lifetime.  If the RHS nvlist is empty, this nvlist will also be
	 * empty.
	 */
	const_nv_list &operator=(const_nv_list const &) noexcept;
	const_nv_list &operator=(nv_list const &) noexcept;

	/*
	 * Return the nvlist pointer stored by this const_nv_list.  Since
	 * const_nv_list is non-owning, the lifetime of the pointer is
	 * unspecified.
	 */
	::nvlist_t const *ptr() const;
};

/*
 * nv_list is a mutable, owning reference to an nvlist.  it will free the
 * nvlist_t on destruction, invalidating any const_nv_lists created from it.
 */
struct nv_list final
	: virtual __detail::__nv_list_base
	, __detail::__const_nv_list
	, __detail::__nv_list
{
	/*
	 * Create a new, empty nv_list.  On failure, throws std::system_error.
	 * The flags argument is passed to nvlist_create().
	 */
	explicit nv_list(int __flags = 0);

	/*
	 * Create an nv_list object that refers to an existing nvlist_t.
	 */
	explicit nv_list(::nvlist_t *) noexcept;

	/*
	 * Create an nv_list by copying an existing nv_list object with
	 * nvlist_clone().  On failure, throws std::system_error.
	 */
	nv_list(nv_list const &);

	/*
	 * Create an nv_list by moving from an existing nv_list.  The
	 * moved-from nvlist is left in an undefined state and must not be
	 * accessed other than to destruct it.
	 */
	nv_list(nv_list &&) noexcept;

	/*
	 * Replace the wrapped nv_list with a copy of the RHS nv_list using
	 * nvlist_clone().  On failure, throws std::system_error.
	 */
	nv_list &operator=(nv_list const &);

	/*
	 * Replace the wrapped nv_list by moving from another nv_list.  The
	 * moved-from nv_list is left in an undefined state and must not be
	 * accessed other than to destruct it.
	 */
	nv_list &operator=(nv_list &&) noexcept;

	/*
	 * Return the pointer stored by this nv_list, without releasing it.
	 * The pointer may be used to modify the nvlist, but must not be
	 * freed (e.g., by passing it to a function like nvlist_xfer()).
	 */
	::nvlist_t *ptr();
	::nvlist_t const *ptr() const;

	/*
	 * Release the pointer held by this nv_list and return it.  The nv_list
	 * will be left in a moved-from state and must not be used other than
	 * to destruct it.
	 */
	::nvlist_t *release() &&;

	/*
	 * Create and return an nv_list by calling nvlist_unpack() on the
	 * provided buffer.  The __flags argument is passed to nvlist_unpack().
	 * On failure, throws std::system_error.
	 */
	[[nodiscard]] static nv_list
		unpack(std::span<std::byte> __data, int __flags = 0);

	/*
	 * As the previous function, but the nv_list is unpacked from the
	 * provided range, which must be a contiguous_range of value type
	 * std::byte.
	 */
	template<std::ranges::contiguous_range _Range>
		requires std::is_same_v<std::byte,
			std::remove_cvref_t<
				std::ranges::range_value_t<_Range>>>
	[[nodiscard]] static nv_list
		unpack(_Range const &__data, int __flags = 0)
	{
		return unpack(std::span<std::byte>(__data), __flags);
	}

	/*
	 * Receive an nv_list from a file descriptor by calling nvlist_recv(),
	 * to which flags is passed.  On failure, throws std::system_error.
	 */
	[[nodiscard]] static auto recv(int __fd, int __flags) -> nv_list;

	/*
	 * Send an nv_list over a file descriptor and receive another nv_list
	 * in response, which is returned, by calling nvlist_xfer(). On
	 * failure, throws std::system_error.
	 *
	 * The source nv_list is moved-from and is left in an undefined state.
	 * The returned nv_list is owning.
	 */
	[[nodiscard]] static nv_list xfer(int __fd,
					  nv_list &&__source,
					  int __flags);
};

} // namespace bsd

#endif	/* !_NVXX_BASE_H_INCLUDED */
