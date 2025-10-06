#include "tcp.hh"
#include <stdexcept>
#include <cstring>

// --- TCPServer Implementation ---
TCPServer::TCPServer(int port){
    _listenSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(_listenSocketFd < 0){
        throw std::runtime_error("Failed to create TCP server socket: " + std::string(strerror(errno)));
    }

    int opt = 1;
    if(setsockopt(_listenSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        close(_listenSocketFd);
        throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if(bind(_listenSocketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        close(_listenSocketFd);
        throw std::runtime_error("Failed to bind TCP server socket to port " + std::to_string(port) + ": " + std::string(strerror(errno)));
    }

    if(listen(_listenSocketFd, 5) < 0){
        close(_listenSocketFd);
        throw std::runtime_error("Failed to listen on TCP server socket: " + std::string(strerror(errno)));
    }
}

TCPServer::~TCPServer(){
    if(_listenSocketFd >= 0){
        close(_listenSocketFd);
    }
}

int TCPServer::acceptConnection(){
    int clientSocket = accept(_listenSocketFd, nullptr, nullptr);
    if(clientSocket < 0){
        throw std::runtime_error("Failed to accept client connection: " + std::string(strerror(errno)));
    }
    return clientSocket;
}

void TCPServer::closeClientSocket(int clientSocket){
    if(clientSocket >= 0){
        close(clientSocket);
    }
}

// --- TCPClient Implementation ---
TCPClient::TCPClient(){
    _sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(_sockFd < 0){
        throw std::runtime_error("Failed to create TCP client socket: " + std::string(strerror(errno)));
    }
}

TCPClient::~TCPClient(){
    if(_sockFd >= 0){
        close(_sockFd);
    }
}

bool TCPClient::connectTo(const std::string& serverIP, int serverPort){
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    
    if(inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr) <= 0){
        return false; // Invalid IP address
    }

    return connect(_sockFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == 0;
}

bool TCPClient::sendData(const char* data, size_t dataSize){
    ssize_t bytesSent = send(_sockFd, data, dataSize, 0);
    return bytesSent == static_cast<ssize_t>(dataSize);
}

ssize_t TCPClient::receiveData(char* buffer, size_t bufferSize){
    return recv(_sockFd, buffer, bufferSize, 0);
}