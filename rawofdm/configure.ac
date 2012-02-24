AC_INIT
AC_PREREQ(2.57)
AC_CONFIG_AUX_DIR([.])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

GR_VERSION
dnl ustar required to have pathnames > 99 chars
_AM_SET_OPTION([tar-ustar])
AM_INIT_AUTOMAKE(raw-ofdm,$RELEASE)

dnl This is kind of non-standard, but it sure shortens up this file :-)
m4_include([config/gr_standalone.m4])
GR_STANDALONE

dnl Check for any libraries you need
dnl AC_CHECK_LIBRARY

dnl Check for header files you need
dnl AC_CHECK_HEADERS(fcntl.h limits.h strings.h sys/ioctl.h sys/time.h unistd.h)
dnl AC_CHECK_HEADERS(sys/mman.h)

dnl Checks for library functions.
dnl AC_CHECK_FUNCS([])

dnl We pick up the boost cppflags, cxxflags and thread lib via GNURADIO_CORE
dnl
dnl If you need additional boost libraries, you'll need to
dnl uncomment AX_BOOST_BASE, plus some of the following:
dnl
dnl calls AC_SUBST(BOOST_CPPFLAGS), AC_SUBST(BOOST_LDFLAGS) and defines HAVE_BOOST
dnl AX_BOOST_BASE([1.35])
dnl
dnl All the rest of these call AC_SUBST(BOOST_<foo>_LIB) and define HAVE_BOOST_<foo>
dnl
dnl AX_BOOST_DATE_TIME
dnl AX_BOOST_FILESYSTEM
dnl AX_BOOST_IOSTREAMS
dnl AX_BOOST_PROGRAM_OPTIONS
dnl AX_BOOST_REGEX
dnl AX_BOOST_SERIALIZATION
dnl AX_BOOST_SIGNALS
dnl AX_BOOST_SYSTEM
dnl AX_BOOST_TEST_EXEC_MONITOR
dnl AX_BOOST_UNIT_TEST_FRAMEWORK
dnl AX_BOOST_WSERIALIZATION

AC_CONFIG_FILES([\
    Makefile \
    config/Makefile \
    src/Makefile \
    src/lib/Makefile \
    src/lib/ofdm/Makefile \
    src/lib/qam/Makefile \
    src/lib/util/Makefile \
    src/lib/rscode/Makefile \
    src/lib/spiral/Makefile \
    src/python/Makefile \
    src/python/run_tests \
])

dnl run_tests is created from run_tests.in.  Make it executable.
AC_CONFIG_COMMANDS([run_tests], [chmod +x src/python/run_tests])

AC_OUTPUT
