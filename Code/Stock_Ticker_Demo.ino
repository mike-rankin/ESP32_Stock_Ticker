//Stock ticker demo sketch

#include <TFT_eSPI.h> 
#include <WiFiUdp.h> 
#include <HTTPClient.h>
#include <ArduinoJson.h> 
#include "Splash.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>  
#include "Wifi_Icons.h"

#include <SensirionI2CScd4x.h>
#include <BH1750.h>
#include "Adafruit_SGP40.h"
#include <Adafruit_NeoPixel.h>
#include "DFRobot_SHT20.h"

TFT_eSPI tft = TFT_eSPI(); 
SensirionI2CScd4x scd4x;
BH1750 lightMeter;
Adafruit_SGP40 sgp;
Adafruit_NeoPixel pixels(1, 7, NEO_GRB + NEO_KHZ800);
DFRobot_SHT20 sht20(&Wire, SHT20_I2C_ADDR);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

uint32_t targetTime = 0;      
const long interval = 60000;  //Update every minute

const char* ssid     = "SSID";      
const char* password = "PASSWORD";         

#define LCD_BACKLIGHT 21

#define TFT_LIGHTGREY 0xD69A
#define TFT_DARKGREY  0x4208 
#define TFT_GREEN     0x07E0
#define TFT_LightBlue 0x867D
#define TFT_Orange    0xFDA0
#define TFT_SILVER    0xC618 
#define TFT_PURPLE    0x780F 

String payload="";
double current=0;

unsigned long previousTime = 0;
const unsigned long eventInterval=300000;     //300000 is 5 minutes
//const unsigned long eventInterval=600000;   //600000 is 10 minutes

StaticJsonDocument<6000> doc;

//int deb=0;
//int brightnes[5]={40,80,120,160,200};
//int b=1;

void setup() 
{
  Serial.begin(115200);
  tft.init();
  Wire.begin(13,4);  //Pico mini
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);
  pinMode(LCD_BACKLIGHT, OUTPUT);
  connect_to_wifi();

  uint16_t error;
  char errorMessage[256];
  scd4x.begin(Wire);
  sgp.begin();
  sht20.initSHT20();
  pixels.begin();
  lightMeter.begin(); 

  reset_screen();
  tft.pushImage(0,0,320,240,Splash);
  delay(3000);
  reset_screen();

  main_page();
  stock_page("BTC","ETH","AAPL","NVDA","MSFT","SPOT","AMZN","BAC","TSLA");     //Edit your preferred stocks here

  timeClient.begin();
  timeClient.setTimeOffset(-10800);  //My timezone

  error = scd4x.stopPeriodicMeasurement();
  if (error) 
  {
   Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
  }

  //Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) 
  {
   Serial.print("Error trying to execute startPeriodicMeasurement(): ");
   errorToString(error, errorMessage, 256);
   Serial.println(errorMessage);
  }

  Serial.println("Waiting for first measurement... (5 sec)");
  blink_led();
}

void loop() 
{
 unsigned long currentTime = millis(); 
 timeClient.update();

 time_t epochTime = timeClient.getEpochTime();
 struct tm *ptm = gmtime ((time_t *)&epochTime);   //Get a time structure

 int monthDay = ptm->tm_mday;
 int currentMonth = ptm->tm_mon+1;
 String currentMonthName = months[currentMonth-1];
 int currentYear = ptm->tm_year+1900;
 String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);

 tft.drawLine( 30, 187, 280, 187, TFT_WHITE);     

 tft.setTextColor(TFT_LightBlue,TFT_BLACK);
 tft.drawString(String(currentDate),30,168,2);
 tft.drawString(String(timeClient.getFormattedTime()),175,168,2);

 digitalWrite(LCD_BACKLIGHT, HIGH);
 Serial.println("Waiting...");
 delay(100);
 main_page();
 read_co2();
 read_light();
 read_gas();
 read_front_temp();
 show_RSSI();

 if (currentTime - previousTime >= eventInterval) 
 {
  Serial.println("Refresh Data");
  reset_screen();
  stock_page("BTC","ETH","AAPL","NVDA","MSFT","SPOT","AMZN","BAC","TSLA");  //Edit your stock names here
  previousTime = currentTime;
  timeClient.update();
 }
}


void connect_to_wifi()
{
 WiFi.begin(ssid, password);
 tft.setTextColor(TFT_GREEN,TFT_BLACK); 
 tft.setTextSize(2); 
 tft.setCursor (45,100); 
 tft.print("Connecting to Wifi");
 delay(2000);

 while (WiFi.status() != WL_CONNECTED)
 {
  delay(150);
  tft.print(".");
  delay(150);
 }
 tft.fillScreen(TFT_BLACK);
 tft.setTextSize(2); 
 tft.setCursor (90,100);  
 tft.print("Connected");
 tft.setCursor (95,130); 
 tft.print("IP address: ");
 tft.setCursor (90,160); 
 tft.print(WiFi.localIP());
 delay(4000);
 tft.fillScreen(TFT_BLACK);
}

void show_RSSI()
{
 tft.drawString(String(WiFi.RSSI()),242,168,2);
 tft.drawString("dBm",265,168,2);   
}


void read_light()
{
 int lux = lightMeter.readLightLevel();
 tft.setTextSize(1);  
 tft.setTextColor(TFT_LightBlue,TFT_BLACK);
 tft.drawString("Light : ",175,188,2);   
 tft.drawString(String(lux),235,188,2);
}

