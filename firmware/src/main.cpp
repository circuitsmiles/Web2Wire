#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <DNSServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>        // Required for explicit SPI bus setup
#include "flag_drawing.h" // <<< NEW: Include for all flag drawing logic

// --- DISPLAY PINS (Adjusted for user's wiring) ---
#define TFT_CS 5  // Chip Select pin
#define TFT_DC 6  // Data Command pin
#define TFT_RST 4 // Reset pin
// *** CORRECTED PINS BASED ON USER FEEDBACK ***
#define TFT_MOSI 35 // MOSI/SDA is connected to GPIO 35
#define TFT_SCLK 36 // SCK/CLK/SCL is connected to GPIO 36

// Display object (1.9 inch, 170x320 resolution)
// NOTE: Using the full constructor to explicitly define all pins (CS, DC, MOSI, SCLK, RST)
// This is the definition of the extern object declared in flag_drawing.h
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// --- NVS & AP CONFIGURATION CONSTANTS (Unchanged) ---
Preferences preferences;
const char *PREFS_NAMESPACE = "assistant_cfg";
const char *PREF_SSID = "ssid_trinity";
const char *PREF_PASS = "password";
const IPAddress apIP(192, 168, 4, 1);
const IPAddress netMask(255, 255, 255, 0);
const byte DNS_PORT = 53;
DNSServer dnsServer;

// --- DEVICE & NETWORK CONFIGURATION (Unchanged) ---
const int LED_PIN = 48; // Built-in NeoPixel
const int NUM_LEDS = 1;
const int BLINK_DURATION_MS = 300; // Faster blink for 5 phases
const int HTTP_PORT = 80;

// --- STATUS REPORTING CONFIGURATION (Unchanged) ---
const long STATUS_INTERVAL_MS = 5000;
unsigned long lastStatusPrint = 0;
const long RECONNECT_COOLDOWN_MS = 10000;
unsigned long lastReconnectAttempt = 0;

// --- API ENDPOINTS (Unchanged) ---
const char* ESP32_API_SECRET = "add_auth_key_here";
const char *COMPLETION_URL = "https://api.circuitsmiles.dev/api/job/complete";

// --- GLOBAL OBJECTS (Unchanged) ---
WebServer server(HTTP_PORT);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- NON-BLOCKING ACTION CONTROL (Updated to ensure proper state management) ---
// 5 Phases for the 5-color blink
enum ActionState
{
    ACTION_IDLE = 0,
    ACTION_BLINK_1, // Red
    ACTION_BLINK_2, // Orange
    ACTION_BLINK_3, // Yellow
    ACTION_BLINK_4, // Green
    ACTION_BLINK_5, // Blue
    ACTION_COMPLETED
};

ActionState currentActionState = ACTION_IDLE;
unsigned long actionStartTime = 0;
uint32_t actionOriginalColor = 0;
bool jobDataChanged = true; // New flag to trigger initial draw and change updates

// Array of 5 colors for the blink sequence (R, O, Y, G, B)
const uint32_t BLINK_COLORS[] = {
    strip.Color(255, 0, 0),   // 1. Red
    strip.Color(255, 128, 0), // 2. Orange
    strip.Color(255, 255, 0), // 3. Yellow
    strip.Color(0, 255, 0),   // 4. Green
    strip.Color(0, 0, 255)     // 5. Blue
};
const int NUM_BLINK_COLORS = 5;

// --- JOB DATA STRUCTURE (Unchanged) ---
struct JobData
{
    String name;
    String country;
    String flag; // Country code (e.g., "FR", "DE", "US")
};

JobData currentJobData = {"Waiting", "for next", "JOB"}; // Default state

// --- FUNCTION PROTOTYPES (Updated) ---
void setLEDColor(uint8_t r, uint8_t g, uint8_t b);
void setLEDColor(uint32_t color);
void startActionSequence(const JobData &data);
void runAction();
void handleStartBlink();
bool notifyServerOfCompletion();
void printWifiStatus();
void drawJobData(const JobData &data);
// drawFlag is now prototyped in flag_drawing.h

