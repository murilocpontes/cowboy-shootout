#ifndef SERVER_HH
#define SERVER_HH

#include "sockets/tcp.hh"
#include "sockets/udp.hh"
#include "player_manager.hh"
#include "match_manager.hh"
#include "message_handler.hh"
#include "broadcast_manager.hh"
#include <atomic>

class GameServer {
private:
    TCPServer tcpServer;
    UDP udpSocket;
    
    // Managers (composition over inheritance)
    PlayerManager playerManager;
    BroadcastManager broadcastManager;
    MatchManager matchManager;
    MessageHandler messageHandler;
    
    std::atomic<bool> serverRunning;

public:
    GameServer(int tcpPort, int udpPort);
    
    void run();
    void stop();
    
private:
    void handleTCPClient(int clientSocket);
    void handleUDPMessages();
    void removePlayer(int tcpSocket);
};

#endif