void read_gas()
{
 uint16_t raw;
 raw = sgp.measureRaw();
 tft.setTextSize(1);  
 tft.setTextColor(TFT_LightBlue,TFT_BLACK);
 tft.drawString("Gas : ",175,203,2);
 tft.drawString(String(raw),235,203,2);
}

void read_front_temp()
{
 float temp = sht20.readTemperature();
 tft.setTextSize(1);  
 tft.setTextColor(TFT_LightBlue,TFT_BLACK);
 tft.drawString("Temp F : ",175,218,2);
 tft.drawString(String(temp,1),235,218,2);
}


void read_co2()
{
  uint16_t error;
  char errorMessage[256];

  delay(100);

  // Read Measurement
  uint16_t co2 = 0;
  float temperature = 0.0f;
  float humidity = 0.0f;
  bool isDataReady = false;
  error = scd4x.getDataReadyFlag(isDataReady);
  if (error) 
  {
    Serial.print("Error trying to execute getDataReadyFlag(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
    return;
  }
  if (!isDataReady) 
  {
    return;
  }
  error = scd4x.readMeasurement(co2, temperature, humidity);
  if (error)
  {
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else if (co2 == 0)
   {
    Serial.println("Invalid sample detected, skipping.");
   } else 
   {
    Serial.print("Co2:");
    Serial.print(co2);
    Serial.print("\t");
    Serial.print("Temperature:");
    Serial.print(temperature);
    Serial.print("\t");
    Serial.print("Humidity:");
    Serial.println(humidity);

    tft.setTextSize(1);  
    tft.setTextColor(TFT_LightBlue,TFT_BLACK);
    tft.drawString("CO2 : ",30,188,2);     
    tft.drawString(String(co2),105,188,2);
    tft.drawString("Temp B : ",30,203,2);
    tft.drawString(String(temperature),105,203,2);
    tft.drawString("Humidity : ",30,218,2);      
    tft.drawString(String(humidity),105,218,2);  
  }
}



 void main_page()
 {
  tft.setTextSize(1);  
  tft.setTextColor(TFT_LightBlue,TFT_BLACK);
  tft.drawString("Symbol",28,0,2);  
  tft.drawString("Price",125,0,2); 
  tft.drawString("Gains/Losses",193,0,2);

  tft.drawLine( 20, 20, 290, 20, TFT_WHITE);     
  tft.drawRoundRect(18, 163, 280, 77, 6, TFT_WHITE);      
}
  
void reset_screen()
{
 tft.fillScreen(TFT_BLACK);
} 

void stock_page(String s1,String s2,String s3,String s4,String s5,String s6,String s7,String s8,String s9)
{
 read_price(40,23,s1);   
 read_price(40,38,s2);  
 read_price(40,53,s3);
 read_price(40,68,s4);
 read_price(40,83,s5);
 read_price(40,98,s6);
 read_price(40,113,s7);
 read_price(40,128,s8);
 read_price(40,143,s9); 
} 

void read_price(int x_i,int y_i,String stock_name)
{ 
 float currentPrice;
 float differenceInPrice; 
 float percentchange; 
 
 HTTPClient http; 
 http.begin("https://finnhub.io/api/v1/quote?symbol="+stock_name+"&token=YOUR-TOKEN-HERE");   //Edit your key here
    
 int httpCode = http.GET();  
 if (httpCode > 0)
 { 
  payload = http.getString();
  char inp[payload.length()];
  payload.toCharArray(inp,payload.length());
  deserializeJson(doc,inp);

  //c = Current price, d = Change, dp = Percent change, pc = Previous close price
  String v=doc["c"];    
  String c=doc["dp"];  
  current=v.toDouble(); 
  percentchange=c.toDouble();
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_Orange,TFT_BLACK);
  tft.drawString(stock_name,x_i,y_i,2);  
  tft.setTextColor(TFT_LIGHTGREY,TFT_BLACK);                       
  tft.drawString(" ",50+x_i,y_i,2);                     
  tft.drawString(String(current),85+x_i,y_i,2);         
  
  if(percentchange<0.0)
  {
   tft.setTextColor(TFT_RED);
   tft.drawString(" ",130+x_i,y_i,2);                     
   tft.drawString(String(percentchange),165+x_i,y_i,2);   
   tft.drawString("  %",195+x_i,y_i,2);                   
  } 
  
  else
  {
   tft.setTextColor(TFT_GREEN);
   tft.drawString(" ",130+x_i,y_i,2);                      
   tft.drawString(String(percentchange),170+x_i,y_i,2);  
   tft.drawString(" %",200+x_i,y_i,2);                   
  }
 }
}

void blink_led()
{
 pixels.clear(); 

 pixels.setPixelColor(0, pixels.Color(25, 0, 0));
 pixels.show();  
 delay(100); 
 pixels.clear();
  
 pixels.setPixelColor(0, pixels.Color(0, 25, 0));
 pixels.show();  
 delay(100);
 pixels.clear();

 pixels.setPixelColor(0, pixels.Color(0, 0, 25));
 pixels.show();  
 delay(100); 
 pixels.clear(); 

 pixels.setPixelColor(0, pixels.Color(0, 0, 0));
 pixels.show();  
 delay(100); 
}


  
