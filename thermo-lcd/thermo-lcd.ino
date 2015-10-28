#include <OneWire.h>
#include <dht.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// ---- Define wanted values for the growbox
int HumMin = 40;
int HumMax = 70;
int TempMin = 20;
int TempMax = 30;

// ---- Define pins
int SwitchPin = 2; // Interrupt
int HotPin = 4; // Led rouge
int FanPin = 5; // Led jaune
int LightPin = 6; // Led verte
int DHTPin = A0;
int DS18S20_Pin = 7;

// ---- Setup sensors
OneWire ds(DS18S20_Pin); //Temperature chip i/o on digital pin 7 by rahulmitra
#define dht_dpin DHTPin //no ; here. Set equal to channel sensor is on 
dht DHT;

// ---- Setup LCD screen
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

//************************************************************
//---------------- Define Special Characters-----------------*
// Create a set of new characters                            *
// 0=fan                                                     *
// 1=degree sign                                             *
// 2=termometer                                              *
// 3=waterdruppel                                            *
// 4=spikes                                                  *
// 5=pump                                                    *
// 6=up arrow                                                *
// 7=down arrow                                              *
//************************************************************
const uint8_t charBitmap[][8] = {
  { 
    B00000,
    B10001,
    B01010,
    B00100,
    B01010,
    B10001,
    B00000,
    B00000           }
  ,
  { 
    0x6, 0x9, 0x9, 0x6, 0, 0, 0, 0           }
  ,
  { 
    B00100,
    B01010,
    B01010,
    B01110,
    B01110,
    B11111,
    B11111,
    B01110           }
  ,
  { 
    B00100,
    B00100,
    B01010,
    B01010,
    B10001,
    B10001,
    B10001,
    B01110          }
  ,
  { 
    B10001,
    B10001,
    B11111,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001           }
  ,
  { 
    B01010,
    B00100,
    B00100,
    B00100,
    B11111,
    B11111,
    B11111          }
  ,
  { 
    0x0, 0x4, 0xE, 0x15, 0x4, 0x4, 0x4, 0x0           }
  ,
  { 
    0x4,0x4,0x4, 0x4, 0x15, 0xE,0x4,0x0           }
};
/// -------- end creation--------------

// --- Mandatory section ---
void setup() {
  Serial.begin(9600);
  lcd.init();
  //upload defined characters to LCD
  int charBitmapSize = (sizeof(charBitmap ) / sizeof (charBitmap[0]));
  for ( int i = 0; i < charBitmapSize; i++ )
  {
    lcd.createChar ( i, (uint8_t *)charBitmap[i] );
  }
  //---------------end upload----------------
  lcd.backlight();
  
  //Setup pin modes
  pinMode(FanPin, OUTPUT);  // Output for fan
  pinMode(HotPin, OUTPUT);  // Output for heater
  pinMode(LightPin, OUTPUT);// Output for light
  digitalWrite(FanPin,LOW);  // fan Off
  digitalWrite(HotPin,LOW);   // heater off
  digitalWrite(LightPin, LOW); // light Off 
  digitalWrite(SwitchPin, HIGH); // Up
  
  //Setup Interrupt
  //attachInterrupt(0, myInterrupt(), FALLING);
  attachInterrupt(digitalPinToInterrupt(2), myInterrupt(), FALLING); 
}


// ---- Main loop section
void loop() {
// ---- Read all values

//Read air temp from DS18B20 sensor
//float ReadTemp = getTemp();

//Read DHT11 sensor values
int chk = DHT.read11(DHTPin);
float ReadHum = DHT.humidity;
float ReadTemp = DHT.temperature;

if (ReadTemp >=  TempMax || ReadHum >=  HumMax)
{
 digitalWrite(HotPin,LOW);   // heater off
 digitalWrite(FanPin,HIGH);  // fan On
}
else{
 //digitalWrite(HotPin,LOW);   // heater Off
 digitalWrite(FanPin,LOW);  // fan Off
};

if (ReadTemp <=  TempMin || ReadHum <=  HumMin)
{
 digitalWrite(HotPin,HIGH);   // heater on
 digitalWrite(FanPin,LOW);  // fan Off
}
else{
 digitalWrite(HotPin,LOW);   // heater Off
 //digitalWrite(FanPin,LOW);  // fan Off
};

//Send data to serial port
Serial.print("t= ");
Serial.println((float)DHT.temperature);
Serial.print("h= ");
Serial.println((float)DHT.humidity);
Serial.print("Switch Status= ");
Serial.println(digitalRead(SwitchState));


//Display data on LCD screen
  lcd.setCursor(0, 0);
    lcd.print("Temp.: ");
    //lcd.print((float)getTemp(), 0);
    lcd.print((float)DHT.temperature, 0);
    lcd.print(char(1));
    lcd.print("C");
  lcd.setCursor(0, 1);
    lcd.print(" Hum.: ");
    lcd.print((float)DHT.humidity, 0);
    lcd.print("%");
    
delay(10000); //just here to slow down the output so it is easier to read
}

void myInterrupt() {
  // do something ...
  digitalWrite(LightPin,HIGH);  
}

// --- Function to read temp fron DS18B20 sensor
float getTemp(){
//returns the temperature from one DS18S20 in DEG Celsius
byte data[12];
byte addr[8];

if ( !ds.search(addr)) {
//no more sensors on chain, reset search
ds.reset_search();
return -1000;
}

if ( OneWire::crc8( addr, 7) != addr[7]) {
Serial.println("CRC is not valid!");
return -1000;
}

if ( addr[0] != 0x10 && addr[0] != 0x28) {
Serial.print("Device is not recognized");
return -1000;
}

ds.reset();
ds.select(addr);
ds.write(0x44,1); // start conversion, with parasite power on at the end

byte present = ds.reset();
ds.select(addr);
ds.write(0xBE); // Read Scratchpad

for (int i = 0; i < 9; i++) { // we need 9 bytes
data[i] = ds.read();
}

ds.reset_search();

byte MSB = data[1];
byte LSB = data[0];

float tempRead = ((MSB << 8) | LSB); //using two's compliment
float TemperatureSum = tempRead / 16;

return TemperatureSum;
} 
