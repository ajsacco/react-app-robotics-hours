#include <deprecated.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522v2.h>
#include <require_cpp11.h>

#include <arduino-timer.h>

#include <LiquidCrystal.h>

#include <Keypad.h>

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
    return;
  }
  
  dropCount++;
  if (dropCount < 2)
    return true;

  Serial.println("Photon Dead! Resetting...");

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
String lcdStatus = "Scan/enter ID:";

void updateEntry(String entry) {
  lcd.clear();
  lcd.print(lcdStatus);
  lcd.setCursor(0,1);
  lcd.print(entry);
}

void setup() {
  pinMode(55, OUTPUT);
  digitalWrite(55, HIGH);
  
  // put your setup code here, to run once:
  lcd.begin(16,2);
  lcd.print(lcdStatus);

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
    Serial.println("RECV: " + inStr);
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

//    Serial.println(command);

    bool upd = false;

    if (command == "IN") {
      lcd.clear();
      lcd.print("Welcome,");
      lcd.setCursor(0, 1);
      lcd.print(arg + "!");

      delay(2000);
      waiting = false;
      upd = true;
    } else if (command == "OUT") {
      lcd.clear();
      lcd.print("Goodbye,");
      lcd.setCursor(0, 1);
      lcd.print(arg + "!");

      delay(2000);
      waiting = false;
      upd = true;
    } else if (command == "PONG") {
      didReceivePong = true;
      if (arg == "CLOUD") {
          Serial.println("Photon Alive");
          if(lastPong != arg) {
            updateEntry(entry);
            waiting = false;
          }
      } else {
          Serial.println("Photon Connecting");
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
    } else if (command == "ERROR") {
      lcd.clear();
      lcd.print("Error:");
      lcd.setCursor(0, 1);
      lcd.print(arg);

      waiting = false;
      upd = true;
      delay(4000);
    } else {
//      Serial.println(inStr);
    }
    
    if (upd) {
      entry = "";
      updateEntry("");
    }
  }
// =================================================

  if (waiting) return;

  // put your main code here, to run repeatedly:
  char key = keypad.getKey();

  if (key){
    if (key == '*') {
      entry = "";
      updateEntry(entry);
    } else if (key == '#') {
      entry = entry.substring(0, entry.length() - 1);
      updateEntry(entry);
    } else if (key == 'A') {
      lcd.clear();
      lcd.print("Please wait...");
      Serial1.println("LOGSID " + entry);
      waiting = true;
      Serial.println(entry);
    } else if (key == 'C') {
      Serial1.println("test");
      updateEntry(entry);
    } else {
      entry += key;
      if (entry.length() >= 17)
        entry = entry.substring(1, 17);
      updateEntry(entry);
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

  lcd.clear();
  lcd.print("Please wait...");

  waiting = true;

  Serial1.println("LOGUID " + uid);
}