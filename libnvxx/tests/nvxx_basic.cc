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

#include <algorithm>
#include <ranges>
#include <list>
#include <vector>
#include <span>
#include <string>
#include <string_view>

#include <atf-c++.hpp>

#include "nvxx.h"

#define TEST_CASE(name)				\
	ATF_TEST_CASE_WITHOUT_HEAD(name)	\
	ATF_TEST_CASE_BODY(name)

/*
 * constructor tests
 */

TEST_CASE(nvxx_ctor_default)
{
	auto nvl = bsd::nv_list();
}

TEST_CASE(nvxx_ctor_error)
{
	auto nvp = ::nvlist_create(0);
	::nvlist_set_error(nvp, EINVAL);
	try {
	ATF_REQUIRE_THROW(bsd::nv_error_state,
			  auto nvl = bsd::nv_list(nvp));
	} catch (bsd::nv_error_state const &) {}
}

TEST_CASE(nvxx_const_ctor_error)
{
	auto nvp = ::nvlist_create(0);
	::nvlist_set_error(nvp, EINVAL);
	ATF_REQUIRE_THROW(bsd::nv_error_state,
			  auto nvl = bsd::const_nv_list(nvp));
}

/*
 * test the NV_FLAG_IGNORE_CASE flag.
 */

TEST_CASE(nvxx_ignore_case)
{
	auto nvl = bsd::nv_list(NV_FLAG_IGNORE_CASE);
	nvl.add_number("TEST number", 42u);
	ATF_REQUIRE_EQ(true, nvl.exists_number("TesT nUMBEr"));

	auto n = nvl.take_number("test NuMbEr");
	ATF_REQUIRE_EQ(42u, n);
	ATF_REQUIRE_EQ(false, nvl.exists_number("TesT nUMBEr"));
}

/*
 * null tests
 */

TEST_CASE(nvxx_add_null)
{
	using namespace std::literals;
	auto constexpr key = "test_null"sv;

	auto nvl = bsd::nv_list();
	nvl.add_null(key);
	ATF_REQUIRE_EQ(true, nvl.exists_null(key));
	ATF_REQUIRE_EQ(true, nvl.exists(key));
}

TEST_CASE(nvxx_add_null_error)
{
	using namespace std::literals;
	auto constexpr key = "test_null"sv;

	auto nvl = bsd::nv_list();
	nvl.set_error(EINVAL);
	ATF_REQUIRE_THROW(bsd::nv_error_state, nvl.add_null(key));
}

TEST_CASE(nvxx_add_duplicate_null)
{
	using namespace std::literals;
	auto constexpr key = "test_null"sv;

	auto nvl = bsd::nv_list();
	nvl.add_null(key);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_null\" already exists",
			     nvl.add_null(key));
}

TEST_CASE(nvxx_free_null)
{
	using namespace std::literals;
	auto constexpr key = "test null"sv;

	auto nvl = bsd::nv_list();

	nvl.add_null(key);
	ATF_REQUIRE_EQ(true, nvl.exists_null(key));

	nvl.free_null(key);
	ATF_REQUIRE_EQ(false, nvl.exists_null(key));
}

/*
 * bool tests
 */

TEST_CASE(nvxx_add_bool)
{
	using namespace std::literals;
	auto constexpr key = "test_bool"sv;
	auto constexpr value = true;

	auto nvl = bsd::nv_list();
	nvl.add_bool(key, value);

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_bool(key));
	ATF_REQUIRE_EQ(value, nvl.get_bool(key));
}

TEST_CASE(nvxx_add_bool_error)
{
	using namespace std::literals;
	auto constexpr key = "test_bool"sv;
	auto constexpr value = true;

	auto nvl = bsd::nv_list();
	nvl.set_error(EINVAL);
	ATF_REQUIRE_THROW(bsd::nv_error_state, nvl.add_bool(key, value));
}

TEST_CASE(nvxx_add_duplicate_bool)
{
	using namespace std::literals;
	auto constexpr key = "test_bool"sv;
	auto constexpr value = true;

	auto nvl = bsd::nv_list();
	nvl.add_bool(key, value);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_bool\" already exists",
			     nvl.add_bool(key, value));
}

