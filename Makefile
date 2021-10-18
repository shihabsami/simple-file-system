CXX = g++
CXXFLAGS = -Wall -Werror -std=c++17 -g

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
DEP = $(SRC:.cpp=.d)

BIN = vsfs

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

.PHONY: clean

clean:
	$(RM) $(OBJ) $(DEP) $(BIN)

-include $(DEP)
