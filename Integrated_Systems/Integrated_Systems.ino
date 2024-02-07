#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include "DHT.h"
#include <HX711_ADC.h>

// Light and Flame sensor system
LiquidCrystal_I2C lcd2(0x26, 16, 2);
const int FlameSensorPin = A0;
const int lightSensorPin = A1;
const int FlameRelay = 2;
const int LightRelay = 3;

// RFID and Servo system
#define SS_PIN 53
#define RST_PIN 5
String UID = "83 B2 55 10";
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);

// DHT sensor system
#define DHTRelay 7
#define DHTPIN 30
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Load cell weight system
HX711_ADC LoadCell(8, 9);
LiquidCrystal_I2C lcd3(0x25, 16, 2);
int taree = 6;
int a = 0;
float b = 0;
const float LE_RATE = 0.5;  // LE rate per gram

void setup() {
    Serial.begin(9600);

    // Initialize LCDs
    lcd.init();
    lcd.backlight();
    lcd2.init();
    lcd2.backlight();
    lcd3.init();
    lcd3.backlight();

    lcd3.setCursor(1, 0);
    lcd3.print("Digital Scale ");
    lcd3.setCursor(0, 1);
    lcd3.print(" 20KG MAX LOAD ");
    delay(1500);
    lcd3.clear();

    // Initialize sensors and relays
    pinMode(LightRelay, OUTPUT);
    pinMode(FlameRelay, OUTPUT);
    digitalWrite(LightRelay, HIGH);
    digitalWrite(FlameRelay, HIGH);

    // Initialize Servo and RFID
    servo.attach(4);
    SPI.begin();
    rfid.PCD_Init();
    servo.write(70);

    // Initialize DHT sensor
    dht.begin();

    // Intialize Load cell weight sesnor
     pinMode(taree, INPUT_PULLUP);
    LoadCell.begin();
    LoadCell.start(1000);

    LoadCell.setCalFactor(116.9999999);

}

void loop() {
    // Light sensor system loop
    checkLightSensor();

    // Flame and DHT sensors systems loop
    checkFlameSensor();

    // RFID and Servo system loop
    rfidLoop();

    // Weight sensor system loop
    loadCell();
}



/////////////////////////////////////////////////////////////////////



void checkLightSensor() {
    int analogValue = analogRead(lightSensorPin);
    Serial.print("Light Sensor Analog reading: ");
    Serial.print(analogValue);

    if (analogValue > 965) {
        Serial.println(" - Dark");
        turnOnLightRelay();
    } else if (analogValue < 960 && analogValue > 890) {
        Serial.println(" - Dim");
        turnOnLightRelay();
    } else {
        Serial.println(" - Bright");
        turnOffLightRelay();
    }
}

void turnOnLightRelay() {
    digitalWrite(LightRelay, LOW);
}

void turnOffLightRelay() {
    digitalWrite(LightRelay, HIGH);
}



/////////////////////////////////////////////////////////////////////



void checkFlameSensor() {
    int FlameSensor = analogRead(FlameSensorPin);

    if (FlameSensor > 600) {
        Serial.println("High Flame Sensor Reading - no fire");
        turnOffFlameRelay();
        lcd2.clear();
        displayDHTReadings();
    } else {
        Serial.println("Fire Detected");
        lcd2.setCursor((16 - strlen("Fire Detected")) / 2, 0);
        lcd2.print("Fire Detected");
        lcd2.setCursor((16 - strlen("STOP!")) / 2, 1);
        lcd2.print("STOP!");
        turnOnFlameRelay();
    }
}

void turnOnFlameRelay() {
    digitalWrite(FlameRelay, LOW);
}

void turnOffFlameRelay() {
    digitalWrite(FlameRelay, HIGH);
}



/////////////////////////////////////////////////////////////////////



void displayDHTReadings() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);

    if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.print("DHT Read Failed!");
        delay(300);
        return;
    }

    controlRelay(t,h);

    lcd2.setCursor(0, 0);
    lcd2.print("Hum:");
    lcd2.print(h, 1);
    lcd2.print("%");
    lcd2.setCursor(0, 1);
    lcd2.print("Temp:");
    lcd2.print(t, 1);
    lcd2.print((char)223);
    lcd2.print("C");
    delay(1000);
    lcd2.clear();
}

void controlRelay(float temperature, float humidity) {
  if (temperature < 23.0 || humidity < 60) {
    analogWrite(DHTRelay, 0);
  } else if (temperature >= 23.0 && temperature < 27.0 || humidity <= 70 && humidity < 80) {
    analogWrite(DHTRelay, 180);
  } else if (temperature >= 27.0 && temperature < 30.0 || humidity <= 80 && humidity < 85) {
    analogWrite(DHTRelay, 220);
  } else {
    analogWrite(DHTRelay, 255);
  }
}


/////////////////////////////////////////////////////////////////////



void rfidLoop() {
    lcd.setCursor(4, 0);
    lcd.print("Welcome!");
    lcd.setCursor(1, 1);
    lcd.print("Put your card");

    if (!rfid.PICC_IsNewCardPresent())
        return;
    if (!rfid.PICC_ReadCardSerial())
        return;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scanning");
    Serial.print("NUID tag is:");
    String ID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        lcd.print(".");
        ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
        ID.concat(String(rfid.uid.uidByte[i], HEX));
        delay(300);
    }
    ID.toUpperCase();

    if (ID.substring(1) == UID) {
        servo.write(145);   // open angle adjustment
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Granted");
        delay(5000);
        lcd.clear();
        lcd.print("Door Closed");
        servo.write(47);   // close angle adjustment
        delay(1500);
        lcd.clear();
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Denied");
        delay(1500);
        lcd.clear();
    }
}



/////////////////////////////////////////////////////////////////////



void loadCell() {
    lcd3.setCursor(0, 0);
    lcd3.print("Weight");
    lcd3.setCursor(9, 0);
    lcd3.print("Fee");
    LoadCell.update();
    float i = LoadCell.getData();

    if (i < 0) {
        i = i * (-1);
    }

    lcd3.setCursor(0, 1);
    lcd3.print(i, 1);
    lcd3.print("g ");

    float leValue = i * LE_RATE;  // Calculate LE value based on weight in grams
    lcd3.setCursor(8, 1);
    lcd3.print(leValue, 2);
    lcd3.print("LE ");

    if (i >= 5000) {
        i = 0;
        lcd3.setCursor(0, 0);
        lcd3.print("  Over Loaded   ");
        delay(200);
    }

    if (digitalRead(taree) == LOW) {
        lcd3.setCursor(0, 1);
        lcd3.print("   Taring...    ");
        LoadCell.start(1000);
        lcd3.setCursor(0, 1);
        lcd3.print("                ");
    }

    //delay(500);  // Adjust delay as needed
}
