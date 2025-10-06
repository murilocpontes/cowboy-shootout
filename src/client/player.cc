#include "client/player.hh"

    //Constructor
Player::Player(){

}
    //Destructor
Player::~Player(){
}

    //Getters
int Player::getHP(){
    return this->HP;
}
int Player::getyPosition(){
    return this->yPosition;
}
int Player::getId(){
    return this->Id;
}
bool Player::getside(){
    return this->side;
}
//Setters
void Player::setHP(int HP){
    this->HP = HP;
}
void Player::setyPosition(int yPos){
    this->yPosition = yPos;
}
void Player::setId(int Id){
    this->Id = Id;
}
void Player::setside(bool side){
    this->side = side;
}

//Functions
void Player::move(){

}
void Player::increaseMoveTime(){
    if(this->moveTime>this->moveCooldown) this->readyToMove = true;
    else this->moveTime++;
}
void Player::shoot(std::queue<Bullet> bulletTrain){
    if(this->readyToShoot){
        this->readyToShoot = false;
        Bullet bullet(this->side, this->yPosition);
        bulletTrain.push(bullet);
    }
}
void Player::increaseReloadTime(){
    if(this->reloadTime>this->reloadDuration) this->readyToShoot = true;
    else this->reloadTime++;
}
void Player::takeHit(int damage){
    this->HP-=damage;
    if(this->HP<=0){
    }
}