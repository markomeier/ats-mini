#include "Common.h"
#include "Themes.h"
#include "Storage.h"
#include "Utils.h"
#include "Menu.h"
#include "BleMode.h"
#include "Draw.h"
#include "Audio_icons.h" // Enthält die Icon-Definitionen[span_0](start_span)[span_0](end_span)

extern void drawLayoutSmeter();
extern void drawLayoutDefault();
extern void drawAbout();

uint32_t lastTuneTime = 0;

//
// La Linea Lauf-Animation (mit dicken, gut erkennbaren Linien)
//
void drawRunningMan(int x, int y, int width, int height) {
    // Bereich unter der Skala sauber löschen, ohne den Rahmen zu beschädigen
    spr.fillRect(0, 126, 320, 47, TH.bg);

    // Berechne eine X-Position, die langsam von links nach rechts über das Display wandert
    uint32_t animCycle = (millis() / 30) % (width - 24); 
    int manX = x + animCycle;
    int manY = y + (height / 2) - 12; // Vertikal zentriert

    // Zeichnet das La Linea Männchen (aus img_mono[span_1](start_span)[span_1](end_span)) mit dicker Strichstärke
    for (int py = 0; py < AUDIO_ICON_HEIGHT; py++) {
        for (int px = 0; px < AUDIO_ICON_WIDTH; px++) {
            uint16_t color = pgm_read_word(&img_mono[py * AUDIO_ICON_WIDTH + px]);[span_2](start_span)[span_2](end_span)
            
            if (color != 0x0000) { 
                if (color == 0xF800) { 
                    color = 0x07E0; // Rot zu Neongrün wandeln
                }
                if (px < 24 && py < 24) {
                    // 2x2 Pixelblock pro Punkt für kräftige, dicke Linien
                    int drawX = manX + px;
                    int drawY = manY + (py - 12);
                    spr.drawPixel(drawX, drawY, color);
                    spr.drawPixel(drawX + 1, drawY, color);     
                    spr.drawPixel(drawX, drawY + 1, color);     
                    spr.drawPixel(drawX + 1, drawY + 1, color); 
                }
            }
        }
    }
}

void drawSaveIndicator(int x, int y)
{
  if(prefsAreWritten() || switchThemeEditor())
  {
    spr.fillRect(x+3, y+2, 3, 5, TH.save_icon);
    spr.fillTriangle(x+1, y+7, x+7, y+7, x+4, y+10, TH.save_icon);
    spr.drawLine(x, y+12, x, y+13, TH.save_icon);
    spr.drawLine(x, y+13, x+8, y+13, TH.save_icon);
    spr.drawLine(x+8, y+12, x+8, y+12, TH.save_icon);
  }
}

void drawBleIndicator(int x, int y)
{
  int8_t status = getBleStatus();

  if(status || switchThemeEditor())
  {
    uint16_t color = (status>0) ? TH.rf_icon_conn : TH.rf_icon;

    if(switchThemeEditor())
      color = millis()&0x2000? TH.rf_icon_conn : TH.rf_icon;

    spr.drawLine(x+3, y+1, x+3, y+13, color);
    spr.drawLine(x+3, y+1, x+6, y+4, color);
    spr.drawLine(x+6, y+4, x, y+10, color);
    spr.drawLine(x, y+4, x+6, y+10, color);
    spr.drawLine(x+6, y+10, x+3, y+13, color);
  }
}

void drawWiFiIndicator(int x, int y)
{
  int8_t status = getWiFiStatus();

  if(status || switchThemeEditor())
  {
    uint16_t color = (status>0) ? TH.rf_icon_conn : TH.rf_icon;

    if(switchThemeEditor())
      color = millis()&0x2000? TH.rf_icon_conn : TH.rf_icon;

    spr.fillArc(x, 15+y, 14, 13, 240, 300, color);
    spr.fillArc(x, 15+y, 9, 8, 240, 300, color);
    spr.fillArc(x, 15+y, 4, 3, 240, 300, color);
  }
}

