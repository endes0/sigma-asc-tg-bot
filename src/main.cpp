#include <Arduino.h>
#include <CTBot.h>
#include <SoftwareSerial.h>

#include "SigmaCmdMap.h"
#include "SigmaTextEncode.h"
#include "tinyxml2.h"

/*
 * Configuration
 */
const char* ssid = "xxxxxxxxx";                       // SSID WiFi network
const char* pass = "xxxxxxxxx";                       // Password  WiFi network
const char* token =
    "xxxxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxx";  // Telegram token
#define LED_DISPLAY_PIN 2

/*
 * Global objects
 */
CTBot myBot;
SoftwareSerial ledSerial(-1, LED_DISPLAY_PIN, true);
tinyxml2::XMLDocument doc;
char final[1024];

/*
 * Functions
 */
// WARN: This function does not check if overflow the buffer
// TODO: Check if overflow the buffer
// TODO: move to SigmaLib
uint16 process_nodes(tinyxml2::XMLNode* node, char* buffer, uint16 buffer_pos,
                     bool center) {
  while (node) {
    // check if it is a text node
    if (node->ToText()) {
      // get the text and add it to the final
      memcpy(buffer + buffer_pos, node->Value(), strlen(node->Value()));

      // fix the encoding
      sigmaEncode(buffer, buffer_pos, buffer_pos + strlen(node->Value()));

      // update the buffer position
      buffer_pos += strlen(node->Value());
    }

    // check if it is wait or speed
    if (node->ToElement()) {
      // get the name of the element and lowercase it
      String name = String(node->ToElement()->Name());
      name.toLowerCase();

      // get the command code and check if it is valid
      char code = xmlNameToCode(name.c_str());
      if (code == 0) {
        // myBot.sendMessage(msg, "Error parsing XML: Unknown command" + name);
        // TODO: throw error
        return buffer_pos;
      }
      buffer[buffer_pos++] = code;

      // get the command atributes if is wait or speed
      if (code == WAIT_CMD || code == SPEED_CMD) {
        // get the attribute
        const tinyxml2::XMLAttribute* time =
            node->ToElement()->FindAttribute("time");
        if (!time) {
          // myBot.sendMessage(msg, "Error parsing XML: Missing time
          // attribute");
          // TODO: throw error
          return buffer_pos;
        }
        buffer[buffer_pos++] = 0x30 + time->IntValue();
      }
    }

    // check if has nocenter attribute
    const tinyxml2::XMLAttribute* nocenter =
        node->ToElement()->FindAttribute("noalign");
    if (nocenter) {
      center = false;
    }

    // recursively process the child nodes
    buffer_pos = process_nodes(node->FirstChild(), buffer, buffer_pos, center);

    // get the next node
    node = node->NextSibling();
  }
  return buffer_pos;
};

/*
 * Main
 */
void setup() {
  // initialize serial communication
  Serial.begin(115200);
  Serial.println("Starting...");

  // initialize the communication with the led board
  ledSerial.begin(2400);

  // initialize the bot
  myBot.wifiConnect(ssid, pass);
  myBot.setTelegramToken(token);

  if (myBot.testConnection()) {
    Serial.println("Bot started");
  } else {
    Serial.println("Bot failed to start");
    // wait and restart
    delay(5000);
    ESP.restart();
  }
}

void loop() {
  TBMessage msg;
  Serial.println("Waiting for message...");

  // check for new messages
  if (myBot.getNewMessage(msg)) {
    Serial.print("New message: ");
    Serial.println(msg.text);
    // check if the message is a command
    if (msg.text.startsWith("/")) {
      // check what type of command it is
      if (msg.text.startsWith("/composition")) {
        uint16 pos = 0;

        Serial.println("Received composition.");

        // TODO: move to SigmaLib
        //  read the xml
        doc.Parse(msg.text.substring(12).c_str());
        if (doc.Error()) {
          myBot.sendMessage(msg.sender.id, "Error parsing XML");
          return;
        }

        Serial.println("Parsed XML.");

        // get the root element
        tinyxml2::XMLElement* root = doc.FirstChildElement("composition");
        if (!root) {
          myBot.sendMessage(msg.sender.id, "Error parsing XML");
          return;
        }

        Serial.print("Processing composition...");

        // process the nodes
        pos = process_nodes(root->FirstChild(), final, 0, true);

        // send to the led panel
        ledSerial.write("\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xBB");
        ledSerial.write(final, pos);
        ledSerial.write(0x80);
      } else if (msg.text.startsWith("/display")) {
        Serial.println("Received display.");

        String text = "[" + msg.sender.username + "]" + msg.text.substring(9);

        // add spaces to the text if it is not long enough
        while (text.length() < 16) {
          text += " ";
        }

        // fix encoding
        strcpy(final, text.c_str());
        sigmaEncode(final, 0, text.length());

        // send to the led panel
        ledSerial.write("\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xBB\x81");
        ledSerial.write(final, text.length());
        ledSerial.write(
            "\x8F\x39\x3A\x3A\x3A\x3A\x3A\x3A\x3A\x3A\x3A\x3A\x3A\x3A\x3A\x3A"
            "\x3A\x3A\x80");
      } else if (msg.text.startsWith("/ping")) {
        // send system info
        myBot.sendMessage(msg.sender.id, "Pong! Current free heap: " +
                                             String(ESP.getFreeHeap()));
      } else if (msg.text.startsWith("/fuzz")) {
        ledSerial.write("\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xBB");

        for (size_t i = 0; i < 255; i++) {
          if (i >= 0x80 && i <= 0x8F) {
            continue;
          }

          ledSerial.print(i);
          ledSerial.write(":");
          ledSerial.write(i);
          ledSerial.write(":::");
        }

        ledSerial.write("\x80");
      }
    }
  }

  delay(500);
}