CXX = g++

CXXFLAGS = -std=c++17 -Wall -Iinclude

LIBS = -lcurl -lmysqlclient

SRC = $(wildcard src/*.cpp)

OBJ = $(patsubst src/%.cpp,obj/%.o,$(SRC))

TARGET = bin/radio

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CXX) $(OBJ) -o $(TARGET) $(LIBS)

obj/%.o: src/%.cpp
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj bin