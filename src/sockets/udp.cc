#include "udp.hh"
#include <stdexcept>
#include <cstring>

UDP::UDP(int port){
    _sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(_sockFd < 0){
        throw std::runtime_error("Failed to create UDP socket: " + std::string(strerror(errno)));
    }
    if(port != 0){
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);
        
        if(bind(_sockFd, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0){
            close(_sockFd);
            throw std::runtime_error("Failed to bind UDP socket to port " + std::to_string(port) + ": " + std::string(strerror(errno)));
        }
    }
}

UDP::~UDP(){
    close(_sockFd);
}

bool UDP::sendTo(const char* data, size_t dataSize, const std::string& receiverIP, int receiverPort){
    sockaddr_in receiverAddress;
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(receiverPort);
    inet_pton(AF_INET, receiverIP.c_str(), &receiverAddress.sin_addr);

    int bytesSent = sendto(_sockFd, data, dataSize, 0, (struct sockaddr*)&receiverAddress, sizeof(receiverAddress));
    return bytesSent == dataSize;
}

ssize_t UDP::receiveFrom(char* buffer, size_t bufferSize, sockaddr_in& senderAddress){
    socklen_t senderAddLen = sizeof(senderAddress);
    ssize_t bytesReceived = recvfrom(_sockFd, buffer, bufferSize, 0, (struct sockaddr*)&senderAddress, &senderAddLen);
    return bytesReceived;
}
