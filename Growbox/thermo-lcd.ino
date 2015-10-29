#include <OneWire.h>
#include <dht.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// ---- Define wanted values for the growbox
int HumMin = 40;
int HumMax = 70;
int TempMin = 20;
int TempMax = 30;

// ---- Define delays
int BackLightDelay = 5; //delay in s
int CheckValueDelay = 10; //delay in s before checking for temperature and humidity in the growbox

// ---- Define pins
int SwitchPin = 2; // Interrupt
int HotPin = 5; // Led rouge
int FanPin = 6; // Led jaune
int LightPin = 7; // Led verte
int DHTPin = A0;
int DS18S20_Pin = 8;

// ---- Setup global variables
volatile int IntState = LOW;
int BacklightState = LOW;
unsigned long LastChange1 = 0;
unsigned long LastChange2 = 0;
unsigned long LastChange3 = 0;
unsigned long CurrentTime = 0;
int chk = 0;
int ReadHum = 0;
int ReadTemp = 0;

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
  // lcd.backlight();
  
  //Setup pin modes
  pinMode(FanPin, OUTPUT);  // Output for fan
  digitalWrite(FanPin,LOW);  // Fan Off
  pinMode(HotPin, OUTPUT);  // Output for heater
  digitalWrite(HotPin,LOW);   // Heater off
  pinMode(LightPin, OUTPUT);// Output for light
  digitalWrite(LightPin, LOW); // Light Off 
  pinMode(SwitchPin, INPUT); // Input for backlight switch
  digitalWrite(SwitchPin, HIGH); // Switch input Up

  //Define interrupts
  attachInterrupt(digitalPinToInterrupt(SwitchPin), BLInt, FALLING); //setup interrupt for backlight switch
  
  //Delay for LCD and DHT sensor
  delay(800);
}


// ---- Main loop section
void loop() {
  CurrentTime = millis();
  
  // --- Backlight Timer
  int BLDelay = BackLightDelay * 1000;
  unsigned long TimeLapse1 = CurrentTime - LastChange1;
  if (IntState == HIGH){
      lcd.backlight();
      BacklightState = HIGH;
      IntState = LOW;
      LastChange1 = CurrentTime;
    }
    if (BacklightState == HIGH && TimeLapse1 > BLDelay){
        lcd.noBacklight();
        LastChange1 = CurrentTime;
    }

  // --- Thermometer timer
  unsigned long TimeLapse2 = CurrentTime - LastChange2;

  if (TimeLapse2 > 800){
    // ---- Read all values
        
    //Read air temp from DS18B20 sensor
    //ReadTemp = getTemp();
    
    //Read DHT11 sensor values
    chk = DHT.read11(DHTPin);
    ReadHum = DHT.humidity;
    ReadTemp = DHT.temperature;  
  
    //Send data to serial port
    Serial.print("t= ");
    Serial.println(ReadTemp);
    Serial.print("h= ");
    Serial.println(ReadHum);
    LastChange2 = CurrentTime;             
  }

  // --- Growbox timer / Main Function
  int CVDelay = CheckValueDelay * 1000;
  unsigned long TimeLapse3 = CurrentTime - LastChange3;
  
  if (TimeLapse3 > CVDelay){ 
    // ---- Read all values
        
    //Read air temp from DS18B20 sensor
    //float ReadTemp = getTemp();
    
    //Read DHT11 sensor values
    //int chk = DHT.read11(DHTPin);
    //float ReadHum = DHT.humidity;
    //float ReadTemp = DHT.temperature;
 
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
    //Serial.print("t= ");
    //Serial.println((float)DHT.temperature);
    //Serial.print("h= ");
    //Serial.println((float)DHT.humidity);         
      
    //delay(10000); //just here to slow down the output so it is easier to read
    LastChange3 = CurrentTime;
  }
  
  // --- Display data on LCD screen
  lcd.setCursor(0, 0);
    lcd.print("Temp.: ");
    //lcd.print((float)getTemp(), 0);
    lcd.print(ReadTemp);
    lcd.print(char(1));
    lcd.print("C");
  lcd.setCursor(0, 1);
    lcd.print(" Hum.: ");
    lcd.print(ReadHum);
    lcd.print("%");
  }
  
void BLInt() {
    IntState = HIGH;  
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
