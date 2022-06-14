#ifndef SIGMACMDMAP_H
#define SIGMACMDMAP_H

// Comands codes
#define CLEAR_CMD 0x8C
#define LEFT_CMD 0x81
#define RIGHT_CMD 0x82
#define UP_CMD 0x83
#define DOWN_CMD 0x84
#define JUMP_CMD 0x85
#define OPEN_CMD 0x86
#define CLOSE_CMD 0x87
#define FLASH_CMD 0x88
#define FLASH_SCROLL_CMD 0x89
#define DOFF_CMD 0x8A
#define BIG_CMD 0x8B
#define RANDOM_CMD 0x8E

#define CLOCK_CMD 0x11

#define WAIT_CMD 0x8F
#define SPEED_CMD 0x8D

// Comands nodes names
#define CLEAR_NAME "clear"
#define LEFT_NAME "left"
#define RIGHT_NAME "right"
#define UP_NAME "up"
#define DOWN_NAME "down"
#define JUMP_NAME "jump"
#define OPEN_NAME "open"
#define CLOSE_NAME "close"
#define FLASH_NAME "flash"
#define FLASH_SCROLL_NAME "flash_scroll"
#define DOFF_NAME "doff"
#define BIG_NAME "big"
#define RANDOM_NAME "random"

#define CLOCK_NAME "clock"

#define WAIT_NAME "wait"
#define SPEED_NAME "speed"

char xmlNameToCode(const char* name);

#endif  // !SIGMAC
