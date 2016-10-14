CC = clang
CXX = clang++

INCLUDES = -I/usr/local/include
LIBRARIES = -L/usr/local/lib

CFLAGS = -std=c11 -Wall -Wextra -pedantic -Wnon-virtual-dtor -Wmissing-prototypes -Wconversion -g -O0
CXXFLAGS = -std=c++11 -stdlib=libc++
CPPFLAGS =  -Wall -Wextra -pedantic -Wnon-virtual-dtor -Wmissing-prototypes -Wconversion -g -O0
LDFLAGS = $(LIBRARIES)

TARGET = build/rt1w
MAIN = main.cpp
CSOURCES = sync.c
CXXSOURCES = 	vec.cpp 	\
	 	ray.cpp 	\
		sphere.cpp 	\
		hitablelist.cpp \
		camera.cpp 	\
		material.cpp 	\
		event.cpp 	\
		wqueue.cpp	\
		bvh.cpp
OBJECTS = $(CSOURCES:.c=.o) $(CXXSOURCES:.cpp=.o) $(MAIN:.cpp=.o)

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
