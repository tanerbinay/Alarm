#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

#define DS3231_ADDRESS 0x68
#define LED_PIN 11
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

LiquidCrystal_I2C lcd(0x3F, 16, 2);

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

byte alarmHour = 0;
byte alarmMinute = 0;
bool isAlarmSet = false;
bool alarmTriggered = false; // alarmTriggered burada tanımlanır
unsigned long alarmStartTime = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(16, 2);
  lcd.backlight();
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED'i başta söndür
}

void loop() {
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231Time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  char key = keypad.getKey();
  if (key == 'A') {
    setAlarm();
    delay(100);
  }

  checkAlarm(hour, minute, second);
  displayTime(hour, minute, second, dayOfMonth, month, year);

  if (alarmTriggered) {
    if (millis() - alarmStartTime >= 120000) { // 120000 ms = 2 dakika
      alarmTriggered = false;
      digitalWrite(LED_PIN, HIGH); // LED'i söndür
    }
  }

  delay(1000);
}

void setAlarm() {
  lcd.clear();
  lcd.print("Saatini gir:");
  delay(100);

  alarmHour = getNumberFromKeypad();
  lcd.clear();
  lcd.print("Dakikasini gir:");
  delay(100);

  alarmMinute = getNumberFromKeypad();
  lcd.clear();
  lcd.print("Alarm ayarlandi!");
  delay(1000);
  isAlarmSet = true;
}

void checkAlarm(byte currentHour, byte currentMinute, byte currentSecond) {
  if (isAlarmSet && currentHour == alarmHour && currentMinute == alarmMinute && currentSecond == 0) {
    digitalWrite(LED_PIN, LOW); // LED'i yak
    alarmStartTime = millis();
    alarmTriggered = true;
    isAlarmSet = false; // Alarmı sıfırla
  }
}

void displayTime(byte hour, byte minute, byte second, byte dayOfMonth, byte month, byte year) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Saat: ");
  printTime(hour, minute, second);

  lcd.setCursor(0, 1);
  lcd.print("Tarih: ");
  printDate(dayOfMonth, month, year);
}

void printTime(byte hour, byte minute, byte second) {
  lcd.print(hour);
  lcd.print(":");
  lcd.print(minute);
  lcd.print(":");
  lcd.print(second);
}

void printDate(byte dayOfMonth, byte month, byte year) {
  lcd.print(dayOfMonth);
  lcd.print("/");
  lcd.print(month);
  lcd.print("/");
  lcd.print(2000 + year);
}

byte getNumberFromKeypad() {
  String input = "";
  while (true) {
    char key = keypad.getKey();
    if (key != NO_KEY) {
      if (key == '#') {
        return input.toInt();
      } else {
        input += key;
        lcd.setCursor(0, 1);
        lcd.print(input);
      }
    }
  }
}

void readDS3231Time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 7);

  *second = bcdToDec(Wire.read() & 0x7F);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3F);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

byte bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));


}
