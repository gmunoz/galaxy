MAINTAINERCLEANFILES    = Makefile.in

INCLUDES                = -I$(top_builddir) -I$(top_srcdir) -I$(top_srcdir)/liberror/src -I$(top_srcdir)/libgalaxy -I$(top_srcdir)/inotify

noinst_HEADERS  = crawler_thread.h event_queue.h ihandler_thread.h inotify_utils.h iwatch_thread.h list.h notifier.h server_thread.h signal_thread.h thread.h watch.h

bin_PROGRAMS    = galaxyd

galaxyd_LDADD       = $(top_builddir)/liberror/src/liberror.la $(top_builddir)/libgalaxy/libgalaxy.la @GLIB_LIBS@

galaxyd_CFLAGS = @GLIB_CFLAGS@

galaxyd_SOURCES     = crawler_thread.c event_queue.c galaxyd.c ihandler_thread.c inotify_utils.c iwatch_thread.c list.c notifier.c server_thread.c signal_thread.c thread.c watch.c
