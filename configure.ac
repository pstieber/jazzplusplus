dnl Process this file with autoconf to produce a configure script.
AC_INIT([jazzplusplus], [5.0.0], pstieber@verizon.net, [Jazz++])

AC_CONFIG_MACRO_DIR([m4])

dnl Change the location of the auxiliary configure files to config.
AC_CONFIG_AUX_DIR([config])

AC_CONFIG_SRCDIR([src/JazzPlusPlusApplication.cpp])

dnl sets build, host, target variables and the same with _alias
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS(src/config.h)

dnl Checks for programs.

AC_PROG_CPP

dnl C-compiler checks
dnl   defines CC with the compiler to use
dnl   defines GCC with yes if using gcc
dnl   defines GCC empty if not using gcc
dnl   defines CFLAGS
dnl
dnl this magic incantation is needed to prevent AC_PROG_CC from setting the
dnl default CFLAGS (something like "-g -O2") -- we don't need this as we add
dnl -g and -O flags ourselves below
CFLAGS=${CFLAGS:=}
AC_PROG_CC

dnl C++-compiler checks
dnl   defines CXX with the compiler to use
dnl   defines GXX with yes if using gxx
dnl   defines GXX empty if not using gxx
dnl   defines CXXFLAGS
dnl
dnl see CFLAGS line above
CXXFLAGS=${CXXFLAGS:=}
AC_PROG_CXX

AC_PROG_LIBTOOL

dnl Make autoconf use C++ for its tests.
AC_LANG(C++)

dnl Checks for libraries.
     
dnl Checks for header files.
AC_STDC_HEADERS
AC_HAVE_HEADERS(string.h fcntl.h sys/file.h sys/param.h)

dnl Checks for library functions.
AC_FUNC_ALLOCA

dnl Check for st_blksize in struct stat
AC_ST_BLKSIZE

dnl Check to see if we are using _Ios_Openmode.  gcc 3.2 doesn't like using
dnl ints and std::_Ios_Openmode interchangeably.  There's probably a simpler
dnl autoconf technique for checking this feature.
AC_MSG_CHECKING(for _Ios_Openmode)
AC_COMPILE_IFELSE([AC_LANG_SOURCE(
[[#include <iostream>
std::_Ios_Openmode test;]])],
result=yes,
result=no)
if test $result = yes; then
  AC_DEFINE(HAVE_IOS_OPENMODE,1,[This symbol should be defined for gcc 3.2 so that we use std::_Ios_Openmode instead of int])
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING([whether to enable debugging])
AC_ARG_ENABLE(
  debug,
  AC_HELP_STRING(
    [--enable-debug],
    [enable debugging code (off by default)]),
  ac_arg_debug="$enableval",
  ac_arg_debug="no")
AC_MSG_RESULT(${enable_debug-no})

AC_MSG_CHECKING([whether to enable optimization])
AC_ARG_ENABLE(
  optimize,
  AC_HELP_STRING(
    [--enable-optimize@<:@=ARG@:>@],
    [enable optimized code with ARG = 0, 1, 2, or 3 (off by default ARG = 0)]),
  [opt_level="$enableval" ac_arg_optimize="yes"],
  ac_arg_optimize="no")
AC_MSG_RESULT(${enable_optimize-no})

AC_MSG_CHECKING([whether to enable profiling])
AC_ARG_ENABLE(
  profile,
  AC_HELP_STRING(
    [--enable-profile],
    [enable gprof profiling code (off by default)]),
  ac_arg_profile="$enableval",
  ac_arg_profile="no")
AC_MSG_RESULT(${enable_profile-no})

dnl To support a new system, you need to add its canonical name (as determined
dnl by config.sub or specified by the configure command line) to this "case"
dnl and also define the shared library flags below - search for
dnl SHARED_LIB_SETUP to find the exact place.
case "${host}" in
  *-darwin* )
  ;;
  *-*-linux* )
    AC_CHECK_LIB(asound, snd_pcm_open, have_alsa=yes, have_alsa=no)
    if [[ x$have_alsa = x"yes" ]] ; then
      LIBS="$LIBS -lasound"
    fi
  ;;
  *)
    AC_MSG_ERROR(unknown system type ${host}.)
esac

AM_CONDITIONAL(USE_ALSA, test "$have_alsa" = yes)

