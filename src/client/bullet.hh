#include <queue>
class Bullet {
    private:
    bool side;
    int damage=2;
    int xPosition;
    int yPosition;
    int vx;

    public:
    //Constructor
    Bullet(bool side, int yPosition);
    //Destructor
    ~Bullet();

    //Getters
    int getdamage();
    int getxPosition();
    int getyPosition();
    int getvx();
    bool getside();
    //Setters
    void setdamage(int damage);
    void setxPosition(int xPos);
    void setyPosition(int yPos);
    void setside(bool side);

    //Functions
    void move();
    void dealDamage();
    
};

class BulletDisplay {
    
};