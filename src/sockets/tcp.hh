#ifndef TCP_HH
#define TCP_HH

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 

#include <string>

class TCPServer{
public:
    TCPServer(int port);
    ~TCPServer();

    int acceptConnection(); // Returns client socket descriptor
    void closeClientSocket(int clientSocket);
    bool sendToClient(int clientSocket, const char* data, size_t dataSize);
    ssize_t receiveFromClient(int clientSocket, char* buffer, size_t bufferSize);

private:
    int _listenSocketFd;
};

class TCPClient{
public:
    TCPClient();
    ~TCPClient();

    bool connectTo(const std::string& serverIP, int serverPort);
    bool sendData(const char* data, size_t dataSize);
    ssize_t receiveData(char* buffer, size_t bufferSize);

private:
    int _sockFd;
};

#endif