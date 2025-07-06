#include "displayDriver.h"

#ifdef LILYGO_T5_V231_DISPLAY


#include "monitor.h"
#include "wManager.h"
#define LILYGO_T5_V213
#include <boards.h>
#include <GxEPD.h>
#include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w  old panel , form GoodDisplay

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <Fonts/FreeMonoBold9pt7b.h> //Font


GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

extern monitor_data mMonitor;
bool ledOn = false;

const char HelloWorld[] = "Hello World!"; //String to print
uint16_t delta = 20;

bool hasChangedScreen = true;


//-----------------------------------------------------------------------------


void lilygot5v231display_Init(void)
{
  Serial.println("LilyGO T5 V2.3.1 display driver initialized");
  
  SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);

  display.init();
  display.setTextColor(GxEPD_BLACK);
  display.eraseDisplay();
  display.fillScreen(GxEPD_WHITE);
  display.update();
  delay(1000);
  Serial.print("width: ");
  Serial.println(display.width());
  Serial.print("height: ");
  Serial.println(display.height());

  Serial.println("LilyGO T5 V2.3.1 display driver initialization completed");
}

void lilygot5v231display_AlternateScreenState(void)
{
  Serial.println("Switching display state");
  ledOn = !ledOn;
}

void lilygot5v231display_AlternateRotation(void)
{
}


double cycleCounter = 0;

unsigned long previousMillisDisplay = 0;

void lilygot5v231display_MinerScreen(unsigned long mElapsed)
{
  mining_data data = getMiningData(mElapsed);

  // Print hashrate to serial
  Serial.printf(">>> Completed %s share(s), %s Khashes, avg. hashrate %s KH/s\n",
                data.completedShares.c_str(), data.totalKHashes.c_str(), data.currentHashRate.c_str());

  // Print extended data to serial for no display devices
  Serial.printf(">>> Valid blocks: %s\n", data.valids.c_str());
  Serial.printf(">>> Block templates: %s\n", data.templates.c_str());
  Serial.printf(">>> Best difficulty: %s\n", data.bestDiff.c_str());
  Serial.printf(">>> 32Bit shares: %s\n", data.completedShares.c_str());
  Serial.printf(">>> Temperature: %s\n", data.temp.c_str());
  Serial.printf(">>> Total MHashes: %s\n", data.totalMHashes.c_str());
  Serial.printf(">>> Time mining: %s\n", data.timeMining.c_str());
  
  unsigned long currentMillis = millis();

  if (hasChangedScreen || previousMillisDisplay == 0 || currentMillis - previousMillisDisplay >= (1000*60))
    { // 0.5sec blink
      hasChangedScreen = false;
      previousMillisDisplay = currentMillis;
      //digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Cambia el estado del LED
      //cleanDisplay();
      
      display.fillScreen(GxEPD_WHITE);
      
      display.setRotation(0);
      display.setTextColor(GxEPD_BLACK);
      display.setCursor(0, 10);
      display.setFont(&FreeMonoBold9pt7b);
      display.println("Valid Blx");
      
      display.print(data.valids.c_str());
      display.print("\n");
      display.println("Blx Templ.");
      display.println(data.templates.c_str());
      display.println("Best Diff.");
      display.println(data.bestDiff.c_str());
      display.println("32Bit shar");
      display.println(data.completedShares.c_str());
      display.println("Temp.");
      display.println(data.temp.c_str());
      display.println("Tot. MH.");
      display.println(data.totalMHashes.c_str());
      display.println(data.timeMining.c_str());
      
      display.updateWindow(0,0,display.width(), display.height());
      delay(1000);
  }
}

