# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstplugin.la to something else, e.g. libmysomething.la     #
##############################################################################
lib_LTLIBRARIES = libgstdrmallocator.la libgstdrmbufferpool.la 

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
libgstdrmallocator_la_SOURCES = gstdrmallocator.c

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstdrmallocator_la_CFLAGS = $(GST_CFLAGS) -I$(top_srcdir)/include
libgstdrmallocator_la_LIBADD = $(GST_LIBS)
libgstdrmallocator_la_LDFLAGS = $(GST_LDFLAGS)
libgstdrmallocator_la_LIBTOOLFLAGS = --tag=disable-static

libgstdrmallocator_includedir = $(includedir)/gstreamer-1.0/gst/allocators
libgstdrmallocator_include_HEADERS = gstdrmallocator.h


libgstdrmbufferpool_la_SOURCES = gstdrmbufferpool.c
libgstdrmbufferpool_la_CFLAGS = $(GST_CFLAGS) -I$(top_srcdir)/include
libgstdrmbufferpool_la_LIBADD = $(GST_LIBS) -ldrm
libgstdrmbufferpool_la_LDFLAGS = $(GST_LDFLAGS)
libgstdrmbufferpool_la_LIBTOOLFLAGS = --tag=disable-static

libgstdrmbufferpool_includedir = $(includedir)/gstreamer-1.0/gst
libgstdrmbufferpool_include_HEADERS = gstdrmbufferpool.h