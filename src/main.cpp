#include <Arduino.h>
// remove default and update FTF lib config to -> TTGO T-Display ST7789V SPI bus TFT
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
// https://github.com/LennartHennigs/Button2
#include "Button2.h"

// Image about
#include "crypto.h"

enum DisplayMode { MONEY, COIN, PRICE, GRAPH, ABOUT };
enum DisplayMode displayMode = MONEY;

// TFT screen size
const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 135;

// GPIOs
const int BUTTON_UP = 35;
const int BUTTON_DOWN = 0;

// WiFi
const char* ssid = "E=mc2";
const char* password = "** your wifi password ***";

// Config Coins
const String ERG_ADDRESS = "9hNSS97pfX14L96cFE2CwaAMWxtu1Zq7thcEJgnVtZyGeJnq5g6";
const String ETH_ADDRESS = "0x257fFb03e617f9c9ebB2e719b4775F85F12c6567";
const String BTC_ADDRESS = "1DeuttaiHr4aGswVszizMYYzDUAVmex63q";
const String ETHERSCAN_API_KEY = "*** your etherscan api key ***";
const String CRYPTOCOMPARE_API_KEY = "*** your cryptocompare api key ***";
const String DISPLAY_MONEY = "EUR";

TFT_eSPI tft = TFT_eSPI(SCREEN_HEIGHT, SCREEN_WIDTH);

Button2 buttonUp, buttonDown;

HTTPClient httpEth, httpErg, httpBtc, httpPrice;

const int REFRESH_TIME_IN_MILLI = 1 * 60 * 1000;
unsigned long lastRefresh = millis();

enum Coin { ETH , ERG , BTC, MAX_COIN };

String coinSymbol[MAX_COIN]  = { "ETH" , "ERG" , "BTC" };
int lineDisplay[MAX_COIN] = { 10, 40, 70 };
int histoColor[MAX_COIN] = { TFT_CYAN, TFT_YELLOW, TFT_MAGENTA };

const int HISTO_SIZE = SCREEN_WIDTH / 2;
double myCrypto[MAX_COIN];
double priceCurrent[MAX_COIN];
double priceOld[MAX_COIN];
double histoPrice[MAX_COIN][HISTO_SIZE];
int histoIndex = 0;

void waitScreen(String text) {
  tft.fillScreen(TFT_BLACK);
  tft.fillSmoothRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);
  tft.drawString(text, 5, (tft.height() / 2) - (tft.fontHeight()) / 2);
}

void initWifi() {
  waitScreen("Connect to wifi...");

	// Connexion au Wifi
	Serial.print("Connecting to WiFi...");
	WiFi.begin(ssid, password);
		while (WiFi.status() != WL_CONNECTED) {
		delay(500);
	}
	Serial.println("ok !");
	Serial.printf("SSID: %s, IP: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void iniTFT() {
  Serial.println("Init TFT");
  tft.init();
  tft.setRotation(1);
  tft.setTextSize(2);
  tft.setTextDatum(TL_DATUM);
}

void checkWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    initWifi();
  }
}

double getMyEthereum() {
  String url = "https://api.etherscan.io/api?module=account&action=balance&address=" + ETH_ADDRESS + "&tag=latest&apikey=" + ETHERSCAN_API_KEY;
	httpEth.begin(url);
	httpEth.addHeader("Accept", "application/json");
	int httpCode = httpEth.GET();
 
	if (httpCode == 200) {
		String payload = httpEth.getString();
    DynamicJsonDocument doc(100);
		deserializeJson(doc, payload);
		String result = doc["result"];
    return ("0.0" + result).toDouble();
  } else {
    Serial.println("ETH error :" + httpCode);
    return 0;
  }
}

