#include "broadcast_manager.hh"
#include "player_manager.hh"
#include "../sockets/packets.hh"
#include <iostream>

void BroadcastManager::broadcastToMatch(int matchId, const char* message, size_t length, int excludePlayerId) {
    auto playersInMatch = playerManager->getPlayersInMatch(matchId);
    
    for(const auto& player : playersInMatch) {
        if(player.id != excludePlayerId) {
            udpSocket->sendTo(message, length, player.udpIP, player.udpPort);
        }
    }
}

void BroadcastManager::broadcastPlayerPosition(int matchId, int playerId, int yPos) {
    char positionMsg[9];
    positionMsg[0] = static_cast<char>(MessageType::PLAYER_POSITION);
    *reinterpret_cast<int*>(positionMsg + 1) = playerId;
    *reinterpret_cast<int*>(positionMsg + 5) = yPos;
    
    broadcastToMatch(matchId, positionMsg, 9, playerId); // Exclude sender
}

void BroadcastManager::broadcastShootAction(int matchId, int shooterId, int targetY) {
    char shootMsg[9];
    shootMsg[0] = static_cast<char>(MessageType::PLAYER_SHOOT);
    *reinterpret_cast<int*>(shootMsg + 1) = shooterId;
    *reinterpret_cast<int*>(shootMsg + 5) = targetY;
    
    std::cout << "BroadcastManager: Broadcasting shoot action: Player " << shooterId 
              << " shot at Y=" << targetY << " in match " << matchId << std::endl;
    
    broadcastToMatch(matchId, shootMsg, 9, shooterId); // Exclude shooter
}

void BroadcastManager::broadcastPlayerDamage(int matchId, int playerId, int damage, int newHealth) {
    char damageMsg[13];
    damageMsg[0] = static_cast<char>(MessageType::PLAYER_DAMAGE);
    *reinterpret_cast<int*>(damageMsg + 1) = playerId;
    *reinterpret_cast<int*>(damageMsg + 5) = damage;
    *reinterpret_cast<int*>(damageMsg + 9) = newHealth;
    
    std::cout << "BroadcastManager: Broadcasting damage: Player " << playerId 
              << " took " << damage << " damage, health: " << newHealth << std::endl;
    
    broadcastToMatch(matchId, damageMsg, 13); // Send to all players
}

void BroadcastManager::broadcastPlayerDeath(int matchId, int playerId) {
    char deathMsg[5];
    deathMsg[0] = static_cast<char>(MessageType::PLAYER_DEATH);
    *reinterpret_cast<int*>(deathMsg + 1) = playerId;
    
    std::cout << "BroadcastManager: Broadcasting death: Player " << playerId << " died" << std::endl;
    
    broadcastToMatch(matchId, deathMsg, 5); // Send to all players
}

void BroadcastManager::broadcastGameEnd(int matchId, int winnerId) {
    char gameEndMsg[5];
    gameEndMsg[0] = static_cast<char>(MessageType::GAME_END);
    *reinterpret_cast<int*>(gameEndMsg + 1) = winnerId;
    
    std::cout << "BroadcastManager: Broadcasting game end: Player " << winnerId 
              << " wins match " << matchId << std::endl;
    
    broadcastToMatch(matchId, gameEndMsg, 5); // Send to all players
}