// --- CUSTOM FLAG DRAWING LOGIC (REMOVED - now in flag_drawing.cpp) ---
// --- DISPLAY FUNCTIONS ---

/**
 * @brief Initializes the ST7789 display for 170x320 resolution.
 */
void setupTFT()
{
    // Explicitly set up the SPI bus before initializing the display.
    // Arguments: SCLK, MISO, MOSI, CS (MISO is -1 as ST7789 is write-only)
    SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

    // Initialize ST7789 for 240x320 (common for this hardware)
    tft.init(170, 320);

    tft.setRotation(1); // Set to landscape mode (320 wide, 170 tall)
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextWrap(true);
    tft.setFont(); // Use default font
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.println("TRINITY PROTOCOL V1.2");
    tft.println("TFT Init OK (320x170).");
    tft.println("-------------------------");
}

// Helper function to draw text, wrapping it to the next line if it exceeds max length.
// Returns the final Y position after printing.
int wrapAndPrintText(const String& text, int x, int y, int maxCharsPerLine, int lineHeight, uint16_t color) {
    tft.setTextColor(color);
    tft.setTextSize(2);
    
    // Check if wrapping is needed
    if (text.length() <= maxCharsPerLine) {
        tft.setCursor(x, y);
        tft.print(text);
        return y + lineHeight; // Advance Y by one line height
    }

    // --- Word Wrapping Logic ---
    String line1 = "";
    int lastSpaceIndex = -1;
    
    // Iterate through the string, finding the best place to break before maxCharsPerLine
    for (int i = 0; i < text.length(); i++) {
        char currentChar = text.charAt(i);

        // Track the position of the last space encountered
        if (currentChar == ' ') {
            lastSpaceIndex = i;
        }

        // Check if adding the next character would exceed the limit
        if (i == maxCharsPerLine) {
            
            if (lastSpaceIndex != -1) {
                // Break at the last full space found before the limit
                line1 = text.substring(0, lastSpaceIndex);
            } else {
                // No space found (very long single word), force break at max length
                line1 = text.substring(0, maxCharsPerLine);
                lastSpaceIndex = maxCharsPerLine;
            }
            
            // Print Line 1
            tft.setCursor(x, y);
            tft.print(line1);
            y += lineHeight; // Advance Y to the next line
            
            // Calculate Line 2: start from after the break point (plus one for the space if broken by space)
            int startOfLine2 = (lastSpaceIndex != -1 && text.charAt(lastSpaceIndex) == ' ') ? lastSpaceIndex + 1 : lastSpaceIndex;
            String line2 = text.substring(startOfLine2);

            // Print Line 2 (trimmed to fit, if necessary)
            tft.setCursor(x, y);
            // This handles names longer than 30 characters by just showing the start of the remainder
            tft.print(line2.substring(0, maxCharsPerLine)); 
            
            return y + lineHeight; // Advance Y for the final position
        }
    }
    
    // Fallback: should not be reached if length check worked, but good practice
    tft.setCursor(x, y);
    tft.print(text);
    return y + lineHeight;
}


