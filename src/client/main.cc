#include "sockets/tcp.hh"
#include "sockets/udp.hh"
#include <iostream>
#include <thread>
#include <string>
#include <atomic>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <map>
#include <unistd.h>
#include "client/player.hh"
#include "sockets/packets.hh"


class GameClient {
private:
    TCPClient tcpClient;
    UDP udpSocket;
    std::atomic<bool> connected;
    std::atomic<bool> gameActive;
    std::atomic<bool> readySent;
    std::atomic<bool> inMatch;
    int playerId;
    int clientUDPPort;
    bool playerSide;
    bool winner;
    GameState State;

    int targetFPS = 30;
    int frameCounter = 60;

    Player *Player1;
    Player *Player2;

    std::map <bool, Player*> Players;
    std::map <bool, std::deque<Bullet*>*> bulletTrains;

    std::map <std::string, Texture2D> textures;

    //Initialization
    void initPlayers(){
        Players[false] = new Player(playerId, false, 5, this->textures["Player1"], this->textures["Bullet1"]);
        Players[true] = new Player(playerId, true, 5, this->textures["Player2"], this->textures["Bullet2"]);
    }
    void initBulletTrains(){
        bulletTrains[false] = new std::deque<Bullet*>;
        bulletTrains[true] = new std::deque<Bullet*>;
    }
    void initTextures(){
        Image image = LoadImage("resources/Player1.png");
        Texture2D tempTexture = LoadTextureFromImage(image);
        this->textures["Player1"] = tempTexture;
        image = LoadImage("resources/Player2.png");
        tempTexture = LoadTextureFromImage(image);
        this->textures["Player2"] = tempTexture;
        image = LoadImage("resources/Bullet1.png");
        tempTexture = LoadTextureFromImage(image);
        this->textures["Bullet1"] = tempTexture;
        image = LoadImage("resources/Bullet2.png");
        tempTexture = LoadTextureFromImage(image);
        this->textures["Bullet2"] = tempTexture;
        UnloadImage(image);
    }
    void initWindow(){

        InitWindow(screenWidth,screenHeight,"Cowboy Shootout!");
        SetTargetFPS(this->targetFPS);
    }
    
public:
    //Constructor
    GameClient(int udpPort) : udpSocket(udpPort), connected(false), gameActive(false), readySent(false), 
                              playerId(-1), clientUDPPort(udpPort) {
        initWindow();
        initTextures();
        initPlayers();
        initBulletTrains();
        this->playerSide = false;
        this->State = MAIN_MENU;
        this->winner = false;
    }
    
    bool connect(const std::string& serverIP, int tcpPort, int udpPort) {
        // Connect via TCP for control messages
        if (!tcpClient.connectTo(serverIP, tcpPort)) {
            std::cerr << "Failed to connect to game server via TCP" << std::endl;
            return false;
        }
        
        connected = true;
        std::cout << "Connected to game server!" << std::endl;
        
        // Send join message with our UDP port
        std::string joinMsg = std::string(1, static_cast<char>(MessageType::PLAYER_JOIN)) + std::to_string(clientUDPPort);
        if (!tcpClient.sendData(joinMsg.c_str(), joinMsg.length())) {
            std::cerr << "Failed to send join message" << std::endl;
            return false;
        }
        
        return true;
    }
    
    void sendReady() {
        if (!connected) return;
    
        char readyMsg[1] = {static_cast<char>(MessageType::PLAYER_READY)};
        std::cout << "Sending ready message (type " << static_cast<int>(MessageType::PLAYER_READY) << ")" << std::endl;
        if (tcpClient.sendData(readyMsg, 1)) {
            std::cout << "Sent ready signal to server" << std::endl;
            readySent=true;
        } else {
            std::cout << "Failed to send ready signal" << std::endl;
        }
    }
    
