#include "bullet.hh"
//Constructor
BulletDisplay::BulletDisplay(Texture2D texture, float xPosition=0, float yPosition=0){
    this->texture = texture;
    this->xPosition = xPosition;
    this->yPosition = yPosition;
    this->xScale = 6;
    this->yScale = 6;
    this->frameWidth = (float)this->texture.width;
    this->frameHeight = (float)this->texture.height;
    this->srcRec = {0.0f, 0.0f, this->frameWidth, this->frameHeight};
    this->destRec = {xPosition,yPosition, this->frameWidth*this->xScale, this->frameHeight*this->yScale};
    this->origin = {this->frameWidth*this->xScale/2,this->frameHeight*this->yScale/2};
}
//Destructor
BulletDisplay::~BulletDisplay(){
}
//Getter
float BulletDisplay::getxPosition(){
    return this->xPosition;
}
float BulletDisplay::getyPosition(){
    return this->yPosition;
}
// Setter
void BulletDisplay::setxPosition(float xPosition){
    this->xPosition = xPosition;
}
void BulletDisplay::setyPosition(float yPosition){
    this->yPosition = yPosition;
}
// Functions
void BulletDisplay::updateDestRect(){
    this->destRec = {this->xPosition, this->yPosition, this->frameWidth*this->xScale, this->frameHeight*this->yScale};
}
void BulletDisplay::updatePosition(int xPosition, int yPosition){
    float xpos=(float)xPosition;
    float ypos=40.0f*yPosition+150.0f;
    this->xPosition = xpos;
    this->yPosition = ypos;
    updateDestRect();
}
void BulletDisplay::draw(){
    DrawTexturePro(this->texture, this->srcRec, this->destRec, this->origin, 0, WHITE);
}

//Constructor
Bullet::Bullet(Texture2D texture, bool side, int yPosition){
    this->side = side;
    this->yPosition = yPosition;
    this->xPosition = 160;
    this->damage=2;
    this->vx = 20;
    this->bulletLifeTime = 48;
    this->bulletTime = 0;
    if(side){
        this->xPosition = screenWidth-this->xPosition;
        this->vx = -this->vx;
    }
    this->alive = true;

    this->display = new BulletDisplay(texture, 0, 0);
    updateDisplay();
}
    //Destructor
Bullet::~Bullet(){
}

//Getters
int Bullet::getdamage(){
    return this->damage;
}
int Bullet::getxPosition(){
    return this->xPosition;
}
int Bullet::getyPosition(){
    return this->yPosition;
}
int Bullet::getvx(){
    return this->vx;
}
int Bullet::getbulletLifeTime(){
    return this->bulletLifeTime;
}
int Bullet::getbulletTime(){
    return this->bulletTime;
}
bool Bullet::getside(){
    return this->side;
}
bool Bullet::getalive(){
    return this->alive;
}
//Setters
void Bullet::setdamage(int damage){
    this->damage = damage;
}
void Bullet::setxPosition(int xPos){
    this->xPosition = xPos;
}
void Bullet::setyPosition(int yPos){
    this->yPosition = yPos;
}
void Bullet::setside(bool side){
    this->side = side;
}
void Bullet::setalive(bool alive){
    this->alive=alive;
}

//Functions
void Bullet::move(){
    this->xPosition+=this->vx;
    this->bulletTime++;
}
bool Bullet::checkHit(int xPos, int yPos){
    int y = this->yPosition-yPos;
    int x = this->xPosition-xPos;
    if(y>=-1 && y<=1 && x<=20 && x>=-20) {
        this->alive=false;
        return true;
        }
    return false;
}
int Bullet::damageDelt(int yPos){
    if(this->yPosition==yPos) return this->damage;
    else return this->damage/2;
}
void Bullet::checkCollision(Bullet *bullet){
    if(!this->alive || !bullet->getalive()) return;
    if(this->side!=bullet->getside() && bullet->getalive() && this->yPosition==bullet->getyPosition()){
        int dx = this->xPosition-bullet->getxPosition();
        if(dx>=-20 && dx<=20){
            this->alive = false;
            bullet->setalive(false);
        }
    }
}
void Bullet::updateDisplay(){
    this->display->updatePosition(this->xPosition, this->yPosition);
}
void Bullet::drawDisplay(){
    if(this->alive) this->display->draw();
}

