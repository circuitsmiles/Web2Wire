#include <WiFi.h>
// Required library for running an HTTP server on the ESP32
#include <WebServer.h>
#include <HTTPClient.h>

// --- NEOPIXEL LIBRARY INCLUSION ---
#include <Adafruit_NeoPixel.h> 

// --- DEVICE & NETWORK CONFIGURATION ---
// **ACTION REQUIRED: Update these with your actual WiFi credentials!**
const char* WIFI_SSID = ""; 
const char* WIFI_PASSWORD = "";

const int LED_PIN = 48; // Common GPIO for the built-in NeoPixel on ESP32-S3 DevKit
const int NUM_LEDS = 1; // Only one NeoPixel on the DevKit
const int BLINK_DURATION_MS = 1000; // 1 second per color
const int NUM_BLINKS = 3; 
const int HTTP_PORT = 80; // Standard HTTP port for the device's server

// --- STATUS REPORTING CONFIGURATION ---
const long STATUS_INTERVAL_MS = 5000; // 5 seconds interval for status reports
unsigned long lastStatusPrint = 0; // Variable to store the last time the status was printed

// --- RECONNECTION TIMING CONFIGURATION ---
const long RECONNECT_COOLDOWN_MS = 10000; // 10 seconds cooldown between reconnection attempts
unsigned long lastReconnectAttempt = 0; // Variable to track last reconnection attempt time

// --- API ENDPOINTS ---
// ESP32 posts to this URL after the action is complete
const char* COMPLETION_URL = "http://192.168.2.10:5000/api/job/complete";

// --- GLOBAL OBJECTS ---
// Web server object that will listen for incoming job requests from the API server
WebServer server(HTTP_PORT); 

// NeoPixel object to control the onboard RGB LED
// Parameters: Number of LEDs, Pin number, Pixel Type (NEO_GRB is common for ESP32-S3)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);


// --- FUNCTION PROTOTYPES ---
void connectToWiFi();
void setLEDColor(uint8_t r, uint8_t g, uint8_t b);
void performBlinkSequence();
void handleStartBlink();
bool notifyServerOfCompletion();
void printWifiStatus();


// --- CORE SETUP ---
void setup() {
    Serial.begin(115200);
    // CRITICAL DEBUG: This line should print immediately if Serial is working.
    Serial.println("--- SETUP STARTED SUCCESSFULLY ---"); 
    
    // Initialize the NeoPixel strip
    strip.begin(); 
    // Set a moderate brightness (0-255). Recommended not to use 255 to save power.
    strip.setBrightness(50); 
    setLEDColor(0, 0, 0); // Start with LED off

    Serial.println("\n--- Web2Wire ESP32 S3 Firmware Boot ---");
    // 1. Connect to WiFi
    connectToWiFi();
    
    // 2. Setup the Web Server
    // Define the endpoint the API server will POST to for starting a job
    server.on("/api/job/start", HTTP_POST, handleStartBlink);
    server.begin();
    Serial.println("HTTP Server started, listening for POST on /api/job/start");
    
    // Initial status print
    printWifiStatus();
    // Initialize both timers to the current time
    lastStatusPrint = millis();
    lastReconnectAttempt = millis();
}


// --- MAIN LOOP ---
void loop() {
    unsigned long currentMillis = millis();
    
    // 1. Core Server Functionality (Non-blocking)
    // Continuously check for and handle any pending incoming client requests
    server.handleClient();

    // 2. Periodic Status Report (Non-blocking Timer)
    if (currentMillis - lastStatusPrint >= STATUS_INTERVAL_MS) {
        lastStatusPrint = currentMillis;
        printWifiStatus();
    }
    
    // 3. Periodic WiFi Status Check and Reconnection Logic (Non-blocking Timer)
    if (WiFi.status() != WL_CONNECTED) {
        // Only attempt to reconnect if the cooldown period has passed
        if (currentMillis - lastReconnectAttempt >= RECONNECT_COOLDOWN_MS) { 
             Serial.println("WiFi disconnected. Attempting reconnection...");
             connectToWiFi();
             // IMPORTANT: Reset the attempt timer immediately after the attempt is made
             lastReconnectAttempt = currentMillis; 
        }
    }
}


// --- WIFI CONNECTIVITY ---
void connectToWiFi() {
    // Explicitly start connecting using the defined credentials
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
    unsigned long timeout = millis() + 30000; // 30-second timeout
    
    Serial.print("Connecting to network: ");
    Serial.print(WIFI_SSID);

    // This is a blocking loop during the connection attempt
    while (WiFi.status() != WL_CONNECTED && millis() < timeout) {
        delay(1000);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
    } else {
        Serial.printf("\nFailed to connect to %s (Error: %d). Will retry in %lu seconds.\n", 
            WIFI_SSID, 
            WiFi.status(), 
            RECONNECT_COOLDOWN_MS / 1000);
    }
}


// --- PERIODIC WIFI STATUS PRINTER ---
void printWifiStatus() {
    Serial.println("-------------------------------------");
    Serial.print("WiFi Status: ");
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("CONNECTED");
        Serial.print("Local IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("Server Port: ");
        Serial.println(HTTP_PORT);
        Serial.printf("Uptime (ms): %lu\n", millis());
    } else {
        Serial.println("DISCONNECTED (Status Code: " + String(WiFi.status()) + ")");
        Serial.println("IP/Port: N/A");
    }
    Serial.println("-------------------------------------");
}


// --- LED CONTROL (Uses Adafruit_NeoPixel Library) ---
void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
    // Set the color for the first (and only) pixel (index 0)
    strip.setPixelColor(0, strip.Color(r, g, b)); 
    strip.show(); // Send the updated color data to the NeoPixel

    if (r > 0 || g > 0 || b > 0) {
        Serial.printf("[LED] Set color: R=%d, G=%d, B=%d\n", r, g, b);
    } else {
        Serial.println("[LED] Turned OFF");
    }
}


// --- INCOMING JOB HANDLER (Triggered by API Server POST) ---
void handleStartBlink() {
    // 1. Acknowledge the request immediately (HTTP 202 Accepted)
    server.send(202, "application/json", "{\"message\": \"Job accepted, processing started.\" }");
    Serial.println("-> Incoming request accepted. Acknowledged to server.");
    
    // 2. Perform the hardware action (Blocking)
    performBlinkSequence();
    
    // 3. Inform the server of action completion (Step 4 of the flow)
    if (notifyServerOfCompletion()) {
        Serial.println("-> Server notified successfully. Queue status update triggered.");
    } else {
        Serial.println("-> ERROR: Failed to notify server of completion.");
    }
}


// --- HARDWARE ACTION SEQUENCE (Step 3) ---
void performBlinkSequence() {
    Serial.println("[ACTION] Starting blink sequence (R-B-G)...");
    
    // Color 1: RED
    setLEDColor(255, 0, 0); 
    delay(BLINK_DURATION_MS);

    // Color 2: BLUE
    setLEDColor(0, 0, 255); 
    delay(BLINK_DURATION_MS);
    
    // Color 3: GREEN
    setLEDColor(0, 255, 0); 
    delay(BLINK_DURATION_MS);
    
    // Turn off LED
    setLEDColor(0, 0, 0);
    Serial.println("[ACTION] Blink sequence complete.");
}


// --- SERVER NOTIFICATION (Step 4) ---
bool notifyServerOfCompletion() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(COMPLETION_URL);
        http.addHeader("Content-Type", "application/json");

        // Send a simple body to confirm completion
        int httpResponseCode = http.POST("{\"status\": \"completed\"}"); 

        if (httpResponseCode > 0) {
            Serial.printf("[SERVER] Completion POST sent. Response code: %d\n", httpResponseCode);
            http.end();
            return true;
        } else {
            Serial.printf("[SERVER] Completion POST failed. Error: %s\n", http.errorToString(httpResponseCode).c_str());
            http.end();
            return false;
        }
    }
    return false;
}