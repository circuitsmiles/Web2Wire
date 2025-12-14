#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <DNSServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h> // Required for explicit SPI bus setup
#include "flag_data.h" // <<< Include for shared flag data

// --- DISPLAY PINS (Adjusted for user's wiring) ---
#define TFT_CS 5    // Chip Select pin
#define TFT_DC 6    // Data Command pin
#define TFT_RST 4   // Reset pin
// *** CORRECTED PINS BASED ON USER FEEDBACK ***
#define TFT_MOSI 35 // MOSI/SDA is connected to GPIO 35
#define TFT_SCLK 36 // SCK/CLK/SCL is connected to GPIO 36

// Display object (1.9 inch, 170x320 resolution)
// NOTE: Using the full constructor to explicitly define all pins (CS, DC, MOSI, SCLK, RST)
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
const char *COMPLETION_URL = "http://192.168.2.10:5000/api/job/complete";

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
    strip.Color(255, 0, 0),    // 1. Red
    strip.Color(255, 128, 0), // 2. Orange
    strip.Color(255, 255, 0), // 3. Yellow
    strip.Color(0, 255, 0),    // 4. Green
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

// --- FUNCTION PROTOTYPES (Unchanged) ---
void setLEDColor(uint8_t r, uint8_t g, uint8_t b);
void startActionSequence(const JobData &data);
void runAction();
void handleStartBlink();
bool notifyServerOfCompletion();
void printWifiStatus();
void drawJobData(const JobData &data);
void drawFlag(const String &flagCode, int x, int y, int scale);


// --- FLAG DRAWING FUNCTION (Unchanged) ---

/**
 * @brief Draws a flag bitmap from PROGMEM at the specified coordinates.
 * @param flagCode The 2-letter country code to look up.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param scale The scaling factor (e.g., 1 for 32x20, 3 for 96x60).
 */
void drawFlag(const String &flagCode, int x, int y, int scale)
{

    // 1. Convert to upper case and search the lookup table
    String code = flagCode;
    code.toUpperCase();
    const uint16_t *bitmapData = nullptr;

    for (int i = 0; i < NUM_FLAGS; i++)
    {
        if (code.equals(FLAG_LOOKUP[i].code))
        {
            bitmapData = FLAG_LOOKUP[i].bitmap;
            break;
        }
    }

    int scaledW = FLAG_W * scale;
    int scaledH = FLAG_H * scale;

    // Fallback: If code is not found, draw a simple 'Unknown' pattern
    if (bitmapData == nullptr)
    {
        tft.drawRect(x, y, scaledW, scaledH, ST77XX_RED);
        tft.fillRect(x + 1, y + 1, scaledW - 2, scaledH - 2, ST77XX_BLACK);
        tft.setCursor(x + 5, y + (scaledH / 2) - 5);
        tft.setTextSize(1);
        tft.setTextColor(ST77XX_RED);
        tft.print(code.c_str());
        return;
    }

    // 2. Draw the bitmap using fast pixel writing with scaling
    tft.startWrite();
    // Set the address window for the entire scaled flag area
    tft.setAddrWindow(x, y, x + scaledW - 1, y + scaledH - 1);

    for (int row = 0; row < FLAG_H; row++)
    {
        for (int r = 0; r < scale; r++)
        { // Row scaling loop: Repeat row 'scale' times
            for (int col = 0; col < FLAG_W; col++)
            {
                // Read 16-bit color from PROGMEM (Flash memory)
                uint16_t color = pgm_read_word(&bitmapData[row * FLAG_W + col]); 

                // Column scaling loop: Write the color 'scale' times
                tft.writeColor(color, scale);
            }
        }
    }
    tft.endWrite();
    tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); // Reset window
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
// --- WEB SERVER HANDLERS (for AP Portal) ---
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

// --- DISPLAY FUNCTIONS ---

/**
 * @brief Initializes the ST7789 display for 170x320 resolution.
 */
