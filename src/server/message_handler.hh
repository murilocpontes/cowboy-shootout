#ifndef MESSAGE_HANDLER_HH
#define MESSAGE_HANDLER_HH

#include "../sockets/packets.hh"
#include <netinet/in.h>

class PlayerManager;
class MatchManager;
class BroadcastManager;
class TCPServer;

class MessageHandler {
private:
    PlayerManager* playerManager;
    MatchManager* matchManager;
    BroadcastManager* broadcastManager;
    TCPServer* tcpServer;

public:
    MessageHandler(PlayerManager* pm, MatchManager* mm, BroadcastManager* bm, TCPServer* tcp)
        : playerManager(pm), matchManager(mm), broadcastManager(bm), tcpServer(tcp) {}
    
    // TCP message handling
    void processTCPMessage(int clientSocket, const char* message, size_t length);
    void handlePlayerJoin(int clientSocket, const char* message, size_t length);
    void handlePlayerReady(int clientSocket);
    void handleMatchStart(int clientSocket, int matchId);
    
    // UDP message handling
    void processUDPMessage(const char* message, const sockaddr_in& senderAddr, ssize_t packetSize);
    void handlePlayerPosition(const char* message, ssize_t packetSize);
    void handlePlayerShoot(const char* message, ssize_t packetSize);
    void handlePlayerHealth(const char* message, ssize_t packetSize);
};

#endif