int IntPin = 2;
int LedPin = 7; // Led verte
volatile int IntState = LOW;
unsigned long LastChange = 0;
unsigned long CurrentTime = 0;

void setup() {
    Serial.begin(9600);
    pinMode(IntPin, INPUT);
    digitalWrite(IntPin, HIGH);
    pinMode(LedPin, OUTPUT);
    digitalWrite(LedPin, LOW);
    attachInterrupt(digitalPinToInterrupt(IntPin), myInt, FALLING);        
}

void loop() {
    CurrentTime = millis();
    unsigned long TimeLapse = CurrentTime - LastChange;
    Serial.println(TimeLapse);
    if (IntState == HIGH){
      digitalWrite(LedPin, HIGH);
      IntState = LOW;
      LastChange = CurrentTime;
    }
    if (digitalRead(LedPin) == HIGH && TimeLapse > 5000){
        digitalWrite(LedPin, LOW);
        LastChange = CurrentTime;
    }
}

void myInt() {
    IntState = HIGH;
}
