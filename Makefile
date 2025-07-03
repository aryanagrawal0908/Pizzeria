# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -pthread -Wall -Wextra -O2

# Target executable
TARGET = pizzeria

# Source files
SOURCES = main.cpp pizzeria.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Check for memory leaks (requires valgrind)
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Static analysis (requires cppcheck)
analyze:
	cppcheck --enable=all --std=c++20 --suppress=missingIncludeSystem $(SOURCES)

# Format code (requires clang-format)
format:
	clang-format -i *.cpp *.h

.PHONY: all clean debug run valgrind analyze format
