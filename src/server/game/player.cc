#include "player.hh"

Player::Player() : tcpSocket(-1), udpPort(0), isReady(false), matchId(-1), 
    yPos(0), side(0), id(0), health(100), isAlive(true) {}
    
void Player::reset() {
    health = 10;
    isAlive = true;
    yPos = 5;
    isReady = false;
    matchId = -1;
}
    


void Player::updateHealth(int newHealth) {
    health = std::max(0, newHealth);
    if (health <= 0) {
        isAlive = false;
    }
}