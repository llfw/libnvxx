/*
 * SPDX-License-Identifier: Unlicense OR MIT
 * Refer to the file 'LICENSE' in the nvxx distribution for license terms.
 */

#include <atf-c++.hpp>

#include "nvxx.h"

#define TEST_CASE(name)				\
	ATF_TEST_CASE_WITHOUT_HEAD(name)	\
	ATF_TEST_CASE_BODY(name)

TEST_CASE(nv_error)
{
	auto do_throw = []{
		throw bsd::nv_error("{} + {} = {}", 1, 1, 2);
	};

	ATF_REQUIRE_THROW_RE(bsd::nv_error, "1 \\+ 1 = 2", do_throw());
}

TEST_CASE(nv_error_state)
{
	auto errv = std::make_error_code(std::errc::invalid_argument);

	auto do_throw = [=]{
		throw bsd::nv_error_state(errv);
	};

	ATF_REQUIRE_THROW(bsd::nv_error_state, do_throw());

	try {
		do_throw();
	} catch (bsd::nv_error_state const &exc) {
		ATF_REQUIRE_EQ(true, (errv == exc.error));
	}
}

TEST_CASE(nv_key_not_found)
{
	using namespace std::literals;
	auto constexpr key = "test_key"s;

	auto do_throw = [=]{
		throw bsd::nv_key_not_found(key);
	};

	ATF_REQUIRE_THROW_RE(bsd::nv_key_not_found,
			     "key \"test_key\" not found",
			     do_throw());
}

TEST_CASE(nv_key_exists)
{
	using namespace std::literals;
	auto constexpr key = "test_key"s;

	auto do_throw = [=]{
		throw bsd::nv_key_exists(key);
	};

	ATF_REQUIRE_THROW_RE(bsd::nv_key_exists,
			     "key \"test_key\" already exists",
			     do_throw());
}

ATF_INIT_TEST_CASES(tcs)
{
	ATF_ADD_TEST_CASE(tcs, nv_error);
	ATF_ADD_TEST_CASE(tcs, nv_error_state);
	ATF_ADD_TEST_CASE(tcs, nv_key_not_found);
	ATF_ADD_TEST_CASE(tcs, nv_key_exists);
}
