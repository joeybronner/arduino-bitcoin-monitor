#include <SPI.h>
#include <cp437font.h>
#include <FC16.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <string.h>

// WiFi credentials
char ssid[] = "freebox_ALMEIDA";
char pass[] = "ziugtiojciech6onuecu";

// HTTP
HTTPClient http;

// Push button
const int wpsButton = D2;

// Buzzer
const int buzzPin = D1;
int frequency = 1000;
int timeOn = 1000;
int timeOff = 1000;
int numGood = 3;
int good[] = {261, 330, 440};

// LED Matrix
const int csPin = D8;
const int displayCount = 4;
const int scrollDelay = 100;

FC16 display = FC16(csPin, displayCount);
int buttonState = 0;
int firstLoop = true;

long interval = 15000;
long previousMillis = 0;

String strBitcoinValueAndSymbol = "";

/*
   --- setup ---
*/
void setup()
{
  // Debug console
  Serial.begin(9600);
  delay(200);
  Serial.println("");
  Serial.println("NodeMCU Started");

  // Button
  pinMode(wpsButton, INPUT);

  // Welcome
  setupMatrix();
  delay(1000);
  
  // Set up WiFi
  setupConnection();

  
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8 * displayCount; col++) {
      // Set LED state on/off for given row, column
      display.setLed(row, col, true);

      // Wait for a while
      delay(15);
    }
  }
  
  delay(200);
  for (int i = 0; i < numGood; i++)
  {
    tone(buzzPin, good[i]);
    delay(200);
  }
  noTone(buzzPin);
  delay(200);

  drawBitcoinLogo(false);
  
  delay(3000);
  display.clearDisplay();
  delay(300);
  display.setText("Bitcoin");
  display.update();
  delay(3000);
  display.clearDisplay();
  delay(300);
}

/*
   --- loop ---
*/
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[loop] no wifi");
    delay(1500);
    setupConnection();
  } else {
    buttonState = digitalRead(wpsButton);
    delay(50);
    if (buttonState == 1 || firstLoop) {
      firstLoop = false;
      previousMillis = millis();
      // HTTP request
      http.begin("http://blockchain.info/tobtc?currency=EUR&value=1");
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK) {
        Serial.print("HTTP response code ");
        Serial.println(httpCode);
        String response = http.getString();
        Serial.println(response);
        float bitcoinValue = 1 / response.toFloat();
        char str[20];
        String strBitcoinValue = floatToString(str, bitcoinValue, 0, 5);
        strBitcoinValueAndSymbol = strBitcoinValue + " EUR";
        display.clearDisplay();
        display.setText(strToChar(strBitcoinValueAndSymbol));
        while (!display.update()) {
          delay(100);
        }
        display.clearDisplay();
      } else {
        Serial.println("Error in HTTP request");
      }
      http.end();
    }
  }
}

void drawBitcoinLogo(bool ledState) {
  display.setLed(0, 15, ledState);
  display.setLed(1, 15, ledState);
  display.setLed(2, 15, ledState);
  display.setLed(3, 15, ledState);
  display.setLed(4, 15, ledState);
  display.setLed(5, 15, ledState);
  display.setLed(6, 15, ledState);
  display.setLed(7, 15, ledState);

  display.setLed(0, 17, ledState);
  display.setLed(1, 17, ledState);
  display.setLed(2, 17, ledState);
  display.setLed(3, 17, ledState);
  display.setLed(4, 17, ledState);
  display.setLed(5, 17, ledState);
  display.setLed(6, 17, ledState);
  display.setLed(7, 15, ledState);

  display.setLed(1, 14, ledState);
  display.setLed(1, 16, ledState);
  display.setLed(1, 18, ledState);
  display.setLed(1, 19, ledState);
  display.setLed(2, 20, ledState);
  display.setLed(3, 21, ledState);
  
  display.setLed(4, 21, ledState);
  display.setLed(4, 20, ledState);
  display.setLed(4, 19, ledState);
  display.setLed(4, 18, ledState);
  display.setLed(4, 16, ledState);
  
  display.setLed(5, 21, ledState);
  display.setLed(6, 20, ledState);
  display.setLed(7, 19, ledState);
  display.setLed(7, 18, ledState);
  display.setLed(7, 16, ledState);
  display.setLed(7, 14, ledState); 
}

void setupMatrix() {
  display.shutdown(false);  // turn on display
  display.setIntensity(8);  // 0 to 15
  display.clearDisplay();   // turn all LED off
  delay(200);
}

void setupConnection() {
  Serial.println("start Wifi connection");
  boolean wifiConnected = connectWifi();

  if (wifiConnected) {
    Serial.println(String("Wifi connected. IP address: ") + WiFi.localIP());
    display.clearDisplay();
    display.setText("Wifi Connected");
    while (!display.update()) {
      delay(100);
    }
    display.clearDisplay();
  } else {
    Serial.println("Wifi connection failed");
  }
}

boolean connectWifi() {
  int timeout = millis();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    if (millis() > timeout + 5000) {
      return false;
    }
  }
  return true;
}

/*
   UTILS -------------------------------
*/

char* strToChar(String s) {
  unsigned int bufSize = s.length() + 1; //String length + null terminator
  char* ret = new char[bufSize];
  s.toCharArray(ret, bufSize);
  return ret;
}

char * floatToString(char * outstr, double val, byte precision, byte widthp) {
  char temp[16]; //increase this if you need more digits than 15
  byte i;
  temp[0] = '\0';
  outstr[0] = '\0';

  if (val < 0.0) {
    strcpy(outstr, "-\0"); //print "-" sign
    val *= -1;
  }

  if ( precision == 0) {
    strcat(outstr, ltoa(round(val), temp, 10)); //prints the int part
  }
  else {
    unsigned long frac, mult = 1;
    byte padding = precision - 1;

    while (precision--)
      mult *= 10;

    val += 0.5 / (float)mult;    // compute rounding factor

    strcat(outstr, ltoa(floor(val), temp, 10)); //prints the integer part without rounding
    strcat(outstr, ".\0"); // print the decimal point

    frac = (val - floor(val)) * mult;

    unsigned long frac1 = frac;

    while (frac1 /= 10)
      padding--;

    while (padding--)
      strcat(outstr, "0\0");   // print padding zeros

    strcat(outstr, ltoa(frac, temp, 10)); // print fraction part
  }

  // generate width space padding
  if ((widthp != 0) && (widthp >= strlen(outstr))) {
    byte J = 0;
    J = widthp - strlen(outstr);

    for (i = 0; i < J; i++) {
      temp[i] = ' ';
    }

    temp[i++] = '\0';
    strcat(temp, outstr);
    strcpy(outstr, temp);
  }

  return outstr;
}