double getMyErgo() {
  String url = "https://api.ergoplatform.com/api/v1/addresses/" +  ERG_ADDRESS + "/balance/confirmed";
	httpErg.begin(url);
	httpErg.addHeader("Accept", "application/json");
	int httpCode = httpErg.GET();
 
	if (httpCode == 200) {
		String payload = httpErg.getString();
    DynamicJsonDocument doc(100);
		deserializeJson(doc, payload);
		String result = doc["nanoErgs"];
    return result.toDouble() / 1000000000;
  } else {
    Serial.println("ERG error :" + httpCode);
    return 0;
  }
}

double getMyBitcoin() {
  String url = "https://blockchain.info/balance?active=" + BTC_ADDRESS;
 	httpBtc.begin(url);
	httpBtc.addHeader("Accept", "application/json");
	int httpCode = httpBtc.GET();
 
	if (httpCode == 200) {
		String payload = httpBtc.getString();
    DynamicJsonDocument doc(500);
		deserializeJson(doc, payload);
		String result = doc[BTC_ADDRESS]["final_balance"];
    return result.toDouble() / 100000000;
  } else {
    Serial.println("getBTC error :" + httpCode);
    return 0;
  }
}

double getPrice(String crypto) {
  String url = "https://min-api.cryptocompare.com/data/price?fsym=" + crypto + "&tsyms=" + DISPLAY_MONEY + "&api_key=" + CRYPTOCOMPARE_API_KEY;
	httpPrice.begin(url);
  httpPrice.addHeader("Accept", "application/json");
	int httpCode = httpPrice.GET();
 
  double value = -1;
	if (httpCode == 200) {
		String payload = httpPrice.getString();
		// Serial.println(payload);
    DynamicJsonDocument doc(100);
		deserializeJson(doc, payload);
    String result = doc[DISPLAY_MONEY];
    value = result.toDouble();
  } else {
    Serial.println("getPrice error :" + httpCode);
  }
  Serial.println(crypto + " = " + value);
  return value;
}

void getPrices() {
  checkWifi();
  waitScreen("Loading prices...");

  for (size_t i = 0; i < MAX_COIN ; i++) {
    Coin coin = (Coin)i;
    priceOld[coin] = priceCurrent[coin];
    priceCurrent[coin] = getPrice(coinSymbol[coin]);
    histoPrice[coin][histoIndex] = priceCurrent[coin];
  }
  
  histoIndex++;
  if (histoIndex >= HISTO_SIZE) {
    histoIndex = 0;
  }
}

void getMyCrypto() {
  checkWifi();
  waitScreen("Loading crypto...");
  
  myCrypto[ETH] = getMyEthereum();
  myCrypto[ERG] = getMyErgo();
  myCrypto[BTC] = getMyBitcoin();
}

int getPriceColor(double newPrice, double oldPrice) {
  if (oldPrice == 0.0) {
    return TFT_DARKGREY;
  } else if (newPrice > oldPrice) {
    return TFT_GREEN;
  } else if (newPrice < oldPrice) {
    return TFT_RED;
  } else {
    return TFT_WHITE;
  }
}

void displayMoney() {
  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, TFT_ORANGE);

  for (size_t i = 0; i < MAX_COIN ; i++) {
    Coin coin = (Coin)i;

    tft.setTextColor(TFT_GOLD);
    tft.drawString(coinSymbol[coin] + " :", 5, lineDisplay[coin]);

    double value = myCrypto[coin] * priceCurrent[coin];
    tft.setTextColor(getPriceColor(priceCurrent[coin], priceOld[coin]));
    tft.drawFloat(value, 5, 120, lineDisplay[coin]);
  }
}

void displayCoin() {
  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, TFT_ORANGE);

  for (size_t i = 0; i < MAX_COIN ; i++) {
    Coin coin = (Coin)i;

    tft.setTextColor(TFT_WHITE);
    tft.drawString(coinSymbol[coin] + " :", 5, lineDisplay[coin]);
    
    tft.setTextColor(TFT_BLUE);
    tft.drawFloat(myCrypto[coin], 5, 120, lineDisplay[coin]);
  }
}

