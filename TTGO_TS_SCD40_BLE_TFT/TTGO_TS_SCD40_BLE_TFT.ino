// This code is based on Sensirion's Arduino Snippets
// Check https://github.com/Sensirion/arduino-snippets for the most recent version.

#include <TFT_eSPI.h>
#include "DataProvider.h"
#include "NimBLELibraryWrapper.h"
#include <SensirionI2CScd4x.h>
#include <Wire.h>
#include "NotoSansMonoSCB20.h"
#include "NotoSansBold36.h"
#include "Final_Frontier_28.h"


#define White     0xFFFF
#define Green     0x07E0
#define Black     0x0000
#define Yellow    0xFFE0
#define DarkGrey  0x4208 
#define Brown     0x9A60
#define Orange    0xFDA0
#define LightBlue 0x867D
#define Red       0xF800

// GadgetBle workflow
static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 5000;
NimBLELibraryWrapper lib;
DataProvider provider(lib, DataType::T_RH_CO2);

SensirionI2CScd4x scd4x;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// added by Kassim
#define notActiveColor    DarkGrey // when bar not lit
#define greenColor        Green   // good CO2 value
#define yellowColor       Yellow  // not so good CO2 value
#define redColor          Red     // worse CO2 value

const byte totalSpriteBars = 12; // total bars

const byte cicle = 0; // total bars


int colorArray[totalSpriteBars + 10] = {notActiveColor};
int heightArray[totalSpriteBars + 10] = {0};

const byte barMaxHeight = 30,
           barMinHeight = 2,
           barWidth = 4,
           barSpacing = 10,
           startX = 8,
           startY = 80,  //160
           none = 78;

void drawCO2_Text(uint16_t co2){
  unsigned long value;
  String co2_Value;

  //sgp.IAQmeasure();  //Grab latest sample
  value = co2;
  
  //if (value > 999) value = 999;
  int sum = 0;
  //co2_Value = value;

  //Shows down the updates
  
  for(int i=0; i<100; i++){
   sum = sum + value;
   delay(2);
  }
  int result = sum/100;
  co2_Value = result;
  sprite.fillRect(15, 20, 108, 40, Black); //Black out old CO2 value
  sprite.setTextColor(White, Black);
  sprite.loadFont(NotoSansBold36);
  if (value > 999){
  sprite.drawString(co2_Value, 15, 20);
  //sprite.drawString(co2_Value, 75, 50, 6);
  sprite.unloadFont(); 
  sprite.drawString("ppm", 100, 35, 2);    
  }
  else {
  sprite.drawString(co2_Value, 25, 20);
  //sprite.drawString(co2_Value, 75, 50, 6);
  sprite.unloadFont(); 
  sprite.drawString("ppm", 90, 35, 2);        
  }

  sprite.setTextColor(LightBlue, Black);
  sprite.drawString("CO2", 55, 60, 2);
  sprite.pushSprite(0, 0);

  /*
  float get_battery_voltage(){
  
  delay(2);
  int sum = 0;
  for(int i=0; i<1000; i++){
    sum = sum + analogRead(ADC_PIN);
  }
  int result = sum/1000;
  return float(result) * (1.437);
  }
  */
}

void drawTemp_Hum(float temp, float hum){
  String Temperature;
  String Humidity;
  Temperature = String(temp, 1);
  Humidity = String(hum, 0);
  sprite.setTextColor(White, Black);
  
    sprite.fillRect(5, 120, 120, 20, Black); //Black out old CO2 value
    sprite.loadFont(NotoSansMonoSCB20);
  //sprite.drawString(Temperature, 40, 180, 4);
  //sprite.drawString(Humidity, 140, 180, 4);
    sprite.drawString(Temperature, 8, 120);
    sprite.drawString(Humidity, 100, 120);   
  
    sprite.unloadFont();
  
  sprite.drawString("'C", 20, 140, 2);
  sprite.drawString("%H", 102, 140, 2);
}

