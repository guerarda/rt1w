CC = clang

INCLUDES = -I/usr/local/include
LIBRARIES = -L/usr/local/lib

CFLAGS = -Wall -pedantic -g -O0 $(INCLUDES)
LDFLAGS = $(LIBRARIES)

TARGET = build/rt1w
MAIN = main.c
SOURCES =
HEADERS =
OBJECTS = $(SOURCES:.c=.o) $(MAIN:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
