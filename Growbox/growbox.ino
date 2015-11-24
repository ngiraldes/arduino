#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ---- Define wanted values for the growbox
int HumMin = 40;
int HumMax = 60;
int TempMin = 20;
int TempMax = 30;

// ---- Define delays
int BackLightDelay = 5; //delay in s
int CheckValueDelay = 10; //delay in s before checking for temperature and humidity in the growbox

// ---- Define pins
int ServoPin = 8; // Servo motor
int SwitchPin = 2; // Interrupt
int HotPin = 5; // Led rouge
int FanPin = 6; // Led jaune
int LightPin = 7; // Led verte
int DHTPin = A0;


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
int angle = 0;   // servo position in degrees 

// --- Setup servo
Servo servo;


// ---- Setup DHT sensor
#define DHTPIN DHTPin
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

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

  // --- Define interrupts
  attachInterrupt(digitalPinToInterrupt(SwitchPin), BLInt, FALLING); //setup interrupt for backlight switch

  // --- initiate DHT sensor
  dht.begin();

  // --- initiate servo motor
  servo.attach(ServoPin);
  servo.write(0);
  
  
  // --- Delay for LCD and DHT sensor
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

  if (TimeLapse2 > 2000){
    // ---- Read all values
        
    //Read air temp from DS18B20 sensor
    //ReadTemp = getTemp();
    
    //Read DHT sensor values
    ReadHum = dht.readHumidity();
    ReadTemp = dht.readTemperature();
  
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
     servo.write(180);  // open flaps
    }
    else{
     //digitalWrite(HotPin,LOW);   // heater Off
     digitalWrite(FanPin,LOW);  // fan Off
     servo.write(0);  // close flaps
    };
    
    if (ReadTemp <=  TempMin || ReadHum <=  HumMin)
    {
     digitalWrite(HotPin,HIGH);   // heater on
     digitalWrite(FanPin,LOW);  // fan Off
     servo.write(0);  // close flaps
    }
    else{
     digitalWrite(HotPin,LOW);   // heater Off
    };
    
    LastChange3 = CurrentTime;
  }
  
  // --- Display data on LCD screen
  lcd.setCursor(0, 0);
    lcd.print("Temp.: ");
    lcd.print(ReadTemp);
    lcd.print(char(1));
    lcd.print("C");
  lcd.setCursor(0, 1);
    lcd.print(" Hum.: ");
    lcd.print(ReadHum);
    lcd.print("%");
  }

// --- Funtion for switch interrupt  
void BLInt() {
    IntState = HIGH;  
}