    void handleTCPMessages() {
        char buffer[1024];
        
        while (connected) {
            ssize_t bytesReceived = tcpClient.receiveData(buffer, sizeof(buffer)-1);
            
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';

                std::cout << buffer << std::endl;
                
                if (strncmp(buffer, "JOINED:", 7) == 0) {
                    playerId = std::stoi(std::string(buffer + 7));
                    std::cout << "Assigned player ID: " << playerId << std::endl;
                }
                else if(bytesReceived >= 1 && buffer[0] == static_cast<char>(MessageType::GAME_END)){
                    char side = static_cast<char> (buffer[1]);
                    bool winnerSide=true;
                    if(side==0) winnerSide=false;
                    winner = false;
                    if(winnerSide == playerSide) winner = true;

                    gameActive = false;
                    State = GAME_OVER;  

                    Players[false]->reset();
                    Players[true]->reset();

                    bulletTrains[false]->clear();
                    bulletTrains[true]->clear();

                    std::cout << "game over!\n";
                
                }
                else if (bytesReceived >= 1 && buffer[0] == static_cast<char>(MessageType::GAME_START)) {
                    std::cout << "Game is starting!" << std::endl;
                    char side = *reinterpret_cast<char*> (buffer+1);
                    std::cout << side;
                    if(side==0){
                        std::cout << "left\n";
                        playerSide=false;
                    } 
                    else {std::cout << "right\n";
                        playerSide=true;
                    }
                    Players[playerSide]->setId(playerId);
                    gameActive = true;
                }
                else {
                    std::cout << "TCP: " << buffer << std::endl;
                }
            } else {
                std::cout << "Disconnected from server" << std::endl;
                connected = false;
                break;
            }
        }
    }
    
    void handleUDPMessages() {
        char buffer[1024];
        sockaddr_in senderAddr;
       
        while (connected) {
            ssize_t packetSize = udpSocket.receiveFrom(buffer, sizeof(buffer), senderAddr);
            if (packetSize>0) {
                if (gameActive) {
                    processGameUpdate(buffer, packetSize);
                }
            }
        }
    }
        
    void processGameUpdate(const char* message, ssize_t packetSize) {
        if (!message) return;
        
        MessageType type = static_cast<MessageType>(message[0]);
        
        switch (type) {
            case MessageType::PLAYER_POSITION: {
                if (packetSize >= 6) {
                    char side = static_cast<char> (message[1]);
                    int enemyPos = *reinterpret_cast<const int*>(message+2);
                    if(side==0) Players[false]->updatePosition(enemyPos);
                    else Players[true]->updatePosition(enemyPos);
                }
                break;
            }
            
            case MessageType::PLAYER_SHOOT: {
                if(packetSize >=6){
                    char side = static_cast<char> (message[1]);
                    int shootPos = *reinterpret_cast<const int*>(message+2);
                    if(side==0) Players[false]->shoot(bulletTrains[false],shootPos);
                    else Players[true]->shoot(bulletTrains[true],shootPos);
                }
                break;
            }

            case MessageType::PLAYER_HEALTH: {
                if(packetSize >=6){
                    char side = static_cast<char> (message[1]);
                    int HP = *reinterpret_cast<const int*>(message+2);
                    if(side==0) Players[false]->setHP(HP);
                    else Players[true]->setHP(HP);
                }
                break;
            }
            default:
                break;
        }
    }
    
    void sendPosition() {
        //std::string positionMsg= static_cast<char>(MessageType::PLAYER_POSITION) + std::to_string(playerId) + std::to_string(Players[playerSide]->getyPosition());
        char positionMsg[9];
        
        positionMsg[0] = static_cast<char>(MessageType::PLAYER_POSITION);
        *reinterpret_cast<int*>(positionMsg + 1) = playerId;
        *reinterpret_cast<int*>(positionMsg + 5) = Players[playerSide]->getyPosition();
        
        //std::cout << "position sent!\n";
        
        udpSocket.sendTo(positionMsg, 9, "127.0.0.1", 8081);
    }
    
    void sendShoot(int yPos) {
        
        char shootMsg[9]; 
        shootMsg[0] = {static_cast<char>(MessageType::PLAYER_SHOOT)};
        *reinterpret_cast<int*>(shootMsg + 1) = playerId;
        *reinterpret_cast<int*>(shootMsg + 5) = yPos;

        //std::cout << "Shot sent!\n";

        udpSocket.sendTo(shootMsg, 9, "127.0.0.1", 8081);
    }

    void sendHealth() {
        char healthMsg[9];
        healthMsg[0] = {static_cast<char>(MessageType::PLAYER_HEALTH)};
        *reinterpret_cast<int*>(healthMsg + 1) = playerId;
        *reinterpret_cast<int*>(healthMsg + 5) = Players[playerSide]->getHP();

        //std::cout << "HP sent!\n";

        udpSocket.sendTo(healthMsg, 9, "127.0.0.1", 8081);
    }

    void draw() {
        BeginDrawing();
            ClearBackground(BLACK);
            Players[false]->drawDisplay();
            Players[true]->drawDisplay();
            for(auto it = bulletTrains[false]->begin(); it!=bulletTrains[false]->end();it++){
                (*it)->drawDisplay();
            }
            for(auto it = bulletTrains[true]->begin(); it!=bulletTrains[true]->end();it++){
                (*it)->drawDisplay();
            }
        EndDrawing();
    }

    void drawMainMenu(){
        char mainMessage[] = "Commands: 'r' to signal ready, 'q' to exit";
        char readyMessage[] = "Player Ready: looking for match...";
        int shift = MeasureText(mainMessage, 20);
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText(mainMessage, screenWidth/2-shift/2,screenHeight/2-200,20,WHITE);
            if(readySent){
                shift = MeasureText(readyMessage,20);
                DrawText(readyMessage,screenWidth/2-shift/2,screenHeight/2-100,20,WHITE); 
            }
        EndDrawing();
    }

    void drawGameOver(){
        char playerLoses[] = "You Lose!";
        char playerWins[] = "You Win!";
        Color color = BLUE;
        if(playerSide) color = RED;

        int shiftLoses = MeasureText(playerLoses,50);
        int shiftWins = MeasureText(playerWins,50);

        BeginDrawing();
            ClearBackground(BLACK);
            if(winner) DrawText(playerWins, screenWidth/2-shiftWins/2,screenHeight/2-25,50,color);
            else DrawText(playerLoses, screenWidth/2-shiftLoses/2,screenHeight/2-25,50,color);
        EndDrawing();
    }

    void readInputGame() {
        if(IsKeyPressed(KEY_R) && State == MAIN_MENU){
            sendReady();
        }
        if(IsKeyPressed(KEY_Q)){
            connected=false;
        }
        if(IsKeyPressed(KEY_LEFT) && State == GAME_MATCH){
            Players[playerSide]->move(-2);
        }
        if(IsKeyPressed(KEY_RIGHT) && State == GAME_MATCH){
            Players[playerSide]->move(+2);
        }
        if(IsKeyPressed(KEY_Z) && State == GAME_MATCH){
            Players[playerSide]->shoot(bulletTrains[playerSide],-1);
            sendShoot(-1);
        }
        if(IsKeyPressed(KEY_X) && State == GAME_MATCH){
            Players[playerSide]->shoot(bulletTrains[playerSide],0);
            sendShoot(0);
        }
        if(IsKeyPressed(KEY_C)){
            Players[playerSide]->shoot(bulletTrains[playerSide],+1);
            sendShoot(1);
        }
    }
    
    void updateDisplays(){
        Players[playerSide]->updateDisplay();
        Players[!playerSide]->updateDisplay();
        for(auto it = bulletTrains[playerSide]->begin(); it!=this->bulletTrains[playerSide]->end();it++){
            (*it)->updateDisplay();
        }
        for(auto it = bulletTrains[!playerSide]->begin(); it!=bulletTrains[!playerSide]->end();it++){
            (*it)->updateDisplay();
        }
    }

    void updateFrameTimers(){
        Players[playerSide]->updateTimer();
        Players[!playerSide]->updateTimer();
        if(!bulletTrains[playerSide]->empty()){
            if(bulletTrains[playerSide]->front()->getbulletLifeTime() <= bulletTrains[playerSide]->front()->getbulletTime()){
            bulletTrains[playerSide]->pop_front();
            }
        }
        if(!bulletTrains[!playerSide]->empty()){
            if(bulletTrains[!playerSide]->front()->getbulletLifeTime() <= bulletTrains[!playerSide]->front()->getbulletTime()){
                bulletTrains[!playerSide]->pop_front();
            }
        }
    }
    
    void moveBullets(){
        for(auto it = bulletTrains[playerSide]->begin(); it!=bulletTrains[playerSide]->end();it++){
            (*it)->move();
        }
        for(auto it = bulletTrains[!playerSide]->begin(); it!=bulletTrains[!playerSide]->end();it++){
            (*it)->move();
        }

        for(auto it = bulletTrains[playerSide]->begin(); it!=bulletTrains[playerSide]->end();it++){
            for(auto jt = bulletTrains[!playerSide]->begin(); jt!=bulletTrains[!playerSide]->end();jt++){
            (*it)->checkCollision(*jt);
            }
        }
        
    }

    void checkHit(){
        if(!bulletTrains[!playerSide]->empty()){
            Players[playerSide]->checkHit(bulletTrains[!playerSide]->front());
        }
        if(!bulletTrains[playerSide]->empty()){
            Players[!playerSide]->checkHit(bulletTrains[playerSide]->front());
        }
    }
    
    void runMainMenu(){
        frameCounter=150;
        drawMainMenu();
        if(gameActive){
            State = GAME_MATCH;
            readySent=false;
        } 
    }

    void runGameMatch(){
        updateFrameTimers();
        moveBullets();
        checkHit();
        updateDisplays();

        sendPosition();
        sendHealth();

        draw();
    }

    void runGameOver(){
        frameCounter--;
        drawGameOver();
        if(frameCounter<0) State = MAIN_MENU;
    }

    void run() {
        // Start message handling threads
        std::thread tcpThread(&GameClient::handleTCPMessages, this);
        std::thread udpThread(&GameClient::handleUDPMessages, this);
        
        tcpThread.detach();
        udpThread.detach();
        
        while(connected && !WindowShouldClose()){
            readInputGame();
            switch (State){
                case MAIN_MENU:
                    runMainMenu();
                    break;

                case GAME_MATCH:
                    runGameMatch();
                    break;

                case GAME_OVER:
                    runGameOver();
                    break;
            }
        }

        CloseWindow();
    }
};

int main() {
    try {
        // Seed random number generator
        srand(time(nullptr) + getpid());
        
        // Use a different UDP port for each client instance
        // Port range 9000-9100 should be enough for testing
        int udpPort = 9000 + (rand() % 100);
        std::cout << "Client using UDP port: " << udpPort << std::endl;
        
        GameClient client(udpPort);
        
        if (!client.connect("127.0.0.1", 8080, udpPort)) {
            return 1;
        }
        
        client.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
    
    return 0;
}