#ifndef UDP_HH
#define UDP_HH

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 

#include <string>

class UDP{
public:
    UDP(int port = 0);
    ~UDP();

    bool sendTo(const char* data, size_t dataSize, const std::string& receiverIP, int receiverPort);
    ssize_t receiveFrom(char* buffer, size_t bufferSize, sockaddr_in& senderAddress);
private:
    int _sockFd;
};


#endif