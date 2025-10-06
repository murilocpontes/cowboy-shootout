#!/bin/bash

echo "=== Game Server Multi-Client Test ==="
echo

# Function to cleanup background processes
cleanup() {
    echo
    echo "🧹 Cleaning up..."
    pkill -f "bin/server" 2>/dev/null || true
    pkill -f "bin/client" 2>/dev/null || true
    exit 0
}

# Set trap to cleanup on exit
trap cleanup INT TERM EXIT

# Start server in background
echo "🚀 Starting game server..."
./bin/server &
SERVER_PID=$!

# Wait for server to start
sleep 2

# Check if server started successfully
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "❌ Server failed to start!"
    exit 1
fi

echo "✅ Server started (PID: $SERVER_PID)"
echo "📡 Server listening on TCP:8080, UDP:8081"
echo

echo "🎮 Testing Instructions:"
echo "1. Multiple terminal windows will open automatically"
echo "2. In each client, type 'ready' and press Enter"
echo "3. When all clients are ready, the game should start"
echo "4. Type 'shoot' to test UDP messages"
echo "5. Type 'quit' to exit a client"
echo

# Function to start a client in a new terminal
start_client() {
    local client_num=$1
    echo "Starting Client $client_num..."
    
    # Try different terminal emulators
    if command -v gnome-terminal >/dev/null 2>&1; then
        gnome-terminal --title="Client $client_num" -- bash -c "
            echo 'Client $client_num starting...';
            echo 'Commands: ready, shoot, quit';
            echo '========================';
            ./bin/client;
            echo 'Client $client_num ended. Press Enter to close.';
            read"
    elif command -v xterm >/dev/null 2>&1; then
        xterm -title "Client $client_num" -e bash -c "
            echo 'Client $client_num starting...';
            echo 'Commands: ready, shoot, quit';
            echo '========================';
            ./bin/client;
            echo 'Client $client_num ended. Press Enter to close.';
            read" &
    elif command -v konsole >/dev/null 2>&1; then
        konsole --title "Client $client_num" -e bash -c "
            echo 'Client $client_num starting...';
            echo 'Commands: ready, shoot, quit';
            echo '========================';
            ./bin/client;
            echo 'Client $client_num ended. Press Enter to close.';
            read" &
    else
        echo "No suitable terminal emulator found. Please run clients manually:"
        echo "./bin/client"
        return 1
    fi
    
    sleep 1
}

echo "🔄 Starting 2 clients automatically..."
start_client 1
start_client 2

echo
echo "✨ Two clients should be starting in separate windows!"
echo "📝 Watch the server output below for connection messages:"
echo "================================================"

# Keep the script running and show server output
tail -f /dev/null &
wait $SERVER_PID