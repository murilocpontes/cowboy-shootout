#include "player.hh"
//Constructor
PlayerDisplay::PlayerDisplay(Texture2D texture, float xPosition=0, float yPosition=0){
    this->texture = texture;
    this->xPosition = xPosition;
    this->yPosition = yPosition;
    this->xScale = 2;
    this->yScale = 2;
    this->frameWidth = (float)this->texture.width;
    this->frameHeight = (float)this->texture.height;
    this->srcRec = {0.0f, 0.0f, this->frameWidth, this->frameHeight};
    this->destRec = {xPosition,yPosition, this->frameWidth*this->xScale, this->frameHeight*this->yScale};
    this->origin = {this->frameWidth,this->frameHeight};
}
//Destructor
PlayerDisplay::~PlayerDisplay(){
}
//Getter
float PlayerDisplay::getxPosition(){
    return this->xPosition;
}
float PlayerDisplay::getyPosition(){
    return this->yPosition;
}
// Setter
void PlayerDisplay::setxPosition(float xPosition){
    this->xPosition = xPosition;
}
void PlayerDisplay::setyPosition(float yPosition){
    this->yPosition = yPosition;
}
// Functions
void PlayerDisplay::updateDestRect(){
    this->destRec = {this->xPosition, this->yPosition, this->frameWidth*this->xScale, this->frameHeight*this->yScale};
}
void PlayerDisplay::updatePosition(int yPosition){
    float ypos=40.0f*yPosition+150.0f;
    this->yPosition = ypos;
    updateDestRect();
}
void PlayerDisplay::draw(){
    DrawTexturePro(this->texture, this->srcRec, this->destRec, this->origin, 0, WHITE);
}

//Constructor
Player::Player(int Id, bool side, int ypos, Texture2D playerTexture, Texture2D bulletTexture){
    this->Id = Id;
    this->side = side;
    this->yPosition = ypos;
    this->xPosition = 120;
    if(side) this->xPosition = screenWidth-this->xPosition;
    this->HP = 10;
    this->reloadDuration = 8;
    this->reloadTime = 8;
    this->readyToShoot = true;
    this->moveCooldown = 3;
    this->moveTime = 3;
    this->readyToMove = true;

    this->display = new PlayerDisplay(playerTexture, (float)this->xPosition, 0);
    updateDisplay();
    this->bulletTexture = bulletTexture;
    std::cout << "Created player" << Id << " HP" << this->HP << "\n";
    std::cout << "xpos and ypos " << this->xPosition << " " << this->yPosition << "\n";
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
void Player::move(int delta){
    int newY = this->yPosition+delta;
    if(newY<12 && newY>-1 && this->readyToMove){
        this->readyToMove = false;
        this->moveTime = 0;
        this->yPosition=newY;
    }
}
void Player::increaseMoveTime(){
    if(this->moveTime>this->moveCooldown) this->readyToMove = true;
    else this->moveTime++;
}
void Player::shoot(std::deque<Bullet*> *bulletTrain, int yPos){
    if(this->readyToShoot){
        this->readyToShoot = false;
        this->reloadTime = 0;
        Bullet *bullet = new Bullet(this->bulletTexture, this->side, this->yPosition+yPos);
        bulletTrain->push_back(bullet);
    }
}
void Player::updateTimer(){
    increaseMoveTime();
    increaseReloadTime();
}
void Player::increaseReloadTime(){
    if(this->reloadTime>this->reloadDuration) this->readyToShoot = true;
    else this->reloadTime++;
}
void Player::checkHit(Bullet* bullet){
    if(bullet->getalive() && bullet->checkHit(this->xPosition, this->yPosition)){
        this->takeDamage(bullet->damageDelt(this->yPosition));
    }
}
void Player::takeDamage(int damage){
    this->HP-=damage;
    std::cout<< "took damage! Hp is "<< this->HP << "\n";
}
void Player::updatePosition(int yPos){
    this->yPosition = yPos;
    this->updateDisplay();
}
void Player::updateDisplay(){
    this->display->updatePosition(this->yPosition);
}

void Player::drawDisplay(){
    this->display->draw();
}

