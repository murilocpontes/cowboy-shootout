#ifndef BROADCAST_MANAGER_HH
#define BROADCAST_MANAGER_HH

#include "../sockets/udp.hh"
#include "../sockets/tcp.hh"

class PlayerManager;
class Player;

class BroadcastManager {
private:
    UDP* udpSocket;
    TCPServer* tcpServer;
    PlayerManager* playerManager;

public:
    BroadcastManager(UDP* udp, TCPServer* tcp, PlayerManager* pm) 
        : udpSocket(udp), tcpServer(tcp), playerManager(pm) {}
    
    // Core broadcast function
    void broadcastToMatch(int matchId, const char* message, size_t length, int excludePlayerId = -1);
    
    // Specific broadcasts
    void broadcastPlayerPosition(int matchId, Player player);
    void broadcastShootAction(int matchId, Player player, int targetY);
    void broadcastPlayerHealth(int matchId, Player player, int newHealth);
    void broadcastPlayerDeath(int matchId, Player player);
    void broadcastGameEnd(int matchId, Player winner, int excludePlayerId = -1);
};

#endif