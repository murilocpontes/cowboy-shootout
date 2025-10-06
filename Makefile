# Cowboy Shootout Makefile
# Compiles client and server applications with socket library

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
DEBUG_FLAGS = -g -DDEBUG

# Raylib flags for client
RAYLIB_CFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Directories
SRC_DIR = src
SOCKET_DIR = $(SRC_DIR)/sockets
CLIENT_DIR = $(SRC_DIR)/client
SERVER_DIR = $(SRC_DIR)/server
GAME_DIR = $(SRC_DIR)/server/game
BUILD_DIR = build
BIN_DIR = bin

# Include paths
INCLUDES = -I$(SRC_DIR)

# Source files
SOCKET_SRCS = $(SOCKET_DIR)/tcp.cc $(SOCKET_DIR)/udp.cc
CLIENT_SRCS = $(CLIENT_DIR)/main.cc
# Server sources
SERVER_SRCS = $(SERVER_DIR)/server.cc \
	          $(SERVER_DIR)/player_manager.cc \
	          $(SERVER_DIR)/match_manager.cc \
	          $(SERVER_DIR)/message_handler.cc \
	          $(SERVER_DIR)/broadcast_manager.cc
# Game entity sources
GAME_SRCS = $(GAME_DIR)/player.cc $(GAME_DIR)/match.cc

# Object files
SOCKET_OBJS = $(SOCKET_SRCS:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)
CLIENT_OBJS = $(CLIENT_SRCS:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)
SERVER_OBJS = $(SERVER_SRCS:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)
GAME_OBJS = $(GAME_SRCS:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)

# Executables
CLIENT_EXEC = $(BIN_DIR)/client
SERVER_EXEC = $(BIN_DIR)/server

# Default target
.PHONY: all
all: $(CLIENT_EXEC) $(SERVER_EXEC)

# Create directories
$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

$(BUILD_DIR)/sockets $(BUILD_DIR)/client $(BUILD_DIR)/server $(BUILD_DIR)/server/game: | $(BUILD_DIR)
	mkdir -p $@

# Compile socket library objects
$(BUILD_DIR)/sockets/%.o: $(SRC_DIR)/sockets/%.cc | $(BUILD_DIR)/sockets
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile client objects (with raylib support)
$(BUILD_DIR)/client/%.o: $(SRC_DIR)/client/%.cc | $(BUILD_DIR)/client
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile server objects
$(BUILD_DIR)/server/%.o: $(SRC_DIR)/server/%.cc | $(BUILD_DIR)/server
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile game objects
$(BUILD_DIR)/server/game/%.o: $(SRC_DIR)/server/game/%.cc | $(BUILD_DIR)/server/game
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link client executable (with raylib)
$(CLIENT_EXEC): $(CLIENT_OBJS) $(SOCKET_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(RAYLIB_CFLAGS)
	@echo "Client built successfully with raylib: $@"

# Link server executable (no raylib)
$(SERVER_EXEC): $(SERVER_OBJS) $(SOCKET_OBJS) $(GAME_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Server built successfully: $@"

# Debug builds
.PHONY: debug
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: all

# Individual targets
.PHONY: client server
client: $(CLIENT_EXEC)
server: $(SERVER_EXEC)

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Build artifacts cleaned"

# Install raylib (Ubuntu/Debian)
.PHONY: install-raylib
install-raylib:
	sudo apt update
	sudo apt install libraylib-dev libraylib4
	@echo "Raylib installed successfully"

# Install raylib from source (alternative method)
.PHONY: install-raylib-source
install-raylib-source:
	@echo "Installing raylib from source..."
	git clone https://github.com/raysan5/raylib.git /tmp/raylib
	cd /tmp/raylib && mkdir build && cd build
	cd /tmp/raylib/build && cmake .. && make -j4
	cd /tmp/raylib/build && sudo make install
	sudo ldconfig
	@echo "Raylib installed from source"

# Test raylib installation
.PHONY: test-raylib
test-raylib:
	@echo "Testing raylib installation..."
	@echo '#include <raylib.h>' > /tmp/test_raylib.cpp
	@echo 'int main() { InitWindow(100, 100, "Test"); CloseWindow(); return 0; }' >> /tmp/test_raylib.cpp
	$(CXX) -o /tmp/test_raylib /tmp/test_raylib.cpp $(RAYLIB_CFLAGS) && echo "✅ Raylib is working!" || echo "❌ Raylib installation failed"
	@rm -f /tmp/test_raylib /tmp/test_raylib.cpp

# Test individual components
.PHONY: test-server-only
test-server-only: server
	@echo "Starting server only..."
	./$(SERVER_EXEC)

# Build only the managers
.PHONY: managers
managers: $(BUILD_DIR)/server/player_manager.o \
	      $(BUILD_DIR)/server/match_manager.o \
	      $(BUILD_DIR)/server/message_handler.o \
	      $(BUILD_DIR)/server/broadcast_manager.o
	@echo "All manager objects compiled successfully"

# Build only game objects
.PHONY: game
game: $(BUILD_DIR)/server/game/player.o \
	  $(BUILD_DIR)/server/game/match.o
	@echo "All game objects compiled successfully"

# Help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all              - Build both client and server (default)"
	@echo "  client           - Build only the client (with raylib)"
	@echo "  server           - Build only the server"
	@echo "  managers         - Compile only manager objects"
	@echo "  game             - Compile only game objects"
	@echo "  debug            - Build with debug flags"
	@echo "  clean            - Remove build artifacts"
	@echo "  install-raylib   - Install raylib via package manager"
	@echo "  install-raylib-source - Install raylib from source"
	@echo "  test-raylib      - Test if raylib is properly installed"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Before building client, install raylib:"
	@echo "  make install-raylib     # Easy way (requires raylib package)"
	@echo "  make test-raylib        # Verify installation"
	@echo "  make client             # Build client with raylib"

# Print variables (for debugging)
.PHONY: vars
vars:
	@echo "SERVER_SRCS: $(SERVER_SRCS)"
	@echo "GAME_SRCS: $(GAME_SRCS)"
	@echo "SERVER_OBJS: $(SERVER_OBJS)"
	@echo "GAME_OBJS: $(GAME_OBJS)"
	@echo "RAYLIB_CFLAGS: $(RAYLIB_CFLAGS)"