#include "flag_drawing.h"
#include <SPI.h> // Required for pgm_read_word
#include <FS.h>  // Included implicitly via Arduino.h, but good practice

// ******************************************************
// ** FLAG GEOMETRY IMPLEMENTATIONS **
// ******************************************************

// --- CUSTOM FLAG DRAWING LOGIC (New Functions) ---

// ******************************************************
// ** IMPROVED BRAZIL FLAG (BR) **
// ******************************************************
/**
 * @brief Draws the Brazil Flag (Better Version: Richer Green, Defined Rhombus, Clearer Sphere/Band).
 */
void drawBRFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale; // 128
         int h = FLAG_H * scale; // 80
         int centerW = x + w / 2;
         int centerY = y + h / 2;
         int radius = scale * 6; // Blue circle radius (~24px)

         // 1. Rich Emerald Green Field
         tft.fillRect(x, y, w, h, ST77XX_RICH_GREEN);

         // 2. Deep Gold Yellow Rhombus (Approximation)
         // The rhombus corners are roughly 17% in from the edges.
         int rW = w * 0.45; // Half-width of rhombus (~57px)
         int rH = h * 0.45; // Half-height of rhombus (~36px)

         // Define vertices for the rhombus (Top, Right, Bottom, Left)
         tft.fillTriangle(centerW, centerY - rH, centerW + rW, centerY, centerW, centerY + rH, ST77XX_DEEP_YELLOW);
         tft.fillTriangle(centerW, centerY - rH, centerW - rW, centerY, centerW, centerY + rH, ST77XX_DEEP_YELLOW);
         
         // 3. Navy Blue Sphere (Order and Progress)
         tft.fillCircle(centerW, centerY, radius, ST77XX_BLUE); // Using standard Blue for contrast

         // 4. White Motto Band (Simplified as a curved arc/sector)
         int bandThickness = scale * 2;
         // Simple white arc line approximation
         tft.drawFastHLine(centerW - radius, centerY + bandThickness / 2, 2 * radius, ST77XX_WHITE);
         tft.drawFastHLine(centerW - radius, centerY + bandThickness / 2 + 1, 2 * radius, ST77XX_WHITE);
         
         // Draw a small white star placeholder for the Southern Cross 
         tft.fillCircle(centerW - scale * 3, centerY - scale * 3, 1, ST77XX_WHITE);
}

// ******************************************************
// ** ARGENTINA FLAG (AR) **
// ******************************************************
void drawARFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int stripeH = h / 3;
         int sunRadius = scale * 2;
         int centerX = x + w / 2;
         int centerY = y + h / 2;

         tft.fillRect(x, y, w, stripeH, ST77XX_ARG_BLUE);    // Top: Blue
         tft.fillRect(x, y + stripeH, w, stripeH, ST77XX_WHITE);    // Middle: White
         tft.fillRect(x, y + 2 * stripeH, w, h - 2 * stripeH, ST77XX_ARG_BLUE);    // Bottom: Blue

         tft.fillCircle(centerX, centerY, sunRadius, ST77XX_GOLD); // Sun of May
}

// ******************************************************
// ** AUSTRIA FLAG (AT) **
// ******************************************************
void drawATFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int stripeH = h / 3;

         tft.fillRect(x, y, w, stripeH, ST77XX_RED);
         tft.fillRect(x, y + stripeH, w, stripeH, ST77XX_WHITE);
         tft.fillRect(x, y + 2 * stripeH, w, h - 2 * stripeH, ST77XX_RED);
}

// ******************************************************
// ** CHILE FLAG (CL) **
// ******************************************************
void drawCLFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int stripeH = h / 2;
         int cantonSize = stripeH; // Square canton 40x40

         tft.fillRect(x, y, w, stripeH, ST77XX_WHITE);    // Top: White
         tft.fillRect(x, y + stripeH, w, h - stripeH, ST77XX_RED); // Bottom: Red

         tft.fillRect(x, y, cantonSize, cantonSize, ST77XX_BLUE); // Blue Canton

         tft.fillCircle(x + cantonSize / 2, y + cantonSize / 2, scale, ST77XX_WHITE); // White Star
}

// ******************************************************
// ** CHINA FLAG (CN) **
// ******************************************************
void drawCNFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int starSize = scale * 3;
         int starX = x + w / 4;
         int starY = y + h / 4;

         tft.fillRect(x, y, w, h, ST77XX_CHINA_RED); // Red Field
         tft.fillCircle(starX, starY, starSize, ST77XX_YELLOW); // Simplified Large Star
}

