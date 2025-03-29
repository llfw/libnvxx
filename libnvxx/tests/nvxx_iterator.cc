/*
 * SPDX-License-Identifier: Unlicense OR MIT
 * Refer to the file 'LICENSE' in the nvxx distribution for license terms.
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

TEST_CASE(nvxx_basic_iterate)
{
	using namespace std::literals;
	auto nvl = bsd::nv_list();
	auto fds = std::array<int, 2>{};
	auto fds2 = std::array<int, 2>{};

	auto binary = std::array<std::byte, 4>{
		static_cast<std::byte>(1),
		static_cast<std::byte>(2),
		static_cast<std::byte>(3),
		static_cast<std::byte>(4)
	};

	auto ret = ::pipe(&fds[0]);
	ATF_REQUIRE_EQ(0, ret);
	ret = ::pipe(&fds2[0]);
	ATF_REQUIRE_EQ(0, ret);

	auto bool_array = std::vector{true, false, false};
	auto number_array = std::vector<std::uint64_t>{2, 3, 4};
	auto string_array = std::vector{"one"sv, "two"sv, "three"sv};

	nvl.add_null("a null");
	nvl.add_number("a number", 42);
	nvl.add_string("a string", "a test string");
	nvl.add_bool("a bool", true);
	nvl.add_binary("a binary", binary);
	nvl.add_bool_range("a bool array", bool_array);
	nvl.add_number_range("a number array", number_array);
	nvl.add_string_range("a string array", string_array);

	auto fdesc = fds[0];
	nvl.move_descriptor("an fd", bsd::nv_fd(fdesc));

	nvl.add_descriptor_range("a descriptor array", fds2);

	auto nvl2 = bsd::nv_list();
	nvl2.add_number("child number", 666);
	nvl.add_nvlist("an nvlist", nvl2);

	auto nvl3 = bsd::nv_list();
	nvl3.add_number("an array number", 4242);
	nvl.add_nvlist_array("an nvlist array", std::span{&nvl3, 1});

	auto i = 0u;
	for (auto &[name, value] : nvl) {
		if (std::holds_alternative<std::uint64_t>(value)) {
			ATF_REQUIRE_EQ("a number"sv, name);
			ATF_REQUIRE_EQ(42, std::get<std::uint64_t>(value));

		} else if (std::holds_alternative<nullptr_t>(value)) {
			ATF_REQUIRE_EQ("a null"sv, name);
			ATF_REQUIRE_EQ(nullptr,
				       std::get<nullptr_t>(value));

		} else if (std::holds_alternative<std::string_view>(value)) {
			ATF_REQUIRE_EQ("a string"sv, name);
			ATF_REQUIRE_EQ("a test string",
				       std::get<std::string_view>(value));

		} else if (std::holds_alternative<bool>(value)) {
			ATF_REQUIRE_EQ("a bool"sv, name);
			ATF_REQUIRE_EQ(true, std::get<bool>(value));

		} else if (std::holds_alternative<int>(value)) {
			ATF_REQUIRE_EQ("an fd"sv, name);
			ATF_REQUIRE_EQ(fdesc, std::get<int>(value));

		} else if (std::holds_alternative<
			   	std::span<std::byte const>>(value)) {
			ATF_REQUIRE_EQ("a binary"sv, name);
			auto data = std::get<std::span<std::byte const>>(value);
			ATF_REQUIRE_EQ(true, std::ranges::equal(binary, data));

		} else if (std::holds_alternative<bsd::const_nv_list>(value)) {
			ATF_REQUIRE_EQ("an nvlist"sv, name);
			ATF_REQUIRE_EQ(666,
				       std::get<bsd::const_nv_list>(value)
				       		.get_number("child number"));

		} else if (std::holds_alternative<
				   std::span<bool const>>(value)) {
			ATF_REQUIRE_EQ("a bool array"sv, name);
			auto data = std::get<std::span<bool const>>(value);
			ATF_REQUIRE_EQ(true, 
				       std::ranges::equal(bool_array, data));

		} else if (std::holds_alternative<
				   std::span<std::uint64_t const>>(value)) {
			ATF_REQUIRE_EQ("a number array"sv, name);
			auto data = std::get<std::span<
					std::uint64_t const>>(value);
			ATF_REQUIRE_EQ(true, 
				       std::ranges::equal(number_array, data));

		} else if (std::holds_alternative<
				std::vector<
					std::string_view>>(value)) {
			ATF_REQUIRE_EQ("a string array"sv, name);
			auto data = std::get<std::vector<
						std::string_view>>(value);
			ATF_REQUIRE_EQ(true, 
				       std::ranges::equal(string_array, data));

		} else if (std::holds_alternative<
				   std::span<int const>>(value)) {
			ATF_REQUIRE_EQ("a descriptor array"sv, name);
			auto data = std::get<std::span<int const>>(value);
			ATF_REQUIRE_EQ(2, std::ranges::size(data));
			// XXX: should test we get the actual descriptors

		} else if (std::holds_alternative<
			   	std::vector<bsd::const_nv_list>>(value)) {
			ATF_REQUIRE_EQ("an nvlist array"sv, name);
			auto data = std::get<std::vector<
					bsd::const_nv_list>>(value);
			ATF_REQUIRE_EQ(1, std::ranges::size(data));
			auto n = data[0].get_number("an array number");
			ATF_REQUIRE_EQ(4242, n);

		} else
			ATF_REQUIRE_EQ(true, false);

		++i;
	}

	ATF_REQUIRE_EQ(12, i);
}

ATF_INIT_TEST_CASES(tcs)
{
	ATF_ADD_TEST_CASE(tcs, nvxx_basic_iterate);
}
