#include "sockets/tcp.hh"
#include "sockets/udp.hh"
#include <iostream>
#include <thread>
#include <string>
#include <atomic>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

// Same message types as server
enum class MessageType : uint8_t {
    PLAYER_JOIN = 1,
    PLAYER_READY = 2,
    GAME_START = 3,
    PLAYER_POSITION = 4,
    PLAYER_SHOOT = 5
};

class GameClient {
private:
    TCPClient tcpClient;
    UDP udpSocket;
    std::atomic<bool> connected;
    std::atomic<bool> gameActive;
    int playerId;
    float playerX, playerY;
    int clientUDPPort;
    
public:
    GameClient(int udpPort) : udpSocket(udpPort), connected(false), gameActive(false), 
                              playerId(-1), playerX(0.0f), playerY(0.0f), clientUDPPort(udpPort) {}
    
    bool connect(const std::string& serverIP, int tcpPort, int udpPort) {
        // Connect via TCP for control messages
        if (!tcpClient.connectTo(serverIP, tcpPort)) {
            std::cerr << "Failed to connect to game server via TCP" << std::endl;
            return false;
        }
        
        connected = true;
        std::cout << "Connected to game server!" << std::endl;
        
        // Send join message with our UDP port
        std::string joinMsg = std::string(1, static_cast<char>(MessageType::PLAYER_JOIN)) + std::to_string(clientUDPPort);
        if (!tcpClient.sendData(joinMsg.c_str(), joinMsg.length())) {
            std::cerr << "Failed to send join message" << std::endl;
            return false;
        }
        
        return true;
    }
    
    void sendReady() {
        if (!connected) return;
        
        char readyMsg[1] = {static_cast<char>(MessageType::PLAYER_READY)};
        std::cout << "Sending ready message (type " << static_cast<int>(MessageType::PLAYER_READY) << ")" << std::endl;
        if (tcpClient.sendData(readyMsg, 1)) {
            std::cout << "Sent ready signal to server" << std::endl;
        } else {
            std::cout << "Failed to send ready signal" << std::endl;
        }
    }
    
    void handleTCPMessages() {
        char buffer[1024];
        
        while (connected) {
            ssize_t bytesReceived = tcpClient.receiveData(buffer, sizeof(buffer)-1);
            
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                
                if (strncmp(buffer, "JOINED:", 7) == 0) {
                    playerId = std::stoi(std::string(buffer + 7));
                    std::cout << "Assigned player ID: " << playerId << std::endl;
                }
                else if (bytesReceived >= 1 && buffer[0] == static_cast<char>(MessageType::GAME_START)) {
                    std::cout << "Game is starting!" << std::endl;
                    gameActive = true;
                }
                else {
                    std::cout << "TCP: " << buffer << std::endl;
                }
            } else {
                std::cout << "Disconnected from server" << std::endl;
                connected = false;
                break;
            }
        }
    }
    
    void handleUDPMessages() {
        char buffer[1024];
        sockaddr_in senderAddr;
        
        while (connected) {
            if (udpSocket.receiveFrom(buffer, sizeof(buffer), senderAddr)) {
                if (gameActive) {
                    processGameUpdate(buffer);
                }
            }
        }
    }
    
    void processGameUpdate(const char* message) {
        if (!message) return;
        
        MessageType type = static_cast<MessageType>(message[0]);
        
        switch (type) {
            case MessageType::PLAYER_POSITION: {
                if (strlen(message) >= 13) {
                    int otherPlayerId = *reinterpret_cast<const int*>(message + 1);
                    float x = *reinterpret_cast<const float*>(message + 5);
                    float y = *reinterpret_cast<const float*>(message + 9);
                    
                    if (otherPlayerId != playerId) {
                        std::cout << "Player " << otherPlayerId << " position: (" 
                                  << x << ", " << y << ")" << std::endl;
                    }
                }
                break;
            }
            
            case MessageType::PLAYER_SHOOT: {
                std::cout << "Someone shot!" << std::endl;
                break;
            }
            
            default:
                break;
        }
    }
    
    void sendPosition(float x, float y) {
        if (!gameActive || playerId == -1) return;
        
        playerX = x;
        playerY = y;
        
        char positionMsg[13];
        positionMsg[0] = static_cast<char>(MessageType::PLAYER_POSITION);
        *reinterpret_cast<int*>(positionMsg + 1) = playerId;
        *reinterpret_cast<float*>(positionMsg + 5) = x;
        *reinterpret_cast<float*>(positionMsg + 9) = y;
        
        udpSocket.sendTo(positionMsg, 13, "127.0.0.1", 8081);
    }
    
    void sendShoot() {
        if (!gameActive) return;
        
        char shootMsg[1] = {static_cast<char>(MessageType::PLAYER_SHOOT)};
        udpSocket.sendTo(shootMsg, 1, "127.0.0.1", 8081);
        std::cout << "Shot fired!" << std::endl;
    }
    
    void gameLoop() {
        float x = 0.0f, y = 0.0f;
        
        while (connected) {
            if (gameActive) {
                // Simulate movement
                x += 0.1f;
                y += 0.05f;
                sendPosition(x, y);
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void run() {
        // Start message handling threads
        std::thread tcpThread(&GameClient::handleTCPMessages, this);
        std::thread udpThread(&GameClient::handleUDPMessages, this);
        std::thread gameThread(&GameClient::gameLoop, this);
        
        tcpThread.detach();
        udpThread.detach();
        gameThread.detach();
        
        // Simple command interface
        std::string command;
        std::cout << "Commands: 'ready' to signal ready, 'shoot' to shoot, 'quit' to exit" << std::endl;
        
        while (connected && std::getline(std::cin, command)) {
            if (command == "ready") {
                sendReady();
            }
            else if (command == "shoot") {
                sendShoot();
            }
            else if (command == "quit") {
                connected = false;
                break;
            }
            else {
                std::cout << "Unknown command. Use: ready, shoot, quit" << std::endl;
            }
        }
    }
};

int main() {
    try {
        // Seed random number generator
        srand(time(nullptr) + getpid());
        
        // Use a different UDP port for each client instance
        // Port range 9000-9100 should be enough for testing
        int udpPort = 9000 + (rand() % 100);
        std::cout << "Client using UDP port: " << udpPort << std::endl;
        
        GameClient client(udpPort);
        
        if (!client.connect("127.0.0.1", 8080, udpPort)) {
            return 1;
        }
        
        client.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
    
    return 0;
}