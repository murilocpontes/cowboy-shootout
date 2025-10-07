#ifndef BULLET_HH
#define BULLET_HH

const int screenWidth = 1200;
const int screenHeight = 700;

#include <../include/raylib.h>
#include <deque>
#include <stdio.h>
#include <iostream>
class BulletDisplay {
    private:
    Texture2D texture;
    Rectangle srcRec;
    Rectangle destRec;
    float frameWidth;
    float frameHeight;
    float xPosition;
    float yPosition;
    float xScale;
    float yScale;
    Vector2 origin;

    public:
    //Constructor
    BulletDisplay(Texture2D texture, float xPosition, float yPosition);
    //Destructor
    ~BulletDisplay();
    //Getter
    float getxPosition();
    float getyPosition();
    //Setter
    void setxPosition(float xPosition);
    void setyPosition(float yPosition);
    //Functions
    void updateDestRect();
    void updatePosition(int xPosition, int yPosition);
    void draw();
};

class Bullet {
    private:
    bool side;
    int damage;
    int xPosition;
    int yPosition;
    int vx;
    bool alive;
    int bulletLifeTime;
    int bulletTime;

    BulletDisplay *display;

    public:
    //Constructor
    Bullet(Texture2D texture, bool side, int yPosition);
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
    bool checkHit(int xPos, int yPos);
    int damageDelt(int yPos);
    void checkCollision(Bullet *bullet);
    void updateDisplay();
    void drawDisplay();
    
};



#endif