void drawCO2_Graph(uint16_t co2) { 

  //static uint32_t prev = 0;
  //if ( millis() - prev < 60000 ) {  // update every second?
  //  return;
  //}
  //prev = millis();
 
  static int minSensorValue = 400, maxSensorValue = 2000;
  //sgp.IAQmeasure();  //Grab latest sample 
  int sensorValue = co2;
  if(sensorValue>2000) sensorValue=2000; 
  Serial.println(sensorValue);
  int barColor = 0; // based on sensor value
  
  if ( sensorValue > 1500 && sensorValue < maxSensorValue ) {
   //Serial.println("Red");
   barColor = greenColor;  //redColor
  }
  else if ( sensorValue > 1000 && sensorValue <= 1500 ){
   //Serial.println("Yellow");
   barColor = greenColor;  //yellowColor
  }

  else if ( sensorValue <= 1000) {
   //Serial.println("Green");
   barColor = greenColor;
  }

  else {
  }

  static int barCounter = 0; 

  if ( barCounter >= totalSpriteBars - 1 ){  
   int len = totalSpriteBars - 1;

   for ( byte x = 0; x < len; x++ ) {
    colorArray[x] =  colorArray[x+1];
    heightArray[x] =  heightArray[x+1];
  }

  colorArray[len] = barColor;
  heightArray[len] = map(sensorValue,minSensorValue,maxSensorValue,barMinHeight,barMaxHeight); // 
 }

 else {
  colorArray[barCounter] = barColor;
  heightArray[barCounter] = map(sensorValue,minSensorValue,maxSensorValue,barMinHeight,barMaxHeight); //  
  barCounter++; // increment this
 }

  for ( byte x = 0; x < totalSpriteBars; x++ ) {
   int getHeight = heightArray[x];
   if  ( getHeight == 0 ) {
     continue;
   }

   int color_ = colorArray[x];
   int startingY = startY + barMaxHeight - getHeight; 
   int newHeight = barMaxHeight-getHeight;
   sprite.fillRect(startX+(x*barSpacing), startY,barWidth,barMaxHeight,notActiveColor);  // remove any previous color 
   sprite.fillRect(startX+(x*barSpacing), startingY,barWidth,getHeight,color_); // update the new color
  }
 sprite.pushSprite(0,0); 
}

void setup() {
  Serial.begin(115200);
  // wait for serial connection from PC
  // comment the following line if you'd like the output
  // without waiting for the interface being ready
  while(!Serial);

  // Initialize the GadgetBle Library
  provider.begin();
  Serial.print("Sensirion GadgetBle Lib initialized with deviceId = ");
  Serial.println(provider.getDeviceIdString());
  
  // init I2C
  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  // stop potentially previously started measurement
  scd4x.stopPeriodicMeasurement();

  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
      Serial.print("Error trying to execute startPeriodicMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  }

  Serial.println("Waiting for first measurement... (5 sec)");
  Serial.println("CO2(ppm)\tTemperature(degC)\tRelativeHumidity(percent)");
  tft.init();
  //tft.setSwapBytes(true);  //For icons
  tft.setRotation(0);
  sprite.createSprite(128, 160);
  inActiveState();
  delay(5000);
}

void inActiveState() {
  sprite.fillRect(20, 120, 240, 40, Black);  //20, 120, 240, 40
  for ( byte x = 0; x < totalSpriteBars; x++ ){
   sprite.fillRect(startX + (x * barSpacing), startY, barWidth, barMaxHeight, notActiveColor);
  }
  sprite.pushSprite(0, 0);
}

void loop() {
  if (millis() - lastMeasurementTimeMs >= measurementIntervalMs) {
    measure_and_report();
  }

  provider.handleDownload();
  delay(3);
}

void measure_and_report() {
  uint16_t error;
  char errorMessage[256];
    
  // Read Measurement
  uint16_t co2;
  float temperature;
  float humidity;

  error = scd4x.readMeasurement(co2, temperature, humidity);
  lastMeasurementTimeMs = millis();

  if (error) {
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
    return;
  }

  if (co2 == 0) {
    Serial.println("Invalid sample detected, skipping.");
    return;
  }
  
  Serial.print("co2:");
  Serial.print(co2);
  Serial.print("\t");
  Serial.print("temperature:");
  Serial.print(temperature);
  Serial.print("\t");
  Serial.print("humidity:");
  Serial.println(humidity);

  provider.writeValueToCurrentSample(co2, Unit::CO2);
  provider.writeValueToCurrentSample(temperature, Unit::T);
  provider.writeValueToCurrentSample(humidity, Unit::RH);

  provider.commitSample();
  
  drawCO2_Text(co2);
  drawCO2_Graph(co2);
  drawTemp_Hum(temperature, humidity);

}
