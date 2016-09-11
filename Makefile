CC = clang
CXX = clang++

INCLUDES = -I/usr/local/include
LIBRARIES = -L/usr/local/lib

CFLAGS = -std=c11
CXXFLAGS = -std=c++11 -stdlib=libc++
CPPFLAGS = -Wall -Wextra -pedantic -g -O0
LDFLAGS = $(LIBRARIES)

TARGET = build/rt1w
MAIN = main.c
CSOURCES =
CXXSOURCES = vec.cpp ray.cpp sphere.cpp hitablelist.cpp camera.cpp
OBJECTS = $(CSOURCES:.c=.o) $(CXXSOURCES:.cpp=.o) $(MAIN:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CFLAGS) $(WFLAGS) $(INCLUDES) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) $(WFLAGS) $(INCLUDES) -c -o $@ $<

%o.: %.cpp
	$(CXX) $(CXXFLAGS) $(WFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: clean
