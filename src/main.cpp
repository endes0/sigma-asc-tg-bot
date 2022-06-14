#include <Arduino.h>
#include <AsyncTelegram2.h>
#include <ESP8266WiFi.h>
#include <time.h>

#include "SigmaCmdMap.h"
#include "tinyxml2.h"

/*
 * Configuration
 */
const char* timezone = "CET-1CEST,M3.5.0,M10.5.0/3";  // Timezone
const char* ssid = "xxxxxxxxx";                       // SSID WiFi network
const char* pass = "xxxxxxxxx";                       // Password  WiFi network
const char* token =
    "xxxxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxx";  // Telegram token

/*
 * Global objects
 */
BearSSL::WiFiClientSecure client;
BearSSL::Session session;
BearSSL::X509List certificate(telegram_cert);
AsyncTelegram2 myBot(client);

// WARN: This function does not check if overflow the buffer
// TODO: Check if overflow the buffer
uint16 process_nodes(tinyxml2::XMLNode* node, char* buffer, uint16 buffer_pos) {
  while (node) {
    // check if it is a text node
    if (node->ToText()) {
      // get the text and add it to the final
      memcpy(buffer + buffer_pos, node->Value(), strlen(node->Value()));
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
  Serial.begin(9600);
  Serial.println("Starting...");

  // initialize WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // initialize client connection
  configTime(timezone, "pool.ntp.org", "time.nist.gov", "time.windows.com");
  client.setSession(&session);
  client.setTrustAnchors(&certificate);
  client.setBufferSizes(1024, 1024);

  // initialize Telegram bot
  myBot.setUpdateTime(2000);
  myBot.setTelegramToken(token);
  if (myBot.begin()) {
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

  // check for new messages
  if (myBot.getNewMessage(msg)) {
    // check if the message is a command
    if (msg.text.startsWith("/")) {
      // check what type of command it is
      if (msg.text.startsWith("/composition")) {
        char final[4096];
        uint16 pos = 0;

        // read the xml
        tinyxml2::XMLDocument doc;
        doc.Parse(msg.text.substring(12).c_str());
        if (doc.Error()) {
          myBot.sendMessage(msg, "Error parsing XML");
          return;
        }

        // get the root element
        tinyxml2::XMLElement* root = doc.FirstChildElement("composition");
        if (!root) {
          myBot.sendMessage(msg, "Error parsing XML");
          return;
        }

        // process the nodes
        pos = process_nodes(root->FirstChild(), final, 0);

        // debug print the final
        Serial.print("Final: ");
        for (uint16 i = 0; i < pos; i++) {
          Serial.print(final[i]);
        }
        Serial.println();

        // TODO: send to the led panel
      }
    }
  }
}