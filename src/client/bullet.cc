#include "client/bullet.hh"
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
bool Bullet::getside(){
    return this->side;
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

//Functions
void Bullet::move(){
    this->xPosition+=this->vx;
}
void Bullet::dealDamage(){

}