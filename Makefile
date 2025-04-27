# Chess Engine Makefile
# Platform detection
ifeq ($(OS),Windows_NT)
    detected_OS := Windows
    # Windows settings
    CXX = g++
    CXXFLAGS = -std=c++20 -O3 -Wall
    # Add proper Windows DLL export flags
    CXXFLAGS += -DWIN32 -D_WINDOWS
    TARGET = chess_engine_wrapper.dll
    # Add -static to include all MinGW runtime dependencies in the DLL
    LDFLAGS = -shared -static
    # On Windows, use del instead of rm
    RM = del /Q
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        detected_OS := Linux
        CXX = g++
        CXXFLAGS = -std=c++20 -O3 -Wall -fPIC
        TARGET = chess_engine_wrapper.so
        LDFLAGS = -shared
        RM = rm -f
    endif
    ifeq ($(UNAME_S),Darwin)
        detected_OS := macOS
        CXX = g++
        CXXFLAGS = -std=c++20 -O3 -Wall -fPIC
        TARGET = chess_engine_wrapper.dylib
        LDFLAGS = -shared
        RM = rm -f
    endif
endif

# Source files
SRC_DIR = src
ENGINE_DIR = $(SRC_DIR)/engine
SRC_FILES = $(SRC_DIR)/ChessEngineWrapper.cpp \
            $(ENGINE_DIR)/ChessEngine.cpp \
            $(ENGINE_DIR)/Evaluation.cpp \
            $(ENGINE_DIR)/transposition_table.cpp 

# Include directories
INCLUDES = -I$(SRC_DIR)

# Default target
all: $(TARGET)

# Build the chess engine wrapper library
$(TARGET): $(SRC_FILES)
	@echo "Building chess engine for $(detected_OS)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^
	@echo "Build complete: $@"

# Clean up build artifacts
clean:
	@echo "Cleaning up build artifacts..."
	$(RM) $(TARGET)

# Run the chess game
run: $(TARGET)
	python run_game.py

# Help target
help:
	@echo "Chess Engine Makefile"
	@echo "Available targets:"
	@echo "  all     - Build the chess engine wrapper (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  run     - Build the chess engine wrapper and run the game"
	@echo "  help    - Display this help message"

.PHONY: all clean run help