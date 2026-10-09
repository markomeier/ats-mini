#include "Common.h"
#include "Themes.h"
#include "Storage.h"
#include "Utils.h"
#include "Menu.h"
#include "BleMode.h"
#include "Draw.h"

uint32_t lastTuneTime = 0;

//
// Draw preferences write indicator
//
void drawSaveIndicator(int x, int y)
{
  if(prefsAreWritten() || switchThemeEditor())
  {
    // Draw preferences write request icon
    spr.fillRect(x+3, y+2, 3, 5, TH.save_icon);
    spr.fillTriangle(x+1, y+7, x+7, y+7, x+4, y+10, TH.save_icon);
    spr.drawLine(x, y+12, x, y+13, TH.save_icon);
    spr.drawLine(x, y+13, x+8, y+13, TH.save_icon);
    spr.drawLine(x+8, y+13, x+8, y+12, TH.save_icon);
  }
}

//
// Draw Bluetooth indicator
//
void drawBleIndicator(int x, int y)
{
  int8_t status = getBleStatus();

  // If need to draw BLE icon...
  if(status || switchThemeEditor())
  {
    uint16_t color = (status>0) ? TH.rf_icon_conn : TH.rf_icon;

    // For the editor, alternate between BLE states every ~8 seconds
    if(switchThemeEditor())
      color = millis()&0x2000? TH.rf_icon_conn : TH.rf_icon;

    spr.drawLine(x+3, y+1, x+3, y+13, color);
    spr.drawLine(x+3, y+1, x+6, y+4, color);
    spr.drawLine(x+6, y+4, x, y+10, color);
    spr.drawLine(x, y+4, x+6, y+10, color);
    spr.drawLine(x+6, y+10, x+3, y+13, color);
  }
}

//
// Draw WiFi indicator
//
void drawWiFiIndicator(int x, int y)
{
  int8_t status = getWiFiStatus();

  // If need to draw WiFi icon...
  if(status || switchThemeEditor())
  {
    uint16_t color = (status>0) ? TH.rf_icon_conn : TH.rf_icon;

    // For the editor, alternate between WiFi states every ~8 seconds
    if(switchThemeEditor())
      color = millis()&0x2000? TH.rf_icon_conn : TH.rf_icon;

    spr.fillArc(x, 15+y, 14, 13, 240, 300, color);
    spr.fillArc(x, 15+y, 9, 8, 240, 300, color);
    spr.fillArc(x, 15+y, 4, 3, 240, 300, color);
  }
}

//
// Draw operation status
//
bool drawStatus(int x, int y)
{
  if(statusLines[0][0] || statusLines[1][0])
  {
    // Draw two lines of operation status
    spr.setTextDatum(TC_DATUM);
    spr.setTextColor(0x07E0);
    spr.drawString(statusLines[0], x, y, FONT_SMALL);
    spr.drawString(statusLines[1], x, y+17, FONT_SMALL);
    return(true);
  }

  return(false);
}

//
// Draw zoomed menu item
//
void drawZoomedMenu(const char *text, bool force)
{
  if (!zoomMenu && !force) return;

  spr.fillSmoothRoundRect(RDS_OFFSET_X - 72 + 1, RDS_OFFSET_Y - 3 + 1, 152, 26, 4, TH.menu_bg);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(TH.menu_item);
  spr.drawString(text, RDS_OFFSET_X + 5, RDS_OFFSET_Y, FONT_LARGE);
  spr.drawRoundRect(RDS_OFFSET_X - 72, RDS_OFFSET_Y - 3, 154, 28, 4, TH.menu_border);
}

//
// Show overlay message in large letters
//
void drawMessage(const char *msg)
{
  if(sleepOn()) return;

  drawZoomedMenu(msg, true);
  spr.pushSprite(0, 0);
}

//
// Draw band and mode indicators
//
void drawBandAndMode(const char *band, const char *mode, int x, int y)
{
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFCA0);
  uint16_t band_width = spr.drawString(band, x, y);

  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0xFCA0);
  uint16_t mode_width = spr.drawString(mode, x + band_width / 2 + 10, y);
}