bool drawStatus(int x, int y)
{
  if(statusLines[0][0] || statusLines[1][0])
  {
    spr.setTextDatum(TC_DATUM);
    spr.setTextColor(0x07E0);
    spr.drawString(statusLines[0], x, y, FONT_SMALL);
    spr.drawString(statusLines[1], x, y+17, FONT_SMALL);
    return(true);
  }

  return(false);
}

void drawZoomedMenu(const char *text, bool force)
{
  if (!zoomMenu && !force) return;

  spr.fillSmoothRoundRect(RDS_OFFSET_X - 72 + 1, RDS_OFFSET_Y - 3 + 1, 152, 26, 4, TH.menu_bg);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(TH.menu_item);
  spr.drawString(text, RDS_OFFSET_X + 5, RDS_OFFSET_Y, FONT_LARGE);
  spr.drawRoundRect(RDS_OFFSET_X - 72, RDS_OFFSET_Y - 3, 154, 28, 4, TH.menu_border);
}

void drawMessage(const char *msg)
{
  if(sleepOn()) return;

  drawZoomedMenu(msg, true);
  spr.pushSprite(0, 0);
}

void drawBandAndMode(const char *band, const char *mode, int x, int y)
{
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFCA0);
  uint16_t band_width = spr.drawString(band, x, y);

  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0xFCA0);
  uint16_t mode_width = spr.drawString(mode, x + band_width / 2 + 10, y);
}

void drawRadioText(int y, int ymax)
{
  const char *rt = getRadioText();
  
  spr.setTextColor(0x07E0);   
  spr.setTextSize(1.0);       
  spr.setFont(&fonts::Font4); 

  if (rt && *rt) 
  {
    spr.setTextDatum(TC_DATUM);
    spr.drawString(rt, 160, y);
  } 
}

