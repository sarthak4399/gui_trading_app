# HFT Trading Application Makefile
# Professional Intraday Trading System with GUI

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
INCLUDES = -Iinclude \
           -I/opt/homebrew/include \
           -I/opt/homebrew/Cellar/nlohmann-json/3.12.0/include \
           -I/opt/homebrew/Cellar/tgui/1.10.0/include \
           -I/opt/homebrew/Cellar/sfml/3.0.1/include
LIBS = -L/opt/homebrew/lib \
       -L/opt/homebrew/Cellar/tgui/1.10.0/lib \
       -L/opt/homebrew/Cellar/sfml/3.0.1/lib \
       -lsfml-system -lsfml-window -lsfml-graphics -lsfml-network -ltgui -lcurl

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Target executable
TARGET = $(BUILD_DIR)/HFTTradingApp

# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

# Link executable
$(TARGET): $(BUILD_DIR) $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LIBS)
	@echo "Build complete: $(TARGET)"

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	@echo "Clean complete"

# Install dependencies (Ubuntu/Debian)
install-deps-ubuntu:
	sudo apt-get update
	sudo apt-get install -y \
		build-essential \
		cmake \
		libsfml-dev \
		libtgui-dev \
		libcurl4-openssl-dev \
		nlohmann-json3-dev \
		pkg-config

# Install dependencies (macOS with Homebrew)
install-deps-macos:
	brew install sfml tgui curl nlohmann-json

# Install dependencies (Arch Linux)
install-deps-arch:
	sudo pacman -S sfml tgui curl nlohmann-json

# Run the application
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -DDEBUG -g3 -O0
debug: $(TARGET)

# Release build
release: CXXFLAGS += -DNDEBUG -O3 -march=native
release: clean $(TARGET)

# Format code (requires clang-format)
format:
	find $(SRC_DIR) $(INC_DIR) -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Check code style
check:
	cppcheck --enable=all --std=c++17 $(SRC_DIR)/ $(INC_DIR)/

# Generate documentation (requires Doxygen)
docs:
	doxygen Doxyfile

# Package for distribution
package: release
	mkdir -p dist/HFTTradingApp
	cp $(TARGET) dist/HFTTradingApp/
	cp -r resources dist/HFTTradingApp/
	cp -r data dist/HFTTradingApp/
	cp README.md dist/HFTTradingApp/
	tar -czf dist/HFTTradingApp.tar.gz -C dist HFTTradingApp
	@echo "Package created: dist/HFTTradingApp.tar.gz"

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build the application (default)"
	@echo "  clean         - Remove build files"
	@echo "  run           - Build and run the application"
	@echo "  debug         - Build with debug symbols"
	@echo "  release       - Build optimized release version"
	@echo "  format        - Format source code"
	@echo "  check         - Run static code analysis"
	@echo "  docs          - Generate documentation"
	@echo "  package       - Create distribution package"
	@echo "  install-deps-ubuntu - Install dependencies on Ubuntu/Debian"
	@echo "  install-deps-macos  - Install dependencies on macOS"
	@echo "  install-deps-arch   - Install dependencies on Arch Linux"

# Phony targets
.PHONY: all clean run debug release format check docs package help \
        install-deps-ubuntu install-deps-macos install-deps-arch

# Dependency tracking
-include $(OBJECTS:.o=.d)

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM -MT $(@:.d=.o) $< > $@
