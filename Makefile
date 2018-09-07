CXX = g++
CXXFLAGS =  -std=c++11 -I ./include
LDFLAGS = -fPIC -shared -lpthread

ifeq ($(OS),Windows_NT)
  PLATFORM="Windows"
  SOURCES = \
  src/uarm.cc \
  src/utils/utils.cc \
  src/serial/serial.cc \
  src/serial/impl/win.cc \
  src/serial/impl/list_ports/list_ports_win.cc

  LIB = uarm.lib
else
  ifeq ($(shell uname),Darwin)
    PLATFORM="MacOS"
	SOURCES = \
	src/uarm.cc \
	src/utils/utils.cc \
	src/serial/serial.cc \
	src/serial/impl/unix.cc \
	src/serial/impl/list_ports/list_ports_osx.cc

	LIB = libuarm.so
  else
    PLATFORM="Unix-Like"
	SOURCES = \
	src/uarm.cc \
	src/utils/utils.cc \
	src/serial/serial.cc \
	src/serial/impl/unix.cc \
	src/serial/impl/list_ports/list_ports_linux.cc

	LIB = libuarm.so
  endif
endif

EXAMPLE_SOURCE = example/uarm_example.cc

all: uarm uarm_example

uarm:
	$(CXX) $(SOURCES) $(CXXFLAGS) $(LDFLAGS) -o $(LIB)
uarm_example:
	$(CXX) $(EXAMPLE_SOURCE) $(CXXFLAGS) -L. -luarm -o test_uarm

install:
	sudo cp -rf include/uarm /usr/include
	sudo cp -rf include/serial /usr/include
	sudo cp -f libuarm.so /usr/lib/

clean:
	rm -f libuarm.so
	rm -f test_uarm