TEST_CASE(nvxx_get_nonexistent_bool)
{
	auto nvl = bsd::nv_list();

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"nonesuch\" not found",
			     (void)nvl.get_bool("nonesuch"));
}

TEST_CASE(nvxx_take_bool)
{
	using namespace std::literals;
	auto constexpr key = "test_bool"sv;
	auto constexpr value = true;

	auto nvl = bsd::nv_list();
	nvl.add_bool(key, value);
	auto b = nvl.take_bool(key);
	ATF_REQUIRE_EQ(value, b);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"test_bool\" not found",
			     (void)nvl.take_bool(key));
}

TEST_CASE(nvxx_free_bool)
{
	using namespace std::literals;
	auto constexpr key = "test_bool"sv;
	auto constexpr value = true;

	auto nvl = bsd::nv_list();
	nvl.add_bool(key, value);
	nvl.free_bool(key);
	ATF_REQUIRE_EQ(false, nvl.exists(key));
}

TEST_CASE(nvxx_add_bool_array)
{
	using namespace std::literals;
	auto constexpr key = "test_bool"sv;

	auto data = std::array<bool, 2>{true, false};

	auto nvl = bsd::nv_list();
	nvl.add_bool_array(key, std::span(data));

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_bool_array(key));

	auto data2 = nvl.get_bool_array(key);
	ATF_REQUIRE_EQ(2, data2.size());
	ATF_REQUIRE_EQ(true, data2[0]);
	ATF_REQUIRE_EQ(false, data2[1]);

	auto data3 = nvl.take_bool_array(key);
	ATF_REQUIRE_EQ(2, data3.size());
	ATF_REQUIRE_EQ(true, data3[0]);
	ATF_REQUIRE_EQ(false, data3[1]);
	ATF_REQUIRE_EQ(false, nvl.exists(key));
}

TEST_CASE(nvxx_add_bool_array_error)
{
	using namespace std::literals;
	auto constexpr key = "test_bool"sv;
	auto value = std::vector<bool>{true, false};

	auto nvl = bsd::nv_list();
	nvl.set_error(EINVAL);
	ATF_REQUIRE_THROW(bsd::nv_error_state, nvl.add_bool_range(key, value));
}

TEST_CASE(nvxx_get_nonexistent_bool_array)
{
	auto nvl = bsd::nv_list();

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"nonesuch\" not found",
			     (void)nvl.get_bool_array("nonesuch"));
}

TEST_CASE(nvxx_add_duplicate_bool_array)
{
	using namespace std::literals;
	auto constexpr key = "test_bool"sv;
	auto data = std::array<bool, 2>{true, false};
	auto value = std::span(data);

	auto nvl = bsd::nv_list();
	nvl.add_bool_array(key, value);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_bool\" already exists",
			     nvl.add_bool_array(key, value));
}

TEST_CASE(nvxx_add_bool_range)
{
	using namespace std::literals;
	auto constexpr key = "test_bool"sv;

	auto data = std::list{true, false};

	auto nvl = bsd::nv_list();
	nvl.add_bool_range(key, data);

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_bool_array(key));

	auto data2 = nvl.get_bool_array(key);
	ATF_REQUIRE_EQ(2, data2.size());
	ATF_REQUIRE_EQ(true, data2[0]);
	ATF_REQUIRE_EQ(false, data2[1]);
}

TEST_CASE(nvxx_add_bool_contig_range)
{
	using namespace std::literals;
	auto constexpr key = "test_bool"sv;

	auto data = std::vector{true, false};

	auto nvl = bsd::nv_list();
	nvl.add_bool_range(key, data);

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_bool_array(key));

	auto data2 = nvl.get_bool_array(key);
	ATF_REQUIRE_EQ(2, data2.size());
	ATF_REQUIRE_EQ(true, data2[0]);
	ATF_REQUIRE_EQ(false, data2[1]);
}

/*
 * number tests
 */

// a literal operator to create std::uint64_ts
constexpr std::uint64_t operator"" _u64(unsigned long long v) {
	return static_cast<std::uint64_t>(v);
}

TEST_CASE(nvxx_add_number)
{
	using namespace std::literals;
	auto constexpr key = "test_number"sv;
	auto constexpr value = 42_u64;

	auto nvl = bsd::nv_list();

	nvl.add_number(key, value);
	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_number(key));
	ATF_REQUIRE_EQ(value, nvl.get_number(key));
}

