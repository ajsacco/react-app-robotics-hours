#include <deprecated.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522v2.h>
#include <require_cpp11.h>

#include <arduino-timer.h>

#include <LiquidCrystal.h>

#include <Keypad.h>

bool devMode = false;
const String kDevLockedChar = "x";
const String kDevLoadingChar = "*";
const String kDevUnlockedChar = "Y";
String devAuth = kDevLockedChar;

const byte kCheckmarkIcon[8] = {
  0b00000,
  0b00001,
  0b00001,
  0b00010,
  0b00010,
  0b10100,
  0b01000,
  0b00000
};
const byte kLockedIcon[8] = {
  0b00000,
  0b01110,
  0b10001,
  0b10001,
  0b11111,
  0b11011,
  0b11011,
  0b11111
};
const byte kUnlockedIcon[8] = {
  0b01110,
  0b10001,
  0b10001,
  0b00001,
  0b11111,
  0b11011,
  0b11011,
  0b11111
};

// Initialize Timer
String lastPong = "DISC";
bool didReceivePong = false;
int dropCount = 0;
Timer<10> timer;
Timer<10> watchdog;
bool ping_photon(void *) {
  Serial1.println("PING");
  watchdog.in(250, watch);
  return true;
}
bool watch(void *) {
  if (didReceivePong) {
    didReceivePong = false;
    dropCount = 0;
    return false;
  }
  
  dropCount++;
  if (dropCount < 10)
    return true;

  Serial.println("Photon Dead! Resetting...");

  dropCount = 0;

  pinMode(55, OUTPUT);
  digitalWrite(55, LOW);
  delay(10);
  digitalWrite(55, HIGH);

  return false;
}

// Initialize Keypad
const int ROWS = 4;
const int COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pin_rows[ROWS] = {42, 43, 44, 45}; //connect to the row pinouts of the keypad
byte pin_column[COLS] = {46, 47, 48, 49}; //connect to the column pinouts of the keypad
String entry = "";
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROWS, COLS );

// Initialize NFC
MFRC522DriverPinSimple ss_pin(53);
MFRC522DriverSPI driver{ss_pin};
MFRC522 mfrc522{driver};

bool waiting = false;

// Initialize LCD
LiquidCrystal lcd(62,63,64,65,66,67);
const String kLCDStatus = "Scan/enter ID:";

void redrawLCD(String entry) {
  lcd.clear();
  if (devMode) {
    lcd.print("Enroll Tag/SID " + devAuth);
    if (devAuth == kDevUnlockedChar) {
      lcd.setCursor(15,0);
      lcd.write((byte)2);
    }
    else if (devAuth == kDevLockedChar) {
      lcd.setCursor(15,0);
      lcd.write((byte)1);
    }
  }
  else
    lcd.print(kLCDStatus);
  lcd.setCursor(0,1);
  lcd.print(entry);
}

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16,2);
  lcd.print(kLCDStatus);
  lcd.createChar(0, kCheckmarkIcon);
  lcd.createChar(1, kLockedIcon);
  lcd.createChar(2, kUnlockedIcon);

  mfrc522.PCD_Init();

  timer.every(500, ping_photon);
  
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {
  timer.tick();
  watchdog.tick();
  
  // read from port 1, send to port 0:
  if (Serial1.available()) {
    String inStr = Serial1.readStringUntil('\n');
    inStr.trim();

    String command = inStr;
    String arg;
    for(int i = 0; i < inStr.length() - 1; i++) {
        if (inStr.substring(i,i+1) == " ") {
            command = inStr.substring(0,i);
            arg = inStr.substring(i+1,inStr.length());
            i = inStr.length();
        }
    }
    
    if (command != "PONG" || arg != "CLOUD")
      Serial.println("RECV: " + inStr);

//    Serial.println(command);

    if (command == "IN") {
      devMode = false;
      lcd.clear();
      lcd.print("Welcome,");
      lcd.setCursor(0, 1);
      lcd.print(arg + "!");

      delay(2000);
      waiting = false;
      entry = "";
      redrawLCD("");
    } else if (command == "OUT") {
      devMode = false;
      lcd.clear();
      lcd.print("Goodbye,");
      lcd.setCursor(0, 1);
      lcd.print(arg + "!");

      delay(2000);
      waiting = false;
      entry = "";
      redrawLCD("");
    } else if (command == "PONG") {
      didReceivePong = true;
      if (arg == "CLOUD") {
          // Serial.println("Photon Alive");
          if(lastPong != arg) {
            redrawLCD(entry);
            waiting = false;
          }
      } else {
          // Serial.println("Photon Connecting");
          if (!waiting || lastPong != arg) {
            lcd.clear();
            lcd.print("Connecting to");
            lcd.setCursor(0, 1);
            if (arg == "DISC")
              lcd.print("WiFi...");
            else
              lcd.print("Internet...");
            waiting = true;
          }
      }

      lastPong = arg;
    } else if (command == "ADMIN") {
      if (devAuth == kDevLoadingChar) {
        waiting = false;
        devAuth = kDevUnlockedChar;
        redrawLCD(entry);
      }
    } else if (command == "SUCCESS") {
      lcd.clear();
      lcd.print("Success!");
      lcd.setCursor(0, 1);
      lcd.print("[" + arg + "]");
      delay(1000);
      waiting = false;
      entry = "";
      redrawLCD(entry);
    } else if (command == "ERROR") {
      lcd.clear();
      lcd.print("Error:");
      lcd.setCursor(0, 1);
      lcd.print(arg);

      waiting = false;
      delay(4000);
      entry = "";
      redrawLCD("");
    } else {
    //  Serial.println(inStr);
    }
  }
// =================================================

  if (waiting) return;

  // put your main code here, to run repeatedly:
  char key = keypad.getKey();

  if (key){
    if (key == '*') {
      entry = "";
      redrawLCD(entry);
    } else if (key == '#') {
      entry = entry.substring(0, entry.length() - 1);
      redrawLCD(entry);
    } else if (key == 'A') {
      lcd.clear();
      lcd.print("Please wait...");
      Serial1.println("LOGSID " + entry);
      waiting = true;
      Serial.println(entry);
    } else if (key == 'B') {
    } else if (key == 'C') {
      Serial1.println("test");
      redrawLCD(entry);
    } else if (key == 'D') {
        if (devMode) {
          devAuth = kDevLockedChar;
        }

        devMode = !devMode;
        redrawLCD(entry);
    } else {
      entry += key;
      if (entry.length() >= 17)
        entry = entry.substring(1, 17);
      redrawLCD(entry);
    }
  }

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? ":0" : ":"));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  String uid = content.substring(1);

  if (devMode) {
    lcd.setCursor(15,0);

    if (devAuth == kDevUnlockedChar) {
      lcd.write((byte)2);
      waiting = true;
      Serial1.println("ADDUID " + entry + " " + uid);
      lcd.clear();
      lcd.print("Please wait...");
    }
    else if (devAuth == kDevLockedChar) {
      lcd.write((byte)1);
      devAuth = kDevLoadingChar;
      lcd.setCursor(0,0);
      lcd.print("Authorizing... ");
      Serial1.println("PERMUID " + uid);
    }
  } else {
    waiting = true;

    lcd.clear();
    lcd.print("Please wait...");

    Serial1.println("LOGUID " + uid);
  }
}
