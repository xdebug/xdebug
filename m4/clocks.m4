AC_DEFUN([AC_XDEBUG_CLOCK],
[
  have_clock_gettime=no

  AC_MSG_CHECKING([for clock_gettime])

  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <time.h>]], [[struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);]])], [
    have_clock_gettime=yes
    AC_MSG_RESULT([yes])
  ], [
    AC_MSG_RESULT([no])
  ])

  if test "$have_clock_gettime" = "no"; then
    AC_MSG_CHECKING([for clock_gettime in -lrt])

    SAVED_LIBS="$LIBS"
    LIBS="$LIBS -lrt"

    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <time.h>]], [[struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);]])], [
      have_clock_gettime=yes
      AC_MSG_RESULT([yes])
    ], [
      LIBS="$SAVED_LIBS"
      AC_MSG_RESULT([no])
    ])
  fi

  if test "$have_clock_gettime" = "yes"; then
    AC_DEFINE([HAVE_XDEBUG_CLOCK_GETTIME], 1, [do we have clock_gettime?])
  fi
  
  AC_CHECK_FUNC(clock_gettime_nsec_np,
    [AC_DEFINE([HAVE_XDEBUG_CLOCK_GETTIME_NSEC_NP], 1, [do we have clock_gettime_nsec_np?])],
	[AC_DEFINE([HAVE_XDEBUG_CLOCK_GETTIME_NSEC_NP], 0, [do we have clock_gettime_nsec_np?])]
  )
  AC_CHECK_FUNCS(gettimeofday)
])
