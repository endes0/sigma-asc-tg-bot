#include "SigmaTextEncode.h"

void sigmaEncode(char* buffer, uint16 buffer_start, uint16 buffer_end) {
  for (int i = buffer_start; i < buffer_end; i++) {
    char c = buffer[i];

    switch (c) {
      case ' ':
        buffer[i] = ':';
        break;
      case ':':
        buffer[i] = ' ';
        break;
      case '<':
        buffer[i] = '_';
        break;
      case '_':
        buffer[i] = '<';
        break;
      case '[':
        buffer[i] = 0x0E;
        break;
      case ']':
        buffer[i] = 0x1E;
        break;
        // TODO: add more characters

      default:
        // discard command codes
        if (c >= 0x80 && c <= 0x8F) {
            buffer[i] = '!';
        }
        break;
    }
  }
}