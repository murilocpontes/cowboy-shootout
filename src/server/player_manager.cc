#include "player_manager.hh"
#include <iostream>
#include <algorithm>

// Player lifecycle
int PlayerManager::addPlayer(int tcpSocket, const std::string& udpIP, int udpPort){
    std::lock_guard<std::mutex> lock(availablePlayersMutex);
    
    Player newPlayer;
    newPlayer.tcpSocket = tcpSocket;
    newPlayer.id = nextPlayerId++;
    newPlayer.udpIP = udpIP;
    newPlayer.udpPort = udpPort;
    newPlayer.isReady = false;
    newPlayer.yPos = 0;
    newPlayer.matchId = -1;
    newPlayer.health = 100;
    newPlayer.isAlive = true;
    
    availablePlayers[tcpSocket] = newPlayer;
    
    std::cout << "PlayerManager: Added Player " << newPlayer.id 
              << " (socket: " << tcpSocket << ", UDP: " << udpIP << ":" << udpPort << ")" << std::endl;
    
    return newPlayer.id;
}

void PlayerManager::removePlayer(int tcpSocket){
    bool foundInAvailable = false;
    
    // Check available players first
{
        std::lock_guard<std::mutex> availableLock(availablePlayersMutex);
        auto availableIt = availablePlayers.find(tcpSocket);
        if(availableIt != availablePlayers.end()){
            std::cout << "PlayerManager: Removing Player " << availableIt->second.id 
                      << " from available players" << std::endl;
            availablePlayers.erase(availableIt);
            foundInAvailable = true;
        }
    }
    
    // Check in-game players if not found in available
    if(!foundInAvailable){
        std::lock_guard<std::mutex> inGameLock(inGamePlayersMutex);
        auto inGameIt = inGamePlayers.find(tcpSocket);
        if(inGameIt != inGamePlayers.end()){
            std::cout << "PlayerManager: Removing Player " << inGameIt->second.id 
                      << " from in-game players" << std::endl;
            inGamePlayers.erase(inGameIt);
        }
    }
}

bool PlayerManager::setPlayerReady(int tcpSocket){
    std::lock_guard<std::mutex> lock(availablePlayersMutex);
    
    auto it = availablePlayers.find(tcpSocket);
    if(it != availablePlayers.end()){
        it->second.isReady = true;
        std::cout << "PlayerManager: Player " << it->second.id << " is now ready!" << std::endl;
        return true;
    }
    
    std::cout << "PlayerManager: Failed to set ready - Player with socket " 
              << tcpSocket << " not found in available players" << std::endl;
    return false;
}

// Player state
void PlayerManager::movePlayerToGame(int tcpSocket, int matchId){
    std::lock_guard<std::mutex> availableLock(availablePlayersMutex);
    std::lock_guard<std::mutex> inGameLock(inGamePlayersMutex);
    
    auto availableIt = availablePlayers.find(tcpSocket);
    if(availableIt != availablePlayers.end()){
        Player player = availableIt->second;
        player.reset(); // Reset health and status for new match
        player.matchId = matchId;
        
        
        inGamePlayers[tcpSocket] = player;
        availablePlayers.erase(availableIt);
        
        std::cout << "PlayerManager: Moved Player " << player.id 
                  << " to in-game (Match " << matchId << ")" << std::endl;
    }
}

void PlayerManager::movePlayerToAvailable(int tcpSocket){
    std::lock_guard<std::mutex> availableLock(availablePlayersMutex);
    std::lock_guard<std::mutex> inGameLock(inGamePlayersMutex);
    
    auto inGameIt = inGamePlayers.find(tcpSocket);
    if(inGameIt != inGamePlayers.end()){
        Player player = inGameIt->second;
        player.isReady = false;  // Need to ready up again
        player.matchId = -1;
        
        availablePlayers[tcpSocket] = player;
        inGamePlayers.erase(inGameIt);
        
        std::cout << "PlayerManager: Moved Player " << player.id 
                  << " to available players" << std::endl;
    }
}

void PlayerManager::updatePlayerPosition(int playerId, int yPos){
    std::lock_guard<std::mutex> lock(inGamePlayersMutex);
    
    for(auto& pair : inGamePlayers){
        if(pair.second.id == playerId){
            pair.second.yPos = yPos;
            return;
        }
    }
}

void PlayerManager::updatePlayerHealth(int playerId, int damage){
    std::lock_guard<std::mutex> lock(inGamePlayersMutex);
    
    for(auto& pair : inGamePlayers){
        if(pair.second.id == playerId && pair.second.isAlive){
            pair.second.takeDamage(damage);
            
            std::cout << "PlayerManager: Player " << playerId << " took " << damage 
                      << " damage, health now: " << pair.second.health << std::endl;
            
            if(!pair.second.isAlive){
                std::cout << "PlayerManager: Player " << playerId << " died!" << std::endl;
            }
            return;
        }
    }
}

// Queries
std::vector<Player> PlayerManager::getReadyPlayers(){
    std::lock_guard<std::mutex> lock(availablePlayersMutex);
    
    std::vector<Player> readyPlayers;
    for(const auto& pair : availablePlayers){
        if(pair.second.isReady){
            readyPlayers.push_back(pair.second);
        }
    }
    
    return readyPlayers;
}

Player* PlayerManager::findPlayerById(int playerId){
    // Check available players first
{
        std::lock_guard<std::mutex> lock(availablePlayersMutex);
        for(auto& pair : availablePlayers){
            if(pair.second.id == playerId){
                return &pair.second;
            }
        }
    }
    
    // Check in-game players
{
        std::lock_guard<std::mutex> lock(inGamePlayersMutex);
        for(auto& pair : inGamePlayers){
            if(pair.second.id == playerId){
                return &pair.second;
            }
        }
    }
    
    return nullptr;
}

Player* PlayerManager::findInGamePlayerById(int playerId){
    std::lock_guard<std::mutex> lock(inGamePlayersMutex);
    
    for(auto& pair : inGamePlayers){
        if(pair.second.id == playerId){
            return &pair.second;
        }
    }
    
    return nullptr;
}

std::vector<Player> PlayerManager::getPlayersInMatch(int matchId){
    std::lock_guard<std::mutex> lock(inGamePlayersMutex);
    
    std::vector<Player> playersInMatch;
    for(const auto& pair : inGamePlayers){
        if(pair.second.matchId == matchId){
            playersInMatch.push_back(pair.second);
        }
    }
    
    return playersInMatch;
}