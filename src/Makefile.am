AM_CPPFLAGS   += $(OPENCV_CFLAGS)
AM_CPPFLAGS   += $(GLEW_CFLAGS)
AM_CPPFLAGS   += $(GLFW_CFLAGS)
AM_CPPFLAGS   += $(LIBDRM_CFLAGS)
AM_CPPFLAGS   += $(JSONCPP_CFLAGS)
AM_CPPFLAGS   += $(TBB_CFLAGS)

bin_PROGRAMS   = conhud

conhud_SOURCES = conhud.cpp rendering.cpp input.cpp

conhud_LDADD   =
conhud_LDADD  += $(OPENCV_LIBS)
conhud_LDADD  += $(GLEW_LIBS)
conhud_LDADD  += $(GLFW_LIBS)
conhud_LDADD  += $(LIBDRM_LIBS)
conhud_LDADD  += $(JSONCPP_LIBS)
conhud_LDADD  += $(TBB_LIBS)

# FIXME
if !DISABLE_DARKNET
  conhud_LDADD += /usr/local/lib/darknet.so
conhud_SOURCES += darknet.cpp
endif

if !DISABLE_OSVR
  AM_CPPFLAGS  += -I/usr/include/osvr
  conhud_LDADD += -losvrClientKit
endif
