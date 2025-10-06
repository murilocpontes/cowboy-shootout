#include "bullet.hh"
//Constructor
Bullet::Bullet(bool side, int yPosition){
    this->side = side;
    this->yPosition = yPosition;
    this->xPosition = 100;
    this->vx = 15;
    if(side){
        this->xPosition = 700;
        this->vx = -15;
    }
    this->alive = true;

    std::cout << "new bullet!\n";
    std::cout << "bullet velocity: " << this->vx << "xpos ypos:" << this->xPosition << " " << this->yPosition << "\n";
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
    std::cout << "bullet velocity: " << this->vx << "xpos ypos: " << this->xPosition << " " << this->yPosition << " bullet Time " << this->bulletTime << "\n";
}
bool Bullet::checkHit(int yPos){
    int y = this->yPosition-yPos;
    if(y>=-1 && y<=1) return true;
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
        if(dx>=-15 && dx<=15){
            std::cout << "Bullet collision!\n";
            this->alive = false;
            bullet->setalive(false);
        }
    }
}