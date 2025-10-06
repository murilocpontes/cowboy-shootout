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
    int id;
    int health;
    bool isAlive;
    
    Player();
    
    void reset();
    
    void takeDamage(int damage);
};

#endif