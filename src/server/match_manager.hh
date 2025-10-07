#ifndef MATCH_MANAGER_HH
#define MATCH_MANAGER_HH

#include "game/match.hh"
#include "player_manager.hh"
#include <map>
#include <mutex>
#include <memory>

class BroadcastManager; // Forward declaration

class MatchManager {
private:
    std::map<int, std::unique_ptr<Match>> activeMatches;
    std::mutex matchesMutex;
    int nextMatchId;
    
    PlayerManager* playerManager;
    BroadcastManager* broadcastManager;

public:
    MatchManager(PlayerManager* pm, BroadcastManager* bm) 
        : nextMatchId(1), playerManager(pm), broadcastManager(bm) {}
    
    // Match lifecycle
    bool tryCreateMatch(int& matchId);
    void startMatch(int matchId);
    void endMatch(int matchId);
    void endMatchWithWinner(int matchId, Player winner);
    
    // Match queries
    Match* findMatch(int matchId);
    int findPlayerMatch(int playerId);
    Player* findWinnerInMatch(int matchId, int deadPlayerId);
    
    // Game loop
    void runMatchGameLoop(int matchId);
};

#endif