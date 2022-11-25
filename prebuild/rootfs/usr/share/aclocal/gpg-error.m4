# gpg-error.m4 - autoconf macro to detect libgpg-error.
# Copyright (C) 2002, 2003, 2004, 2011, 2014, 2018, 2020, 2021
#               g10 Code GmbH
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This file is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# Last-changed: 2021-02-16


dnl AM_PATH_GPG_ERROR([MINIMUM-VERSION,
dnl                   [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl Test for libgpg-error and define GPG_ERROR_CFLAGS, GPG_ERROR_LIBS,
dnl GPG_ERROR_MT_CFLAGS, and GPG_ERROR_MT_LIBS.  The _MT_ variants are
dnl used for programs requireing real multi thread support.
dnl
dnl If a prefix option is not used, the config script is first
dnl searched in $SYSROOT/bin and then along $PATH.  If the used
dnl config script does not match the host specification the script
dnl is added to the gpg_config_script_warn variable.
dnl
AC_DEFUN([AM_PATH_GPG_ERROR],
[ AC_REQUIRE([AC_CANONICAL_HOST])
  min_gpg_error_version=ifelse([$1], ,0.0,$1)
  PKG_CHECK_MODULES(GPG_ERROR, [gpg-error >= $min_gpg_error_version], [ok=yes], [ok=no])
  if test $ok = yes; then
    ifelse([$2], , :, [$2])
    if test -z "$GPGRT_CONFIG"; then
      gpg_error_config_host=`$PKG_CONFIG --variable=host gpg-error`
    fi
    if test x"$gpg_error_config_host" != xnone ; then
      if test x"$gpg_error_config_host" != x"$host" ; then
  AC_MSG_WARN([[
***
*** The config script "$GPG_ERROR_CONFIG" was
*** built for $gpg_error_config_host and thus may not match the
*** used host $host.
*** You may want to use the configure option --with-libgpg-error-prefix
*** to specify a matching config script or use \$SYSROOT.
***]])
        gpg_config_script_warn="$gpg_config_script_warn libgpg-error"
      fi
    fi
  else
    ifelse([$3], , :, [$3])
  fi
])
