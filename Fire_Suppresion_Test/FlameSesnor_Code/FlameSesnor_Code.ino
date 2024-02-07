#include <LiquidCrystal.h>
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
int relayPin = 6; // Motor Relay
int flamePin = 7; // Flame sensor connected to digital pin 7
int flameValue = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(flamePin, INPUT);
  pinMode(relayPin,OUTPUT);
  lcd.print("Flame Sensor");
}

void loop() {
  flameValue = digitalRead(flamePin); // Read the flame sensor value
  
  
  if (flameValue == HIGH) {
    digitalWrite(relayPin,HIGH);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Flame Sensor");
    lcd.setCursor(2, 1);
    lcd.print("Fire detected!");
    digitalRead(flamePin);
    Serial.println(flamePin);
  } else {
    digitalWrite(relayPin,LOW);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Flame Sensor");
    lcd.setCursor(2, 1);
    lcd.print("No fire");
    Serial.println(flamePin);
  }
  delay(1000);
  
}