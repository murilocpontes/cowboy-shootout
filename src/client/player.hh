#ifndef PLAYER_HH
#define PLAYER_HH

#include "client/bullet.hh"
class Player {
    private:
    std::deque <Bullet> bulletTrain;
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
    std::deque<Bullet> *getbulletTrain();
    //Setters
    void setHP(int HP);
    void setxPosition(int xPos);
    void setyPosition(int yPos);
    void setId(int Id);
    void setside(bool side);

    //Functions
    void move();
    void increaseMoveTime();
    void shoot();
    void increaseReloadTime();
    void checkHit(Bullet bullet);
    void takeDamage(int damage);
    void popBulletFront();
    void moveBullets();
    void checkBulletTimes();
    
};

class PlayerDisplay {

};

#endif