#include <Adafruit_SSD1306.h>                                                 //Include the required libraries
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <NTPClient.h> 
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128                                                      //Define the OLED display width and height
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1                                                     // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C                                                   //I2C address for display
#define upLED 13
#define downLED 12
Adafruit_SSD1306 display (SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);    //Create the display object

const char* ssid = "dd-wrt";                                            //Set your WiFi network name and password
const char* password = "buffaloroam";

WiFiClient client;                                                            //Create a new WiFi client
HTTPClient http;

const char* url = "https://api.coingecko.com/api/v3/simple/price?vs_currencies=usd&include_24hr_change=true&ids=";
const char* coins[] = {"ethereum","ripple", "dogecoin","bitcoin"};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);                                                       //Start the serial monitor
  
  pinMode(upLED, OUTPUT);                                                     //Define the LED pin outputs
  pinMode(downLED, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))                   //Connect to the display
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();                                                     //Clear the display
  display.setTextColor(SSD1306_WHITE);                                        //Set the text colour to white
  //display.drawBitmap(0, 0, bitcoinLogo, 128, 64, WHITE);                             //Display bitmap from array
  display.display();
  delay(2000);

  display.clearDisplay();                                                     //Clear the display
  display.setTextSize(1);                                                     //Set display parameters
  display.setTextColor(WHITE);
  display.println("Connecting to WiFi...");
  display.display();

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)                                        //Connect to the WiFi network
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  display.println("Connected to: ");                                           //Display message once connected
  display.print(ssid);
  display.display();
  delay(1500);
  display.clearDisplay();
  display.display();
}

void display_coin_data(String coin) {
  digitalWrite(upLED, LOW);
  digitalWrite(downLED, LOW);
  Serial.println("Processing coin " + coin);
  Serial.print("Connecting to ");
  Serial.println(url + coin);
  
  http.begin(url + coin);
  int httpCode = http.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);
  
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, http.getString());
  if (error) {
    Serial.println("deserializeJson Failed " + String(httpCode));
    Serial.println(error.f_str());
    //display.clearDisplay();
    //display.setTextSize(1);
    //printCenter("Unknown coin " + coin, 0, 25);
    //display.display();
    http.end();
    delay(5000);
    return;
  }
  
  String value = doc[coin]["usd"].as<String>();
  display.clearDisplay();                                                               //Clear the OLED display
  display.setTextSize(1);
  printCenter(coin + "/USD", 0, 0);
  display.setTextSize(2);
  printCenter("$" + value, 0, 25);
  display.setTextSize(1);
  auto change = doc[coin]["usd_24h_change"].as<String>();
  printCenter("24hr change: $" + change, 0, 55);
  display.display();
  if (doc[coin]["usd_24h_change"] > 0) {
    digitalWrite(upLED, HIGH);
    digitalWrite(downLED, LOW);
  } else {
    digitalWrite(upLED, LOW);
    digitalWrite(downLED, HIGH);
  }
  http.end();
}

void loop() {
  // put your main code here, to run repeatedly:
  esp_sleep_enable_timer_wakeup(15000000);
  for (int i=0; i < (sizeof(coins)/sizeof(coins[0])); i++) {
    display_coin_data(coins[i]);
    delay(10000);
  }
}

void printCenter(const String buf, int x, int y)                          //Function to centre the current price in the display width
{
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(buf, x, y, &x1, &y1, &w, &h);                     //Calculate string width
  display.setCursor((x - w / 2) + (128 / 2), y);                          //Set cursor to print string in centre
  display.print(buf);                                                     //Display string
}
