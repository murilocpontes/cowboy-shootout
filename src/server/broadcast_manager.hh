#ifndef BROADCAST_MANAGER_HH
#define BROADCAST_MANAGER_HH

#include "../sockets/udp.hh"

class PlayerManager;
class Player;

class BroadcastManager {
private:
    UDP* udpSocket;
    PlayerManager* playerManager;

public:
    BroadcastManager(UDP* udp, PlayerManager* pm) 
        : udpSocket(udp), playerManager(pm) {}
    
    // Core broadcast function
    void broadcastToMatch(int matchId, const char* message, size_t length, int excludePlayerId = -1);
    
    // Specific broadcasts
    void broadcastPlayerPosition(int matchId, Player player);
    void broadcastShootAction(int matchId, Player player, int targetY);
    void broadcastPlayerDamage(int matchId, Player player, int damage, int newHealth);
    void broadcastPlayerDeath(int matchId, Player player);
    void broadcastGameEnd(int matchId, Player winner);
};

#endif