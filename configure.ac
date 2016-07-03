#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([galaxy], [0.0.1], [munozga@gmail.com])
AC_CONFIG_SRCDIR([galaxyd/galaxyd.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_SUBDIRS([liberror])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lerror':
#AC_CHECK_LIB([error], [err_msg])
# FIXME: Replace `main' with a function in `-lgalaxy':
#AC_CHECK_LIB([galaxy], [galaxy_connect])
# FIXME: Replace `main' with a function in `-lpcre':
AC_CHECK_LIB([pcre], [pcre_compile])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([glib-2.0], [g_hash_table_new])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h fcntl.h inttypes.h stddef.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h sys/un.h unistd.h pcre.h pthread.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_STAT
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([ftruncate memmove memset pathconf select socket strdup strerror])
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.0.0],
	[ADD_PC_REQUIRES([glib-2.0 >= 2.0.0])],
	[AC_MSG_ERROR([Cannot find glib2/gtk2/pango])]
)

# Checks for system services.

# Finish.
AC_CONFIG_FILES([Makefile libgalaxy/Makefile galaxyd/Makefile galaxy/Makefile])
AC_OUTPUT
