CXX      ?= c++ -std=c++14
CPPFLAGS ?= $(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_image opencv)
LDFLAGS  ?= $(shell pkg-config --libs sdl2 SDL2_ttf SDL2_image opencv)
PANDOC   ?= pandoc

PACKAGE  := conhud
VERSION  := $(shell cat ../VERSION)

SOURCES  := conhud.cpp
TARGETS  := conhud

%.html: %.rst
	$(PANDOC) -o $@ -t html5 -s $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -o $@ -c $<

%: %.o
	$(CXX) $(LDFLAGS) $(TARGET_ARCH) -o $@ $^ $(LOADLIBES) $(LDLIBS)

conhud: conhud.o

all: build

build: $(TARGETS)
    

check: $(TARGETS)
	@echo "not implemented"; exit 2 # TODO

dist:
	@echo "not implemented"; exit 2 # TODO

install:
	@echo "not implemented"; exit 2 # TODO

uninstall:
	@echo "not implemented"; exit 2 # TODO

clean:
	@rm -f *~ *.o $(TARGETS)

distclean: clean

mostlyclean: clean

.PHONY: check dist install clean distclean mostlyclean
.SECONDARY:
.SUFFIXES:
