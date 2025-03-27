/*
 * SPDX-License-Identifier: Unlicense OR MIT
 * Refer to the file 'LICENSE' in the nvxx distribution for license terms.
 */

#include "nvxx.h"

namespace bsd {

/*
 * type encoders/decoders
 */

/* bool */

void
nv_encoder<bool>::encode(nv_list &nvl, std::string_view key, bool value)
{
	nvl.add_bool(key, value);
}

bool
nv_decoder<bool>::decode(const_nv_list const &nvl, std::string_view key)
{
	return (nvl.get_bool(key));
}

/* uint64_t */

void
nv_encoder<std::uint64_t>::encode(nv_list &nvl,
				  std::string_view key,
				  std::uint64_t value)
{
	nvl.add_number(key, value);
}

std::uint64_t
nv_decoder<std::uint64_t>::decode(const_nv_list const &nvl,
				  std::string_view key)
{
	return (nvl.get_number(key));
}

/* string */

void
nv_encoder<std::string>::encode(nv_list &nvl,
				std::string_view key,
				std::string const &value)
{
	nvl.add_string(key, value);
}

std::string
nv_decoder<std::string>::decode(const_nv_list const &nvl,
				std::string_view key)
{
	return (std::string(nvl.get_string(key)));
}

/* string_view */

void
nv_encoder<std::string_view>::encode(nv_list &nvl,
				     std::string_view key,
				     std::string_view value)
{
	nvl.add_string(key, value);
}

std::string_view
nv_decoder<std::string_view>::decode(const_nv_list const &nvl,
				     std::string_view key)
{
	return (nvl.get_string(key));
}

/* nv_list */

void
nv_encoder<nv_list>::encode(nv_list &nvl,
			    std::string_view key,
			    nv_list const &value)
{
	nvl.add_nvlist(key, value);
}

nv_list
nv_decoder<nv_list>::decode(const_nv_list const &nvl,
			    std::string_view key)
{
	return (nv_list(nvl.get_nvlist(key)));
}

/* const_nv_list */

void
nv_encoder<const_nv_list>::encode(nv_list &nvl,
				  std::string_view key,
				  const_nv_list const &value)
{
	nvl.add_nvlist(key, value);
}

const_nv_list
nv_decoder<const_nv_list>::decode(const_nv_list const &nvl,
				  std::string_view key)
{
	return (nvl.get_nvlist(key));
}

} // namespace bsd