// ******************************************************
// ** COLOMBIA FLAG (CO) **
// ******************************************************
void drawCOFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int stripeH1 = h / 2;
         int stripeH23 = h / 4;

         tft.fillRect(x, y, w, stripeH1, ST77XX_YELLOW); // Top: Yellow (1/2)
         tft.fillRect(x, y + stripeH1, w, stripeH23, ST77XX_BLUE); // Middle: Blue (1/4)
         tft.fillRect(x, y + stripeH1 + stripeH23, w, h - (stripeH1 + stripeH23), ST77XX_RED); // Bottom: Red (1/4)
}

// ******************************************************
// ** DENMARK FLAG (DK) **
// ******************************************************
void drawDKFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int crossW = scale * 2; // Thickness of the white cross (~8px)
         int crossOffset = w / 3; // Cross centered at 1/3rd the width

         tft.fillRect(x, y, w, h, ST77XX_RED); // Red Field

         // White Nordic Cross (Vertical)
         tft.fillRect(x + crossOffset - crossW / 2, y, crossW, h, ST77XX_WHITE);
         // White Nordic Cross (Horizontal)
         tft.fillRect(x, y + h / 2 - crossW / 2, w, crossW, ST77XX_WHITE);
}

// ******************************************************
// ** EGYPT FLAG (EG) **
// ******************************************************
void drawEGFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int stripeH = h / 3;
         int centerRadius = scale * 3;
         int centerX = x + w / 2;
         int centerY = y + h / 2;

         tft.fillRect(x, y, w, stripeH, ST77XX_RED);
         tft.fillRect(x, y + stripeH, w, stripeH, ST77XX_WHITE);
         tft.fillRect(x, y + 2 * stripeH, w, h - 2 * stripeH, ST77XX_BLACK);

         // Simplified Eagle (Gold circle)
         tft.fillCircle(centerX, centerY, centerRadius, ST77XX_EGYPT_GOLD);
}

// ******************************************************
// ** FINLAND FLAG (FI) **
// ******************************************************
void drawFIFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int crossW = scale * 2; // Thickness of the blue cross (~8px)
         int crossOffset = w / 3; // Cross centered at 1/3rd the width

         tft.fillRect(x, y, w, h, ST77XX_WHITE); // White Field

         // Blue Nordic Cross (Vertical)
         tft.fillRect(x + crossOffset - crossW / 2, y, crossW, h, ST77XX_BLUE);
         // Blue Nordic Cross (Horizontal)
         tft.fillRect(x, y + h / 2 - crossW / 2, w, crossW, ST77XX_BLUE);
}

// ******************************************************
// ** GREECE FLAG (GR) **
// ******************************************************
void drawGRFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int stripeH = h / 9; // 9 stripes total
         int cantonSize = stripeH * 5; // 5 stripes high

         // 1. Stripes (Blue/White)
         for (int i = 0; i < 9; i++)
         {
                  uint16_t color = (i % 2 == 0) ? ST77XX_BLUE : ST77XX_WHITE;
                  tft.fillRect(x, y + i * stripeH, w, stripeH, color);
         }

         // 2. Blue Canton (top left)
         tft.fillRect(x, y, cantonSize, cantonSize, ST77XX_BLUE);

         // 3. White Cross in Canton
         int crossW = stripeH;
         tft.fillRect(x + cantonSize / 2 - crossW / 2, y, crossW, cantonSize, ST77XX_WHITE);
         tft.fillRect(x, y + cantonSize / 2 - crossW / 2, cantonSize, crossW, ST77XX_WHITE);
}

// ******************************************************
// ** INDONESIA FLAG (ID) **
// ******************************************************
void drawIDFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int stripeH = h / 2;

         tft.fillRect(x, y, w, stripeH, ST77XX_RED);
         tft.fillRect(x, y + stripeH, w, h - stripeH, ST77XX_WHITE);
}

// ******************************************************
// ** ITALY FLAG (IT) **
// ******************************************************
void drawITFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int stripeW = w / 3;

         tft.fillRect(x, y, stripeW, h, ST77XX_GREEN);
         tft.fillRect(x + stripeW, y, stripeW, h, ST77XX_WHITE);
         tft.fillRect(x + 2 * stripeW, y, w - 2 * stripeW, h, ST77XX_RED);
}

// ******************************************************
// ** KENYA FLAG (KE) **
// ******************************************************
void drawKEFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int stripeH = h / 6;
         int centerX = x + w / 2;
         int centerY = y + h / 2;
         int shieldRadius = scale * 5;

         // Stripes: Black, White, Red, White, Green
         tft.fillRect(x, y, w, stripeH * 2, ST77XX_KE_BLACK); // Black
         tft.fillRect(x, y + stripeH * 2, w, stripeH, ST77XX_WHITE);    // White
         tft.fillRect(x, y + stripeH * 3, w, stripeH, ST77XX_KE_RED);    // Red
         tft.fillRect(x, y + stripeH * 4, w, stripeH, ST77XX_WHITE);    // White
         tft.fillRect(x, y + stripeH * 5, w, h - stripeH * 5, ST77XX_KE_GREEN); // Green

         // Simplified Shield (Black circle, symbolizing the Masaii shield)
         tft.fillCircle(centerX, centerY, shieldRadius, ST77XX_BLACK);
}

// ******************************************************
// ** MEXICO FLAG (MX) **
// ******************************************************
void drawMXFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int stripeW = w / 3;
         int sealRadius = scale * 3;
         int centerX = x + stripeW + stripeW / 2;

         tft.fillRect(x, y, stripeW, h, ST77XX_GREEN);    // Green
         tft.fillRect(x + stripeW, y, stripeW, h, ST77XX_WHITE);    // White
         tft.fillRect(x + 2 * stripeW, y, w - 2 * stripeW, h, ST77XX_RED); // Red

         // Simplified Seal (Green circle in the center)
         tft.fillCircle(centerX, y + h / 2, sealRadius, ST77XX_GREEN);
}

// ******************************************************
// ** NEW ZEALAND FLAG (NZ) **
// ******************************************************
void drawNZFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int cantonW = w / 2;
         int cantonH = h / 2;
         int starSize = scale * 2;

         tft.fillRect(x, y, w, h, ST77XX_BLUE); // Background Blue

         drawGBFlag(x, y, scale); // Simplified Union Jack in Canton (uses overdraw)

         // Southern Cross (4 red stars with white fimbriation)
         int scX = x + cantonW + (w - cantonW) / 4;
         int scY = y + cantonH + (h - cantonH) / 4;

         // Draw 4 white borders (fimbriation)
         tft.fillCircle(scX, scY, starSize + 1, ST77XX_WHITE);
         tft.fillCircle(scX + scale * 4, scY, starSize + 1, ST77XX_WHITE);
         tft.fillCircle(scX, scY + scale * 4, starSize + 1, ST77XX_WHITE);
         tft.fillCircle(scX + scale * 4, scY + scale * 4, starSize + 1, ST77XX_WHITE);

         // Draw 4 red stars (circles)
         tft.fillCircle(scX, scY, starSize, ST77XX_RED);
         tft.fillCircle(scX + scale * 4, scY, starSize, ST77XX_RED);
         tft.fillCircle(scX, scY + scale * 4, starSize, ST77XX_RED);
         tft.fillCircle(scX + scale * 4, scY + scale * 4, starSize, ST77XX_RED);
}

// ******************************************************
// ** NORWAY FLAG (NO) **
// ******************************************************
void drawNOFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int crossW = scale * 4; // Thickness of the total cross (~16px)
         int crossOffset = w / 3; 

         tft.fillRect(x, y, w, h, ST77XX_RED); // Red Field

         // White Nordic Cross (Vertical) - Outer layer
         tft.fillRect(x + crossOffset - crossW / 2, y, crossW, h, ST77XX_WHITE);
         // White Nordic Cross (Horizontal) - Outer layer
         tft.fillRect(x, y + h / 2 - crossW / 2, w, crossW, ST77XX_WHITE);

         // Inner Blue Cross (Vertical)
         tft.fillRect(x + crossOffset - scale / 2, y, scale, h, ST77XX_BLUE);
         // Inner Blue Cross (Horizontal)
         tft.fillRect(x, y + h / 2 - scale / 2, w, scale, ST77XX_BLUE);
}

// ******************************************************
// ** POLAND FLAG (PL) **
// ******************************************************
void drawPLFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int stripeH = h / 2;

         tft.fillRect(x, y, w, stripeH, ST77XX_WHITE); // Top: White
         tft.fillRect(x, y + stripeH, w, h - stripeH, ST77XX_RED); // Bottom: Red
}

// ******************************************************
// ** PORTUGAL FLAG (PT) **
// ******************************************************
void drawPTFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int greenW = w * 2 / 5;
         int redW = w - greenW;
         int centerX = x + greenW;
         int centerY = y + h / 2;
         int shieldRadius = scale * 4;

         tft.fillRect(x, y, greenW, h, ST77XX_PORT_GREEN); // Left: Green
         tft.fillRect(x + greenW, y, redW, h, ST77XX_PORT_RED); // Right: Red

         // Simplified Coat of Arms (Gold circle)
         tft.fillCircle(centerX, centerY, shieldRadius, ST77XX_GOLD);
}