void drawFrequency(uint32_t freq, int x, int y, int ux, int uy, uint8_t hl)
{
  static uint32_t lastFreq = 0;

  if (freq != lastFreq) {
    lastFreq = freq;
    lastTuneTime = millis();
  }

  uint16_t unitColor = ((millis() - lastTuneTime) < 800) ? 0xFFE0 : 0x07E0;

  struct Line { int x, y, w; };

  const Line hlDigitsFM[] =
  {
    { x - 30 - 32 * 0 -  0, y + 28, 27 }, 
    { x - 30 - 32 * 0 - 16, y + 28, 27 + 16 }, 
    { x - 30 - 32 * 1 -  0, y + 28, 27 }, 
    { x - 30 - 32 * 1 - 22, y + 28, 27 + 22 }, 
    { x - 30 - 32 * 2 - 12, y + 28, 27 }, 
    { x - 30 - 32 * 2 - 28, y + 28, 27 + 16 }, 
    { x - 30 - 32 * 3 - 12, y + 28, 27 }, 
    { x - 30 - 32 * 3 - 28, y + 28, 27 + 16 }, 
    { x - 30 - 32 * 4 +  4, y + 28, 11 }, 
  };

  const Line hlDigitsAMSSB[] =
  {
    { x + 12 + 14 * 2 -  0, y + 28, 12 }, 
    { x + 12 + 14 * 2 -  7, y + 28, 12 + 7 }, 
    { x + 12 + 14 * 1 -  0, y + 28, 12 }, 
    { x + 12 + 14 * 1 -  7, y + 28, 12 + 7 }, 
    { x + 12 + 14 * 0 -  0, y + 28, 12 }, 
    { x + 12 + 14 * 0 - 11, y + 28, 12 + 11 }, 
    { x - 30 - 32 * 0 -  0, y + 28, 27 }, 
    { x - 30 - 32 * 0 - 16, y + 28, 27 + 16 }, 
    { x - 30 - 32 * 1 -  0, y + 28, 27 }, 
    { x - 30 - 32 * 1 - 16, y + 28, 27 + 16 }, 
    { x - 30 - 32 * 2 -  0, y + 28, 27 }, 
    { x - 30 - 32 * 2 - 16, y + 28, 27 + 16 }, 
    { x - 30 - 32 * 3 -  0, y + 28, 27 }, 
    { x - 30 - 32 * 3 - 16, y + 28, 27 + 16 }, 
    { x - 30 - 32 * 4 -  0, y + 28, 27 }, 
  };

  bool selectOn = hl & 0x80;
  const struct Line *li;

  hl &= 0x7F;

  spr.setTextDatum(MR_DATUM);
  spr.setTextColor(TH.freq_text);

  if(currentMode==FM)
  {
    li = hl<ITEM_COUNT(hlDigitsFM)? &hlDigitsFM[hl] : 0;

    spr.drawFloat(freq/100.00, 2, x, y, FONT_DIGITS);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(unitColor); 
    spr.drawString("MHz", ux, uy);
  }
  else
  {
    li = hl<ITEM_COUNT(hlDigitsAMSSB)? &hlDigitsAMSSB[hl] : 0;

    if(isSSB())
    {
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
      spr.drawNumber(freq, x, y, FONT_DIGITS);
      spr.setTextDatum(ML_DATUM);
      spr.drawString(".000", 4+x, 17+y, FONT_LARGE);
    }

    spr.setTextColor(unitColor); 
    spr.drawString("kHz", ux, uy);
  }

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

void drawScale(uint32_t freq)
{
  spr.fillCircle(160, 140, 5, 0xF800);
  
  spr.drawFastVLine(160, 145, 24, 0xF800);
  spr.drawFastVLine(161, 145, 24, 0xF800);

  spr.drawFastHLine(0, 148, 320, TH.scale_line); 
  spr.drawFastHLine(0, 169, 320, TH.scale_line); 

  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TH.scale_text);

  int16_t slack = 3;
  int16_t offset = ((freq % 10) / 10.0 + slack) * 8;

  freq = freq / 10 - 20 - slack;

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
        spr.drawFastVLine(x, 149, 20, lineColor);
        spr.drawFastVLine(x + 1, 149, 20, lineColor);

        if(currentMode == FM)
          spr.drawFloat(freq / 10.0, 1, x, 140, FONT_SMALL);
        else if(freq >= 100)
          spr.drawFloat(freq / 100.0, 3, x, 140, FONT_SMALL);
        else
          spr.drawNumber(freq * 10, x, 140, FONT_SMALL);
      }
      else if((freq % 5) == 0 && (freq % 10) != 0)
      {
        spr.drawFastVLine(x, 157, 12, lineColor);
      }
      else
      {
        spr.drawFastVLine(x, 162, 7, lineColor);
      }
    }
  }
}

void drawSMeter(int strength, int x, int y)
{
  static int peakBar = 0;
  static uint32_t lastPeakTime = 0;
  static uint32_t lastDecayTime = 0;

  uint32_t now = millis();

  if (strength >= peakBar) {
    peakBar = strength;
    lastPeakTime = now;
    lastDecayTime = now;
  } 
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
// Stereo-Indikator entfernt
//
void drawStereoIndicator(int x, int y, bool stereo)
{
  // Absichtlich leer
}

void drawStationName(const char *name, int x, int y)
{
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(TH.rds_text);
  spr.drawString(name, x, y, FONT_LARGE);
}

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

void drawScanGraphs(uint32_t freq)
{
  int16_t offset = (freq % 10) / 10.0 * 8;
  freq = freq / 10 - 20;

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
  spr.fillTriangle(156, 125, 160, 130, 164, 125, TH.scale_pointer);
  spr.drawLine(160, 130, 160, 169, TH.scale_pointer);
}

void drawScreen()
{
  if(sleepOn()) return;

  spr.fillSprite(TH.bg);

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

  // Nach 3 Sekunden Inaktivität rennt das dicke La Linea Männchen los
  if ((millis() - lastTuneTime) > 3000) 
  {
    drawRunningMan(10, 142, 300, 26);
  } 
  else 
  {
    drawScale(currentFrequency);
  }

  spr.pushSprite(0, 0);
}