//
// Draw radio text (RDS und Signalwerte in grosser Schrift Font 4)
//
void drawRadioText(int y, int ymax)
{
  const char *rt = getRadioText();
  
  spr.setTextColor(0x07E0);   // Neongrün
  spr.setTextSize(1.0);       // Standard-Skalierung für saubere Pixel

  // Fall 1: RDS-Text vorhanden -> Zentriert mit Font 4 gross zeichnen
  if (rt && *rt) 
  {
    spr.setTextDatum(TC_DATUM);
    spr.drawString(rt, 160, y, 4);
  } 
  // Fall 2: Kein RDS -> Signalwerte in Font 4 (kompakt formatiert für 2-stellige Werte)
  else 
  {
    spr.setTextDatum(TL_DATUM);

    char sigBuf[32];
    // Straffes Format, damit selbst bei S:99 | SNR:30 dB nichts rechts abgeschnitten wird
    snprintf(sigBuf, sizeof(sigBuf), "S:%d | SNR:%d dB", rx.getCurrentRSSI(), rx.getCurrentSNR());
    
    // Font 4 verwenden (groß & gut lesbar)
    // X = 90 schiebt den Text passgenau rechts neben die gelbe Box
    spr.drawString(sigBuf, 90, y - 6, 4);
  }
}

//
// Draw frequency
//
void drawFrequency(uint32_t freq, int x, int y, int ux, int uy, uint8_t hl)
{
  // --- AUTOMATISCHE TUNING-ERKENNUNG ---
  static uint32_t lastFreq = 0;

  if (freq != lastFreq) {
    lastFreq = freq;
    lastTuneTime = millis(); // Kurbeln erkannt -> Timer triggern!
  }

  // Farbwahl: Gelb (0xFFE0) während des Drehens (800ms), sonst Neongrün (0x07E0)
  uint16_t unitColor = ((millis() - lastTuneTime) < 800) ? 0xFFE0 : 0x07E0;
  // --------------------------------------------------------------------------

  struct Line { int x, y, w; };

  const Line hlDigitsFM[] =
  {
    { x - 30 - 32 * 0 -  0, y + 28, 27 }, //         .01
    { x - 30 - 32 * 0 - 16, y + 28, 27 + 16 }, //    .05
    { x - 30 - 32 * 1 -  0, y + 28, 27 }, //         .10
    { x - 30 - 32 * 1 - 22, y + 28, 27 + 22 }, //    .50
    { x - 30 - 32 * 2 - 12, y + 28, 27 }, //        1.00
    { x - 30 - 32 * 2 - 28, y + 28, 27 + 16 }, //   5.00
    { x - 30 - 32 * 3 - 12, y + 28, 27 }, //       10.00
    { x - 30 - 32 * 3 - 28, y + 28, 27 + 16 }, //  50.00
    { x - 30 - 32 * 4 +  4, y + 28, 11 }, //      100.00
  };

  const Line hlDigitsAMSSB[] =
  {
    { x + 12 + 14 * 2 -  0, y + 28, 12 }, //           .001
    { x + 12 + 14 * 2 -  7, y + 28, 12 + 7 }, //       .005
    { x + 12 + 14 * 1 -  0, y + 28, 12 }, //           .010
    { x + 12 + 14 * 1 -  7, y + 28, 12 + 7 }, //       .050
    { x + 12 + 14 * 0 -  0, y + 28, 12 }, //           .100
    { x + 12 + 14 * 0 - 11, y + 28, 12 + 11 }, //      .500
    { x - 30 - 32 * 0 -  0, y + 28, 27 }, //          1.000
    { x - 30 - 32 * 0 - 16, y + 28, 27 + 16 }, //     5.000
    { x - 30 - 32 * 1 -  0, y + 28, 27 }, //         10.000
    { x - 30 - 32 * 1 - 16, y + 28, 27 + 16 }, //    50.000
    { x - 30 - 32 * 2 -  0, y + 28, 27 }, //        100.000
    { x - 30 - 32 * 2 - 16, y + 28, 27 + 16 }, //   500.000
    { x - 30 - 32 * 3 -  0, y + 28, 27 }, //       1000.000
    { x - 30 - 32 * 3 - 16, y + 28, 27 + 16 }, //  5000.000
    { x - 30 - 32 * 4 -  0, y + 28, 27 }, //      10000.000
  };

  // Top bit specifies if the digit selector is on
  bool selectOn = hl & 0x80;
  const struct Line *li;

  // Lower 7 bits specify the selected digit
  hl &= 0x7F;

  spr.setTextDatum(MR_DATUM);
  spr.setTextColor(TH.freq_text);

  if(currentMode==FM)
  {
    // Determine where underscore is located
    li = hl<ITEM_COUNT(hlDigitsFM)? &hlDigitsFM[hl] : 0;

    // FM frequency
    spr.drawFloat(freq/100.00, 2, x, y, FONT_DIGITS);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(unitColor); // Dynamische Farbe (Gelb beim Tunen, sonst Neongrün)
    spr.drawString("MHz", ux, uy);
  }
  else
  {
    // Determine where underscore is located
    li = hl<ITEM_COUNT(hlDigitsAMSSB)? &hlDigitsAMSSB[hl] : 0;

    if(isSSB())
    {
      // SSB frequency
      char text[32];
      freq = freq * 1000 + currentBFO;
      sprintf(text, "%3.3lu", freq / 1000);
      spr.drawString(text, x, y, FONT_DIGITS);
      spr.setTextDatum(ML_DATUM);
      sprintf(text, ".%3.3lu", freq % 1000);
      spr.drawString(text, 4+x, 17+y, FONT_LARGE);
    }
    else
    {
      // AM frequency
      spr.drawNumber(freq, x, y, FONT_DIGITS);
      spr.setTextDatum(ML_DATUM);
      spr.drawString(".000", 4+x, 17+y, FONT_LARGE);
    }

    // SSB/AM frequencies are measured in kHz
    spr.setTextColor(unitColor); // Dynamische Farbe (Gelb beim Tunen, sonst Neongrün)
    spr.drawString("kHz", ux, uy);
  }

  // If drawing an underscore...
  if(li)
  {
    if(selectOn)
    {
      spr.fillRoundRect(li->x + 1, li->y - 1, li->w - 2, 3, 1, TH.freq_hl_sel);
      spr.fillTriangle(li->x, li->y, li->x + 2, li->y - 2, li->x + 2, li->y - 2, TH.freq_hl_sel);
      spr.fillTriangle(li->x + li->w - 1, li->y, li->x + li->w - 3, li->y - 2, li->x + li->w - 3, li->y + 2, TH.freq_hl_sel);
    }
    else
    {
      spr.fillRoundRect(li->x, li->y - 1, li->w, 3, 1, TH.freq_hl);
    }
  }
}