// ******************************************************
// ** SOUTH AFRICA FLAG (ZA) **
// ******************************************************
void drawZAFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int centerY = y + h / 2;
         int stripeH = h / 6;

         // 1. Black (Simplified Y-shape)
         tft.fillTriangle(x, centerY - stripeH, x, centerY + stripeH, x + w / 2, centerY, ST77XX_BLACK);
         
         // 2. White fimbriation (Around black)
         tft.fillTriangle(x, centerY - stripeH - 1, x, centerY + stripeH + 1, x + w / 2 + 1, centerY, ST77XX_WHITE);
         tft.fillTriangle(x, centerY - stripeH, x, centerY + stripeH, x + w / 2, centerY, ST77XX_BLACK); // redraw black

         // 3. Yellow/Gold fimbriation
         tft.fillTriangle(x + w / 2, centerY, x + w, y, x + w, h, ST77XX_DEEP_YELLOW);

         // 4. Red (Top)
         tft.fillRect(x + w / 2, y, w - w / 2, centerY - stripeH - 1, ST77XX_RED);
         // 5. Blue (Bottom)
         tft.fillRect(x + w / 2, centerY + stripeH + 1, w - w / 2, h - (centerY + stripeH + 1), ST77XX_SA_BLUE);

         // 6. Green fills the black Y
         tft.fillTriangle(x, centerY - stripeH, x, centerY + stripeH, x + w / 2, centerY, ST77XX_KE_GREEN);
}

// ******************************************************
// ** SOUTH KOREA FLAG (KR) **
// ******************************************************
void drawKRFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int centerX = x + w / 2;
         int centerY = y + h / 2;
         int radius = scale * 5;

         tft.fillRect(x, y, w, h, ST77XX_WHITE); // White Field

         // Simplified Taegeuk (Red/Blue Circle)
         tft.fillCircle(centerX, centerY, radius, ST77XX_RED);
         tft.fillCircle(centerX, centerY - radius / 2, radius / 2, ST77XX_BLUE);
         tft.fillCircle(centerX, centerY + radius / 2, radius / 2, ST77XX_RED);
}

// ******************************************************
// ** SPAIN FLAG (ES) **
// ******************************************************
void drawESFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int redH = h / 4;
         int yellowH = h / 2;
         int centerX = x + w / 3;
         int centerY = y + h / 2;
         int shieldRadius = scale * 3;

         tft.fillRect(x, y, w, redH, ST77XX_RED);    // Top: Red
         tft.fillRect(x, y + redH, w, yellowH, ST77XX_YELLOW); // Middle: Yellow
         tft.fillRect(x, y + redH + yellowH, w, h - (redH + yellowH), ST77XX_RED); // Bottom: Red

         // Simplified Coat of Arms (Blue circle)
         tft.fillCircle(centerX, centerY, shieldRadius, ST77XX_BLUE);
}

// ******************************************************
// ** SWEDEN FLAG (SE) **
// ******************************************************
void drawSEFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int crossW = scale * 2; // Thickness of the yellow cross (~8px)
         int crossOffset = w / 3; // Cross centered at 1/3rd the width

         tft.fillRect(x, y, w, h, ST77XX_BLUE); // Blue Field

         // Yellow Nordic Cross (Vertical)
         tft.fillRect(x + crossOffset - crossW / 2, y, crossW, h, ST77XX_YELLOW);
         // Yellow Nordic Cross (Horizontal)
         tft.fillRect(x, y + h / 2 - crossW / 2, w, crossW, ST77XX_YELLOW);
}

// ******************************************************
// ** SWITZERLAND FLAG (CH) **
// ******************************************************
void drawCHFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int crossSize = scale * 2; 

         tft.fillRect(x, y, w, h, ST77XX_RED); // Red Field

         // White Cross (approx 6:1 ratio, simplified)
         // Vertical bar
         tft.fillRect(x + w / 2 - crossSize / 2, y + crossSize, crossSize, h - 2 * crossSize, ST77XX_WHITE);
         // Horizontal bar
         tft.fillRect(x + crossSize, y + h / 2 - crossSize / 2, w - 2 * crossSize, crossSize, ST77XX_WHITE);
}

// ******************************************************
// ** TURKEY FLAG (TR) **
// ******************************************************
void drawTRFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;
         int h = FLAG_H * scale;
         int centerX = x + w / 2;
         int centerY = y + h / 2;
         int largeR = scale * 6; // Large circle for crescent
         int smallR = scale * 5; // Small circle for crescent 'bite'
         int starR = scale * 2; // Star radius

         tft.fillRect(x, y, w, h, ST77XX_TURK_RED); // Red Field

         // Simplified Crescent (White Circle - Inner Red Circle)
         tft.fillCircle(centerX - scale * 2, centerY, largeR, ST77XX_WHITE);
         tft.fillCircle(centerX - scale, centerY, smallR, ST77XX_TURK_RED);

         // Simplified Star (White Circle)
         tft.fillCircle(centerX + scale * 4, centerY, starR, ST77XX_WHITE);
}

