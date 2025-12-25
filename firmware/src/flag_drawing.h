#ifndef FLAG_DRAWING_H
#define FLAG_DRAWING_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// --- FLAG DIMENSIONS (MUST match the bitmaps) ---
#define FLAG_W 32
#define FLAG_H 20
#define FLAG_SIZE (FLAG_W * FLAG_H) // 640 words

// The TFT display object is defined in the main sketch, so we declare it as extern here
// to allow the drawing functions to access it.
extern Adafruit_ST7789 tft;

// --- DISPLAY COLORS (Standard and Flag-Specific) ---
// These definitions rely on the extern 'tft' object for color conversion.

// Standard Colors
#define ST77XX_BLACK        0x0000
#define ST77XX_WHITE        0xFFFF
#define ST77XX_RED          0xF800
#define ST77XX_GREEN        0x07E0
#define ST77XX_BLUE         0x001F
#define ST77XX_CYAN         0x07FF
#define ST77XX_MAGENTA      0xF81F
#define ST77XX_YELLOW       0xFFE0
#define ST77XX_ORANGE       tft.color565(255, 165, 0) // Standard orange

// Custom Flag Colors
#define ST77XX_GOLD         tft.color565(255, 204, 0)
#define ST77XX_SAFFRON      tft.color565(255, 153, 51)
#define ST77XX_ORANGE_IE    tft.color565(255, 136, 62)
#define ST77XX_NAVY         tft.color565(0, 0, 128)
#define ST77XX_DARKGREEN    tft.color565(0, 102, 0)
#define ST77XX_PARIS_BLUE   tft.color565(0, 85, 164)
#define ST77XX_RICH_GREEN  tft.color565(0, 132, 61)  
#define ST77XX_DEEP_YELLOW tft.color565(255, 199, 44) 
#define ST77XX_ARG_BLUE   tft.color565(117, 170, 219) 
#define ST77XX_CHINA_RED  tft.color565(238, 30, 52)  
#define ST77XX_KE_RED    tft.color565(190, 0, 0)   
#define ST77XX_KE_GREEN   tft.color565(0, 128, 0)   
#define ST77XX_PORT_RED   tft.color565(204, 32, 53)  
#define ST77XX_PORT_GREEN  tft.color565(0, 102, 0)   
#define ST77XX_EGYPT_GOLD  tft.color565(205, 164, 52) 
#define ST77XX_SA_BLUE   tft.color565(0, 36, 114)  
#define ST77XX_TURK_RED   tft.color565(227, 10, 23)
#define ST77XX_KE_BLACK   tft.color565(0, 0, 0)

// --- FUNCTION PROTOTYPES ---

void drawUSFlag(int x, int y, int scale);
void drawBRFlag(int x, int y, int scale); 
void drawGBFlag(int x, int y, int scale);
void drawINFlag(int x, int y, int scale);    // India
void drawDEFlag(int x, int y, int scale);    // Germany
void drawFRFlag(int x, int y, int scale);    // France
void drawNLFlag(int x, int y, int scale);    // Netherlands
void drawIEFlag(int x, int y, int scale);    // Ireland
void drawJPFlag(int x, int y, int scale);    // Japan
void drawAUFlag(int x, int y, int scale);    // Australia
void drawBEFlag(int x, int y, int scale);    // Belgium
void drawRUFlag(int x, int y, int scale);    // Russia
void drawCAFlag(int x, int y, int scale);    // Canada

// New flag drawing prototypes (Requested)
void drawARFlag(int x, int y, int scale);    // Argentina
void drawATFlag(int x, int y, int scale);    // Austria
void drawCLFlag(int x, int y, int scale);    // Chile
void drawCNFlag(int x, int y, int scale);    // China
void drawCOFlag(int x, int y, int scale);    // Colombia
void drawDKFlag(int x, int y, int scale);    // Denmark
void drawEGFlag(int x, int y, int scale);    // Egypt
void drawFIFlag(int x, int y, int scale);    // Finland
void drawGRFlag(int x, int y, int scale);    // Greece
void drawIDFlag(int x, int y, int scale);    // Indonesia
void drawITFlag(int x, int y, int scale);    // Italy
void drawKEFlag(int x, int y, int scale);    // Kenya
void drawMXFlag(int x, int y, int scale);    // Mexico
void drawNZFlag(int x, int y, int scale);    // New Zealand
void drawNOFlag(int x, int y, int scale);    // Norway
void drawPLFlag(int x, int y, int scale);    // Poland
void drawPTFlag(int x, int y, int scale);    // Portugal
void drawZAFlag(int x, int y, int scale);    // South Africa
void drawKRFlag(int x, int y, int scale);    // South Korea
void drawESFlag(int x, int y, int scale);    // Spain
void drawSEFlag(int x, int y, int scale);    // Sweden
void drawCHFlag(int x, int y, int scale);    // Switzerland
void drawTRFlag(int x, int y, int scale);    // Turkey

/**
 * @brief Main dispatcher function: Draws a flag using custom geometry or falls back to bitmap lookup.
 * @param flagCode The 2-letter country code (e.g., "US", "BR").
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param scale The scaling factor (e.g., 4x for 128x80 on screen).
 */
void drawFlag(const String &flagCode, int x, int y, int scale);

#endif // FLAG_DRAWING_H