void drawJobData(const JobData &data)
{
    tft.fillScreen(ST77XX_BLACK);

    // The screen is 320 pixels wide and 170 pixels tall (Rotation 1)
    int margin = 5;
    int lineH = 20;
    // --- MODIFICATION: CHANGED FROM 15 TO 14 CHARACTERS ---
    const int MAX_CHARS_PER_LINE = 14; 

    tft.fillScreen(ST77XX_BLACK);
    int halfWidth = tft.width() / 2; // 160 pixels

    // Title on the left
    tft.setTextSize(2);
    tft.setCursor(margin, margin);
    tft.setTextColor(ST77XX_CYAN);
    tft.println("INCOMING JOB:");

    // Separator line
    tft.drawFastVLine(halfWidth, 0, tft.height(), tft.color565(50, 50, 50));

    // Text Block (Left Side - 160 wide)
    int yPos = margin + lineH + 5;
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(margin, yPos);
    tft.print("Name: ");

    yPos += lineH; // Move to the line below "Name: "

    // yPos is updated by the helper function to the final position after printing one or two lines
    yPos = wrapAndPrintText(data.name, margin, yPos, MAX_CHARS_PER_LINE, lineH, ST77XX_YELLOW);

    yPos += 5; // Extra spacing before the next section
    tft.setTextSize(2);
    tft.setCursor(margin, yPos);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Origin:");

    yPos += lineH;
    tft.setTextSize(2);
    tft.setCursor(margin, yPos);
    tft.setTextColor(ST77XX_YELLOW);
    
    // Apply word wrapper for country/origin
    yPos = wrapAndPrintText(data.country, margin, yPos, MAX_CHARS_PER_LINE, lineH, ST77XX_YELLOW);


    yPos += 5; // Add a little space before CODE
    tft.setTextSize(1);
    tft.setCursor(margin, yPos);
    tft.setTextColor(ST77XX_RED);
    tft.print("CODE: ");
    tft.setTextColor(ST77XX_ORANGE);
    tft.print(data.flag);

    // Flag Block (Right Side - 160 wide) 
    // Draw the 32x20 flag scaled up by 4x (128x80 pixels total)
    int flagScale = 4;
    int flagW = FLAG_W * flagScale; // 128
    int flagH = FLAG_H * flagScale; // 80

    // Centered in the right half: 160 + (160 - 128) / 2 = 176
    int flagX = halfWidth + (halfWidth - flagW) / 2; // 176
    // Centered vertically in the available space
    int flagY = (tft.height() - flagH) / 2; // (170 - 80) / 2 = 45

    drawFlag(data.flag, flagX, flagY, flagScale); // Uses the function from flag_drawing.cpp

    // Status text at the bottom
    tft.setTextSize(1);
    tft.setCursor(margin, tft.height() - 15);
    tft.setTextColor(ST77XX_GREEN);

    // Display the current status dynamically
    if (currentActionState == ACTION_IDLE)
    {
        tft.print("STATUS: READY. AWAITING TRANSMISSION.");
    }
    else
    {
        tft.print("STATUS: PROCESSING... LED BLINK x5");
    }
}

// ... [CONFIG_HTML remains the same] ...
const char CONFIG_HTML[] = R"raw(
<!DOCTYPE html>
<html>
<head>
<title>Trinity Wi-Fi Setup</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
 /* Base styles - Dark, Grid-like background */
 body { 
 font-family: 'Courier New', monospace; 
 background-color: #0d0d0d; 
 color: #00ff00; /* Neon Green */
 margin: 0; 
 padding: 20px; 
 /* Subtle grid pattern for sci-fi look */
 background-image: linear-gradient(0deg, transparent 24%, rgba(0, 255, 0, 0.05) 25%, rgba(0, 255, 0, 0.05) 26%, transparent 27%, transparent 74%, rgba(0, 255, 0, 0.05) 75%, rgba(0, 255, 0, 0.05) 76%, transparent 77%, transparent), linear-gradient(90deg, transparent 24%, rgba(0, 255, 0, 0.05) 25%, rgba(0, 255, 0, 0.05) 26%, transparent 27%, transparent 74%, rgba(0, 255, 0, 0.05) 75%, rgba(0, 255, 0, 0.05) 76%, transparent 77%, transparent);
 background-size: 50px 50px;
 }
 /* Container - Dark metallic panel */
 .container { 
 max-width: 400px; 
 margin: 60px auto 0; 
 background: rgba(34, 34, 34, 0.95); /* Dark Gray/Black */
 padding: 30px; 
 border-radius: 10px; 
 border: 2px solid #00ccff; /* Sci-fi Blue Border */
 box-shadow: 0 0 20px rgba(0, 255, 0, 0.5); /* Neon Green Glow */
 }
 h1 { 
 color: #00ff00; 
 text-align: center; 
 text-shadow: 0 0 10px #00ff00; 
 margin-bottom: 25px;
 }
 label {
 display: block;
 margin-top: 15px;
 color: #00ccff; /* Light Blue/Cyan */
 font-size: 1.1em;
 }
 /* Input fields - Look like glowing data ports */
 input[type="text"], input[type="password"] { 
 width: 100%; 
 padding: 12px; 
 margin: 8px 0 20px 0; 
 display: inline-block; 
 border: 1px solid #00ccff; /* Sci-fi Blue Border */
 background-color: #111111; /* Very dark input background */
 color: #00ff00; /* Neon Green text input */
 border-radius: 4px; 
 box-sizing: border-box; 
 box-shadow: 0 0 5px rgba(0, 255, 0, 0.3);
 transition: box-shadow 0.3s, border-color 0.3s;
 }
 input[type="text"]:focus, input[type="password"]:focus {
 border-color: #00ffff;
 box-shadow: 0 0 10px #00ffff;
 outline: none;
 }
 /* Submit button - Bright green action element */
 input[type="submit"] { 
 background-color: #00ff00; 
 color: #111111; /* Dark text on bright button */
 padding: 14px 20px; 
 margin: 15px 0 8px 0; 
 border: none; 
 border-radius: 4px; 
 cursor: pointer; 
 width: 100%; 
 font-size: 16px; 
 font-weight: bold;
 box-shadow: 0 0 10px rgba(0, 255, 0, 0.7); /* Stronger glow */
 transition: background-color 0.3s, box-shadow 0.3s;
 }
 input[type="submit"]:hover { 
 background-color: #33ff33; 
 box-shadow: 0 0 15px #00ff00; 
 }
 .note { 
 color: #aaaaaa; 
 font-size: 0.9em; 
 text-align: center; 
 margin-top: 25px; 
 }
</style>
</head>
<body>
<div class="container">
<h1>TRINITY PROTOCOL SETUP</h1>
<form method="get" action="/save">
 <label for="ssid">NETWORK SSID:</label>
 <input type="text" id="ssid" name="ssid" required>

 <label for="pass">SECURITY KEY:</label>
 <input type="password" id="pass" name="pass">

 <input type="submit" value="ESTABLISH CONNECTION">
</form>
<div class="note">// DATA WILL BE ENCRYPTED AND STORED IN NVS FLASH MEMORY. //</div>
</div>
</body>
</html>
)raw";
// --- WEB SERVER HANDLERS (for AP Portal - Unchanged) ---
void handleRoot()
{
    server.send(200, "text/html", CONFIG_HTML);
}

