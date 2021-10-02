CXX = g++
CXXFLAGS = -Wall -Werror -std=c++17 -g

all: vsfs

vsfs: main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	$(RM) vsfs *.o
