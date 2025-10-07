#include "broadcast_manager.hh"
#include "player_manager.hh"
#include "../sockets/packets.hh"
#include <iostream>


void BroadcastManager::broadcastToMatch(int matchId, const char* message, size_t length, int excludePlayerId){
    auto playersInMatch = playerManager->getPlayersInMatch(matchId);
    
    for(const auto& player : playersInMatch){
        if(player.id != excludePlayerId){
            udpSocket->sendTo(message, length, player.udpIP, player.udpPort);
        }
    }
}

void BroadcastManager::broadcastPlayerPosition(int matchId, Player player){
    char positionMsg[6];

    positionMsg[0] = static_cast<char>(MessageType::PLAYER_POSITION);
    positionMsg[1] = static_cast<char>(player.side);
    *reinterpret_cast<int*>(positionMsg + 2) = player.yPos;
    
    //std::cout << "BroadcastManager: Broadcasting Position: Player " << player.id
    //          << " at yPos " << player.yPos << " in match " << matchId << std::endl;

    broadcastToMatch(matchId, positionMsg, 6, player.id); // Exclude sender

}

void BroadcastManager::broadcastShootAction(int matchId, Player player, int targetY){
    char shootMsg[9];
    shootMsg[0] = static_cast<char>(MessageType::PLAYER_SHOOT);
    shootMsg[1] = static_cast<char>(player.side);
    *reinterpret_cast<int*>(shootMsg + 2) = targetY;
    
    //std::cout << "BroadcastManager: Broadcasting shoot action: Player " << player.id 
    //          << " shot at Y=" << targetY << " in match " << matchId << std::endl;
    
    broadcastToMatch(matchId, shootMsg, 6, player.id); // Exclude shooter
}

void BroadcastManager::broadcastPlayerHealth(int matchId, Player player, int newHealth){
    char healthMsg[6];
    healthMsg[0] = static_cast<char>(MessageType::PLAYER_HEALTH);
    healthMsg[1] = static_cast<char>(player.side);
    *reinterpret_cast<int*>(healthMsg + 2) = newHealth;

    //std::cout << "BroadcastManager: Broadcasting damage: Player " << player.id 
    //          << " took damage! Remaining health: " << newHealth << std::endl;
    
    broadcastToMatch(matchId, healthMsg, 6, player.id); // Send to all (health control on server)
}

void BroadcastManager::broadcastPlayerDeath(int matchId, Player player){
    char deathMsg[2];
    deathMsg[0] = static_cast<char>(MessageType::PLAYER_DEATH);
    deathMsg[1] = static_cast<char>(player.side);
    
    std::cout << "BroadcastManager: Broadcasting death: Player " << player.id << " died" << std::endl;
    
    broadcastToMatch(matchId, deathMsg, 2); // Send to all players
}

void BroadcastManager::broadcastGameEnd(int matchId, Player winner){
    char gameEndMsg[2];
    gameEndMsg[0] = static_cast<char>(MessageType::GAME_END);
    gameEndMsg[1] = static_cast<char>(winner.side);
    
    std::cout << "BroadcastManager: Broadcasting game end: Player " << winner.id 
              << " wins match " << matchId << std::endl;
    
    broadcastToMatch(matchId, gameEndMsg, 2); // Send to all players
}