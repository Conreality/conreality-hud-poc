CXX      := c++ -std=c++14 -I/usr/include/osvr /usr/local/lib/darknet.so -DENABLE_OSVR

CPPFLAGS := $(CPPFLAGS) $(shell pkg-config --cflags opencv glu gl glew glfw3 libdrm jsoncpp tbb)
LDFLAGS  := $(LDFLAGS) $(shell pkg-config --libs opencv glu gl glew glfw3 libdrm jsoncpp tbb) -losvrClientKit -lpthread

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -o $@ -c $<

%: %.o
	$(CXX) $(TARGET_ARCH) -o $@ $^ $(LOADLIBES) $(LDLIBS) $(LDFLAGS)

conhud: conhud.o

all: conhud

clean:
	@rm -f *~ *.o conhud

.PHONY: clean
.SECONDARY:
.SUFFIXES:
