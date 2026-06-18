#include <Adafruit_Fingerprint.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

Servo doorLock;

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define BUZZER 4
#define GREEN_LED 5
#define RED_LED 6
#define SERVO_PIN 9

int failedAttempts = 0;

void setup()
{
  pinMode(BUZZER, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  doorLock.attach(SERVO_PIN);
  doorLock.write(0); // Locked position

  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
  finger.begin(57600);

  lcd.setCursor(0,0);
  lcd.print("Door Lock Sys");

  if (finger.verifyPassword())
  {
    lcd.setCursor(0,1);
    lcd.print("Sensor Ready");
  }
  else
  {
    lcd.setCursor(0,1);
    lcd.print("Sensor Error");
    while(1);
  }

  delay(2000);
  lcd.clear();
}

void loop()
{
  lcd.setCursor(0,0);
  lcd.print("Place Finger");

  int id = getFingerprintID();

  if (id > 0)
  {
    accessGranted(id);
  }

  delay(50);
}

int getFingerprintID()
{
  uint8_t p = finger.getImage();

  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.image2Tz();

  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.fingerFastSearch();

  if (p == FINGERPRINT_OK)
  {
    return finger.fingerID;
  }

  accessDenied();
  return -1;
}

void accessGranted(int id)
{
  failedAttempts = 0;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Access Granted");
  lcd.setCursor(0,1);
  lcd.print("ID:");
  lcd.print(id);

  digitalWrite(GREEN_LED, HIGH);

  tone(BUZZER, 1000, 300);

  doorLock.write(90);   // Unlock

  delay(5000);

  doorLock.write(0);    // Lock

  digitalWrite(GREEN_LED, LOW);

  lcd.clear();
}

void accessDenied()
{
  failedAttempts++;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Access Denied");

  digitalWrite(RED_LED, HIGH);

  tone(BUZZER, 500, 500);

  delay(1000);

  digitalWrite(RED_LED, LOW);

  if(failedAttempts >= 3)
  {
    triggerAlarm();
    failedAttempts = 0;
  }

  lcd.clear();
}

void triggerAlarm()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SECURITY ALERT");

  for(int i=0;i<10;i++)
  {
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 2000);

    delay(200);

    digitalWrite(RED_LED, LOW);
    noTone(BUZZER);

    delay(200);
  }
}