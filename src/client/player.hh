#ifndef PLAYER_HH
#define PLAYER_HH

#include "bullet.hh"
class PlayerDisplay {
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
    PlayerDisplay(Texture2D texture, float xPosition, float yPosition);
    //Destructor
    ~PlayerDisplay();
    //Getter
    float getxPosition();
    float getyPosition();
    //Setter
    void setxPosition(float xPosition);
    void setyPosition(float yPosition);
    //Functions
    void updateDestRect();
    void updatePosition(int yPosition);
    void draw();
};

class HealthDisplay {
    private:
    Color color;
    int xPosition;
    int yPosition;
    bool side;

    public:
    //Constructor
    HealthDisplay(bool side, Color color, int xPosition, int yPosition);
    //Destructor
    ~HealthDisplay();
    //Getter
    //Setter
    void setxPosition(int xPosition);
    void setyPosition(int yPosition);
    void sethealth(int HP);
    void setcolor(Color color);
    //Functions
    void draw(int health);
};

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
    PlayerDisplay *display;
    HealthDisplay *hpBar;
    Texture2D bulletTexture;

    public:
    //Constructor
    Player(int Id, bool side,int ypos, Texture2D playerTexture, Texture2D bulletTexture);
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
    void move(int delta);
    void increaseMoveTime();
    void shoot(std::deque<Bullet*> *bulletTrain, int yPos);
    void increaseReloadTime();
    void updateTimer();
    bool checkHit(Bullet* bullet);
    void takeDamage(int damage);
    void updatePosition(int yPos);
    void updateDisplay();
    void drawDisplay();
    void reset();
    
};



#endif