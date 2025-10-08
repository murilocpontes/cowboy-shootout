#ifndef PLAYER_HH
#define PLAYER_HH

#include <string>

class Player {
public:
    int tcpSocket;
    std::string udpIP;
    int udpPort;
    bool isReady;
    int matchId;
    int yPos;
    bool side; // 0 -> left, 1 -> right
    int id;
    int health;
    bool isAlive;
    
    Player();
    
    void reset();
    
    void updateHealth(int health);
};

#endif