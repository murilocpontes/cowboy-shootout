#ifndef BULLET_HH
#define BULLET_HH

#include <../include/raylib.h>
#include <deque>
#include <stdio.h>
#include <iostream>
class Bullet {
    private:
    bool side;
    int damage=2;
    int xPosition;
    int yPosition;
    int vx;
    bool alive;
    int bulletLifeTime = 44;
    int bulletTime = 0;

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
    int getbulletLifeTime();
    int getbulletTime();
    bool getside();
    bool getalive();
    //Setters
    void setdamage(int damage);
    void setxPosition(int xPos);
    void setyPosition(int yPos);
    void setbulletLifeTime(int time);
    void setbulletTime(int time);
    void setside(bool side);
    void setalive(bool alive);

    //Functions
    void move();
    bool checkHit(int yPos);
    int damageDelt(int yPos);
    void checkCollision(Bullet *bullet);
    
};

class BulletDisplay {
    
};

#endif