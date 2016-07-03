MAINTAINERCLEANFILES    = Makefile.in

INCLUDES                = -I$(top_builddir) -I$(top_srcdir) -I$(top_srcdir)/liberror/src -I$(top_srcdir)/libgalaxy -I$(top_srcdir)/inotify

bin_PROGRAMS    = galaxy
galaxy_LDADD       = $(top_builddir)/liberror/src/liberror.la $(top_builddir)/libgalaxy/libgalaxy.la
galaxy_SOURCES     = galaxy.c thread.c
