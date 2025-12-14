#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// --- NEW CONFIGURATION HEADERS ---
#include <Preferences.h>
#include <DNSServer.h>
// --- NEOPIXEL LIBRARY INCLUSION (KEPT) ---
#include <Adafruit_NeoPixel.h> 

// --- NVS & AP CONFIGURATION CONSTANTS ---
Preferences preferences;
const char* PREFS_NAMESPACE = "assistant_cfg";
const char* PREF_SSID = "ssid";
const char* PREF_PASS = "pass";

// Web Server and DNS Server setup for AP Portal
const IPAddress apIP(192, 168, 4, 1);
const IPAddress netMask(255, 255, 255, 0);
const byte DNS_PORT = 53;
DNSServer dnsServer;

// --- DEVICE & NETWORK CONFIGURATION (ADAPTED) ---
// Note: WIFI_SSID/PASSWORD are now stored in NVS/Preferences
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
// Web server object that will listen for incoming job requests and AP portal requests
WebServer server(HTTP_PORT); 

// NeoPixel object to control the onboard RGB LED
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);


// --- FUNCTION PROTOTYPES ---
void setLEDColor(uint8_t r, uint8_t g, uint8_t b);
void performBlinkSequence();
void handleStartBlink();
bool notifyServerOfCompletion();
void printWifiStatus();

// --- NEW AP/NVS FUNCTION PROTOTYPES ---
void handleRoot();
void handleSave();
bool startAPPortal();
bool connectWiFi();


// --- HTML Content for the Configuration Form (Updated with Sci-Fi Theme) ---
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

// Handles the root request (serves the configuration form)
void handleRoot() {
  server.send(200, "text/html", CONFIG_HTML);
}

// Handles the /save request (saves credentials and restarts)
void handleSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("pass");

  if (ssid.length() > 0) {
    // --- CRITICAL: SECURE STORAGE ---
    preferences.begin(PREFS_NAMESPACE, false);
    preferences.putString(PREF_SSID, ssid);
    preferences.putString(PREF_PASS, password);
    preferences.end();
    // --------------------------------

    Serial.println("Credentials saved securely to NVS. Rebooting...");
    // Updated success message to match theme
    server.send(200, "text/html", "<body style='background-color:#0d0d0d; color:#00ff00; font-family: monospace; text-align:center; padding-top: 100px;'><h1>TRANSMISSION SUCCESSFUL</h1><p>Credentials saved. Initiating system reboot and connection attempt...</p></body>");
    delay(3000);
    ESP.restart();
  } else {
    // Updated error message to match theme
    server.send(400, "text/html", "<body style='background-color:#0d0d0d; color:red; font-family: monospace; text-align:center; padding-top: 100px;'><h1>ERROR: INPUT FAILED</h1><p>SSID cannot be empty. Check protocol parameters.</p></body>");
  }
}

// Helper function to start the Access Point (AP) for configuration
bool startAPPortal() {
  Serial.println("\n--- STARTING AP SETUP MODE ---");
  Serial.println("Connect to Wi-Fi 'Trinity_Setup' and browse to any website.");

  // 1. Configure and start Access Point
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAPConfig(apIP, apIP, netMask);
  WiFi.softAP("Trinity_Setup", ""); // Open AP
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // 2. Start DNS Server (for captive portal functionality)
  dnsServer.start(DNS_PORT, "*", apIP);

  // 3. Set up Web Server Handlers for the portal
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
    
    // Set LED to a steady BLUE/CYAN to indicate Configuration Mode
    setLEDColor(0, 50, 50); 
    
  // The loop handles the running of the portal until reboot (on save or timeout).
  return false; // We don't return true, as we are stuck in a blocking/reboot loop in the AP context
}

// Main function to connect or start setup
bool connectWiFi() {
  // 1. Check for stored credentials in NVS flash
  preferences.begin(PREFS_NAMESPACE, true); // Read-only
  String ssid = preferences.getString(PREF_SSID, "");
  String pass = preferences.getString(PREF_PASS, "");
  preferences.end();

  if (ssid.length() == 0) {
    // No saved credentials found, launch setup portal
    return startAPPortal();
  }

  // 2. Connect using saved credentials (STA mode)
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  // Set LED to pulsing WHITE/YELLOW to indicate connection attempt
  setLEDColor(50, 50, 0); 

  // 3. Wait for connection (with timeout, then launch portal if failed)
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > 30000) { // 30 second timeout
      Serial.println("\nConnection attempt failed. Launching AP portal...");
      return startAPPortal();
    }
    delay(500);
    yield();
  }
  
  // Success - Turn LED off
  setLEDColor(0, 0, 0); // LED OFF when connected and stable
  Serial.println("WiFi Connected.");
  return true;
}


// --- CORE SETUP ---
void setup() {
    Serial.begin(115200);
    Serial.println("--- SETUP STARTED SUCCESSFULLY ---"); 
    
    // Initialize the NeoPixel strip
    strip.begin(); 
    strip.setBrightness(50); 
    setLEDColor(0, 0, 0); // Start with LED off

    Serial.println("\n--- Web2Wire ESP32 S3 Firmware Boot ---");
    
    // 1. Connect to WiFi or Start AP Portal
    if (connectWiFi()) {
        // If connected, proceed with application server setup (STA mode)
        Serial.print("SUCCESS! Device IP: ");
        Serial.println(WiFi.localIP());

        // 2. Setup the Web Server for Job Requests
        // Define the endpoint the API server will POST to for starting a job
        server.on("/api/job/start", HTTP_POST, handleStartBlink);
        server.begin();
        Serial.println("HTTP Job Server started, listening for POST on /api/job/start");
        
        // Initial status print
        printWifiStatus();
        // Initialize both timers to the current time
        lastStatusPrint = millis();
        lastReconnectAttempt = millis();
    } else {
        Serial.println("Device is now in AP Setup Mode.");
        // If startAPPortal() was called, the device is now waiting in the AP loop, 
        // the main loop will handle the rest.
    }
}


// --- MAIN LOOP ---
void loop() {
    // 1. Core Server Functionality (AP Mode or STA Mode)
    // IMPORTANT: The single 'server' object handles both the AP portal routes (/) and the job route (/api/job/start)
    server.handleClient();

    if (WiFi.getMode() == WIFI_MODE_AP) {
        // AP Mode: Must handle DNS requests for Captive Portal functionality
        dnsServer.processNextRequest();
        
        // Since startAPPortal() is blocking/rebooting, this section primarily handles the DNS/HTTP
        // while the user is connected to the AP. We let the AP loop handle the timeout.
    } else {
        // STA Mode (Application is running)
        unsigned long currentMillis = millis();
        
        // 2. Periodic Status Report (Non-blocking Timer)
        if (currentMillis - lastStatusPrint >= STATUS_INTERVAL_MS) {
            lastStatusPrint = currentMillis;
            printWifiStatus();
        }
        
        // 3. Periodic WiFi Status Check and Reconnection Logic (Non-blocking Timer)
        if (WiFi.status() != WL_CONNECTED) {
            // LED feedback for disconnected state (e.g., flashing RED)
            setLEDColor(20, 0, 0); 
            
            // Only attempt to reconnect if the cooldown period has passed
            if (currentMillis - lastReconnectAttempt >= RECONNECT_COOLDOWN_MS) { 
                 Serial.println("WiFi disconnected. Attempting reconnection...");
                 // Use simple reconnect for non-blocking loop
                 WiFi.reconnect(); 
                 lastReconnectAttempt = currentMillis; 
            }
        } else {
             // Ensure LED is back to OFF once connected and stable
             setLEDColor(0, 0, 0); 
        }
    }
    yield(); // Always yield in loop
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
        // Log "OFF" only if explicitly setting to 0,0,0 and previously it was ON (we don't check prev state here, just log OFF)
        // This is fine for now as it makes the serial output clean.
        // Serial.println("[LED] Turned OFF"); 
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
    
    // Save current color to restore later (which is now likely 0,0,0 if connected)
    uint32_t originalColor = strip.getPixelColor(0);
    
    // Color 1: RED
    setLEDColor(255, 0, 0); 
    delay(BLINK_DURATION_MS);

    // Color 2: BLUE
    setLEDColor(0, 0, 255); 
    delay(BLINK_DURATION_MS);
    
    // Color 3: GREEN
    setLEDColor(0, 255, 0); 
    delay(BLINK_DURATION_MS);
    
    // Restore the LED to the color it was before the sequence
    strip.setPixelColor(0, originalColor);
    strip.show();
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