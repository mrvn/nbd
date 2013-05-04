SUBDIRS = man doc
bin_PROGRAMS = nbd-server nbd-trdump
sbin_PROGRAMS = @NBD_CLIENT_NAME@
lib_LTLIBRARIES = libnbd-common.la libnbd-server.la libnbd-client.la
EXTRA_PROGRAMS = nbd-client make-integrityhuge
TESTS_ENVIRONMENT=$(srcdir)/simple_test
TESTS = cmd cfg1 cfgmulti cfgnew cfgsize write flush integrity dirconfig list #integrityhuge
check_PROGRAMS = nbd-tester-client
nbd_client_SOURCES = nbd-client.c cliserv.h
nbd_server_SOURCES = nbd-server.c cliserv.h lfs.h nbd.h
nbd_tester_client_SOURCES = nbd-tester-client.c cliserv.h netdb-compat.h
nbd_trdump_SOURCES = nbd-trdump.c cliserv.h nbd.h
nbd_server_CFLAGS = @CFLAGS@ @GLIB_CFLAGS@
nbd_tester_client_CFLAGS = @CFLAGS@ @GLIB_CFLAGS@
nbd_trdump_CFLAGS = @CFLAGS@ @GLIB_CFLAGS@
nbd_client_LDADD = @GLIB_LIBS@ libnbd-client.la
nbd_server_LDADD = @GLIB_LIBS@ libnbd-server.la
nbd_tester_client_LDADD = @GLIB_LIBS@ libnbd-client.la
make_integrityhuge_SOURCES = make-integrityhuge.c cliserv.h nbd.h
libnbd_common_la_SOURCES = libnbd-common.c nbd-common.h
libnbd_client_la_SOURCES = libnbd-client.c nbd-client.h
libnbd_client_la_LIBADD = libnbd-common.la
libnbd_server_la_SOURCES = libnbd-server.c nbd-server.h
libnbd_server_la_LIBADD = libnbd-common.la
include_HEADERS = nbd-common.h nbd-client.h nbd-server.h
EXTRA_DIST = gznbd simple_test integrity-test.tr integrityhuge-test.tr maketr CodingStyle autogen.sh
ACLOCAL_AMFLAGS = -I m4
dist-hook:
	rm -Rf `find $(distdir) -name '.svn' -type d -print`
cmd:
cfg1:
cfgmulti:
cfgnew:
cfgsize:
write:
flush:
integrity:
integrityhuge:
dirconfig:
list:
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