if [[ x$have_alsa = x"yes" ]] ; then

  dnl ---------------------------------------
  dnl check if we are to enable alsa support
  dnl ---------------------------------------
  AC_MSG_CHECKING([whether to enable ALSA])
  AC_ARG_ENABLE(alsa, AC_HELP_STRING([--enable-alsa], [enable ALSA support]))
  AC_MSG_RESULT(${enable_alsa-no})

  if test x$enable_alsa = xyes ; then
    AC_DEFINE(DEV_ALSA, 1, [use alsa drivers])
  fi
fi

dnl --------------------------------------------
dnl check if we are to enable sequencer2 support
dnl --------------------------------------------
AC_MSG_CHECKING([whether to enable sequencer2])
AC_ARG_ENABLE(
  sequencer2,
  AC_HELP_STRING(
    [--enable-sequencer2],
    [enable sequencer2 support (off by default)]))
AC_MSG_RESULT(${enable_sequencer2-no})

if test x$enable_sequencer2 = xyes ; then
   AC_DEFINE(DEV_SEQUENCER2, 1, [support /dev/sequencer2])
fi

dnl ----------------------------------------
dnl check if we are to enable mpu401 support
dnl ----------------------------------------
AC_MSG_CHECKING([whether to enable Jazz++ mpu401 driver])
AC_ARG_ENABLE(
  mpu401,
  AC_HELP_STRING([--enable-mpu401], [enable support jazz mpu401 driver]))
AC_MSG_RESULT(${enable_mpu401-no})

if test x$enable_mpu401 = xyes ; then
  AC_MSG_RESULT(Mpu401 is enabled)
  AC_DEFINE(DEV_MPU401, 1, [support jazz's own driver over tcp/ip])
fi

dnl Check for wxWidgets
AM_OPTIONS_WXCONFIG
MIN_WX_VERSION="2.9.4"
AM_PATH_WXCONFIG(
  $MIN_WX_VERSION,
  [WX_FOUND=1],
  [WXFOUND=0],
  [std],
  [--debug=$ac_arg_debug])

if test "$WX_FOUND" != 1; then
  AC_MSG_ERROR([
    wxWidgets must be installed on your system, but the
    wx-config script couldn't be found.

    Please check that wx-config is in your PATH, the
    directory where wxWidgets libraries are installed
    (returned by 'wx-config --libs' command) is in
    LD_LIBRARY_PATH or equivalent variable, and the
    wxWidgets version is $MIN_VERSION or above.
  ])
fi

JAZZ_PATH=\${top_srcdir}
AC_SUBST(JAZZ_PATH)

dnl PORTMIDI_PATH=$JAZZ_PATH/portmidi
dnl AC_SUBST(PORTMIDI_PATH)

TLDEBUG=
if test x$ac_arg_debug = xyes ; then
  TLDEBUG="-g -fno-inline"
else
  TLDEBUG="-DNDEBUG"
fi

PROFILE=
if test x$ac_arg_profile = xyes ; then
  dnl The debug flag must be on for profiling to work.
  if test x$TLDEBUG = x ; then
    TLDEBUG="-g -fno-inline"
  fi
  PROFILE="-pg"
fi

OPTIMISE=-O0
if test x$ac_arg_optimize = xyes ; then
  if test x$opt_level = x; then
    OPTIMISE="-O3"
  elif test x$opt_level = x0 ; then
    OPTIMISE="-O1"
  elif test x$opt_level = x1 ; then
    OPTIMISE="-O1"
  elif test x$opt_level = x2 ; then
    OPTIMISE="-O2"
  elif test x$opt_level = x3 ; then
    OPTIMISE="-O3"
  fi
fi

EXTRA_CFLAGS="$TLDEBUG $PROFILE $OPTIMISE"

BITMAPS_PATH="$JAZZ_PATH/Bitmaps"
AC_SUBST(BITMAPS_PATH)
BITMAPS_FLAGS="-I$BITMAPS_PATH"

# CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CPPFLAGS=`echo -Wall -Wno-unknown-pragmas $BITMAPS_FLAGS $WX_CPPFLAGS | sed 's/ \+/ /g'`

CFLAGS=`echo $EXTRA_CFLAGS $CFLAGS $WX_CFLAGS_ONLY | sed 's/ \\+/ /g'`

CXXFLAGS=`echo $EXTRA_CFLAGS $CXXFLAGS $WX_CXXFLAGS_ONLY | sed 's/ \+/ /g'`

LDFLAGS="$LDFLAGS $WX_LIBS"

AC_CONFIG_FILES([\
Makefile src/Makefile \
src/HelpFiles/Makefile \
src/HelpFiles/images/Makefile \
conf/Makefile \
Info.plist
])

AC_OUTPUT