//
// Draw tuner scale Roehrenradiostyle
//
void drawScale(uint32_t freq)
{
  // 1. Roter Kreis oben (Mittelpunkt x=160, y=140, Radius=5)
  spr.fillCircle(160, 140, 5, 0xF800);
  
  // 2. Dickerer roter Zeigerstrich (2 Pixel breit, von Y=145 bis Y=169)
  spr.drawFastVLine(160, 145, 24, 0xF800);
  spr.drawFastVLine(161, 145, 24, 0xF800);

  // 3. Führungslinien (Ober- und Unterkante)
  spr.drawFastHLine(0, 148, 320, TH.scale_line); // Obere Führungslinie
  spr.drawFastHLine(0, 169, 320, TH.scale_line); // Untere Führungslinie

  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TH.scale_text);

  // Extra frequencies to draw outside the screen boundaries
  int16_t slack = 3;

  // Scale offset
  int16_t offset = ((freq % 10) / 10.0 + slack) * 8;

  // Start drawing frequencies from the left
  freq = freq / 10 - 20 - slack;

  // Get band edges
  const Band *band = getCurrentBand();
  uint32_t minFreq = band->minimumFreq / 10;
  uint32_t maxFreq = band->maximumFreq / 10;

  for(int i=0 ; i<(slack + 41 + slack) ; i++, freq++)
  {
    int16_t x = i * 8 - offset;
    if(freq >= minFreq && freq <= maxFreq)
    {
      uint16_t lineColor = (i==20) && (!offset || (!(freq%5) && offset==1))?
        0xF800 : TH.scale_line;

      if((freq % 10) == 0)
      {
        // 10er-Hauptstriche (doppelt für fettere Optik)
        spr.drawFastVLine(x, 149, 20, lineColor);
        spr.drawFastVLine(x + 1, 149, 20, lineColor);

        // Frequenzzahlen direkt über der Skala
        if(currentMode == FM)
          spr.drawFloat(freq / 10.0, 1, x, 140, FONT_SMALL);
        else if(freq >= 100)
          spr.drawFloat(freq / 100.0, 3, x, 140, FONT_SMALL);
        else
          spr.drawNumber(freq * 10, x, 140, FONT_SMALL);
      }
      else if((freq % 5) == 0 && (freq % 10) != 0)
      {
        // 5er-Striche (mittellang)
        spr.drawFastVLine(x, 157, 12, lineColor);
      }
      else
      {
        // 1er-Striche (kurz)
        spr.drawFastVLine(x, 162, 7, lineColor);
      }
    }
  }
}

