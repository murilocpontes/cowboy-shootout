#include "message_handler.hh"
#include "player_manager.hh"
#include "match_manager.hh"
#include "broadcast_manager.hh"
#include "../sockets/tcp.hh"
#include <iostream>
#include <string>
#include <cstring>

// TCP message handling
void MessageHandler::processTCPMessage(int clientSocket, const char* message, size_t length){
    if(length < 1) return;
    
    MessageType type = static_cast<MessageType>(message[0]);
    std::cout << "MessageHandler: Received TCP message type: " << static_cast<int>(type) 
              << " from socket " << clientSocket << std::endl;
    
    switch(type){
        case MessageType::PLAYER_JOIN:
            handlePlayerJoin(clientSocket, message, length);
            break;
            
        case MessageType::PLAYER_READY:
            handlePlayerReady(clientSocket);
            break;
            
        default:
            std::cout << "MessageHandler: Unknown TCP message type: " << static_cast<int>(type) << std::endl;
            break;
    }
}

void MessageHandler::handlePlayerJoin(int clientSocket, const char* message, size_t length){
    if(length < 2){
        std::cout << "MessageHandler: Invalid PLAYER_JOIN message length: " << length << std::endl;
        return;
    }
    
    // Extract UDP port from message
    std::string portStr(message + 1, length - 1);
    int udpPort;
    
    try{
        udpPort = std::stoi(portStr);
    } 
    catch(const std::exception& e){
        std::cout << "MessageHandler: Failed to parse UDP port from: '" << portStr << "'" << std::endl;
        udpPort = 9000; // Default port
    }
    
    // Add player
    int playerId = playerManager->addPlayer(clientSocket, "127.0.0.1", udpPort);
    
    // Send confirmation
    std::string response = "JOINED:" + std::to_string(playerId);
    tcpServer->sendToClient(clientSocket, response.c_str(), response.length());
    
    std::cout << "MessageHandler: Player " << playerId << " joined successfully" << std::endl;
}

void MessageHandler::handlePlayerReady(int clientSocket){
    if(playerManager->setPlayerReady(clientSocket)){
        int matchId = -1;
        bool matchStarted = matchManager->tryCreateMatch(matchId);
        if(matchStarted){
            handleMatchStart(clientSocket, matchId);
        }
    } else {
        std::cout << "MessageHandler: Failed to set player ready for socket " << clientSocket << std::endl;
    }
}

void MessageHandler::handleMatchStart(int clientSocket, int matchId){
    // Notify players that their match is starting
    auto playersInMatch = playerManager->getPlayersInMatch(matchId);
    // Create TCP message for game start
    // Message format: type(1) + opponentId(4) = 5 bytes
    char gameStartMsg[5];
    gameStartMsg[0] = static_cast<char>(MessageType::GAME_START);
    
    for(const auto& player : playersInMatch){
        int opponentId = -1;
        for(const auto& otherPlayer : playersInMatch){
            if(otherPlayer.id != player.id){
                opponentId = otherPlayer.id;
                break;
            }
        }

        *reinterpret_cast<int*>(gameStartMsg + 1) = opponentId;
        if(tcpServer->sendToClient(player.tcpSocket, gameStartMsg, 1)){
            std::cout << "MessageHandler: Sent game start message to Player " << player.id 
                << " (Match: " << matchId << ", Opponent: " << opponentId << ")" << std::endl;
        } else {
            std::cout << "MessageHandler: Failed to send game start to Player " << player.id << std::endl;
        }
    }
    
}

// UDP message handling
void MessageHandler::processUDPMessage(const char* message, const sockaddr_in& senderAddr){
    if(!message) 
        return;
    
    MessageType type = static_cast<MessageType>(message[0]);
    
    switch(type){
        case MessageType::PLAYER_POSITION:
            handlePlayerPosition(message);
            break;
            
        case MessageType::PLAYER_SHOOT:
            handlePlayerShoot(message);
            break;
            
        case MessageType::PLAYER_DAMAGE:
            handlePlayerDamage(message);
            break;
            
        default:
            std::cout << "MessageHandler: Unknown UDP message type: " << static_cast<int>(type) << std::endl;
            break;
    }
}

void MessageHandler::handlePlayerPosition(const char* message){
    if(strlen(message) < 9){
        std::cout << "MessageHandler: Invalid PLAYER_POSITION message length" << std::endl;
        return;
    }
    
    int playerId = *reinterpret_cast<const int*>(message + 1);
    int yPos = *reinterpret_cast<const int*>(message + 5);
    
    // Update player position
    playerManager->updatePlayerPosition(playerId, yPos);
    
    // Find player to get match ID
    Player* player = playerManager->findInGamePlayerById(playerId);
    if(player && player->matchId != -1){
        // Broadcast position to other players in the match
        broadcastManager->broadcastPlayerPosition(player->matchId, playerId, yPos);
    }
}

void MessageHandler::handlePlayerShoot(const char* message){
    if(strlen(message) < 9){
        std::cout << "MessageHandler: Invalid PLAYER_SHOOT message length" << std::endl;
        return;
    }
    
    int shooterId = *reinterpret_cast<const int*>(message + 1);
    int targetY = *reinterpret_cast<const int*>(message + 5);
    
    // Find player to get match ID
    Player* player = playerManager->findInGamePlayerById(shooterId);
    if(player && player->matchId != -1 && player->isAlive){
        std::cout << "MessageHandler: Player " << shooterId << " shot at Y=" << targetY 
                  << " in match " << player->matchId << std::endl;
        
        // Broadcast shoot action to other players in the match
        broadcastManager->broadcastShootAction(player->matchId, shooterId, targetY);
    }
}

void MessageHandler::handlePlayerDamage(const char* message){
    if(strlen(message) < 9){
        std::cout << "MessageHandler: Invalid PLAYER_DAMAGE message length" << std::endl;
        return;
    }
    
    int playerId = *reinterpret_cast<const int*>(message + 1);
    int damage = *reinterpret_cast<const int*>(message + 5);
    
    // Find player to get match ID and current health
    Player* player = playerManager->findInGamePlayerById(playerId);
    if(player && player->matchId != -1 && player->isAlive){
        int oldHealth = player->health;
        
        // Update player health using
        playerManager->updatePlayerHealth(playerId, damage);
        
        // Get updated player state
        player = playerManager->findInGamePlayerById(playerId);
        if(player){
            // Broadcast damage to all players in match
            broadcastManager->broadcastPlayerDamage(player->matchId, playerId, damage, player->health);
            
            // Check if player died
            if(!player->isAlive){
                std::cout << "MessageHandler: Player " << playerId << " died in match " << player->matchId << std::endl;
                
                // Broadcast death
                broadcastManager->broadcastPlayerDeath(player->matchId, playerId);
                
                // Find winner and end match
                int winnerId = matchManager->findWinnerInMatch(player->matchId, playerId);
                if(winnerId != -1){
                    matchManager->endMatchWithWinner(player->matchId, winnerId);
                }
            }
        }
    }
}