void lilygot5v231display_BTCprice(unsigned long mElapsed)
{
  Serial.printf("Other Screen");
  
  unsigned long currentMillis = millis();
  if (hasChangedScreen || previousMillisDisplay == 0 || currentMillis - previousMillisDisplay >= (1000*60))
    { // 0.5sec blink
      hasChangedScreen = false;
      previousMillisDisplay = currentMillis;
      clock_data data = getClockData(mElapsed);
      
      Serial.printf(">>> Price: %s\n", data.btcPrice.c_str());
      
      //display.eraseDisplay();
      display.fillScreen(GxEPD_WHITE);
      //display.eraseDisplay();
      display.setFont(&FreeMonoBold9pt7b);
      display.setRotation(0);
      display.setTextColor(GxEPD_BLACK);
      display.setCursor(0, 10);
      display.print("Price ");
      display.print("\n");
      display.print(data.btcPrice.c_str());
      display.print("\n");

      display.updateWindow(0,0,display.width(), display.height());
      delay(1000);
    }
  
  
  /*if (hasChangedScreen) tft.pushImage(0, 0, priceScreenWidth, priceScreenHeight, priceScreen);
  printPoolData();
  hasChangedScreen = false;

  clock_data data = getClockData(mElapsed);

 // Create background sprite to print data at once
  createBackgroundSprite(270,36);

  // Print background screen
  background.pushImage(0, -130, priceScreenWidth, priceScreenHeight, priceScreen);
  // Hashrate
  render.setFontSize(25);
  render.setFontColor(TFT_BLACK);
  render.rdrawString(data.currentHashRate.c_str(), 95, 0, TFT_BLACK);

  // Print BlockHeight
  render.setFontSize(18);
  render.rdrawString(data.blockHeight.c_str(), 254, 9, TFT_WHITE);

  // Push prepared background to screen
  background.pushSprite(0, 130);
  // Delete sprite to free the memory heap
  background.deleteSprite(); 

  createBackgroundSprite(169,105);
  // Print background screen
  background.pushImage(-130, -3, priceScreenWidth, priceScreenHeight, priceScreen);
  
  // Print Hour
  background.setFreeFont(FSSB9);
  background.setTextSize(1);
  background.setTextDatum(TL_DATUM);
  background.setTextColor(TFT_BLACK);
  background.drawString(data.currentTime.c_str(), 202-130, 0, GFXFF);
 
  // Print BTC Price
  background.setFreeFont(FF24);
  background.setTextDatum(TL_DATUM);
  background.setTextSize(1);
  background.setTextColor(0xDEDB, TFT_BLACK);
  background.drawString(data.btcPrice.c_str(), 0, 50, GFXFF);
 
  // Push prepared background to screen
  background.pushSprite(130, 3);

  // Delete sprite to free the memory heap
  background.deleteSprite();   

  Serial.printf(">>> Completed %s share(s), %s Khashes, avg. hashrate %s KH/s\n",
                data.completedShares.c_str(), data.totalKHashes.c_str(), data.currentHashRate.c_str());

  #ifdef DEBUG_MEMORY
  // Print heap
  printheap();
  #endif*/
}

void lilygot5v231display_LoadingScreen(void)
{
  Serial.println("Initializing...");
  display.println("Hello World");
  
}

void lilygot5v231display_SetupScreen(void)
{
  Serial.println("Setup...");
}

// Variables para controlar el parpadeo con millis()
unsigned long previousMillis = 0;
char currentScreen = 0;

void lilygot5v231display_DoLedStuff(unsigned long frame)
{
  unsigned long currentMillis = millis();

  if (currentScreen != currentDisplayDriver->current_cyclic_screen) hasChangedScreen ^= true;
    currentScreen = currentDisplayDriver->current_cyclic_screen;

  if (!ledOn)
  {
    
    return;
  }

  switch (mMonitor.NerdStatus)
  {

  case NM_waitingConfig:
    
    break;

  case NM_Connecting:
    if (currentMillis - previousMillis >= 500)
    { // 0.5sec blink
      previousMillis = currentMillis;
      //digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Cambia el estado del LED
    }
    break;

  case NM_hashing:
    if (currentMillis - previousMillis >= 100)
    { // 0.1sec blink
      previousMillis = currentMillis;
      //digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Cambia el estado del LED
    }
    break;
  }
}

void lilygot5v231display_AnimateCurrentScreen(unsigned long frame)
{
}

CyclicScreenFunction lilygot5v231display_CyclicScreens[] = {lilygot5v231display_MinerScreen,lilygot5v231display_BTCprice};

DisplayDriver lilygoT5v231DisplayDriver = {
    lilygot5v231display_Init,
    lilygot5v231display_AlternateScreenState,
    lilygot5v231display_AlternateRotation,
    lilygot5v231display_LoadingScreen,
    lilygot5v231display_SetupScreen,
    lilygot5v231display_CyclicScreens,
    lilygot5v231display_AnimateCurrentScreen,
    lilygot5v231display_DoLedStuff,
    SCREENS_ARRAY_SIZE(lilygot5v231display_CyclicScreens),
    0,
    display.width(),
    display.height(),
};
#endif
