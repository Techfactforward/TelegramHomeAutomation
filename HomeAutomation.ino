#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "your-SSID";
const char* password = "your-WiFi-password";

// Telegram Bot credentials
#define BOT_TOKEN "your-bot-token"
#define CHAT_ID "your-chat-id"

// Define relay pins
#define RELAY1_PIN D1
#define RELAY2_PIN D2

// Initialize Telegram Bot
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Relay states
bool relay1State = false;
bool relay2State = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize relay pins
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  
  // Ensure both relays are OFF at start
  digitalWrite(RELAY1_PIN, LOW); 
  digitalWrite(RELAY2_PIN, LOW); 

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  client.setInsecure();  // Needed to handle SSL connections for Telegram

  Serial.println("Telegram bot ready.");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
  }

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (chat_id == CHAT_ID) {
      if (text == "/start") {
        sendWelcomeMessage();
      } else if (text == "/relay1_on") {
        handleRelay1On();
      } else if (text == "/relay1_off") {
        handleRelay1Off();
      } else if (text == "/relay2_on") {
        handleRelay2On();
      } else if (text == "/relay2_off") {
        handleRelay2Off();
      } else if (text == "/reset") {
        handleReset();
      } else if (text == "/status") {
        handleStatus();
      } else {
        bot.sendMessage(CHAT_ID, "Unknown command. Available commands:\n/relay1_on\n/relay1_off\n/relay2_on\n/relay2_off\n/reset\n/status", "");
      }
    } else {
      bot.sendMessage(chat_id, "Unauthorized access", "");
    }
  }
}

// Send the list of available commands
void sendWelcomeMessage() {
  String welcomeMessage = "Welcome to the Relay Control Bot!\n\nAvailable commands:\n";
  welcomeMessage += "/relay1_on - Turn Relay 1 ON\n";
  welcomeMessage += "/relay1_off - Turn Relay 1 OFF\n";
  welcomeMessage += "/relay2_on - Turn Relay 2 ON\n";
  welcomeMessage += "/relay2_off - Turn Relay 2 OFF\n";
  welcomeMessage += "/reset - Turn both relays OFF\n";
  welcomeMessage += "/status - Get the status of both relays\n";
  
  bot.sendMessage(CHAT_ID, welcomeMessage, "");
}

// Handler functions

void handleRelay1On() {
  if (!relay1State) { // Only change state if different
    digitalWrite(RELAY1_PIN, HIGH);
    relay1State = true;
    bot.sendMessage(CHAT_ID, "Relay 1 turned ON", "");
  }
}

void handleRelay1Off() {
  if (relay1State) { // Only change state if different
    digitalWrite(RELAY1_PIN, LOW);
    relay1State = false;
    bot.sendMessage(CHAT_ID, "Relay 1 turned OFF", "");
  }
}

void handleRelay2On() {
  if (!relay2State) { // Only change state if different
    digitalWrite(RELAY2_PIN, HIGH);
    relay2State = true;
    bot.sendMessage(CHAT_ID, "Relay 2 turned ON", "");
  }
}

void handleRelay2Off() {
  if (relay2State) { // Only change state if different
    digitalWrite(RELAY2_PIN, LOW);
    relay2State = false;
    bot.sendMessage(CHAT_ID, "Relay 2 turned OFF", "");
  }
}

void handleReset() {
  bool changed = false;
  if (relay1State) {
    digitalWrite(RELAY1_PIN, LOW);
    relay1State = false;
    changed = true;
  }
  if (relay2State) {
    digitalWrite(RELAY2_PIN, LOW);
    relay2State = false;
    changed = true;
  }
  if (changed) {
    bot.sendMessage(CHAT_ID, "Both relays have been reset to OFF", "");
  } else {
    bot.sendMessage(CHAT_ID, "Relays were already OFF", "");
  }
}

void handleStatus() {
  String statusMessage = "Relay 1: " + String(relay1State ? "ON" : "OFF") + "\n" +
                         "Relay 2: " + String(relay2State ? "ON" : "OFF") + "\n";
  bot.sendMessage(CHAT_ID, statusMessage, "");
}

void reconnectWiFi() {
  Serial.print("Reconnecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nReconnected to WiFi!");
}