TEST_CASE(nvxx_add_number_error)
{
	using namespace std::literals;
	auto constexpr key = "test_number"sv;
	auto constexpr value = 42_u64;

	auto nvl = bsd::nv_list();
	nvl.set_error(EINVAL);

	ATF_REQUIRE_THROW(bsd::nv_error_state, nvl.add_number(key, value));
}

TEST_CASE(nvxx_add_duplicate_number)
{
	using namespace std::literals;
	auto constexpr key = "test_number"sv;
	auto constexpr value = 42u;

	auto nvl = bsd::nv_list();
	nvl.add_number(key, value);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_number\" already exists",
			     nvl.add_number(key, value));
}

TEST_CASE(nvxx_get_nonexistent_number)
{
	auto nvl = bsd::nv_list();
	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"nonesuch\" not found",
			     (void)nvl.get_number("nonesuch"));
}

TEST_CASE(nvxx_take_number)
{
	using namespace std::literals;
	auto constexpr key = "test_number"sv;
	auto constexpr value = 42_u64;

	auto nvl = bsd::nv_list();

	nvl.add_number(key, value);
	auto n = nvl.take_number(key);
	ATF_REQUIRE_EQ(value, n);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"test_number\" not found",
			     (void)nvl.take_number(key));
}

TEST_CASE(nvxx_free_number)
{
	using namespace std::literals;
	auto constexpr key = "test_number"sv;
	auto constexpr value = 42_u64;

	auto nvl = bsd::nv_list();

	nvl.add_number(key, value);
	nvl.free_number(key);
	ATF_REQUIRE_EQ(false, nvl.exists(key));
}

TEST_CASE(nvxx_add_number_array)
{
	using namespace std::literals;
	auto constexpr key = "test_number"sv;
	auto constexpr size = 16u;

	auto data = std::array<std::uint64_t, size>{};
	std::ranges::copy(std::ranges::iota_view{0u, size}, data.begin());

	auto nvl = bsd::nv_list();
	nvl.add_number_array(key, std::span(data));

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_number_array(key));

	auto data2 = nvl.get_number_array(key);
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data2));

	auto data3 = nvl.take_number_array(key);
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data3));
	ATF_REQUIRE_EQ(false, nvl.exists(key));
}

TEST_CASE(nvxx_add_number_array_error)
{
	using namespace std::literals;
	auto constexpr key = "test_number"sv;
	auto value = std::vector<std::uint64_t>{42, 666};

	auto nvl = bsd::nv_list();
	nvl.set_error(EINVAL);
	ATF_REQUIRE_THROW(bsd::nv_error_state,
			  nvl.add_number_range(key, value));
}

TEST_CASE(nvxx_get_nonexistent_number_array)
{
	auto nvl = bsd::nv_list();

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"nonesuch\" not found",
			     (void)nvl.get_number_array("nonesuch"));
}

TEST_CASE(nvxx_add_duplicate_number_array)
{
	using namespace std::literals;
	auto constexpr key = "test_number"sv;
	auto value = std::vector{42_u64, 1024_u64};

	auto nvl = bsd::nv_list();
	nvl.add_number_range(key, value);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_number\" already exists",
			     nvl.add_number_range(key, value));
}

TEST_CASE(nvxx_add_number_range)
{
	using namespace std::literals;
	auto constexpr key = "test_number"sv;
	auto constexpr size = 16u;

	auto data = std::list(std::from_range,
			      std::ranges::iota_view{0_u64, size});

	auto nvl = bsd::nv_list();
	nvl.add_number_range(key, data);

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_number_array(key));

	auto data2 = nvl.get_number_array(key);
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data2));
}

TEST_CASE(nvxx_add_number_contig_range)
{
	using namespace std::literals;
	auto constexpr key = "test_number"sv;
	auto constexpr size = 16u;

	auto data = std::vector(std::from_range,
				std::ranges::iota_view{0_u64, size});

	auto nvl = bsd::nv_list();
	nvl.add_number_range(key, data);

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_number_array(key));

	auto data2 = nvl.get_number_array(key);
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data2));
}

/*
 * string tests
 */

