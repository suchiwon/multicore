# Compiler and Compile options.
CC = g++
CXXFLAGS = -g -Wall -std=c++11
# Macros specifying path for compile.
SRCS := $(wildcard src/.cpp)
OBJS := $(SRCS:.cpp=.o)
BIN = ./bin/
INC = ./include/
LIB = ./lib/ -lpthread

# Pre-Processor.
CPPFLAGS += -I$(INC)

# Compile command.
Project1_test: ./src/main.cpp
	mkdir bin
	g++ -g -o ./bin/project2 ./src/main.cpp -lpthread -std=c++11
# Delete binary & object files.
clean:
	rm -r bin *.txt
