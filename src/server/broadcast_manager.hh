#ifndef BROADCAST_MANAGER_HH
#define BROADCAST_MANAGER_HH

#include "../sockets/udp.hh"

class PlayerManager;

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
    void broadcastPlayerPosition(int matchId, int playerId, int yPos);
    void broadcastShootAction(int matchId, int shooterId, int targetY);
    void broadcastPlayerDamage(int matchId, int playerId, int damage, int newHealth);
    void broadcastPlayerDeath(int matchId, int playerId);
    //void broadcastGameStart(int matchId, int playerId, bool playerSide, int enemyId);
    void broadcastGameEnd(int matchId, int winnerId);
};

#endif