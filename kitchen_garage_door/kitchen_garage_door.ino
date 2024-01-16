#include <LiquidCrystal.h>

// is it time to just OO everything into arrays?

// sensors 1&2 are hooked up to the drawers
int sensor1Pin = 26;
int sensor1ActivatedTime = 0;
bool sensor1NormallyClosed = true;

int sensor2Pin = 28;
int sensor2ActivatedTime = 0;
bool sensor2NormallyClosed = true;

// Laudry Room door
int sensor3Pin = 27;
int sensor3ActivatedTime = 0;
bool sensor3NormallyClosed = false;

// Fridge Door
int sensor4Pin = 6;
int sensor4ActivatedTime = 0;
bool sensor4NormallyClosed = true;

LiquidCrystal lcd(0, 1, 2, 3, 4, 5);

int redLEDPin = 29;
int redLEDPin2 = 7;

void setup() {
  pinMode(sensor1Pin, INPUT);
  pinMode(sensor2Pin, INPUT);
  pinMode(sensor3Pin, INPUT);
  pinMode(sensor4Pin, INPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(redLEDPin2, OUTPUT);

  lcd.begin(16, 2);
  lcd.clear();
}

void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);

  bool sensor1Active = registerSensor(sensor1Pin, sensor1ActivatedTime, sensor1NormallyClosed);
  bool sensor2Active = registerSensor(sensor2Pin, sensor2ActivatedTime, sensor2NormallyClosed);
  bool sensor3Active = registerSensor(sensor3Pin, sensor3ActivatedTime, sensor3NormallyClosed);
  bool sensor4Active = registerSensor(sensor4Pin, sensor4ActivatedTime, sensor4NormallyClosed);

  // there's some business logic all wrapped up in this part..
  // the drawers will always only take up the top line
  // either as "Drawers 1&2" or as "Drawer x:"
  // if the fridge or laundry door is open it'll take up the first or second line (depending on if drawers was used)
  // if fridge and laundry are open it takes up both lines
  // if fridge and laundry and either drawer are open it flashes a different message.

  if(sensor1Active && sensor2Active){
    lcd.print("Drawers 1&2: ");

    // this is NOT going to matter since the drawers are co-planar and the door opener won't care about it
    // really the drawers could just be one item with the sensors in parallel..
    // but let's print the time for the most recently opened drawer.
    int newestChange = sensor1ActivatedTime;
    if(sensor1ActivatedTime < sensor2ActivatedTime) {
      newestChange = sensor2ActivatedTime;
    }

    int switchOpenDurationSecs = (millis() / 1000) - newestChange;
    printSecs(switchOpenDurationSecs);
  } else if(sensor1Active){
    lcd.print("Drawer 1: ");
    printSecs((millis() / 1000) - sensor1ActivatedTime);
  } else if(sensor2Active){
    lcd.print("Drawer 2: ");
    printSecs((millis() / 1000) - sensor2ActivatedTime);
  }

  // if we printed about the drawers, move to line 2
  if(sensor1Active || sensor2Active){
    lcd.setCursor(0,1);
  }

  // if the fridge and laundry are both open we're just going to wipe the LCD and print those.
  // I'll check at the end if everything is open and do something special
  if(sensor3Active && sensor4Active){
    int newestChange = sensor3ActivatedTime;
    if(sensor3ActivatedTime < sensor4ActivatedTime) {
      newestChange = sensor4ActivatedTime;
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Laundry & Fridge");
    lcd.setCursor(0,1);
    printSecs((millis() / 1000) - newestChange);
  }
  else if(sensor3Active){
    lcd.print("Laundry Rm: ");
    printSecs((millis() / 1000) - sensor3ActivatedTime);
  } else if(sensor4Active){
    lcd.print("Fridge door: ");
    printSecs((millis() / 1000) - sensor4ActivatedTime);
  }

  if((sensor1Active || sensor2Active) && sensor3Active && sensor4Active){
    lcd.clear();

    //Alternate line 0 and 1 every second
    lcd.setCursor(0, (millis() / 1000) % 2);

    lcd.print("   Everything");
    lcd.setCursor(0, 1-((millis() / 1000) % 2));
    lcd.print("     is open");
  }

  // everything inactive, handle the red LED
  if(!sensor1Active && !sensor2Active && !sensor3Active && !sensor4Active){
    lcd.print("All Clear");
    digitalWrite(redLEDPin, LOW);
    digitalWrite(redLEDPin2, LOW);
  } else {
    digitalWrite(redLEDPin, HIGH);
    digitalWrite(redLEDPin2, HIGH);
  }

  delay(100);  // rest and prevent some flickering (or is this causing flickering?)
}

// This both "checks" the switch and starts it's timer if needed, or shuts off it's timer
bool registerSensor(int switchPin, int &switchActiveTime, bool normallyClosed) {
  bool switchActive = digitalRead(switchPin);
  if (normallyClosed) {
    switchActive = !switchActive;
  }

  if (switchActive) {
    if (switchActiveTime == 0) {
      switchActiveTime = millis() / 1000;
    }
  } else {
    switchActiveTime = 0;
  }
  return switchActive;
}

// Need to ensure these don't exceed 3 chars for LCD purposes
// 0s -> 59s
// 1m -> 59m
// 1h -> 23h
// 1d+
void printSecs(int secs) {
  if (secs < 60) {
    lcd.print(secs, DEC);
    lcd.print("s");
    return;
  }
  if (secs < 60 * 60) {
    lcd.print(secs / 60, DEC);
    lcd.print("m");
    return;
  }
  if (secs < 60 * 60 * 24) {
    lcd.print(secs / 60 / 60, DEC);
    lcd.print("h");
    return;
  } else {
    lcd.print("1d+");
    return;
  }
}