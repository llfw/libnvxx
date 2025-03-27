/*
 * SPDX-License-Identifier Unlicense OR MIT
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

using namespace bsd;

/*
 * bool
 */

TEST_CASE(nv_encoder_bool)
{
	auto v = true;
	auto nvl = nv_list{};

	nv_encoder<bool>{}.encode(nvl, "test", v);

	auto v2 = nv_decoder<bool>{}.decode(nvl, "test");
	ATF_REQUIRE_EQ(v, v2);
}

TEST_CASE(nv_encoder_bool_vector)
{
	auto v = std::vector<bool>{true, false, false, true};
	auto nvl = nv_list{};

	nv_encoder<std::vector<bool>>{}.encode(nvl, "test", v);

	auto v2 = nv_decoder<std::vector<bool>>{}.decode(nvl, "test");
	ATF_REQUIRE_EQ(true, std::ranges::equal(v, v2));
}

/*
 * std::uint64_t
 */

TEST_CASE(nv_encoder_uint64)
{
	auto v = std::uint64_t{42};
	auto nvl = nv_list{};

	nv_encoder<std::uint64_t>{}.encode(nvl, "test", v);

	auto v2 = nv_decoder<std::uint64_t>{}.decode(nvl, "test");
	ATF_REQUIRE_EQ(v, v2);
}

TEST_CASE(nv_encoder_uint64_vector)
{
	auto v = std::vector<std::uint64_t>{1, 2, 42, 666};
	auto nvl = nv_list{};

	nv_encoder<std::vector<std::uint64_t>>{}.encode(nvl, "test", v);

	auto v2 = nv_decoder<std::vector<std::uint64_t>>{}.decode(nvl, "test");
	ATF_REQUIRE_EQ(true, std::ranges::equal(v, v2));
}

/*
 * string
 */

TEST_CASE(nv_encoder_string)
{
	auto v = std::string("testing");
	auto nvl = nv_list{};

	nv_encoder<std::string>{}.encode(nvl, "test", v);

	auto v2 = nv_decoder<std::string>{}.decode(nvl, "test");
	ATF_REQUIRE_EQ(v, v2);
}

TEST_CASE(nv_encoder_string_vector)
{
	auto v = std::vector<std::string>{"foo", "bar", "baz", "quux"};
	auto nvl = nv_list{};

	nv_encoder<std::vector<std::string>>{}.encode(nvl, "test", v);

	auto v2 = nv_decoder<std::vector<std::string>>{}.decode(nvl, "test");
	ATF_REQUIRE_EQ(true, std::ranges::equal(v, v2));
}

/*
 * string_view
 */

TEST_CASE(nv_encoder_string_view)
{
	auto v = std::string_view("testing");
	auto nvl = nv_list{};

	nv_encoder<std::string_view>{}.encode(nvl, "test", v);
	auto v2 = nv_decoder<std::string_view>{}.decode(nvl, "test");
	ATF_REQUIRE_EQ(v, v2);
}

TEST_CASE(nv_encoder_string_view_vector)
{
	auto v = std::vector<std::string_view>{"foo", "bar", "baz", "quux"};
	auto nvl = nv_list{};

	nv_encoder<std::vector<std::string_view>>{}.encode(nvl, "test", v);

	auto v2 = nv_decoder<std::vector<std::string_view>>{}
			.decode(nvl, "test");
	ATF_REQUIRE_EQ(true, std::ranges::equal(v, v2));
}

/*
 * nv_list
 */

TEST_CASE(nv_encoder_nv_list)
{
	auto v = nv_list{};
	v.add_number("int", 42);
	auto nvl = nv_list{};

	nv_encoder<nv_list>{}.encode(nvl, "test", v);
	auto v2 = nv_decoder<nv_list>{}.decode(nvl, "test");
	ATF_REQUIRE_EQ(v.get_number("int"), v2.get_number("int"));
}

/*
 * const_nv_list
 */

TEST_CASE(nv_encoder_const_nv_list)
{
	auto v = nv_list{};
	v.add_number("int", 42);
	auto nvl = nv_list{};

	nv_encoder<const_nv_list>{}.encode(nvl, "test", const_nv_list(v));
	auto v2 = nv_decoder<const_nv_list>{}.decode(nvl, "test");
	ATF_REQUIRE_EQ(v.get_number("int"), v2.get_number("int"));
}

/*
 * std::optional<>
 */

TEST_CASE(nv_encoder_optional)
{
	auto v = std::optional<std::uint64_t>{42};
	auto nvl = nv_list{};

	nv_encoder<std::optional<std::uint64_t>>{}.encode(nvl, "test", v);

	auto v2 = nv_decoder<
		std::optional<std::uint64_t>>{}.decode(nvl, "test");
	ATF_REQUIRE_EQ(true, v2.has_value());
	ATF_REQUIRE_EQ(*v, *v2);

	v2 = nv_decoder<std::optional<std::uint64_t>>{}.decode(nvl, "nonesuch");
	ATF_REQUIRE_EQ(false, v2.has_value());
}

/*
 * nv_(de)serialize()
 */

struct object {
	std::uint64_t int_value{};
	std::string string_value{};
	std::vector<std::uint64_t> array_value;
};

template<>
struct bsd::nv_schema<::object> {
	auto get() {
		return	bsd::nv_field("int value", &object::int_value)
			>> bsd::nv_field("string value", &object::string_value)
			>> bsd::nv_field("array value", &object::array_value);
	}
};

TEST_CASE(nv_serialize) 
{
	auto obj = object{42, "quux", {42, 666, 1024}};
	auto nvl = bsd::nv_serialize(obj);

	auto obj2 = object{};
	bsd::nv_deserialize(nvl, obj2);
	ATF_REQUIRE_EQ(42, obj2.int_value);
	ATF_REQUIRE_EQ("quux", obj2.string_value);
	ATF_REQUIRE_EQ(true, std::ranges::equal(obj2.array_value,
						std::vector{42, 666, 1024}));
}

ATF_INIT_TEST_CASES(tcs)
{
	ATF_ADD_TEST_CASE(tcs, nv_encoder_bool);
	ATF_ADD_TEST_CASE(tcs, nv_encoder_bool_vector);
	ATF_ADD_TEST_CASE(tcs, nv_encoder_uint64);
	ATF_ADD_TEST_CASE(tcs, nv_encoder_uint64_vector);
	ATF_ADD_TEST_CASE(tcs, nv_encoder_string);
	ATF_ADD_TEST_CASE(tcs, nv_encoder_string_vector);
	ATF_ADD_TEST_CASE(tcs, nv_encoder_string_view);
	ATF_ADD_TEST_CASE(tcs, nv_encoder_string_view_vector);
	ATF_ADD_TEST_CASE(tcs, nv_encoder_nv_list);
	//ATF_ADD_TEST_CASE(tcs, nv_encoder_nv_list_vector);
	ATF_ADD_TEST_CASE(tcs, nv_encoder_const_nv_list);
	//ATF_ADD_TEST_CASE(tcs, nv_encoder_const_nv_list_vector);
	ATF_ADD_TEST_CASE(tcs, nv_encoder_optional);

	ATF_ADD_TEST_CASE(tcs, nv_serialize);
}
