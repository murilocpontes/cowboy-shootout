#include "sockets/tcp.hh"
#include "sockets/udp.hh"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <map>
#include <atomic>
#include <chrono>
#include <cstring>

// Simple game packet types
enum class MessageType : uint8_t {
    PLAYER_JOIN = 1,
    PLAYER_READY = 2,
    GAME_START = 3,
    PLAYER_POSITION = 4,
    PLAYER_SHOOT = 5
};

struct Player {
    int tcpSocket;
    std::string name;
    std::string udpIP;
    int udpPort;
    bool isReady;
    float x, y;  // Position
    int id;
};

class GameServer {
private:
    TCPServer tcpServer;
    UDP udpSocket;
    std::map<int, Player> players;  // tcpSocket -> Player
    std::mutex playersMutex;
    std::atomic<bool> gameRunning;
    std::atomic<bool> serverRunning;
    int nextPlayerId;
    
public:
    GameServer(int tcpPort, int udpPort) 
        : tcpServer(tcpPort), udpSocket(udpPort), gameRunning(false), serverRunning(true), nextPlayerId(1) {}
    
    void removePlayer(int tcpSocket) {
        std::lock_guard<std::mutex> lock(playersMutex);
        auto it = players.find(tcpSocket);
        if (it != players.end()) {
            std::cout << "Player " << it->second.name << " disconnected" << std::endl;
            players.erase(it);
            tcpServer.closeClientSocket(tcpSocket);
            
            // Check if we need to end the game
            if (gameRunning && players.size() < 2) {
                std::cout << "Not enough players, ending game" << std::endl;
                gameRunning = false;
            }
        }
    }
    
    void handleTCPClient(int clientSocket) {
        char buffer[1024];
        
        while (serverRunning) {
            ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
            
            if (bytesReceived <= 0) {
                removePlayer(clientSocket);
                break;
            }
            
            buffer[bytesReceived] = '\0';
            processTCPMessage(clientSocket, buffer, bytesReceived);
        }
    }
    
    void processTCPMessage(int clientSocket, const char* message, size_t length) {
        if (length < 1) return;
        
        MessageType type = static_cast<MessageType>(message[0]);
        std::cout << "Received TCP message type: " << static_cast<int>(type) << " from socket " << clientSocket << std::endl;
        
        switch (type) {
            case MessageType::PLAYER_JOIN: {
                if (length < 2) {  // Need at least message type + 1 character for port
                    std::cout << "Invalid PLAYER_JOIN message length: " << length << std::endl;
                    break;
                }
                
                std::lock_guard<std::mutex> lock(playersMutex);
                Player newPlayer;
                newPlayer.tcpSocket = clientSocket;
                newPlayer.id = nextPlayerId++;
                newPlayer.name = "Player" + std::to_string(newPlayer.id);
                newPlayer.isReady = false;
                newPlayer.x = 0.0f;
                newPlayer.y = 0.0f;
                
                // Extract UDP port from message (simple format: type + port as string)
                std::string portStr(message + 1, length - 1);
                try {
                    newPlayer.udpPort = std::stoi(portStr);
                } catch (const std::exception& e) {
                    std::cout << "Failed to parse UDP port from: '" << portStr << "'" << std::endl;
                    newPlayer.udpPort = 9000;  // Default fallback
                }
                newPlayer.udpIP = "127.0.0.1"; // Assume localhost for demo
                
                players[clientSocket] = newPlayer;
                
                std::cout << "Player " << newPlayer.name << " joined (UDP port: " 
                          << newPlayer.udpPort << ")" << std::endl;
                
                // Send confirmation
                std::string response = "JOINED:" + std::to_string(newPlayer.id);
                send(clientSocket, response.c_str(), response.length(), 0);
                break;
            }
            
            case MessageType::PLAYER_READY: {
                std::lock_guard<std::mutex> lock(playersMutex);
                auto it = players.find(clientSocket);
                if (it != players.end()) {
                    it->second.isReady = true;
                    std::cout << "Player " << it->second.name << " is ready!" << std::endl;
                    
                    // Debug: Show all player states
                    std::cout << "Current player states:" << std::endl;
                    for (const auto& p : players) {
                        std::cout << "  " << p.second.name << " (ready: " << (p.second.isReady ? "YES" : "NO") << ")" << std::endl;
                    }
                    
                    // Check if all players are ready
                    bool allReady = areAllPlayersReady();
                    std::cout << "All players ready: " << (allReady ? "YES" : "NO") << ", Player count: " << players.size() << std::endl;
                    
                    if (allReady && players.size() >= 2) {
                        std::cout << "Starting game now!" << std::endl;
                        startGame();
                    } else {
                        std::cout << "Waiting for more players or players to be ready..." << std::endl;
                    }
                }
                break;
            }
            
            default:
                std::cout << "Unknown TCP message type: " << static_cast<int>(type) << std::endl;
                break;
        }
    }
    