void handleSave()
{
    String ssid = server.arg("ssid");
    String password = server.arg("pass");
    if (ssid.length() > 0)
    {
         preferences.begin(PREFS_NAMESPACE, false);
         preferences.putString(PREF_SSID, ssid);
         preferences.putString(PREF_PASS, password);
         preferences.end();
         Serial.println("Credentials saved securely to NVS. Rebooting...");
         server.send(200, "text/html", "<body style='background-color:#0d0d0d; color:#00ff00; font-family: monospace; text-align:center; padding-top: 100px;'><h1>TRANSMISSION SUCCESSFUL</h1><p>Credentials saved. Initiating system reboot and connection attempt...</p></body>");
         delay(3000);
         ESP.restart();
    }
    else
    {
         server.send(400, "text/html", "<body style='background-color:#0d0d0d; color:red; font-family: monospace; text-align:center; padding-top: 100px;'><h1>ERROR: INPUT FAILED</h1><p>SSID cannot be empty. Check protocol parameters.</p></body>");
    }
}
bool startAPPortal()
{
    Serial.println("\n--- STARTING AP SETUP MODE ---");
    Serial.println("Connect to Wi-Fi 'Trinity_Setup' and browse to any website.");
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAPConfig(apIP, apIP, netMask);
    // FIX: Explicitly set the channel (e.g., 6) to improve AP visibility/stability on ESP32-S3
    WiFi.softAP("Trinity_Setup", "", 6);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    dnsServer.start(DNS_PORT, "*", apIP);
    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.begin();
    setLEDColor(0, 50, 50);
    return false;
}
bool connectWiFi()
{
    preferences.begin(PREFS_NAMESPACE, true);
    String ssid = preferences.getString(PREF_SSID, "");
    String pass = preferences.getString(PREF_PASS, "");
    preferences.end();
    if (ssid.length() == 0)
    {
         return startAPPortal();
    }
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    setLEDColor(50, 50, 0);
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
         if (millis() - startTime > 30000)
         {
             Serial.println("\nConnection attempt failed. Launching AP portal...");
             return startAPPortal();
         }
         delay(500);
    }
    setLEDColor(0, 0, 0);
    Serial.println("WiFi Connected.");
    return true;
}

// --- HTTP HANDLERS & CORE LOGIC (Unchanged) ---

void setLEDColor(uint8_t r, uint8_t g, uint8_t b)
{
    strip.setPixelColor(0, strip.Color(r, g, b));
    strip.show();
}

void setLEDColor(uint32_t color)
{
  strip.setPixelColor(0, color);
  strip.show();
}

// --- ACTION LOGIC (Unchanged) ---
void startActionSequence(const JobData &data)
{
    // Save current data
    currentJobData = data;
    jobDataChanged = true; // Signal display update

    // Start blink sequence
    actionStartTime = millis();
    currentActionState = ACTION_BLINK_1;
    // Set the initial color
    setLEDColor(255, 0, 0);
    Serial.printf("Action started for Job: %s from %s (%s)\n", data.name.c_str(), data.country.c_str(), data.flag.c_str());
}