void displayPrice() {
  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, TFT_ORANGE);

  for (size_t i = 0; i < MAX_COIN ; i++) {
    Coin coin = (Coin)i;

    tft.setTextColor(TFT_VIOLET);
    tft.drawString(coinSymbol[coin] + " :", 5, lineDisplay[coin]);
    
    tft.setTextColor(TFT_MAGENTA);
    tft.drawFloat(priceCurrent[coin], 5, 120, lineDisplay[coin]);
  }
}

void displayGraph() {
  tft.fillScreen(TFT_BLACK);

  int maxHistoHeight = SCREEN_HEIGHT / MAX_COIN;

  for (size_t c = 0; c < MAX_COIN ; c++) {
    Coin coin = (Coin)c;
    int index = histoIndex;

    double min[MAX_COIN] = { 99999, 99999, 99999 };
    double max[MAX_COIN] = { -99999, -99999, -99999 };

    for (size_t i = 0; i < HISTO_SIZE; i++) {
      double v = histoPrice[coin][i];

      if (v != 0) {
        if (v > max[coin]) {
          max[coin] = v;
        }
        if (v < min[coin]) {
          min[coin] = v;
        }
      }
    }
    min[coin] = min[coin] - 1;  // (min[coin] / 3);
    max[coin] = max[coin] + 1;  // (max[coin] / 3);

    tft.drawLine(0, maxHistoHeight * coin, SCREEN_WIDTH, maxHistoHeight * coin, TFT_LIGHTGREY);

    int xp = -1;
    int yp = -1;

    for (size_t xc = 0; xc < HISTO_SIZE; xc++) {

      double value = histoPrice[coin][index];

      if (value != 0) {
        int yc = map(value, min[coin], max[coin], 0, maxHistoHeight);
        
        int y = yc + (maxHistoHeight * coin);
        int x = xc * 2;

        if (xp == -1 && yp == -1) {
          xp = x;
          yp = y;
        }

        tft.drawLine(x, y, xp, yp, histoColor[coin]);

        xp = x;
        yp = y;
      }

      index++;
      if (index >= HISTO_SIZE) {
        index = 0;
      }
    }
  }
}

void displayAbout() {
  tft.fillScreen(TFT_BLACK);

  tft.setSwapBytes(true);
  tft.pushImage(0, 0, 240, 120, crypto);

  tft.setTextColor(TFT_RED);
  tft.drawString("(c) Deuttai", 5, SCREEN_HEIGHT - tft.fontHeight());
}

void refreshScreen() {
  switch (displayMode) {
    case MONEY:
      displayMoney();
      break;
    case COIN:
      displayCoin();
      break;
    case PRICE:
      displayPrice();
      break;
    case GRAPH:
      displayGraph();
      break;
    case ABOUT:
      displayAbout();
      break;
  }
}

void changeMode() {
  switch (displayMode) {
    case MONEY:
      displayMode = COIN;
      break;
    case COIN:
      displayMode = PRICE;
      break;
    case PRICE:
      displayMode = GRAPH;
      break;
    case GRAPH:
      displayMode = ABOUT;
      break;
    case ABOUT:
      displayMode = MONEY;
      break;
  }
}

void buttonPressedHandler(Button2& btn) {
    switch (btn.getPin()) {
      case BUTTON_DOWN:
        getPrices();
        refreshScreen();
        break;
      case BUTTON_UP:
        changeMode();
        refreshScreen();
        break;
      default:
        break;
    }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(20);
  }

  iniTFT();

  initWifi();

  getMyCrypto();

  getPrices();

  refreshScreen();

  buttonUp.begin(BUTTON_UP);
  buttonUp.setPressedHandler(buttonPressedHandler);
  
  buttonDown.begin(BUTTON_DOWN);
  buttonDown.setPressedHandler(buttonPressedHandler);
}

void loop() {
  while (true) {
    buttonUp.loop();
    buttonDown.loop();

    delay(100);

    if (millis() - lastRefresh > REFRESH_TIME_IN_MILLI) {
      lastRefresh = millis();
      getPrices();
      refreshScreen();
    }
  }
}
