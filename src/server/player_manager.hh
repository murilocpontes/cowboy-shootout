#ifndef PLAYER_MANAGER_HH
#define PLAYER_MANAGER_HH

#include "game/player.hh"
#include <map>
#include <mutex>
#include <vector>

class PlayerManager {
private:
    std::map<int, Player> availablePlayers;
    std::map<int, Player> inGamePlayers;
    std::mutex availablePlayersMutex;
    std::mutex inGamePlayersMutex;
    int nextPlayerId;

public:
    PlayerManager() : nextPlayerId(1) {}
    
    // Player lifecycle
    int addPlayer(int tcpSocket, const std::string& udpIP, int udpPort);
    void removePlayer(int tcpSocket);
    bool setPlayerReady(int tcpSocket);
    
    // Player state
    void movePlayerToGame(int tcpSocket, int matchId, bool side);
    void movePlayerToAvailable(int tcpSocket);
    void updatePlayerPosition(int playerId, int yPos);
    void updatePlayerHealth(int playerId, int health);
    
    // Queries
    std::vector<Player> getReadyPlayers();
    Player* findPlayerById(int playerId);
    Player* findInGamePlayerById(int playerId);
    std::vector<Player> getPlayersInMatch(int matchId);
};

#endif