    void handleUDPMessages() {
        char buffer[1024];
        sockaddr_in senderAddr;
        
        while (serverRunning) {
            if (udpSocket.receiveFrom(buffer, sizeof(buffer), senderAddr)) {
                if (gameRunning) {
                    processUDPMessage(buffer, senderAddr);
                }
            }
        }
    }
    
    void processUDPMessage(const char* message, const sockaddr_in& senderAddr) {
        if (!message) return;
        
        MessageType type = static_cast<MessageType>(message[0]);
        
        switch (type) {
            case MessageType::PLAYER_POSITION: {
                // Format: type + playerId + x + y (simple binary format)
                if (strlen(message) >= 13) {
                    int playerId = *reinterpret_cast<const int*>(message + 1);
                    float x = *reinterpret_cast<const float*>(message + 5);
                    float y = *reinterpret_cast<const float*>(message + 9);
                    
                    // Update player position and broadcast to others
                    updatePlayerPosition(playerId, x, y);
                    broadcastPlayerPosition(playerId, x, y);
                }
                break;
            }
            
            case MessageType::PLAYER_SHOOT: {
                // Broadcast shoot event to all players
                std::lock_guard<std::mutex> lock(playersMutex);
                for (const auto& pair : players) {
                    const Player& player = pair.second;
                    udpSocket.sendTo(message, strlen(message), player.udpIP, player.udpPort);
                }
                break;
            }
            
            default:
                break;
        }
    }
    
    bool areAllPlayersReady() {
        for (const auto& pair : players) {
            if (!pair.second.isReady) {
                return false;
            }
        }
        return !players.empty();
    }
    
    void startGame() {
        if (gameRunning) {
            std::cout << "Game is already running!" << std::endl;
            return;
        }
        
        gameRunning = true;
        std::cout << "=== GAME STARTING with " << players.size() << " players! ===" << std::endl;
        
        // Notify all players via TCP that game is starting
        char gameStartMsg[1] = {static_cast<char>(MessageType::GAME_START)};
        for (const auto& pair : players) {
            send(pair.first, gameStartMsg, 1, 0);
            std::cout << "Sent game start message to " << pair.second.name << std::endl;
        }
        
        // Start game loop
        std::thread gameLoopThread(&GameServer::gameLoop, this);
        gameLoopThread.detach();
    }
    
    void gameLoop() {
        auto lastUpdate = std::chrono::steady_clock::now();
        const auto updateInterval = std::chrono::milliseconds(33); // ~30 FPS
        
        while (gameRunning && serverRunning) {
            auto now = std::chrono::steady_clock::now();
            
            if (now - lastUpdate >= updateInterval) {
                // Send game state updates via UDP
                broadcastGameState();
                lastUpdate = now;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    void updatePlayerPosition(int playerId, float x, float y) {
        std::lock_guard<std::mutex> lock(playersMutex);
        for (auto& pair : players) {
            if (pair.second.id == playerId) {
                pair.second.x = x;
                pair.second.y = y;
                break;
            }
        }
    }
    
    void broadcastPlayerPosition(int playerId, float x, float y) {
        char positionMsg[13];
        positionMsg[0] = static_cast<char>(MessageType::PLAYER_POSITION);
        *reinterpret_cast<int*>(positionMsg + 1) = playerId;
        *reinterpret_cast<float*>(positionMsg + 5) = x;
        *reinterpret_cast<float*>(positionMsg + 9) = y;
        
        std::lock_guard<std::mutex> lock(playersMutex);
        for (const auto& pair : players) {
            const Player& player = pair.second;
            if (player.id != playerId) { // Don't send back to sender
                udpSocket.sendTo(positionMsg, 13, player.udpIP, player.udpPort);
            }
        }
    }
    
    void broadcastGameState() {
        // Simple game state broadcast
        std::lock_guard<std::mutex> lock(playersMutex);
        for (const auto& pair : players) {
            const Player& player = pair.second;
            broadcastPlayerPosition(player.id, player.x, player.y);
        }
    }
    
    void run() {
        std::cout << "Game server starting..." << std::endl;
        std::cout << "TCP (control) on port 8080" << std::endl;
        std::cout << "UDP (game data) on port 8081" << std::endl;
        
        // Start UDP handler thread
        std::thread udpThread(&GameServer::handleUDPMessages, this);
        udpThread.detach();
        
        // Handle TCP connections in main thread
        while (serverRunning) {
            try {
                int newClient = tcpServer.acceptConnection();
                std::cout << "New client connected (socket " << newClient << ")" << std::endl;
                
                std::thread clientThread(&GameServer::handleTCPClient, this, newClient);
                clientThread.detach();
                
            } catch (const std::exception& e) {
                std::cerr << "Error accepting client: " << e.what() << std::endl;
            }
        }
    }
    
    void stop() {
        serverRunning = false;
        gameRunning = false;
    }
};

int main() {
    try {
        GameServer gameServer(8080, 8081);  // TCP port 8080, UDP port 8081
        gameServer.run();
    } catch (const std::exception& e) {
        std::cerr << "Server startup failed: " << e.what() << std::endl;
    }
    
    return 0;
}