#include <Arduino.h>
#include <CTBot.h>
#include <SoftwareSerial.h>

#include "SigmaCmdMap.h"
#include "tinyxml2.h"

/*
 * Configuration
 */
const char* ssid = "xxxxxxxxx";                       // SSID WiFi network
const char* pass = "xxxxxxxxx";                       // Password  WiFi network
const char* token =
    "xxxxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxx";  // Telegram token

/*
 * Global objects
 */
CTBot myBot;

SoftwareSerial ledSerial(-1, LED_DISPLAY_PIN, true);

tinyxml2::XMLDocument doc;
char final[1024];

// WARN: This function does not check if overflow the buffer
// TODO: Check if overflow the buffer
// TODO: center the text
uint16 process_nodes(tinyxml2::XMLNode* node, char* buffer, uint16 buffer_pos) {
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

    // recursively process the child nodes
    buffer_pos = process_nodes(node->FirstChild(), buffer, buffer_pos);

    // get the next node
    node = node->NextSibling();
  }
  return buffer_pos;
};

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
    // Serial.println(" Free heap: " + String(ESP.getFreeHeap()));
    Serial.print("New message: ");
    Serial.println(msg.text);
    // check if the message is a command
    if (msg.text.startsWith("/")) {
      // check what type of command it is
      if (msg.text.startsWith("/composition")) {
        uint16 pos = 0;

        Serial.println("Received composition.");

        // read the xml
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
        pos = process_nodes(root->FirstChild(), final, 0);

        // debug print the final
        Serial.print("Final: ");
        for (uint16 i = 0; i < pos; i++) {
          Serial.print(final[i]);
        }
        Serial.println();

        // send to the led panel
        ledSerial.write("\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xBB");
        ledSerial.write(final, pos);
        ledSerial.write(0x80);
      } else if (msg.text.startsWith("/ping")) {
        // send system info
        myBot.sendMessage(msg.sender.id, "Pong! Current free heap: " +
                                             String(ESP.getFreeHeap()));
      }
    }
  }

  delay(500);
}