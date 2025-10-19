// ----------------------------------------------------
// Smart Door Lock with RFID: ESP32 Code
// Microcontroller: ESP32/ESP8266
// ----------------------------------------------------

#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// --- Configuration ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Pin Definitions
#define RST_PIN    22         // Reset Pin for RFID
#define SS_PIN     21         // Slave Select Pin for RFID (VSPI CS)
const int BUZZER_PIN = 19;    // Passive Buzzer pin
const int SERVO_PIN = 18;     // Servo motor pin

// Servo positions
const int LOCK_POS = 0;       // Angle for Locked state (e.g., 0 degrees)
const int UNLOCK_POS = 90;    // Angle for Unlocked state (e.g., 90 degrees)
const int UNLOCK_DURATION = 5000; // Duration the door stays unlocked (in ms)

// Authorized UID List (Example: Add your own card UIDs here)
// Format: byte uidList[][4] = { {UID1_B1, UID1_B2, UID1_B3, UID1_B4}, {UID2_B1, ...} };
// Use the Serial Monitor to find your tag's UID.
byte authorizedUids[][4] = {
  {0x92, 0x1A, 0x56, 0xCD}, // Example Tag 1
  {0xAB, 0xCD, 0xEF, 0x12}  // Example Tag 2
};
const int NUM_AUTHORIZED_TAGS = sizeof(authorizedUids) / sizeof(authorizedUids[0]);

// Access Log variables
#define LOG_SIZE 5
String accessLog[LOG_SIZE];
int logIndex = 0;

// Hardware Objects
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
Servo lockServo;                   // Create Servo object
WebServer server(80);

// --- Functions ---

/**
 * Checks if the scanned UID matches any authorized UIDs.
 */
bool isAuthorized(byte *uid) {
  for (int i = 0; i < NUM_AUTHORIZED_TAGS; i++) {
    if (memcmp(uid, authorizedUids[i], 4) == 0) {
      return true; // Match found
    }
  }
  return false; // No match
}

/**
 * Converts a 4-byte UID to a human-readable hex string.
 */
String uidToString(byte *uid) {
  String s = "";
  for (byte i = 0; i < 4; i++) {
    if (uid[i] < 0x10) {
      s += "0";
    }
    s += String(uid[i], HEX);
  }
  s.toUpperCase();
  return s;
}

/**
 * Adds an entry to the rolling access log.
 */
void addToLog(String entry) {
  // Move older logs down and insert new log at the top
  for (int i = LOG_SIZE - 1; i > 0; i--) {
    accessLog[i] = accessLog[i - 1];
  }
  accessLog[0] = entry;
}

/**
 * Actuator control functions
 */
void lockDoor() {
  lockServo.write(LOCK_POS);
  Serial.println("Door Locked.");
}

void unlockDoor() {
  lockServo.write(UNLOCK_POS);
  Serial.println("Door UNLOCKED.");
  delay(UNLOCK_DURATION);
  lockDoor(); // Re-lock after duration
}

void successTone() {
  tone(BUZZER_PIN, 1000, 150); // Short high beep
  delay(150);
  noTone(BUZZER_PIN);
  tone(BUZZER_PIN, 1500, 150); // Short higher beep
  delay(150);
  noTone(BUZZER_PIN);
}

void errorAlarm() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 500, 300); // Low, long beep
    delay(400);
    noTone(BUZZER_PIN);
  }
}

// --- Web Server Handler ---

String getPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="10"> 
<title>Smart Lock Monitor</title>
<style>
body { font-family: 'Verdana', sans-serif; background: #fafafa; margin: 0; padding: 20px;}
.container { max-width: 600px; margin: auto; background: #fff; padding: 30px; border-radius: 10px; box-shadow: 0 4px 12px rgba(0,0,0,0.1); }
h1 { color: #00796b; text-align: center; margin-bottom: 25px; }
.status-box { padding: 15px; border-radius: 8px; font-weight: bold; font-size: 1.2em; text-align: center; margin-bottom: 20px; }
.status-box.locked { background-color: #ffcdd2; color: #c62828; }
.status-box.unlocked { background-color: #c8e6c9; color: #2e7d32; }
.log-area { background: #f5f5f5; border: 1px solid #eee; padding: 15px; border-radius: 8px; }
.log-area h2 { color: #546e7a; font-size: 1.1em; margin-top: 0; border-bottom: 1px solid #cfd8dc; padding-bottom: 5px; }
.log-item { padding: 8px 0; border-bottom: 1px dotted #e0e0e0; font-size: 0.9em; }
.log-item:last-child { border-bottom: none; }
.log-item.granted { color: #2e7d32; font-weight: bold; }
.log-item.denied { color: #c62828; font-style: italic; }
.info { font-size: 0.8em; color: #757575; text-align: center; margin-top: 20px;}
</style>
</head>
<body>
<div class="container">
<h1>Smart Door Lock 🔐</h1>

<div class="status-box %s">
    Door State: %s
</div>

<div class="log-area">
    <h2>Recent Access Log (Last %d Attempts)</h2>
    %s
</div>

<div class="info">
    Current Authorized Tags: %d
</div>
</div>
</body>
</html>
)rawliteral";

  // Determine lock status for display
  int currentPos = lockServo.read();
  String lock_class = (currentPos == LOCK_POS) ? "locked" : "unlocked";
  String lock_text = (currentPos == LOCK_POS) ? "LOCKED" : "UNLOCKED";
  
  // Format the access log HTML
  String log_html = "";
  for (int i = 0; i < LOG_SIZE; i++) {
    if (accessLog[i].length() > 0) {
      String status_class = accessLog[i].indexOf("GRANTED") > 0 ? "granted" : "denied";
      log_html += "<div class='log-item " + status_class + "'>" + accessLog[i] + "</div>";
    }
  }
  
  // Format the final HTML content
  return String::format(
    html.c_str(),
    lock_class.c_str(),
    lock_text.c_str(),
    LOG_SIZE,
    log_html.c_str(),
    NUM_AUTHORIZED_TAGS
  );
}

// Handler for the root page ("/")
void handleRoot() {
  server.send(200, "text/html", getPage());
}

void setup() {
  Serial.begin(115200);
  
  // Pin setup
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Servo setup
  lockServo.attach(SERVO_PIN);
  lockDoor(); // Start in locked state

  // RFID setup
  SPI.begin();       // Initialize SPI bus
  mfrc522.PCD_Init(); // Initialize MFRC522

  // --- WiFi Connection ---
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // --- Web Server Initialization ---
  server.on("/", HTTP_GET, handleRoot);
  server.begin();
  Serial.println("HTTP Server started.");
}

void loop() {
  server.handleClient(); // Handle incoming web requests

  // Check if a new tag is present
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    
    // Get the UID of the tag
    byte *uid = mfrc522.uid.uidByte;
    String uidString = uidToString(uid);
    String logEntry = "";
    
    // Check authorization
    if (isAuthorized(uid)) {
      logEntry = "Access GRANTED for UID: " + uidString;
      Serial.println(logEntry);
      addToLog(logEntry);
      successTone();
      unlockDoor(); // Unlocks and re-locks after duration
    } else {
      logEntry = "Access DENIED for UID: " + uidString;
      Serial.println(logEntry);
      addToLog(logEntry);
      errorAlarm();
    }
    
    // Halt reading (prevents continuous re-reading)
    mfrc522.PICC_HaltA();
  }
  
  delay(50); 
}
