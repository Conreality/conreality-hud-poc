dnl configure.ac                                        -*- Autoconf -*-
dnl
dnl Process this file with `autoconf` to produce a configure script.
dnl
dnl This is free and unencumbered software released into the public domain.

dnl The build system requires Autoconf, Automake, GNU Make, and pkg-config.
dnl It does _not_ and _should_ not require GNU Libtool.
AC_PREREQ([2.68])

dnl Define version information:
m4_define([VERSION_MAJOR],
  m4_esyscmd([cut -d'.' -f1 VERSION | tr -d '\n']))
m4_define([VERSION_MINOR],
  m4_esyscmd([cut -d'.' -f2 VERSION | tr -d '\n']))
m4_define([VERSION_PATCH],
  m4_esyscmd([cut -d'.' -f3 VERSION | tr -d '\n']))
m4_define([VERSION_STRING],
  m4_esyscmd([git describe --dirty --always | tr -d '\n']))

dnl Define package information:
AC_INIT([Conreality HUD], [VERSION_STRING],
  [conreality@googlegroups.com], [conreality-hud],
  [https://github.com/conreality/conreality-hud])

dnl Configure Autoconf:
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([.aclocal])
AC_CONFIG_MACRO_DIR([.aclocal])
AC_CONFIG_LIBOBJ_DIR([lib])

dnl Configure Automake:
AM_INIT_AUTOMAKE([foreign -Wall -Werror tar-pax dist-xz no-dist-gzip subdir-objects nostdinc])
AM_SILENT_RULES([yes])

dnl Check for configuration options:
# --with-stdlib=libstdc++|libc++
AC_ARG_WITH([stdlib],
  [AS_HELP_STRING([--with-stdlib=LIB], [specify the C++ standard library to use [default=system]])],
  [], [with_stdlib=system])
AS_IF([test "x$with_stdlib" != "xsystem"],
  [CXXFLAGS="$CXXFLAGS -stdlib=$with_stdlib"
   LDFLAGS="$LDFLAGS -stdlib=$with_stdlib"])
# --enable-debug/--disable-debug
AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug], [instrument binaries with debugging support])],
  [], [enable_debug=no])
AS_IF([test "x$enable_debug" != "xno"],
  [AC_DEFINE([DEBUG], [1], [Enable debugging support.])],
  [AC_DEFINE([NDEBUG], [1], [Disable assertions.])])
AM_CONDITIONAL([ENABLE_DEBUG], [test "x$enable_debug" != "xno"])
# --enable-*/--disable-*
AC_DEFUN([AX_ARG_ENABLE_FEATURE], [dnl
  m4_define([FEATURE_ID], m4_translit([$1], [-a-z], [_A-Z]))
  AC_ARG_ENABLE([$1], [AS_HELP_STRING([--disable-$1], [disable $2])])
  AS_IF([test "x$enable_$1" == "xno"], [
    AC_DEFINE([DISABLE_]FEATURE_ID, 1, [Define to disable $2.])])
  AM_CONDITIONAL([DISABLE_]FEATURE_ID, [test "x$enable_$1" == "xno"])
])
AX_ARG_ENABLE_FEATURE([darknet], [Darknet support]) # --enable-darknet/--disable-darknet
AX_ARG_ENABLE_FEATURE([openhmd], [OpenHMD support]) # --enable-openhmd/--disable-openhmd
AX_ARG_ENABLE_FEATURE([openvr],  [OpenVR support])  # --enable-openvr/--disable-openvr
AX_ARG_ENABLE_FEATURE([osvr],    [OSVR support])    # --enable-osvr/--disable-osvr

dnl Check for programs:
AC_PROG_CXX(clang++ g++ c++)
AC_PROG_CXXCPP
AX_CXX_COMPILE_STDCXX([14], [noext])
PKG_PROG_PKG_CONFIG([0.29])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_CHECK_PROGS([PANDOC], [pandoc])
AC_LANG([C++])

dnl Check for libraries:
PKG_CHECK_MODULES([OPENCV], [opencv >= 3],
  [AC_DEFINE([HAVE_OPENCV], [1], [Define to 1 if you have OpenCV 3+.])])
AM_CONDITIONAL([HAVE_OPENCV], [test "x$OPENCV_LIBS" != "x"])

PKG_CHECK_MODULES([GLEW], [glew],
  [AC_DEFINE([HAVE_GLEW], [1], [Define to 1 if you have GLEW (OpenGL Extension Wrangler Library).])])
AM_CONDITIONAL([HAVE_GLEW], [test "x$GLEW_LIBS" != "x"])

PKG_CHECK_MODULES([GLFW], [glfw3],
  [AC_DEFINE([HAVE_GLFW], [1], [Define to 1 if you have GLFW3.])])
AM_CONDITIONAL([HAVE_GLFW], [test "x$GLFW_LIBS" != "x"])

PKG_CHECK_MODULES([LIBDRM], [libdrm],
  [AC_DEFINE([HAVE_LIBDRM], [1], [Define to 1 if you have libdrm.])])
AM_CONDITIONAL([HAVE_LIBDRM], [test "x$LIBDRM_LIBS" != "x"])

PKG_CHECK_MODULES([JSONCPP], [jsoncpp >= 1.7],
  [AC_DEFINE([HAVE_JSONCPP], [1], [Define to 1 if you have JsonCpp.])])
AM_CONDITIONAL([HAVE_JSONCPP], [test "x$JSONCPP_LIBS" != "x"])

PKG_CHECK_MODULES([TBB], [tbb],
  [AC_DEFINE([HAVE_TBB], [1], [Define to 1 if you have Intel TBB (Threading Building Blocks).])])
AM_CONDITIONAL([HAVE_TBB], [test "x$TBB_LIBS" != "x"])

dnl Check for header files:

dnl Check for types:

dnl Check for structures:

dnl Check for compiler characteristics:
AC_CANONICAL_HOST
AS_CASE([$host_cpu],
  [amd64],    [host_cpu="x86_64"],
  [armv6l],   [host_cpu="arm"],
  [armv7l],   [host_cpu="arm"],
  [i?86],     [host_cpu="x86"],
  [x86],      [host_cpu="x86"],
  [x86_64],   [host_cpu="x86_64"],
  [AC_MSG_ERROR([unsupported CPU architecture: $host_cpu])])
AS_CASE([$host_os],
  [darwin*],  [host_type="posix" host_os="darwin"],
  [freebsd*], [host_type="posix" host_os="bsd"],
  [linux*],   [host_type="posix" host_os="linux"],
  [netbsd*],  [host_type="posix" host_os="bsd"],
  [openbsd*], [host_type="posix" host_os="bsd"],
  [AC_MSG_ERROR([unsupported OS platform: $host_os])])
AM_CPPFLAGS="$AM_CPPFLAGS -I\$(top_srcdir)/src"
AM_CXXFLAGS="$AM_CXXFLAGS -Wall -Wextra -pipe"
AM_LDFLAGS="$AM_LDFLAGS"
AS_CASE([$host_os], [linux*], [AM_LDFLAGS="$AM_LDFLAGS -pthread"], [])
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

dnl Check for library functions:

dnl Check for system services:

dnl Generate output:
AC_CONFIG_FILES([Makefile src/Makefile test/Makefile])
AC_SUBST([PACKAGE_VERSION_MAJOR], ["VERSION_MAJOR"])
AC_SUBST([PACKAGE_VERSION_MINOR], ["VERSION_MINOR"])
AC_SUBST([PACKAGE_VERSION_PATCH], ["VERSION_PATCH"])
AC_CONFIG_FILES([src/version.h])
AC_OUTPUT
