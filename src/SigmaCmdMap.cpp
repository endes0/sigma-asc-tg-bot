#include "SigmaCmdMap.h"

#include <Arduino.h>

char xmlNameToCode(const char* name) {
  if (strcmp(name, CLEAR_NAME) == 0) {
    return CLEAR_CMD;
  } else if (strcmp(name, LEFT_NAME) == 0) {
    return LEFT_CMD;
  } else if (strcmp(name, RIGHT_NAME) == 0) {
    return RIGHT_CMD;
  } else if (strcmp(name, UP_NAME) == 0) {
    return UP_CMD;
  } else if (strcmp(name, DOWN_NAME) == 0) {
    return DOWN_CMD;
  } else if (strcmp(name, JUMP_NAME) == 0) {
    return JUMP_CMD;
  } else if (strcmp(name, OPEN_NAME) == 0) {
    return OPEN_CMD;
  } else if (strcmp(name, CLOSE_NAME) == 0) {
    return CLOSE_CMD;
  } else if (strcmp(name, FLASH_NAME) == 0) {
    return FLASH_CMD;
  } else if (strcmp(name, FLASH_SCROLL_NAME) == 0) {
    return FLASH_SCROLL_CMD;
  } else if (strcmp(name, DOFF_NAME) == 0) {
    return DOFF_CMD;
  } else if (strcmp(name, BIG_NAME) == 0) {
    return BIG_CMD;
  } else if (strcmp(name, RANDOM_NAME) == 0) {
    return RANDOM_CMD;
  } else if (strcmp(name, CLOCK_NAME) == 0) {
    return CLOCK_CMD;
  } else if (strcmp(name, WAIT_NAME) == 0) {
    return WAIT_CMD;
  } else if (strcmp(name, SPEED_NAME) == 0) {
    return SPEED_CMD;
  } else {
    return 0;
  }
}