void runAction()
{
    if (currentActionState == ACTION_IDLE || currentActionState == ACTION_COMPLETED)
    {
         return;
    }

    unsigned long currentMillis = millis();
    int phaseIndex = currentActionState - ACTION_BLINK_1; // 0 to 4

    // Time check for phase transition
    if (currentMillis - actionStartTime >= BLINK_DURATION_MS)
    {
         // Move to the next state
         currentActionState = (ActionState)(currentActionState + 1);
         actionStartTime = currentMillis; // Reset timer

         if (currentActionState <= ACTION_BLINK_5)
         {
             // Set the next color
             setLEDColor(BLINK_COLORS[currentActionState - ACTION_BLINK_1]);
         }
         else
         {
             // Sequence complete
             Serial.println("Action sequence complete. Notifying server...");
             currentActionState = ACTION_COMPLETED;
             if (notifyServerOfCompletion())
             {
                  Serial.println("Server notified. Transitioning to IDLE.");
             }
             else
             {
                  Serial.println("Failed to notify server.");
             }
             currentActionState = ACTION_IDLE; // Final transition
             jobDataChanged = true;         // Force redraw back to idle state
         }
    }
}
// --- HTTP HANDLERS (Unchanged) ---
void handleStartBlink()
{
    if (currentActionState != ACTION_IDLE)
    {
         server.send(429, "application/json", "{\"status\": \"busy\", \"message\": \"Device is currently processing a job.\"}") ;
         return;
    }

    // Check if content is JSON
    if (server.hasArg("plain") == false)
    {
         server.send(400, "application/json", "{\"status\": \"error\", \"message\": \"Expected JSON payload.\"}") ;
         return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (error)
    {
         Serial.print("JSON deserialization failed: ");
         Serial.println(error.c_str());
         server.send(400, "application/json", "{\"status\": \"error\", \"message\": \"Invalid JSON payload.\"}") ;
         return;
    }

    JobData incomingData;
    incomingData.name = doc["name"] | "Unknown Task";
    incomingData.country = doc["country"] | "Unknown Location";
    incomingData.flag = doc["flag"] | "??"; // Default to a simple unknown code

    startActionSequence(incomingData);

    // Respond immediately
    server.send(200, "application/json", "{\"status\": \"processing\", \"message\": \"Job accepted. Initiating processing sequence.\"}") ;
}
bool notifyServerOfCompletion()
{
    HTTPClient http;
    http.begin(COMPLETION_URL);
    http.addHeader("Content-Type", "application/json");

    String authHeaderValue = "Bearer "; 
    authHeaderValue += ESP32_API_SECRET; 

    http.addHeader("Authorization", authHeaderValue);

    JsonDocument doc;
    doc["job_name"] = currentJobData.name;
    doc["device_id"] = WiFi.macAddress();
    doc["status"] = "completed";

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);

    http.end();

    if (httpResponseCode > 0)
    {
         Serial.printf("Server Response: %d\n", httpResponseCode);
         return true;
    }
    else
    {
         Serial.printf("Error notifying server: %s\n", http.errorToString(httpResponseCode).c_str());
         return false;
    }
}
void printWifiStatus()
{
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// --- CORE SETUP (Fixed Initialization) ---
void setup()
{
    Serial.begin(115200);
    Serial.println("--- SETUP STARTED SUCCESSFULLY ---");

    // 1. Initialize NeoPixel
    strip.begin();
    strip.setBrightness(50);
    setLEDColor(0, 0, 0);

    // 2. Initialize TFT Display
    setupTFT();

    // *** NEW VISUAL TEST *** // This brief color change confirms the display is working and receiving data
    tft.fillScreen(ST77XX_MAGENTA);
    delay(500);
    tft.fillScreen(ST77XX_BLACK);
    // *** END VISUAL TEST ***

    // 3. Connect to WiFi or Start AP Portal
    if (connectWiFi())
    {
         Serial.print("SUCCESS! Device IP: ");
         Serial.println(WiFi.localIP());

         // 4. Setup the Web Server for Job Requests
         server.on("/api/job/start", HTTP_POST, handleStartBlink);
         server.begin();
         Serial.println("HTTP Job Server started, listening for POST on /api/job/start");

         printWifiStatus();
         lastStatusPrint = millis();
         lastReconnectAttempt = millis();
    }
    else
    {
         Serial.println("Device is now in AP Setup Mode.");
    }
}

// --- MAIN LOOP (Updated to fix constant drawing) ---
void loop()
{
    server.handleClient();
    if (WiFi.getMode() == WIFI_MODE_AP)
    {
         dnsServer.processNextRequest();
    }
    else
    {
         unsigned long currentMillis = millis();

         // Check and run the non-blocking hardware action
         runAction();

         // Only redraw the display when we are idle AND the job data has changed
         if (currentActionState == ACTION_IDLE && jobDataChanged)
         {
             drawJobData(currentJobData);
             jobDataChanged = false; // Reset flag until a new job comes in
             setLEDColor(0, 0, 0);   // Keep LED off when idle
         }

         // Periodic status print
         if (currentMillis - lastStatusPrint >= STATUS_INTERVAL_MS)
         {
             printWifiStatus();
             lastStatusPrint = currentMillis;
         }

         // WiFi connection health check (simple reconnect logic)
         if (WiFi.status() != WL_CONNECTED && currentMillis - lastReconnectAttempt >= RECONNECT_COOLDOWN_MS)
         {
             Serial.println("WiFi disconnected. Attempting reconnect...");
             WiFi.reconnect();
             lastReconnectAttempt = currentMillis;
         }
    }
}