TEST_CASE(nvxx_add_string)
{
	using namespace std::literals;
	auto constexpr key = "test_string"sv;
	auto constexpr value = "testing value"sv;

	auto nvl = bsd::nv_list();
	nvl.add_string(key, value);

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_string(key));
	ATF_REQUIRE_EQ(value, nvl.get_string(key));
}

TEST_CASE(nvxx_add_duplicate_string)
{
	using namespace std::literals;
	auto constexpr key = "test_string"sv;
	auto constexpr value = "testing value"sv;

	auto nvl = bsd::nv_list();
	nvl.add_string(key, value);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_string\" already exists",
			     nvl.add_string(key, value));
}

TEST_CASE(nvxx_get_nonexistent_string)
{
	auto nvl = bsd::nv_list();
	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"nonesuch\" not found",
			     (void)nvl.get_string("nonesuch"));
}

TEST_CASE(nvxx_take_string)
{
	using namespace std::literals;
	auto constexpr key = "test_string"sv;
	auto constexpr value = "testing value"sv;

	auto nvl = bsd::nv_list();
	nvl.add_string(key, value);
	auto s = nvl.take_string(key);
	ATF_REQUIRE_EQ(value, s);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"test_string\" not found",
			     (void)nvl.take_string(key));
}

TEST_CASE(nvxx_free_string)
{
	using namespace std::literals;
	auto constexpr key = "test_string"sv;
	auto constexpr value = "testing value"sv;

	auto nvl = bsd::nv_list();
	nvl.add_string(key, value);
	nvl.free_string(key);
	ATF_REQUIRE_EQ(false, nvl.exists(key));
}

TEST_CASE(nvxx_add_string_array)
{
	using namespace std::literals;
	auto constexpr key = "test_string"sv;

	auto data = std::array<std::string_view, 2>{"one"sv, "two"sv};

	auto nvl = bsd::nv_list();
	nvl.add_string_array(key, std::span(data));

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_string_array(key));

	auto data2 = nvl.get_string_array(key);
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data2));

	auto data3 = nvl.take_string_array(key);
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data3));
	ATF_REQUIRE_EQ(false, nvl.exists(key));
}

TEST_CASE(nvxx_add_string_error)
{
	using namespace std::literals;
	auto constexpr key = "test_string"sv;
	auto constexpr value = "test"sv;

	auto nvl = bsd::nv_list{};
	nvl.set_error(EINVAL);
	ATF_REQUIRE_THROW(bsd::nv_error_state, nvl.add_string(key, value));
}

TEST_CASE(nvxx_add_string_array_error)
{
	using namespace std::literals;
	auto constexpr key = "test_string"sv;
	auto values = std::vector{"one"sv, "two"sv, "three"sv};

	auto nvl = bsd::nv_list();
	nvl.set_error(EINVAL);
	ATF_REQUIRE_THROW(bsd::nv_error_state,
			  nvl.add_string_range(key, values));
}

TEST_CASE(nvxx_get_nonexistent_string_array)
{
	auto nvl = bsd::nv_list();

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"nonesuch\" not found",
			     (void)nvl.get_string_array("nonesuch"));
}

TEST_CASE(nvxx_add_duplicate_string_array)
{
	using namespace std::literals;
	auto constexpr key = "test_string"sv;
	auto value = std::vector{"one"sv, "two"sv, "three"sv};

	auto nvl = bsd::nv_list();
	nvl.add_string_range(key, value);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_string\" already exists",
			     nvl.add_string_range(key, value));
}

TEST_CASE(nvxx_add_string_range)
{
	using namespace std::literals;
	auto constexpr key = "test_string"sv;

	auto data = std::list<std::string_view>{"one"sv, "two"sv};

	auto nvl = bsd::nv_list();
	nvl.add_string_range(key, data);

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_string_array(key));

	auto data2 = nvl.get_string_array(key);
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data2));

	auto data3 = nvl.take_string_array(key);
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data3));
	ATF_REQUIRE_EQ(false, nvl.exists(key));
}

TEST_CASE(nvxx_add_string_contig_range)
{
	using namespace std::literals;
	auto constexpr key = "test_string"sv;

	auto data = std::vector<std::string_view>{"one"sv, "two"sv};

	auto nvl = bsd::nv_list();
	nvl.add_string_range(key, data);

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_string_array(key));

	auto data2 = nvl.get_string_array(key);
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data2));

	auto data3 = nvl.take_string_array(key);
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data3));
	ATF_REQUIRE_EQ(false, nvl.exists(key));
}