//
// Draw S-meter mit flüssigem Peak-Hold Abfalleffekt
//
void drawSMeter(int strength, int x, int y)
{
  static int peakBar = 0;
  static uint32_t lastPeakTime = 0;
  static uint32_t lastDecayTime = 0;

  uint32_t now = millis();

  // Neuer Höchstwert erreicht -> Peak anheben & Timer zurücksetzen
  if (strength >= peakBar) {
    peakBar = strength;
    lastPeakTime = now;
    lastDecayTime = now;
  } 
  // Nach 1.2 Sekunden Inaktivität schrittweise alle 150ms abfallen lassen
  else if (now - lastPeakTime > 1200) {
    if (now - lastDecayTime > 150) {
      if (peakBar > 0) peakBar--;
      lastDecayTime = now;
    }
  }

  spr.drawTriangle(x + 1, y + 1, x + 11, y + 1, x + 6, y + 6, 0xFCA0);
  spr.drawLine(x + 6, y + 1, x + 6, y + 14, 0xFCA0);

  for(int i = 0; i < 17; i++)
  {
    int barX = 15 + x + (i * 4);

    if (i == peakBar && peakBar > 0)
    {
      // Peak-Hold: Der aktuell gehaltene/abfallende Peak-Balken ist ROTE ZIEL-MARKE
      spr.fillRect(barX, 2 + y, 2, 12, 0xF800);
    }
    else if (i < 10 && i < strength)
    {
      spr.fillRect(barX, 2 + y, 2, 12, 0xFCA0);
    }
    else if (i < strength)
    {
      spr.fillRect(barX, 2 + y, 2, 12, TH.smeter_bar_plus);
    }
    else
    {
      spr.fillRect(barX, 2 + y, 2, 12, TH.smeter_bar_empty);
    }
  }
}

//
// Draw stereo indicator
//
void drawStereoIndicator(int x, int y, bool stereo)
{
  if(stereo)
  {
    // Split S-meter into two rows
    spr.fillRect(15 + x, 7 + y, 4 * 17 - 2, 2, TH.bg);
  }
}

//
// Draw RDS station name (also CB channel, etc)
//
void drawStationName(const char *name, int x, int y)
{
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(TH.rds_text);
  spr.drawString(name, x, y, FONT_LARGE);
}

//
// Draw long (EIBI) station name
//
void drawLongStationName(const char *name, int x, int y)
{
  int width = spr.textWidth(name, FONT_SMALL);
  spr.setTextColor(TH.rds_text);

  if((x + width) >= 320)
  {
    spr.setTextDatum(TL_DATUM);
    spr.drawString(name, x, y, FONT_SMALL);
  }
  else if(width <= 60)
  {
    spr.setTextDatum(TC_DATUM);
    spr.drawString(name, x + (320 - x) / 3, y, FONT_SMALL);
  }
  else
  {
    spr.setTextDatum(TC_DATUM);
    spr.drawString(name, x + (320 - x + width) / 4, y, FONT_SMALL);
  }
}

//
// Draw scan graphs
//
void drawScanGraphs(uint32_t freq)
{
  // Scale offset
  int16_t offset = (freq % 10) / 10.0 * 8;

  // Start drawing frequencies from the left
  freq = freq / 10 - 20;

  // Get band edges
  const Band *band = getCurrentBand();
  uint32_t minFreq = band->minimumFreq / 10;
  uint32_t maxFreq = band->maximumFreq / 10;

  for(int i=0 ; i<41 ; i++, freq++)
  {
    int16_t x = i * 8 - offset;

    if(freq >= minFreq && freq <= maxFreq)
    {
      if((freq % 5) == 0) {
        for(int y=0; y<42; y+=2) {
          spr.drawPixel(x, 169-y, TH.scan_grid);
        }
      }

      if((freq+1) <= maxFreq) {
        for(int xd=x; xd<(x+8); xd+=2) {
          spr.drawPixel(xd, 169-40, TH.scan_grid);
          spr.drawPixel(xd, 169-30, TH.scan_grid);
          spr.drawPixel(xd, 169-20, TH.scan_grid);
          spr.drawPixel(xd, 169-10, TH.scan_grid);
          spr.drawPixel(xd, 169-0, TH.scan_grid);
        }
        int snr1 = 40 * scanGetSNR(freq * 10);
        int snr2 = 40 * scanGetSNR((freq+1) * 10);
        spr.drawLine(x, 169-snr1, x+8, 169-snr2, TH.scan_snr);
        int rssi1 = 40 * scanGetRSSI(freq * 10);
        int rssi2 = 40 * scanGetRSSI((freq+1) * 10);
        spr.drawLine(x, 169-rssi1, x+8, 169-rssi2, TH.scan_rssi);
      }
    }
  }
  // Scale pointer
  spr.fillTriangle(156, 125, 160, 130, 164, 125, TH.scale_pointer);
  spr.drawLine(160, 130, 160, 169, TH.scale_pointer);
}

//
// Draw screen according to given command
//
void drawScreen()
{
  if(sleepOn()) return;

  // Clear screen buffer
  spr.fillSprite(TH.bg);

  // About screen is a special case
  if(currentCmd==CMD_ABOUT)
  {
    drawAbout();
    return;
  }

  switch(uiLayoutIdx)
  {
    case UI_SMETER:
      drawLayoutSmeter();
      break;
    default:
      drawLayoutDefault();
      break;
  }

  spr.pushSprite(0, 0);
}
