#ifndef PLAYER_HH
#define PLAYER_HH

#include "bullet.hh"
class Player {
    private:
    int Id;
    bool side;
    int HP;
    int xPosition;
    int yPosition;
    int reloadDuration;
    int reloadTime;
    bool readyToShoot;
    int moveCooldown;
    int moveTime;
    bool readyToMove;

    public:
    //Constructor
    Player(int Id, bool side,int ypos);
    //Destructor
    ~Player();

    //Getters
    int getHP();
    int getxPosition();
    int getyPosition();
    int getId();
    bool getside();
    //Setters
    void setHP(int HP);
    void setxPosition(int xPos);
    void setyPosition(int yPos);
    void setId(int Id);
    void setside(bool side);

    //Functions
    void move();
    void increaseMoveTime();
    void shoot(std::deque<Bullet> *bulletTrain);
    void increaseReloadTime();
    void checkHit(Bullet bullet);
    void takeDamage(int damage);
    
};

class PlayerDisplay {

};

#endif