#ifndef PACKETS_HH
#define PACKETS_HH

#include <string>
#include <cstdint>

enum class MessageType : uint8_t {
    PLAYER_JOIN = 1,
    PLAYER_READY = 2,
    GAME_START = 3,
    PLAYER_POSITION = 4,
    PLAYER_SHOOT = 5,  
    PLAYER_HEALTH = 6,
    PLAYER_DEATH = 7, 
    GAME_END = 8 
};

#endif