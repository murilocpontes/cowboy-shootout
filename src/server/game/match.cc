#include "match.hh"

Match::Match(int id, int p1, int p2) 
    : matchId(id), player1Id(p1), player2Id(p2), 
        gameRunning(false), winnerId(-1) {}

Match::~Match(){
    if (gameThread.joinable()) {
        gameThread.join();
    }
}

int Match::getOtherPlayer(int playerId) const {
    return (player1Id == playerId) ? player2Id : player1Id;
}

bool Match::hasPlayer(int playerId) const {
    return (player1Id == playerId) ? player2Id : player1Id;
}
