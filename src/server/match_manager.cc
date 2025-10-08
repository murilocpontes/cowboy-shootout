#include "match_manager.hh"
#include "broadcast_manager.hh"
#include "sockets/packets.hh"

#include <iostream>


bool MatchManager::tryCreateMatch(int& matchId){
    // Get ready players
    auto readyPlayers = playerManager->getReadyPlayers();
    
    if(readyPlayers.size() < 2){
        std::cout << "MatchManager: Not enough ready players for a match" << std::endl;
        return false;
    }
    
    // Get first two ready players
    Player player1 = readyPlayers[0];
    Player player2 = readyPlayers[1];

    // Create match
    matchId = nextMatchId++;

    auto match = std::make_unique<Match>(matchId, player1.id, player2.id);
    
    // Move players to in-game
    playerManager->movePlayerToGame(player1.tcpSocket, matchId, 0);
    playerManager->movePlayerToGame(player2.tcpSocket, matchId, 1);
    
    // Store match
{
        std::lock_guard<std::mutex> lock(matchesMutex);
        activeMatches[matchId] = std::move(match);
    }
    
    std::cout << "MatchManager: Created match " << matchId << " between Player" 
              << player1.id << " and Player" << player2.id << std::endl;
    
    // Start the match
    startMatch(matchId);
    return true;
}

void MatchManager::startMatch(int matchId){
    std::lock_guard<std::mutex> lock(matchesMutex);
    auto matchIt = activeMatches.find(matchId);
    if(matchIt == activeMatches.end())
        return;
    
    Match& match = *matchIt->second;
    match.gameRunning = true;
    
    std::cout << "MatchManager: === STARTING MATCH " << matchId << " ===" << std::endl;
    
    // Start game loop for this match
    match.gameThread = std::thread(&MatchManager::runMatchGameLoop, this, matchId);
    match.gameThread.detach();
}

void MatchManager::endMatch(int matchId){
{
        std::lock_guard<std::mutex> lock(matchesMutex);
        auto matchIt = activeMatches.find(matchId);
        if(matchIt == activeMatches.end()) return;
        
        matchIt->second->gameRunning = false;
    }
    
    std::cout << "MatchManager: === ENDING MATCH " << matchId << " ===" << std::endl;
    
    // Move players back to available using PlayerManager
    auto playersInMatch = playerManager->getPlayersInMatch(matchId);
    for(const auto& player : playersInMatch){
        playerManager->movePlayerToAvailable(player.tcpSocket);
    }
    
    // Remove the match
{
        std::lock_guard<std::mutex> lock(matchesMutex);
        activeMatches.erase(matchId);
    }
}

void MatchManager::endMatchWithWinner(int matchId, Player winner, int excludePlayerId){
{
        std::lock_guard<std::mutex> lock(matchesMutex);
        auto matchIt = activeMatches.find(matchId);
        if(matchIt == activeMatches.end()) return;
        
        matchIt->second->gameRunning = false;
        matchIt->second->winnerId = winner.id;
    }
    
    std::cout << "MatchManager: === MATCH " << matchId << " ENDED - WINNER: Player " << winner.id << " ===" << std::endl;
    
    // Broadcast game end to all players
    broadcastManager->broadcastGameEnd(matchId, winner, excludePlayerId);
    
    // End the match
    endMatch(matchId);
}

Match* MatchManager::findMatch(int matchId){
    std::lock_guard<std::mutex> lock(matchesMutex);
    auto it = activeMatches.find(matchId);
    return (it != activeMatches.end()) ? it->second.get() : nullptr;
}

int MatchManager::findPlayerMatch(int playerId){
    Player* player = playerManager->findInGamePlayerById(playerId);
    return player ? player->matchId : -1;
}

Player* MatchManager::findWinnerInMatch(int matchId, int deadPlayerId){
    std::lock_guard<std::mutex> lock(matchesMutex);
    auto matchIt = activeMatches.find(matchId);
    if(matchIt != activeMatches.end()){
        Match& match = *matchIt->second;
        return playerManager->findInGamePlayerById(match.getOtherPlayer(deadPlayerId));
    }
    return nullptr;
}

void MatchManager::runMatchGameLoop(int matchId){
    const auto targetFrameTime = std::chrono::microseconds(1000000/60);
    auto nextFrameTime = std::chrono::steady_clock::now() + targetFrameTime;
    
    while(true){
    {
            std::lock_guard<std::mutex> lock(matchesMutex);
            auto matchIt = activeMatches.find(matchId);
            if(matchIt == activeMatches.end() || !matchIt->second->gameRunning){
                break;
            }
        }
        
        // Broadcast position updates for all players in this match
        auto playersInMatch = playerManager->getPlayersInMatch(matchId);
        for(const auto& player : playersInMatch){
            broadcastManager->broadcastPlayerPosition(matchId, player);
        }
        
        std::this_thread::sleep_until(nextFrameTime);
        nextFrameTime += targetFrameTime;
    }
}