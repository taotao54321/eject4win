.PHONY: all clean

CXX      ?= g++
CXXFLAGS ?= -std=c++14 -Wall -Wextra -O2
CPPFLAGS ?= -MMD -MP
LDFLAGS  ?= -s

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

TARGET := eject.exe

all: $(TARGET)

eject.exe: $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	-$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