// ******************************************************
// ** Simple Tricolors (AT, ID, IT, PL, DE, FR, NL, RU, IE) **
// ******************************************************
// Existing drawINFlag, drawDEFlag, drawFRFlag, drawNLFlag, drawIEFlag, drawJPFlag, 
// drawAUFlag, drawBEFlag, drawRUFlag, drawCAFlag logic remains here...
// (Using the logic from the user's initial prompt)

/**
  * @brief Draws the India Flag (Saffron, White, Green + simplified Chakra).
  */
void drawINFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;    // 128
         int h = FLAG_H * scale;    // 80
         int stripeH = h / 3;          // 26, 27, 27 (approx for 4x scale)
         int radius = scale * 5;    // Radius of the Chakra (20 pixels at 4x)
         int centerX = x + w / 2;
         int centerY = y + h / 2;

         // 1. Tricolor Stripes
         tft.fillRect(x, y, w, stripeH, ST77XX_SAFFRON);    // Top: Saffron
         tft.fillRect(x, y + stripeH, w, stripeH, ST77XX_WHITE);    // Middle: White
         tft.fillRect(x, y + 2 * stripeH, w, h - 2 * stripeH, ST77XX_DARKGREEN);    // Bottom: Green

         // 2. Ashoka Chakra (Simplified as a Navy Blue circle)
         tft.fillCircle(centerX, centerY, radius, ST77XX_NAVY);
         tft.drawCircle(centerX, centerY, radius, ST77XX_NAVY);
}

/**
  * @brief Draws the Germany Flag (Black, Red, Gold horizontal tricolor).
  */
void drawDEFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale; // 128
         int h = FLAG_H * scale; // 80
         int stripeH = h / 3;         // 26, 27, 27 (approx for 4x scale)

         // 1. Black
         tft.fillRect(x, y, w, stripeH, ST77XX_BLACK);
         // 2. Red
         tft.fillRect(x, y + stripeH, w, stripeH, ST77XX_RED);
         // 3. Gold
         tft.fillRect(x, y + 2 * stripeH, w, h - 2 * stripeH, ST77XX_GOLD);
}

/**
  * @brief Draws the France Flag (Blue, White, Red vertical tricolor).
  */
void drawFRFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale; // 128
         int h = FLAG_H * scale; // 80
         int stripeW = w / 3;         // 42, 43, 43 (approx for 4x scale)

         // 1. Blue (Paris Blue is traditional)
         tft.fillRect(x, y, stripeW, h, ST77XX_PARIS_BLUE);
         // 2. White
         tft.fillRect(x + stripeW, y, stripeW, h, ST77XX_WHITE);
         // 3. Red
         tft.fillRect(x + 2 * stripeW, y, w - 2 * stripeW, h, ST77XX_RED);
}

/**
  * @brief Draws the Netherlands Flag (Red, White, Blue horizontal tricolor).
  */
void drawNLFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale; // 128
         int h = FLAG_H * scale; // 80
         int stripeH = h / 3;         // 26, 27, 27 (approx for 4x scale)

         // 1. Red
         tft.fillRect(x, y, w, stripeH, ST77XX_RED);
         // 2. White
         tft.fillRect(x, y + stripeH, w, stripeH, ST77XX_WHITE);
         // 3. Blue
         tft.fillRect(x, y + 2 * stripeH, w, h - 2 * stripeH, ST77XX_BLUE);
}

/**
  * @brief Draws the Ireland Flag (Green, White, Orange vertical tricolor).
  */
void drawIEFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale; // 128
         int h = FLAG_H * scale; // 80
         int stripeW = w / 3;         // 42, 43, 43 (approx for 4x scale)

         // 1. Green
         tft.fillRect(x, y, stripeW, h, ST77XX_GREEN);
         // 2. White
         tft.fillRect(x + stripeW, y, stripeW, h, ST77XX_WHITE);
         // 3. Orange
         tft.fillRect(x + 2 * stripeW, y, w - 2 * stripeW, h, ST77XX_ORANGE_IE);
}

/**
  * @brief Draws the Japan Flag (White field, Red Hinomaru disc).
  */
void drawJPFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;    // 128
         int h = FLAG_H * scale;    // 80
         int radius = scale * 8;    // Radius (32 pixels at 4x scale)
         int centerX = x + w / 2;
         int centerY = y + h / 2;

         // 1. White Field
         tft.fillRect(x, y, w, h, ST77XX_WHITE);

         // 2. Red Disc (Hinomaru)
         tft.fillCircle(centerX, centerY, radius, ST77XX_RED);
}

/**
  * @brief Draws the Australia Flag (Simplified Blue Ensign, stars as circles).
  */
void drawAUFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale; // 128
         int h = FLAG_H * scale; // 80
         int cantonW = w / 2;
         int cantonH = h / 2;
         int starSize = scale * 2;

         // 1. Background Blue
         tft.fillRect(x, y, w, h, ST77XX_BLUE);

         // 2. Simplified Union Jack in Canton (using pre-existing logic)
         drawGBFlag(x, y, scale);

         // 3. Commonwealth Star (Simplified as a large white circle below the canton)
         int cx = x + cantonW / 2;
         int cy = y + h - (h / 4);
         tft.fillCircle(cx, cy, starSize + scale, ST77XX_WHITE);

         // 4. Southern Cross (Simplified placement of 5 white circles)
         int scX = x + cantonW + (w - cantonW) / 4;
         int scY = y + cantonH + (h - cantonH) / 4;
         tft.fillCircle(scX, scY, starSize, ST77XX_WHITE);
         tft.fillCircle(scX + scale * 4, scY, starSize, ST77XX_WHITE);
         tft.fillCircle(scX, scY + scale * 4, starSize, ST77XX_WHITE);
         tft.fillCircle(scX + scale * 4, scY + scale * 4, starSize, ST77XX_WHITE);
         tft.fillCircle(scX + scale * 2, scY + scale * 8, starSize, ST77XX_WHITE); // Pointer star
}

/**
  * @brief Draws the Belgium Flag (Black, Yellow, Red vertical tricolor).
  */
void drawBEFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale; // 128
         int h = FLAG_H * scale; // 80
         int stripeW = w / 3;         // 42, 43, 43 (approx for 4x scale)

         // 1. Black
         tft.fillRect(x, y, stripeW, h, ST77XX_BLACK);
         // 2. Yellow
         tft.fillRect(x + stripeW, y, stripeW, h, ST77XX_YELLOW);
         // 3. Red
         tft.fillRect(x + 2 * stripeW, y, w - 2 * stripeW, h, ST77XX_RED);
}

/**
  * @brief Draws the Russia Flag (White, Blue, Red horizontal tricolor).
  */
void drawRUFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale; // 128
         int h = FLAG_H * scale; // 80
         int stripeH = h / 3;         // 26, 27, 27 (approx for 4x scale)

         // 1. White
         tft.fillRect(x, y, w, stripeH, ST77XX_WHITE);
         // 2. Blue
         tft.fillRect(x, y + stripeH, w, stripeH, ST77XX_BLUE);
         // 3. Red
         tft.fillRect(x, y + 2 * stripeH, w, h - 2 * stripeH, ST77XX_RED);
}

/**
  * @brief Draws the Canada Flag (Red-White-Red triband with simplified Maple Leaf).
  */
void drawCAFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale; // 128
         int h = FLAG_H * scale; // 80
         int bandW = w / 4;             // 32 pixels wide for the red bands (1:2:1 ratio)
         int centerW = w / 2;         // 64 pixels wide for the white band
         int leafRadius = scale * 6; // Radius of the simplified leaf (24 pixels at 4x scale)
         int centerX = x + w / 2;
         int centerY = y + h / 2;

         // 1. Left Red Band
         tft.fillRect(x, y, bandW, h, ST77XX_RED);
         // 2. White Center Band
         tft.fillRect(x + bandW, y, centerW, h, ST77XX_WHITE);
         // 3. Right Red Band
         tft.fillRect(x + bandW + centerW, y, w - (bandW + centerW), h, ST77XX_RED);

         // 4. Simplified Maple Leaf (Red Circle in the center)
         tft.fillCircle(centerX, centerY, leafRadius, ST77XX_RED);
}


// --- CUSTOM FLAG DRAWING LOGIC (Existing US/GB functions) ---

/**
  * @brief Draws the US Flag (Stars and Stripes) using geometry.
  * @param x X coordinate.
  * @param y Y coordinate.
  * @param scale The scaling factor (e.g., 4x for 128x80 on screen).
  */
void drawUSFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale;      // 128
         int h = FLAG_H * scale;      // 80
         int stripeH = h / 13;          // Height of one of the 13 stripes (~6 pixels)
         int cantonW = w * 2 / 5;    // Width of the blue union (approx 5/13 of the height, but adjusted for visual fit)
         int cantonH = h * 7 / 13; // Height of 7 stripes (~43 pixels)

         // 1. Draw the 13 Stripes (Red and White)
         for (int i = 0; i < 13; i++)
         {
                  uint16_t color = (i % 2 == 0) ? ST77XX_RED : ST77XX_WHITE;
                  tft.fillRect(x, y + i * stripeH, w, stripeH, color);
         }

         // Fill the remaining tiny gap at the bottom if 13 doesn't divide evenly
         tft.fillRect(x, y + 13 * stripeH, w, h - 13 * stripeH, ST77XX_RED);

         // 2. Draw the Blue Union (Canton)
         tft.fillRect(x, y, cantonW, cantonH, ST77XX_BLUE);

         // 3. Simple Star Pattern (5 placeholder stars for recognition)
         int starColor = ST77XX_WHITE;
         int starSize = scale > 3 ? 3 : 2; // Make stars slightly larger for 4x scale

         // Lambda to draw a simple circle as a 'star' placeholder
         auto drawStar = [&](int cx, int cy, int size)
         {
                  tft.fillCircle(cx, cy, size, starColor);
         };

         // Star placement coordinates (relative to the canton)
         int paddingX = cantonW / 6;
         int paddingY = cantonH / 6;

         // Top-left, Top-right, Bottom-left, Bottom-right, Center
         drawStar(x + paddingX, y + paddingY, starSize);
         drawStar(x + cantonW - paddingX, y + paddingY, starSize);
         drawStar(x + paddingX, y + cantonH - paddingY, starSize);
         drawStar(x + cantonW - paddingX, y + cantonH - paddingY, starSize);
         drawStar(x + cantonW / 2, y + cantonH / 2, starSize);
}

/**
  * @brief Draws the UK Flag (Union Jack) using geometry.
  * @param x X coordinate.
  * @param y Y coordinate.
  * @param scale The scaling factor.
  */
void drawGBFlag(int x, int y, int scale)
{
         int w = FLAG_W * scale; // 128
         int h = FLAG_H * scale; // 80

         // 1. Background Blue
         tft.fillRect(x, y, w, h, ST77XX_BLUE);

         // 2. Simplified St. Andrew's Cross (White diagonal)
         int stACW = scale * 2;                                                          // Thickness of the white diagonal lines
         tft.drawLine(x, y, x + w, y + h, ST77XX_WHITE); // Top-Left to Bottom-Right
         tft.drawLine(x, y + h, x + w, y, ST77XX_WHITE); // Bottom-Left to Top-Right
         // Thicken the cross using offsets for better visibility
         for (int i = 1; i < stACW / 2; i++)
         {
                  tft.drawLine(x, y + i, x + w, y + h + i, ST77XX_WHITE);
                  tft.drawLine(x + i, y, x + w + i, y + h, ST77XX_WHITE);
                  tft.drawLine(x, y + h - i, x + w, y - i, ST77XX_WHITE);
                  tft.drawLine(x + i, y + h, x + w + i, y, ST77XX_WHITE);
         }

         // 3. St. George's Cross (Red vertical/horizontal)
         int stGCW = scale * 3;                                                                                          // Thickness of the red cross
         tft.fillRect(x, y + h / 2 - stGCW / 2, w, stGCW, ST77XX_RED); // Horizontal
         tft.fillRect(x + w / 2 - stGCW / 2, y, stGCW, h, ST77XX_RED); // Vertical

         // 4. St. Patrick's Cross (Red diagonal - Thinner, drawn *over* St. Andrew's cross)
         // For simplicity, we draw it as a thin line centered on the existing white cross,
         // which effectively creates the white border around the red diagonal of the actual flag.
         int stPCW = scale;
         tft.drawLine(x + stACW / 2, y, x + w - stACW / 2, y + h, ST77XX_RED);
         tft.drawLine(x + stACW / 2, y + h, x + w - stACW / 2, y, ST77XX_RED);
}

// ******************************************************
// ** FLAG DRAWING DISPATCHER (for geometry or bitmap) **
// ******************************************************

/**
 * @brief Draws a flag bitmap from PROGMEM or uses custom geometry for specific flags.
 * @param flagCode The 2-letter country code to look up.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param scale The scaling factor (e.g., 1 for 32x20, 3 for 96x60).
 */
