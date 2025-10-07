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

    double previousTime = GetTime();
    double currentTime = 0.0;
    double updateDrawTime = 0.0;
    double waitTime = 0.0;
    float deltaTime = 0.0f;

    float timeCounter = 0.0f;
    int targetFPS = 30;

    Player *Player1;
    Player *Player2;

    std::map <bool, Player*> Players;
    std::map <bool, std::deque<Bullet*>*> bulletTrains;

    std::deque<Bullet*> *bulletTrain1 = new std::deque<Bullet*>;
    std::deque<Bullet*> *bulletTrain2 = new std::deque<Bullet*>;

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
        this->playerSide = true;
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
                
                if (strncmp(buffer, "JOINED:", 7) == 0) {
                    playerId = std::stoi(std::string(buffer + 7));
                    std::cout << "Assigned player ID: " << playerId << std::endl;
                }
                else if (bytesReceived >= 1 && buffer[0] == static_cast<char>(MessageType::GAME_START)) {
                    std::cout << "Game is starting!" << std::endl;
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
            if (udpSocket.receiveFrom(buffer, sizeof(buffer), senderAddr)) {
                if (gameActive) {
                    processGameUpdate(buffer);
                }
            }
        }
    }
    
    void processGameUpdate(const char* message) {
        if (!message) return;
        
        MessageType type = static_cast<MessageType>(message[0]);
        
        switch (type) {
            case MessageType::PLAYER_POSITION: {
                if (strlen(message) >= 13) {
                    int otherPlayerId = *reinterpret_cast<const int*>(message + 1);
                    float x = *reinterpret_cast<const float*>(message + 5);
                    float y = *reinterpret_cast<const float*>(message + 9);
                    
                    if (otherPlayerId != playerId) {
                        std::cout << "Player " << otherPlayerId << " position: (" 
                                  << x << ", " << y << ")" << std::endl;
                    }
                }
                break;
            }
            
            case MessageType::PLAYER_SHOOT: {
                std::cout << "Someone shot!" << std::endl;
                break;
            }
            
            default:
                break;
        }
    }
    
    void sendPosition() {
        if (!gameActive || playerId == -1) return;

        char positionMsg[13];
        /*
        positionMsg[0] = static_cast<char>(MessageType::PLAYER_POSITION);
        *reinterpret_cast<int*>(positionMsg + 1) = playerId;
        *reinterpret_cast<float*>(positionMsg + 5) = x;
        *reinterpret_cast<float*>(positionMsg + 9) = y;
        */
        
        udpSocket.sendTo(positionMsg, 13, "127.0.0.1", 8081);
    }
    
    void sendShoot() {
        if (!gameActive) return;
        
        char shootMsg[1] = {static_cast<char>(MessageType::PLAYER_SHOOT)};
        udpSocket.sendTo(shootMsg, 1, "127.0.0.1", 8081);
        std::cout << "Shot fired!" << std::endl;
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

    void readInputMenu(){
        if(IsKeyPressed(KEY_R)){
            sendReady();
        }
        if(IsKeyPressed(KEY_Q)){
            connected=false;
        }
    }

    void drawMenu(){
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

    void readInputGame() {
        Players[!playerSide]->shoot(bulletTrains[!playerSide],0);
        if(IsKeyPressed(KEY_LEFT)){
            Players[playerSide]->move(-2);
        }
        if(IsKeyPressed(KEY_RIGHT)){
            Players[playerSide]->move(+2);
        }
        if(IsKeyPressed(KEY_Z)){
            Players[playerSide]->shoot(bulletTrains[playerSide],-1);
        }
        if(IsKeyPressed(KEY_X)){
            Players[playerSide]->shoot(bulletTrains[playerSide],0);
        }
        if(IsKeyPressed(KEY_C)){
            Players[playerSide]->shoot(bulletTrains[playerSide],+1);
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
    
    void gameLoop() {
        
        while (connected) {
            if (gameActive) {
                
                

            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
        
    }
    
    void run() {
        // Start message handling threads
        std::thread tcpThread(&GameClient::handleTCPMessages, this);
        std::thread udpThread(&GameClient::handleUDPMessages, this);
        std::thread gameThread(&GameClient::gameLoop, this);
        
        tcpThread.detach();
        udpThread.detach();
        gameThread.detach();
        /*
        // Simple command interface
        std::cout << "Commands: 'r' to signal ready, 'q' to exit" << std::endl;
        while(connected && !WindowShouldClose()){
            readInputMenu();
            drawMenu();
            if(gameActive) break;
        }*/

        while(connected && !WindowShouldClose()){
            
            readInputGame();
            updateFrameTimers();
            moveBullets();
            checkHit();
            updateDisplays();
            draw();

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