void setupTFT()
{
    // Explicitly set up the SPI bus before initializing the display.
    // This is critical if the display uses non-default SCLK/MOSI pins.
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

/**
 * @brief Displays the job data (name, country, and a flag representation).
 * @param data The JobData struct containing the information.
 */
void drawJobData(const JobData &data)
{
    tft.fillScreen(ST77XX_BLACK);

    // The screen is 320 pixels wide and 170 pixels tall (Rotation 1)
    int margin = 5;
    int lineH = 20;

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

    yPos += lineH;
    tft.setTextSize(2);
    tft.setCursor(margin, yPos);
    tft.setTextColor(ST77XX_YELLOW);
    tft.print(data.name.substring(0, 15)); // 15 chars fits 160 width

    yPos += lineH + 5;
    tft.setTextSize(2);
    tft.setCursor(margin, yPos);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Origin:");

    yPos += lineH;
    tft.setTextSize(2);
    tft.setCursor(margin, yPos);
    tft.setTextColor(ST77XX_YELLOW);
    tft.print(data.country.substring(0, 15));

    yPos += lineH + 5;
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

    // Centered in the right half: 160 + (160 - 128) / 2 = 160 + 16 = 176
    int flagX = halfWidth + (halfWidth - flagW) / 2; // 176
    // Centered vertically in the available space
    int flagY = (tft.height() - flagH) / 2; // (170 - 80) / 2 = 45

    drawFlag(data.flag, flagX, flagY, flagScale);

    // Status text at the bottom
    tft.setTextSize(1);
    tft.setCursor(margin, tft.height() - 15);
    tft.setTextColor(ST77XX_GREEN);
    
    // Display the current status dynamically
    if (currentActionState == ACTION_IDLE) {
        tft.print("STATUS: READY. AWAITING TRANSMISSION.");
    } else {
        tft.print("STATUS: PROCESSING... LED BLINK x5");
    }
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
            setLEDColor(0, 0, 0); // Keep LED off when idle
        }

        // Periodic status print
        if (currentMillis - lastStatusPrint >= STATUS_INTERVAL_MS)
        {
            lastStatusPrint = currentMillis;
            printWifiStatus();
        }

        // WiFi connection maintenance
        if (WiFi.status() != WL_CONNECTED)
        {
            setLEDColor(20, 0, 0);

            if (currentMillis - lastReconnectAttempt >= RECONNECT_COOLDOWN_MS)
            {
                Serial.println("WiFi disconnected. Attempting reconnection...");
                WiFi.reconnect();
                lastReconnectAttempt = currentMillis;
            }
        }
    }
    yield();
}

// --- PERIODIC WIFI STATUS PRINTER (Unchanged) ---
void printWifiStatus()
{
    Serial.println("-------------------------------------");
    Serial.print("WiFi Status: ");

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("CONNECTED");
        Serial.print("Local IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("Server Port: ");
        Serial.println(HTTP_PORT);
        Serial.printf("Uptime (ms): %lu\n", millis());
    }
    else
    {
        Serial.println("DISCONNECTED (Status Code: " + String(WiFi.status()) + ")");
        Serial.println("IP/Port: N/A");
    }
    Serial.println("-------------------------------------");
}

// --- LED CONTROL (Unchanged) ---
void setLEDColor(uint8_t r, uint8_t g, uint8_t b)
{
    strip.setPixelColor(0, strip.Color(r, g, b));
    strip.show();
}

// --- INCOMING JOB HANDLER (Updated to set the jobDataChanged flag) ---
void handleStartBlink()
{
    // 1. Acknowledge the request immediately
    server.send(202, "application/json", "{\"message\": \"Job accepted, processing started (non-blocking).\" }");
    Serial.println("-> Incoming request accepted. Acknowledged to server.");

    // 2. Parse the JSON body
    // Determine the appropriate size for the JSON document
    JsonDocument doc;
    String requestBody = server.arg("plain");

    DeserializationError error = deserializeJson(doc, requestBody);

    if (error)
    {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }

    // 3. Extract data and update global state
    currentJobData.name = doc["name"] | "UNKNOWN NAME";
    currentJobData.country = doc["country"] | "UNKNOWN COUNTRY";
    currentJobData.flag = doc["flag"] | "N/A";
    
    // *** CRITICAL UPDATE: Set flag so the idle loop knows to redraw later ***
    jobDataChanged = true; 

    Serial.printf("-> Job Data: Name=%s, Country=%s, Flag=%s\n",
                  currentJobData.name.c_str(), currentJobData.country.c_str(), currentJobData.flag.c_str());

    // 4. Start the non-blocking hardware action
    if (currentActionState == ACTION_IDLE)
    {
        startActionSequence(currentJobData);
    }
    else
    {
        Serial.println("WARNING: Action already running. Ignoring new job request.");
    }
}

// --- HARDWARE ACTION SEQUENCE START (Updated to ensure flag is drawn when sequence starts) ---
void startActionSequence(const JobData &data)
{
    Serial.println("[ACTION] Starting non-blocking 5-color blink sequence...");

    // Update the display immediately when the job starts
    drawJobData(data);
    jobDataChanged = false; // We just drew it

    // Save current color and set the initial color (BLINK_COLORS[0] = Red) immediately
    actionOriginalColor = strip.getPixelColor(0);
    strip.setPixelColor(0, BLINK_COLORS[0]);
    strip.show();

    // Move to the first state and start the timer
    currentActionState = ACTION_BLINK_1;
    actionStartTime = millis();
}

// --- HARDWARE ACTION SEQUENCE RUNNER (Fixes bug in state transition logic) ---
void runAction()
{
    if (currentActionState == ACTION_IDLE)
    {
        return;
    }

    // Must use currentMillis for non-blocking timing
    unsigned long currentMillis = millis();
    unsigned long timeElapsed = currentMillis - actionStartTime;

    if (timeElapsed < BLINK_DURATION_MS)
    {
        return; // Wait for the current phase duration
    }

    // Determine the index of the current phase (0 to 4)
    int phaseIndex = currentActionState - ACTION_BLINK_1;

    if (phaseIndex < NUM_BLINK_COLORS - 1)
    {
        // Transition to the next color (e.g., Red -> Orange)
        Serial.printf("[ACTION] Phase %d/%d complete. Moving to next color.\n",
                      phaseIndex + 1, NUM_BLINK_COLORS);

        // Advance to the next state
        currentActionState = (ActionState)(currentActionState + 1);

        // Set the new color
        uint32_t nextColor = BLINK_COLORS[phaseIndex + 1];
        strip.setPixelColor(0, nextColor);
        strip.show();

        actionStartTime = currentMillis; // Reset timer for the next phase
    }
    else
    {
        // We have completed the final blink (Phase 5: Blue)
        Serial.println("[ACTION] Phase 5/5: BLUE complete. Moving to COMPLETED state.");

        // 1. Inform the server of action completion
        if (notifyServerOfCompletion())
        {
            Serial.println("-> Server notified successfully. Job finished.");
        }
        else
        {
            Serial.println("-> ERROR: Failed to notify server of completion.");
        }
        
        // 2. Restore original color and reset state
        strip.setPixelColor(0, actionOriginalColor); // Should be black (0,0,0) when idle
        strip.show();
        Serial.println("[ACTION] Blink sequence complete. Restoring LED state.");
        
        // 3. Set the display to update in the main loop to show the idle screen next cycle
        jobDataChanged = true; 
        currentActionState = ACTION_IDLE; // Reset to idle state
    }
}

// --- SERVER NOTIFICATION (Unchanged) ---
bool notifyServerOfCompletion()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin(COMPLETION_URL);
        http.addHeader("Content-Type", "application/json");

        // Send the job data back for confirmation
        String payload = "{\"status\": \"completed\", \"name\": \"" + currentJobData.name + "\", \"country\": \"" + currentJobData.country + "\"}";
        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0)
        {
            Serial.printf("[SERVER] Completion POST sent. Response code: %d\n", httpResponseCode);
            http.end();
            return true;
        }
        else
        {
            Serial.printf("[SERVER] Completion POST failed. Error: %s\n", http.errorToString(httpResponseCode).c_str());
            http.end();
            return false;
        }
    }
    return false;
}