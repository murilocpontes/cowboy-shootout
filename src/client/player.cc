#include "client/player.hh"

    //Constructor
Player::Player(int Id=1, bool side=false, int ypos=5){
    this->Id = Id;
    this->side = side;
    this->yPosition = ypos;
    this->xPosition = 100;
    if(side) this->xPosition = 700;
    this->HP = 5;
    this->reloadDuration = 10;
    this->reloadTime = 10;
    this->readyToShoot = true;
    this->moveCooldown = 2;
    this->moveTime = 2;
    this->readyToMove = true;

    std::cout << "Created player" << Id << " HP" << this->HP << "\n";
    std::cout << "xpos and ypos " << this->xPosition << " " << this->yPosition << "\n";
    this->shoot();
}
    //Destructor
Player::~Player(){
}

    //Getters
int Player::getHP(){
    return this->HP;
}
int Player::getxPosition(){
    return this->xPosition;
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

std::deque<Bullet>* Player::getbulletTrain(){
    return &(this->bulletTrain);
}
//Setters
void Player::setHP(int HP){
    this->HP = HP;
}
void Player::setxPosition(int xPos){
    this->xPosition = xPos;
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
void Player::shoot(){
    if(this->readyToShoot){
        this->readyToShoot = false;
        Bullet bullet(this->side, this->yPosition);
        this->bulletTrain.push_back(bullet);
    }
}
void Player::increaseReloadTime(){
    if(this->reloadTime>this->reloadDuration) this->readyToShoot = true;
    else this->reloadTime++;
}
void Player::checkHit(Bullet bullet){
    if(bullet.getalive() && this->xPosition==bullet.getxPosition()){
        if(bullet.checkHit(this->yPosition)){
            this->takeDamage(bullet.damageDelt(this->yPosition));
        }
    }
}
void Player::takeDamage(int damage){
    this->HP-=damage;
    std::cout<< "took damage! Hp is "<< this->HP << "\n";
}

void Player::popBulletFront(){
    this->bulletTrain.pop_front();
}

void Player::moveBullets(){
    for(auto it = this->bulletTrain.begin(); it<this->bulletTrain.end(); it++){
        it->move();
    }
    this->checkBulletTimes();
}

void Player::checkBulletTimes(){
    bool erased=true;
    while(erased){
        erased=false;
        if(!bulletTrain.empty()) {
            Bullet bullet = bulletTrain.front();
            if(bullet.getbulletTime()>=bullet.getbulletLifeTime()){
                bulletTrain.pop_front();
                erased = true;
                std::cout << "erased bullet! at BulletTime: " << bullet.getbulletTime() << " and xpos ypos: " << bullet.getxPosition() << " " << bullet.getyPosition() << " \n";
            }
        }
    }
}