void drawFlag(const String &flagCode, int x, int y, int scale)
{
    // 1. Convert to upper case and check for custom drawn flags
    String code = flagCode;
    code.toUpperCase();

    // --- CUSTOM GEOMETRY FLAGS ---
         // --- EXISTING FLAGS ---
         if (code.equals("US")) { drawUSFlag(x, y, scale); return; }
         if (code.equals("GB")) { drawGBFlag(x, y, scale); return; }
         if (code.equals("IN")) { drawINFlag(x, y, scale); return; }
         if (code.equals("DE")) { drawDEFlag(x, y, scale); return; }
         if (code.equals("FR")) { drawFRFlag(x, y, scale); return; }
         if (code.equals("NL")) { drawNLFlag(x, y, scale); return; }
         if (code.equals("IE")) { drawIEFlag(x, y, scale); return; }
         if (code.equals("JP")) { drawJPFlag(x, y, scale); return; }
         if (code.equals("AU")) { drawAUFlag(x, y, scale); return; }
         if (code.equals("BE")) { drawBEFlag(x, y, scale); return; }
         if (code.equals("RU")) { drawRUFlag(x, y, scale); return; }
         if (code.equals("CA")) { drawCAFlag(x, y, scale); return; }

         // --- IMPROVED / NEW FLAGS ---
         if (code.equals("BR")) { drawBRFlag(x, y, scale); return; }
         if (code.equals("AR")) { drawARFlag(x, y, scale); return; }
         if (code.equals("AT")) { drawATFlag(x, y, scale); return; }
         if (code.equals("CL")) { drawCLFlag(x, y, scale); return; }
         if (code.equals("CN")) { drawCNFlag(x, y, scale); return; }
         if (code.equals("CO")) { drawCOFlag(x, y, scale); return; }
         if (code.equals("DK")) { drawDKFlag(x, y, scale); return; }
         if (code.equals("EG")) { drawEGFlag(x, y, scale); return; }
         if (code.equals("FI")) { drawFIFlag(x, y, scale); return; }
         if (code.equals("GR")) { drawGRFlag(x, y, scale); return; }
         if (code.equals("ID")) { drawIDFlag(x, y, scale); return; }
         if (code.equals("IT")) { drawITFlag(x, y, scale); return; }
         if (code.equals("KE")) { drawKEFlag(x, y, scale); return; }
         if (code.equals("MX")) { drawMXFlag(x, y, scale); return; }
         if (code.equals("NZ")) { drawNZFlag(x, y, scale); return; }
         if (code.equals("NO")) { drawNOFlag(x, y, scale); return; }
         if (code.equals("PL")) { drawPLFlag(x, y, scale); return; }
         if (code.equals("PT")) { drawPTFlag(x, y, scale); return; }
         if (code.equals("ZA")) { drawZAFlag(x, y, scale); return; }
         if (code.equals("KR")) { drawKRFlag(x, y, scale); return; }
         if (code.equals("ES")) { drawESFlag(x, y, scale); return; }
         if (code.equals("SE")) { drawSEFlag(x, y, scale); return; }
         if (code.equals("CH")) { drawCHFlag(x, y, scale); return; }
         if (code.equals("TR")) { drawTRFlag(x, y, scale); return; }
         else {
            int scaledW = FLAG_W * scale;
            int scaledH = FLAG_H * scale;
            tft.drawRect(x, y, scaledW, scaledH, ST77XX_RED);
            tft.fillRect(x + 1, y + 1, scaledW - 2, scaledH - 2, ST77XX_BLACK);
            tft.setCursor(x + 5, y + (scaledH / 2) - 5);
            tft.setTextSize(1);
            tft.setTextColor(ST77XX_RED);
            tft.print(code.c_str());
            return;
         }

    // // --- FALLBACK TO BITMAP LOOKUP FOR ALL OTHER FLAGS ---
    // const uint16_t *bitmapData = nullptr;

    // for (int i = 0; i < NUM_FLAGS; i++)
    // {
    //     // Check against the array of bitmap data codes defined in flag_data.h
    //     if (code.equals(FLAG_LOOKUP[i].code))
    //     {
    //         bitmapData = FLAG_LOOKUP[i].bitmap;
    //         break;
    //     }
    // }

    // int scaledW = FLAG_W * scale;
    // int scaledH = FLAG_H * scale;

    // // Fallback: If code is not found, draw a simple 'Unknown' pattern
    // if (bitmapData == nullptr)
    // {
    //     tft.drawRect(x, y, scaledW, scaledH, ST77XX_RED);
    //     tft.fillRect(x + 1, y + 1, scaledW - 2, scaledH - 2, ST77XX_BLACK);
    //     tft.setCursor(x + 5, y + (scaledH / 2) - 5);
    //     tft.setTextSize(1);
    //     tft.setTextColor(ST77XX_RED);
    //     tft.print(code.c_str());
    //     return;
    // }

    // // 2. Draw the bitmap using fast pixel writing with scaling
    // tft.startWrite();
    // // Set the address window for the entire scaled flag area
    // tft.setAddrWindow(x, y, x + scaledW - 1, y + scaledH - 1);

    // for (int row = 0; row < FLAG_H; row++)
    // {
    //     for (int r = 0; r < scale; r++)
    //     { // Row scaling loop: Repeat row 'scale' times
    //         for (int col = 0; col < FLAG_W; col++)
    //         {
    //             // Read 16-bit color from PROGMEM (Flash memory)
    //             uint16_t color = pgm_read_word(&bitmapData[row * FLAG_W + col]);

    //             // Column scaling loop: Write the color 'scale' times
    //             tft.writeColor(color, scale);
    //         }
    //     }
    // }
    // tft.endWrite();
    // tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); // Reset window
}