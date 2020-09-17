#############################################################
#
# gat-aml-drmbufferpool1
#
#############################################################
GST_AML_DRMBUFFERPOOL1_VERSION = 1.0
GST_AML_DRMBUFFERPOOL1_SITE = $(TOPDIR)/../multimedia/gst-aml-drmbufferpool1/gst-aml-drmbufferpool-1.0
GST_AML_DRMBUFFERPOOL1_SITE_METHOD = local

GST_AML_DRMBUFFERPOOL1_INSTALL_STAGING = YES
GST_AML_DRMBUFFERPOOL1_AUTORECONF = YES
GST_AML_DRMBUFFERPOOL1_DEPENDENCIES = libdrm gstreamer1 gst1-plugins-base host-pkgconf

$(eval $(autotools-package))

