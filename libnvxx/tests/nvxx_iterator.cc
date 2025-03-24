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

TEST_CASE(nvxx_basic_iterate)
{
	using namespace std::literals;
	auto nvl = bsd::nv_list();

	nvl.add_number("a number", 42);
	nvl.add_string("a string", "a test string");
	nvl.add_bool("a bool", true);

	auto begin = std::ranges::begin(nvl);
	auto end = std::ranges::end(nvl);

	auto i = 0u;
	while (begin != end) {
		auto name = begin->first;
		auto const &value = begin->second;

		if (std::holds_alternative<std::uint64_t>(value)) {
			ATF_REQUIRE_EQ("a number"sv, name);
			ATF_REQUIRE_EQ(42, std::get<std::uint64_t>(value));
		} else if (std::holds_alternative<std::string_view>(value)) {
			ATF_REQUIRE_EQ("a string"sv, name);
			ATF_REQUIRE_EQ("a test string",
				       std::get<std::string_view>(value));
		} else if (std::holds_alternative<bool>(value)) {
			ATF_REQUIRE_EQ("a bool"sv, name);
			ATF_REQUIRE_EQ(true, std::get<bool>(value));
		} else
			ATF_REQUIRE_EQ(true, false);

		++i;
		++begin;
	}
	ATF_REQUIRE_EQ(3, i);
}

ATF_INIT_TEST_CASES(tcs)
{
	ATF_ADD_TEST_CASE(tcs, nvxx_basic_iterate);
}
