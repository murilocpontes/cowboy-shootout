#ifndef MATCH_HH
#define MATCH_HH

#include <thread>
#include <mutex>
#include <atomic>


class Match {
public:
    int matchId;
    int player1Id;
    int player2Id;
    std::atomic<bool> gameRunning;
    int winnerId;
    std::thread gameThread;
    
    Match(int id, int p1, int p2);
    
    ~Match();
    
    int getOtherPlayer(int playerId) const;
    
    bool hasPlayer(int playerId) const;
};

#endif