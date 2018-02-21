CXX      := c++ -std=c++14

CPPFLAGS := $(CPPFLAGS) $(shell pkg-config --cflags opencv glu gl glew glfw3 libdrm jsoncpp tbb) -I/usr/include/osvr -DENABLE_OSVR
LDFLAGS  := $(LDFLAGS) $(shell pkg-config --libs opencv glu gl glew glfw3 libdrm jsoncpp tbb) -losvrClientKit -lpthread /usr/local/lib/darknet.so

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