/*
 * nv_list tests
 */

TEST_CASE(nvxx_add_nvlist)
{
	auto nvl = bsd::nv_list(), nvl2 = bsd::nv_list();

	nvl2.add_number("test_number", 42);
	nvl.add_nvlist("test_nvlist", nvl2);

	ATF_REQUIRE_EQ(true, nvl.exists("test_nvlist"));
	ATF_REQUIRE_EQ(true, nvl.exists_nvlist("test_nvlist"));
	ATF_REQUIRE_EQ(42, nvl.get_nvlist("test_nvlist")
		       		.get_number("test_number"));
}

TEST_CASE(nvxx_add_nvlist_error)
{
	using namespace std::literals;
	auto key = "test_nvlist"sv;
	auto value = bsd::nv_list{};

	auto nvl = bsd::nv_list{};
	nvl.set_error(EINVAL);
	ATF_REQUIRE_THROW(bsd::nv_error_state, nvl.add_nvlist(key, value));
}

TEST_CASE(nvxx_add_duplicate_nvlist)
{
	using namespace std::literals;
	auto constexpr key = "test_nvlist"sv;
	auto value = bsd::nv_list{};

	auto nvl = bsd::nv_list{};
	nvl.add_nvlist(key, value);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_nvlist\" already exists",
			     nvl.add_nvlist(key, value));
}

TEST_CASE(nvxx_get_nonexistent_nvlist)
{
	auto nvl = bsd::nv_list();
	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"nonesuch\" not found",
			     (void)nvl.get_nvlist("nonesuch"));
}

TEST_CASE(nvxx_take_nvlist)
{
	using namespace std::literals;
	auto constexpr key = "test_nvlist"sv;
	auto nvl = bsd::nv_list(), nvl2 = bsd::nv_list();

	nvl2.add_number("test_number", 42);
	nvl.add_nvlist(key, nvl2);

	auto nvl3 = nvl.take_nvlist(key);
	ATF_REQUIRE_EQ(42, nvl3.get_number("test_number"));

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"test_nvlist\" not found",
			     (void)nvl.take_nvlist(key));
}

TEST_CASE(nvxx_free_nvlist)
{
	auto nvl = bsd::nv_list(), nvl2 = bsd::nv_list();

	nvl2.add_number("test_number", 42);
	nvl.add_nvlist("test_nvlist", nvl2);

	nvl.free_nvlist("test_nvlist");
	ATF_REQUIRE_EQ(false, nvl.exists("test_nvlist"));
}

TEST_CASE(nvxx_add_nvlist_array)
{
	using namespace std::literals;
	auto constexpr key = "nvls"sv;

	auto nvls = std::vector<bsd::nv_list>();

	{
		auto nvl_ = bsd::nv_list();
		nvl_.add_number("one", 1);
		nvls.push_back(nvl_);
	}

	{
		auto nvl_ = bsd::nv_list();
		nvl_.add_number("two", 2);
		nvls.push_back(nvl_);
	}

	auto nvl = bsd::nv_list();
	nvl.add_nvlist_array(key, std::span(nvls));

	ATF_REQUIRE_EQ(true, nvl.exists(key));
	ATF_REQUIRE_EQ(true, nvl.exists_nvlist_array(key));

	auto nvls2 = nvl.get_nvlist_array(key);

	auto n1 = nvls2[0].get_number("one");
	ATF_REQUIRE_EQ(n1, 1);

	auto n2 = nvls2[1].get_number("two");
	ATF_REQUIRE_EQ(n2, 2);
}

TEST_CASE(nvxx_add_nvlist_array_error)
{
	using namespace std::literals;
	auto constexpr key = "nvls"sv;
	auto value = std::vector{bsd::nv_list{}, bsd::nv_list{}};

	auto nvl = bsd::nv_list();
	nvl.set_error(EINVAL);
	ATF_REQUIRE_THROW(bsd::nv_error_state,
			  nvl.add_nvlist_array(key, value));
}

TEST_CASE(nvxx_get_nonexistent_nvlist_array)
{
	auto nvl = bsd::nv_list();

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"nonesuch\" not found",
			     (void)nvl.get_nvlist_array("nonesuch"));
}

