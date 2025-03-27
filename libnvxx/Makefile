# SPDX-License-Identifier: Unlicense OR MIT
# Refer to the file 'LICENSE' in the nvxx distribution for license terms.

.include <bsd.opts.mk>

PREFIX?=	/usr/local
LIB_CXX=	nvxx
LIBDIR=		${PREFIX}/lib
INCLUDEDIR=	${PREFIX}/include
SHLIB_MAJOR=	1
INCS=		nvxx.h			\
		nvxx_base.h		\
		nvxx_util.h		\
		nvxx_iterator.h		\
		nvxx_serialize.h
SRCS=		nvxx.cc			\
		nv_list.cc		\
		const_nv_list.cc	\
		nvxx_iterator.cc	\
		nvxx_serialize.cc
CXXSTD=		c++23
CXXFLAGS+=	-W -Wall -Wextra -Werror
LDADD=		-lnv

HAS_TESTS=
SUBDIR.${MK_TESTS}+= tests

.include <bsd.lib.mk>
