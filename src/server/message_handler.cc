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
    // Message format: type(1) + side(1) = 2 bytes
    char gameStartMsg[2];
    gameStartMsg[0] = static_cast<char>(MessageType::GAME_START);
    
    for(const auto& player : playersInMatch){
        gameStartMsg[1] = static_cast<char>(player.side);
        if(tcpServer->sendToClient(player.tcpSocket, gameStartMsg, 2)){
            std::cout << "MessageHandler: Sent game start message to Player " << player.id 
                << " (Match: " << matchId << ", Side: " << player.side << ")" << std::endl;
        } else {
            std::cout << "MessageHandler: Failed to send game start to Player " << player.id << std::endl;
        }
    }
    
}

// UDP message handling
void MessageHandler::processUDPMessage(const char* message, const sockaddr_in& senderAddr, ssize_t packetSize){
    if(!message) 
        return;
    
    MessageType type = static_cast<MessageType>(message[0]);
    
    switch(type){
        case MessageType::PLAYER_POSITION:
            handlePlayerPosition(message, packetSize);
            break;
            
        case MessageType::PLAYER_SHOOT:
            handlePlayerShoot(message, packetSize);
            break;
            
        case MessageType::PLAYER_HEALTH:
            handlePlayerHealth(message, packetSize);
            break;
            
        default:
            std::cout << "MessageHandler: Unknown UDP message type: " << static_cast<int>(type) << std::endl;
            break;
    }
}

void MessageHandler::handlePlayerPosition(const char* message, ssize_t packetSize){
    if(packetSize < 9){
        std::cout << "MessageHandler: Invalid PLAYER_POSITION message length" << std::endl;
        std::cout << "mssg received: " << message << "length "<<strlen(message) << std::endl;
        return;
    }
    
    int playerId = *reinterpret_cast<const int*>(message + 1);
    int yPos = *reinterpret_cast<const int*>(message + 5);
    

    // Update player position
    playerManager->updatePlayerPosition(playerId, yPos);

    // The players position are sent on the match loop thread
}

void MessageHandler::handlePlayerShoot(const char* message, ssize_t packetSize){
    if(packetSize < 9){
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
        
        // Broadcast shoot action IMMEDIATELLY to other players in the match
        broadcastManager->broadcastShootAction(player->matchId, *player, targetY);
    }
}

void MessageHandler::handlePlayerHealth(const char* message, ssize_t packetSize){
    if(packetSize < 9){
        std::cout << "MessageHandler: Invalid PLAYER_HEALTH message length" << std::endl;
        return;
    }
    
    int playerId = *reinterpret_cast<const int*>(message + 1);
    int health = *reinterpret_cast<const int*>(message + 5);
    
    // Find player to get match ID and current health
    Player* player = playerManager->findInGamePlayerById(playerId);
    if(player && player->matchId != -1 && player->isAlive){
        // Update player health using
        playerManager->updatePlayerHealth(playerId, health);
        
        // Get updated player state
        player = playerManager->findInGamePlayerById(playerId);
        if(player){
            // Broadcast damage to all players in match
            broadcastManager->broadcastPlayerHealth(player->matchId, *player, player->health);
            
            // Check if player died
            if(!player->isAlive){
                std::cout << "MessageHandler: Player " << playerId << " died in match " << player->matchId << std::endl;
                
                // Broadcast death
                broadcastManager->broadcastPlayerDeath(player->matchId, *player);
                
                // Find winner and end match
                auto winner = matchManager->findWinnerInMatch(player->matchId, playerId);
                if(winner){
                    matchManager->endMatchWithWinner(player->matchId, *winner);
                }
            }
        }
    }
}