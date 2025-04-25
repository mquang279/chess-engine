# Chess GUI Makefile

# Compiler & flags
CC = g++
CFLAGS = -Wall -O3 -march=native -std=c++20 -Isrc
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system

# Source files
SOURCES = src/ChessGUI.cpp src/main.cpp src/engine/ChessEngine.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = chess-gui

# Default rule
all: $(EXECUTABLE)

# Linking the executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Generic rule for compiling a source file to an object file
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to install SFML if not present
packages:
	@if ! dpkg -l | grep libsfml-dev -c >>/dev/null; then \
		echo "SFML not found. Downloading and installing..."; \
		sudo apt-get install libsfml-dev; \
	else \
		echo "SFML already installed."; \
	fi

# Copy required assets from the main project
setup-assets:
	mkdir -p src/assets/fonts src/assets/sounds src/assets/themes/tartanian
	cp -r ../src/assets/fonts/* src/assets/fonts/
	cp -r ../src/assets/sounds/* src/assets/sounds/
	cp -r ../src/assets/themes/tartiana/* src/assets/themes/tartanian/

# Clean rule
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all packages setup-assets clean