# SPDX-License-Identifier Unlicense OR MIT
# Refer to the file 'LICENSE' in the nvxx distribution for license terms.

PREFIX?=		/usr/local
TESTSDIR?=		${PREFIX}/tests/nvxx
ATF_TESTS_CXX=		nvxx_basic nvxx_iterator nvxx_serialize
CXXSTD=			c++23
# Note that we can't use -Werror here because it breaks ATF.
CXXFLAGS+=		-W -Wall -Wextra
CFLAGS+=		-I${.CURDIR:H}
LDFLAGS+=		-lprivateatf-c++ -L${.OBJDIR:H} -lnvxx
LDFLAGS.nvxx_basic+=	-lnv

.include <bsd.test.mk>
