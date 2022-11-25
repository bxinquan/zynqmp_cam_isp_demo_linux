# libgcrypt.m4 - Autoconf macros to detect libgcrypt
# Copyright (C) 2002, 2003, 2004, 2011, 2014, 2018, 2020 g10 Code GmbH
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This file is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# Last-changed: 2020-09-27


dnl AM_PATH_LIBGCRYPT([MINIMUM-VERSION,
dnl                   [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl Test for libgcrypt and define LIBGCRYPT_CFLAGS and LIBGCRYPT_LIBS.
dnl MINIMUM-VERSION is a string with the version number optionally prefixed
dnl with the API version to also check the API compatibility. Example:
dnl a MINIMUM-VERSION of 1:1.2.5 won't pass the test unless the installed
dnl version of libgcrypt is at least 1.2.5 *and* the API number is 1.  Using
dnl this features allows to prevent build against newer versions of libgcrypt
dnl with a changed API.
dnl
dnl If a prefix option is not used, the config script is first
dnl searched in $SYSROOT/bin and then along $PATH.  If the used
dnl config script does not match the host specification the script
dnl is added to the gpg_config_script_warn variable.
dnl
AC_DEFUN([AM_PATH_LIBGCRYPT],
[ AC_REQUIRE([AC_CANONICAL_HOST])

  tmp=ifelse([$1], ,1:1.2.0,$1)
  if echo "$tmp" | grep ':' >/dev/null 2>/dev/null ; then
     req_libgcrypt_api=`echo "$tmp"     | sed 's/\(.*\):\(.*\)/\1/'`
     min_libgcrypt_version=`echo "$tmp" | sed 's/\(.*\):\(.*\)/\2/'`
  else
     req_libgcrypt_api=0
     min_libgcrypt_version="$tmp"
  fi

  PKG_CHECK_MODULES(LIBGCRYPT, [libgcrypt >= $min_libgcrypt_version], [ok=yes], [ok=no])

  if test $ok = yes; then
     # If we have a recent libgcrypt, we should also check that the
     # API is compatible
     if test "$req_libgcrypt_api" -gt 0 ; then
        tmp=`$PKG_CONFIG --variable=api_version libgcrypt`
        if test "$tmp" -gt 0 ; then
           AC_MSG_CHECKING([LIBGCRYPT API version])
           if test "$req_libgcrypt_api" -eq "$tmp" ; then
             AC_MSG_RESULT([okay])
           else
             ok=no
             AC_MSG_RESULT([does not match. want=$req_libgcrypt_api got=$tmp])
           fi
        fi
     fi
  fi
  if test $ok = yes; then
    ifelse([$2], , :, [$2])
    if test -z "$use_gpgrt_config"; then
      libgcrypt_config_host=`$PKG_CONFIG --variable=host libgcrypt`
    else
      libgcrypt_config_host=`$LIBGCRYPT_CONFIG --variable=host 2>/dev/null || echo none`
    fi
    if test x"$libgcrypt_config_host" != xnone ; then
      if test x"$libgcrypt_config_host" != x"$host" ; then
  AC_MSG_WARN([[
***
*** The config script "$LIBGCRYPT_CONFIG" was
*** built for $libgcrypt_config_host and thus may not match the
*** used host $host.
*** You may want to use the configure option --with-libgcrypt-prefix
*** to specify a matching config script or use \$SYSROOT.
***]])
        gpg_config_script_warn="$gpg_config_script_warn libgcrypt"
      fi
    fi
  else
    ifelse([$3], , :, [$3])
  fi
  AC_SUBST(LIBGCRYPT_CFLAGS)
  AC_SUBST(LIBGCRYPT_LIBS)
])
