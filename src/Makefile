#!/bin/make
CC = gcc
CFLAGS = -O2 -Wall
CXX = g++
CXXFLAGS = -O2 -Wall
LDFLAGS = -lm -lz
OBJS = saptapper

.PHONY: all clean
.SUFFIXES: .c .cpp .o

all: $(OBJS)

clean:
	$(RM) $(OBJS)

$(OBJS): \
	saptapper.o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $^
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $^
.c:
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
.cpp:
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
