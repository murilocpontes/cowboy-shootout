#include "client/bullet.hh"
class Player {
    private:
    int Id;
    bool side;
    int HP=5;
    int yPosition=5;
    int reloadDuration=10;
    int reloadTime;
    bool readyToShoot=true;
    int moveCooldown=2;
    int moveTime;
    bool readyToMove=true;

    public:
    //Constructor
    Player();
    //Destructor
    ~Player();

    //Getters
    int getHP();
    int getyPosition();
    int getId();
    bool getside();
    //Setters
    void setHP(int HP);
    void setyPosition(int yPos);
    void setId(int Id);
    void setside(bool side);

    //Functions
    void move();
    void increaseMoveTime();
    void shoot(std::queue<Bullet> bulletTrain);
    void increaseReloadTime();
    void takeHit(int damage);
    
};

class PlayerDisplay {

};