#include "server.hh"
#include <iostream>
#include <thread>

GameServer::GameServer(int tcpPort, int udpPort)
    : tcpServer(tcpPort), udpSocket(udpPort), serverRunning(true),
      broadcastManager(&udpSocket, &tcpServer, &playerManager),
      matchManager(&playerManager, &broadcastManager),
      messageHandler(&playerManager, &matchManager, &broadcastManager, &tcpServer)
      {}

void GameServer::removePlayer(int tcpSocket){
    // Get player before removing to check if they're in a match
    Player* player = nullptr;
    
    // Check both available and in-game players
    player = playerManager.findPlayerBySocket(tcpSocket);

    if(player)
        std::cout << "GAMESERVER: Player matchId: " << player->matchId << std::endl;
    else
        std::cout << "Player not found!!!!!!" << std::endl;
    
    // If player was in a match, end it
    if(player && player->matchId != -1){
        int otherId = matchManager.findMatch(player->matchId)->getOtherPlayer(player->id);
        auto other = playerManager.findInGamePlayerById(otherId);
        matchManager.endMatchWithWinner(player->matchId, *other, player->id);
    }

    // Remove player using PlayerManager
    playerManager.removePlayer(tcpSocket);
    
    tcpServer.closeClientSocket(tcpSocket);
}

void GameServer::handleTCPClient(int clientSocket){
    char buffer[1024];
    
    while(serverRunning){
        ssize_t bytesReceived = tcpServer.receiveFromClient(clientSocket, buffer, sizeof(buffer)-1);
        
        if(bytesReceived <= 0){
            removePlayer(clientSocket);
            break;
        }
        
        buffer[bytesReceived] = '\0';
        messageHandler.processTCPMessage(clientSocket, buffer, bytesReceived);
    }
}

void GameServer::handleUDPMessages(){
    char buffer[1024];
    sockaddr_in senderAddr;
    
    while(serverRunning){
        ssize_t packetSize = udpSocket.receiveFrom(buffer, sizeof(buffer), senderAddr);
        if(packetSize > 0){
            messageHandler.processUDPMessage(buffer, senderAddr, packetSize);
        }
    }
}

void GameServer::run(){
    std::cout << "Game server starting..." << std::endl;
    std::cout << "TCP on port 8080" << std::endl;
    std::cout << "UDP on port 8081" << std::endl;
    
    // Start UDP handler thread
    std::thread udpThread(&GameServer::handleUDPMessages, this);
    udpThread.detach();
    
    // Handle TCP connections in main thread
    while(serverRunning){
        try {
            int newClient = tcpServer.acceptConnection();
            std::cout << "New client connected (socket " << newClient << ")" << std::endl;
            
            std::thread clientThread(&GameServer::handleTCPClient, this, newClient);
            clientThread.detach();
            
        } 
        catch(const std::exception& e){
            std::cerr << "Error accepting client: " << e.what()<< std::endl;
        }
    }
}

void GameServer::stop(){
    serverRunning = false;
}

int main(){
    try {
        GameServer gameServer(8080, 8081);
        gameServer.run();
    } catch(const std::exception& e){
        std::cerr << "Server startup failed: " << e.what()<< std::endl;
    }
    
    return 0;
}