TEST_CASE(nvxx_add_duplicate_nvlist_array)
{
	using namespace std::literals;
	auto constexpr key = "test_nvlist"sv;
	auto value = std::vector<bsd::nv_list>(2);

	auto nvl = bsd::nv_list();
	nvl.add_nvlist_range(key, value);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_nvlist\" already exists",
			     nvl.add_nvlist_range(key, value));
}

/*
 * descriptor
 */

TEST_CASE(nvxx_add_descriptor)
{
	auto fds = std::array<int, 2>{};
	auto ret = ::pipe(&fds[0]);
	ATF_REQUIRE_EQ(0, ret);

	auto guard = std::unique_ptr<int[],
		decltype([](auto fds) {
			::close(fds[0]);
			::close(fds[1]); 
		})>(&fds[0]);

	auto nvl = bsd::nv_list();
	nvl.add_descriptor("test_descriptor", fds[0]);
	ATF_REQUIRE_EQ(true, nvl.exists("test_descriptor"));
	ATF_REQUIRE_EQ(true, nvl.exists_descriptor("test_descriptor"));

	auto fd = nvl.get_descriptor("test_descriptor");
	ret = ::write(fd, "1234", 4);
	ATF_REQUIRE_EQ(4, ret);

	auto buf = std::array<char, 4>{};
	ret = ::read(fds[1], &buf[0], 4);
	ATF_REQUIRE_EQ(4, ret);

	ATF_REQUIRE_EQ('1', buf[0]);
	ATF_REQUIRE_EQ('2', buf[1]);
	ATF_REQUIRE_EQ('3', buf[2]);
	ATF_REQUIRE_EQ('4', buf[3]);
}

TEST_CASE(nvxx_add_descriptor_error)
{
	using namespace std::literals;
	auto key = "test_descriptor"sv;

	auto nvl = bsd::nv_list{};
	nvl.set_error(EINVAL);
	ATF_REQUIRE_THROW(bsd::nv_error_state,
			  nvl.add_descriptor(key, 0));
}

TEST_CASE(nvxx_get_nonexistent_descriptor)
{
	auto nvl = bsd::nv_list();

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"nonesuch\" not found",
			     (void)nvl.get_descriptor("nonesuch"));
}

TEST_CASE(nvxx_add_duplicate_descriptor)
{
	using namespace std::literals;
	auto constexpr key = "test_descriptor"sv;

	auto fds = std::array<int, 2>{};
	auto ret = ::pipe(&fds[0]);
	ATF_REQUIRE_EQ(0, ret);

	auto guard = std::unique_ptr<int[],
		decltype([](auto fds) {
			::close(fds[0]);
			::close(fds[1]); 
		})>(&fds[0]);

	auto nvl = bsd::nv_list{};
	nvl.add_descriptor(key, fds[0]);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_descriptor\" already exists",
			     nvl.add_descriptor(key, fds[0]));
}

/*
 * binary
 */

TEST_CASE(nvxx_add_binary)
{
	auto nvl = bsd::nv_list();
	auto data = std::array<std::byte, 16>{};

	for (auto i = 0u; i < data.size(); ++i)
		data[i] = static_cast<std::byte>(i);

	nvl.add_binary("test_binary", std::span(data));

	ATF_REQUIRE_EQ(true, nvl.exists("test_binary"));
	ATF_REQUIRE_EQ(true, nvl.exists_binary("test_binary"));
	auto data2 = nvl.get_binary("test_binary");
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data2));
}

TEST_CASE(nvxx_add_binary_error)
{
	using namespace std::literals;
	auto constexpr key = "test_binary"sv;
	auto value = std::array<std::byte, 16>{};

	auto nvl = bsd::nv_list();
	nvl.set_error(EINVAL);
	ATF_REQUIRE_THROW(bsd::nv_error_state, nvl.add_binary(key, value));
}

TEST_CASE(nvxx_add_duplicate_binary)
{
	using namespace std::literals;
	auto constexpr key = "test_binary"sv;
	auto constexpr value = std::array<std::byte, 16>{};

	auto nvl = bsd::nv_list();
	nvl.add_binary(key, value);

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_binary\" already exists",
			     nvl.add_binary(key, value));
}

