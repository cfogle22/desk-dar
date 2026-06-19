#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

String serial_read_string() {
  String input = "";

  // Read characters until Enter is pressed (CR or LF)
  while (true) {
    // wait for available character
    while (Serial.available() == 0) {
      delay(1);
    }

    char c = (char)Serial.read();

    // handle backspace/delete
    if (c == 8 || c == 127) {
      if (input.length() > 0) input.remove(input.length() - 1);
      continue;
    }

    // stop on CR or LF
    if (c == '\r' || c == '\n') {
      // if CR was received and next is LF, consume it
      if (c == '\r' && Serial.peek() == '\n') Serial.read();
      break;
    }

    input += c;
  }

  input.trim(); // Remove any leading/trailing whitespace
  return input;
}

#endif