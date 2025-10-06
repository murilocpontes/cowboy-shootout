# Cowboy Shootout Makefile
# Compiles client and server applications with socket library

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
DEBUG_FLAGS = -g -DDEBUG

# Directories
SRC_DIR = src
LIB_DIR = library
INC_DIR = include
SOCKET_DIR = $(SRC_DIR)/sockets
CLIENT_DIR = $(SRC_DIR)/client
SERVER_DIR = $(SRC_DIR)/server
BUILD_DIR = build
BIN_DIR = bin

# Include paths
INCLUDES = -I$(SRC_DIR)


# Source files
SOCKET_SRCS = $(SOCKET_DIR)/tcp.cc $(SOCKET_DIR)/udp.cc
CLIENT_SRCS = $(CLIENT_DIR)/main.cc $(CLIENT_DIR)/player.cc $(CLIENT_DIR)/bullet.cc
SERVER_SRCS = $(SERVER_DIR)/main.cc

# Object files
SOCKET_OBJS = $(SOCKET_SRCS:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)
CLIENT_OBJS = $(CLIENT_SRCS:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)
SERVER_OBJS = $(SERVER_SRCS:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)

# Executables
CLIENT_EXEC = $(BIN_DIR)/client
SERVER_EXEC = $(BIN_DIR)/server

# Default target
.PHONY: all
all: $(CLIENT_EXEC) $(SERVER_EXEC)

# Create directories
$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

$(BUILD_DIR)/sockets $(BUILD_DIR)/client $(BUILD_DIR)/server: | $(BUILD_DIR)
	mkdir -p $@

# Compile socket library objects
$(BUILD_DIR)/sockets/%.o: $(SRC_DIR)/sockets/%.cc | $(BUILD_DIR)/sockets
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile client objects
$(BUILD_DIR)/client/%.o: $(SRC_DIR)/client/%.cc | $(BUILD_DIR)/client
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile server objects
$(BUILD_DIR)/server/%.o: $(SRC_DIR)/server/%.cc | $(BUILD_DIR)/server
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link client executable
$(CLIENT_EXEC): $(CLIENT_OBJS) $(SOCKET_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Client built successfully: $@"

# Link server executable
$(SERVER_EXEC): $(SERVER_OBJS) $(SOCKET_OBJS) | $(BIN_DIR)
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

# Install (copy to system path - optional)
.PHONY: install
install: all
	sudo cp $(CLIENT_EXEC) /usr/local/bin/cowboy-client
	sudo cp $(SERVER_EXEC) /usr/local/bin/cowboy-server
	@echo "Installed to /usr/local/bin/"

# Uninstall
.PHONY: uninstall
uninstall:
	sudo rm -f /usr/local/bin/cowboy-client /usr/local/bin/cowboy-server
	@echo "Uninstalled from /usr/local/bin/"

# Test targets
.PHONY: test-local
test-local: all
	@echo "Starting server in background..."
	./$(SERVER_EXEC) &
	@sleep 2
	@echo "Starting client..."
	./$(CLIENT_EXEC)
	@pkill -f $(SERVER_EXEC) || true

# Help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all       - Build both client and server (default)"
	@echo "  client    - Build only the client"
	@echo "  server    - Build only the server"
	@echo "  debug     - Build with debug flags"
	@echo "  clean     - Remove build artifacts"
	@echo "  install   - Install binaries to /usr/local/bin/"
	@echo "  uninstall - Remove installed binaries"
	@echo "  test-local- Start server and client for testing"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make                 # Build everything"
	@echo "  make client          # Build only client"
	@echo "  make debug           # Build with debug info"
	@echo "  make clean && make   # Clean rebuild"

# Force rebuild
.PHONY: rebuild
rebuild: clean all

# Print variables (for debugging Makefile)
.PHONY: vars
vars:
	@echo "CXX: $(CXX)"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "SOCKET_SRCS: $(SOCKET_SRCS)"
	@echo "SOCKET_OBJS: $(SOCKET_OBJS)"
	@echo "CLIENT_EXEC: $(CLIENT_EXEC)"
	@echo "SERVER_EXEC: $(SERVER_EXEC)"