TEST_CASE(nvxx_get_nonexistent_binary)
{
	auto nvl = bsd::nv_list();

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"nonesuch\" not found",
			     (void)nvl.get_binary("nonesuch"));
}


TEST_CASE(nvxx_take_binary)
{
	using namespace std::literals;
	auto constexpr key = "test_binary"sv;
	auto nvl = bsd::nv_list();

	auto data = std::array<std::byte, 16>{};

	for (auto i = 0u; i < data.size(); ++i)
		data[i] = static_cast<std::byte>(i);

	nvl.add_binary(key, std::span(data));

	auto data2 = nvl.take_binary(key);
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data2));

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"test_binary\" not found",
			     (void)nvl.take_binary(key));
}

TEST_CASE(nvxx_add_binary_range)
{
	auto nvl = bsd::nv_list();
	auto data = std::array<std::byte, 16>{};

	for (auto i = 0u; i < data.size(); ++i)
		data[i] = static_cast<std::byte>(i);

	nvl.add_binary_range("test_binary", data);

	ATF_REQUIRE_EQ(true, nvl.exists("test_binary"));
	ATF_REQUIRE_EQ(true, nvl.exists_binary("test_binary"));
	auto data2 = nvl.get_binary("test_binary");
	ATF_REQUIRE_EQ(true, std::ranges::equal(data, data2));
}

ATF_INIT_TEST_CASES(tcs)
{
	ATF_ADD_TEST_CASE(tcs, nvxx_ctor_default);
	ATF_ADD_TEST_CASE(tcs, nvxx_ctor_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_const_ctor_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_ignore_case);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_null);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_null_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_duplicate_null);
	ATF_ADD_TEST_CASE(tcs, nvxx_free_null);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_bool);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_bool_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_duplicate_bool);
	ATF_ADD_TEST_CASE(tcs, nvxx_get_nonexistent_bool);
	ATF_ADD_TEST_CASE(tcs, nvxx_take_bool);
	ATF_ADD_TEST_CASE(tcs, nvxx_free_bool);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_bool_array);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_bool_array_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_duplicate_bool_array);
	ATF_ADD_TEST_CASE(tcs, nvxx_get_nonexistent_bool_array);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_bool_range);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_bool_contig_range);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_number);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_number_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_duplicate_number);
	ATF_ADD_TEST_CASE(tcs, nvxx_get_nonexistent_number);
	ATF_ADD_TEST_CASE(tcs, nvxx_take_number);
	ATF_ADD_TEST_CASE(tcs, nvxx_free_number);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_number_array);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_number_array_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_duplicate_number_array);
	ATF_ADD_TEST_CASE(tcs, nvxx_get_nonexistent_number_array);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_number_range);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_number_contig_range);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_string);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_string_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_duplicate_string);
	ATF_ADD_TEST_CASE(tcs, nvxx_get_nonexistent_string);
	ATF_ADD_TEST_CASE(tcs, nvxx_take_string);
	ATF_ADD_TEST_CASE(tcs, nvxx_free_string);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_string_array);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_string_array_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_duplicate_string_array);
	ATF_ADD_TEST_CASE(tcs, nvxx_get_nonexistent_string_array);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_string_range);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_string_contig_range);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_nvlist);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_nvlist_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_duplicate_nvlist);
	ATF_ADD_TEST_CASE(tcs, nvxx_get_nonexistent_nvlist);
	ATF_ADD_TEST_CASE(tcs, nvxx_take_nvlist);
	ATF_ADD_TEST_CASE(tcs, nvxx_free_nvlist);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_nvlist_array);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_nvlist_array_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_duplicate_nvlist_array);
	ATF_ADD_TEST_CASE(tcs, nvxx_get_nonexistent_nvlist_array);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_descriptor);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_descriptor_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_duplicate_descriptor);
	ATF_ADD_TEST_CASE(tcs, nvxx_get_nonexistent_descriptor);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_binary);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_binary_error);
	ATF_ADD_TEST_CASE(tcs, nvxx_add_duplicate_binary);
	ATF_ADD_TEST_CASE(tcs, nvxx_get_nonexistent_binary);
	ATF_ADD_TEST_CASE(tcs, nvxx_take_binary);

	ATF_ADD_TEST_CASE(tcs, nvxx_add_binary_range);
}
