CXX := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -Wpedantic -O2

TARGET := cfg_comparator

SRCS := main.cpp lexer.cpp parser.cpp token.cpp
OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
