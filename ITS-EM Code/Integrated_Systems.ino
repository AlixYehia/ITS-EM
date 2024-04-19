 /*   RFID system
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

 

 /*    Light system shift register pins

 *         LED  D12       D11
 *     5V   0   MISO GND  MOSI  D9  5V  
 *     |    |    |    |    |    |    |    |
 *  |--|----|----|----|----|----|----|----|--|
 *  |  VCC  Q0   DS  OE  ST_CP SH_CP MR  Q7' |
 *  |                                        |          
 *  |  Q1   Q2   Q3  Q4    Q5   Q6   Q7  GND |
 *  |--|----|----|----|----|----|----|----|--| 
 *     |    |    |    |    |    |    |    |
 *    LED  LED  LED  LED  LED  LED  LED  GND
 *     1    2    3    4    5    6    7
*/


#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include "DHT.h"
#include <HX711_ADC.h>



// Light and Flame sensor systems
LiquidCrystal_I2C lcd2(0x26, 16, 2);
const int FlameSensorPin = A0;
const int lightSensorPin = A1;
const int FlameRelay = 2;
const int LightRelay = 3;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 /* light system */

// Ultrasonic sensors system
const int trigPin1 = 13;          // Ultrasonic sensor 1 trig pin
const int echoPin1 = 14;          // Ultrasonic sensor 1 echo pin
const int transistorPin1 = 8;     // Transistor control pin for shift register 1
const int trigPin2 = 15;          // Ultrasonic sensor 2 trig pin
const int echoPin2 = 16;          // Ultrasonic sensor 2 echo pin
const int transistorPin2 = 17;    // Transistor control pin for shift register 2

// Shift register pins
const int DS = 12;               // DS - data serial
const int SH_CP = 9;             // SH_CP - shift register clock pin
const int latchPin1 = 11;
const int latchPin2 = 10;

int lastDistance1 = 0;           // Variable to store the last measured distance for sensor 1
int lastDistance2 = 0;           // Variable to store the last measured distance for sensor 2
int activeLeds1 = 0x00;          // Initial active LED state for shift register 1 (8 LEDs off)
int activeLeds2 = 0x00;          // Initial active LED state for shift register 2 (8 LEDs off)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////



// RFID and Servo system
#define SS_PIN 53
#define RST_PIN 5
String UID = "83 B2 55 10";
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);


// DHT sensor system
#define DHTRelay 18
#define DHTPIN 30
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


// Load cell weight system
const int Load_cell_data_in = 6;
const int Load_cell_clock_pin = 7;

HX711_ADC LoadCell(Load_cell_data_in, Load_cell_clock_pin);    
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

    // Initialize Light system ultraosnic sensors and register pins
    pinMode(trigPin1, OUTPUT);
    pinMode(echoPin1, INPUT);
    pinMode(trigPin2, OUTPUT);
    pinMode(echoPin2, INPUT);
    pinMode(DS, OUTPUT);       // DS - data serial
    pinMode(SH_CP, OUTPUT);    // SH_CP - shift register clock pin

    pinMode(transistorPin1, OUTPUT);  // Transistor 1 control pin for shift register 1
    pinMode(transistorPin2, OUTPUT);  // Transistor 2 control pin for shift register 2

    updateShiftRegister(activeLeds1, latchPin1);  // Update shift register 1
    updateShiftRegister(activeLeds2, latchPin2);  // Update shift register 2

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



///////////////////////////////Light System//////////////////////////////////////



void updateShiftRegister(byte leds, int latchPin) {
  digitalWrite(latchPin, LOW);
  shiftOut(DS, SH_CP, MSBFIRST, leds);
  digitalWrite(latchPin, HIGH);
}


void checkLightSensor() {
    int analogValue = analogRead(lightSensorPin);
    Serial.print("Light Sensor Analog reading: ");
    Serial.print(analogValue);

    if (analogValue > 1000) {
        Serial.println(" - Dark");
        turnOnLightRelay();
    } else if (analogValue < 1000 && analogValue > 960) {
        Serial.println(" - Dim");
        turnOnLightRelay();
    } else {
        Serial.println(" - Bright");
        turnOffLightRelay();
    }
}

void turnOnLightRelay() {
    digitalWrite(LightRelay, LOW);

    long duration1, distance1;
    long duration2, distance2;

    // Ultrasonic sensor 1 measurements
    digitalWrite(trigPin1, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin1, LOW);
    duration1 = pulseIn(echoPin1, HIGH);
    distance1 = duration1 * 0.034 / 2; // Calculate distance in cm

    // Ultrasonic sensor 2 measurements
    digitalWrite(trigPin2, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin2, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin2, LOW);
    duration2 = pulseIn(echoPin2, HIGH);
    distance2 = duration2 * 0.034 / 2; // Calculate distance in cm

    // Update LEDs for sensor 1
    if (distance1 != lastDistance1) {
      lastDistance1 = distance1;
      Serial.print("Distance 1: ");
      Serial.print(distance1);
      Serial.println(" cm");

      if (distance1 <= 25) {
        analogWrite(transistorPin1, 255); // Full intensity
        activeLeds1 = 0xFF;
      } else {
        analogWrite(transistorPin1, 10);  // Lower intensity
        activeLeds1 = 0xFF;
      }
      updateShiftRegister(activeLeds1, latchPin1);  // Update shift register 1
    }

    // Update LEDs for sensor 2
    if (distance2 != lastDistance2) {
      lastDistance2 = distance2;
      Serial.print("Distance 2: ");
      Serial.print(distance2);
      Serial.println(" cm");

      if (distance2 <= 25) {
        analogWrite(transistorPin2, 255); // Full intensity
        activeLeds2 = 0xFF;
      } else {
        analogWrite(transistorPin2, 10);  // Lower intensity
        activeLeds2 = 0xFF;  
      }
      updateShiftRegister(activeLeds2, latchPin2);  // Update shift register 2
    }

}

void turnOffLightRelay() {
    digitalWrite(LightRelay, HIGH);
}



///////////////////////////////Fire Suppression and Weather Monitoring systems//////////////////////////////////////



void checkFlameSensor() {
    int FlameSensor = analogRead(FlameSensorPin);

    if (FlameSensor > 600) {
        Serial.println("High Flame Sensor Reading - no fire");
        turnOffFlameRelay();
        lcd2.clear();
        displayDHTReadings();      // Display weather info if there is no fire threat
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



////////////////////////////////Weather info when no fire threat/////////////////////////////////////



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




///////////////////////////////Automatic tolling using RFID system//////////////////////////////////////



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
        servo.write(160);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Granted");
        delay(5000);
        lcd.clear();
        lcd.print("Door Closed");
        servo.write(70);
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




///////////////////////////////Truck Weighing Management System//////////////////////////////////////



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
}
