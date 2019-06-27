#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#include<SoftwareSerial.h>
#include<LiquidCrystal.h>
//SoftwareSerial mySerial(10,11);//finger
#define mySerial Serial1
SoftwareSerial serial(50, 24); //gsm

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
char msg[70];
String text;
char a[13];
uint8_t id;
char otp[4], textMessage[70];
const byte ROWS = 4;
const byte COLS = 4;
char f, k;
int relay = A8;
int v, w;
int c = 0, i = 0, j = 0, t = 0, y, m = 0;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void setup()
{

  serial.begin(9600);
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
  // AT command to set SIM900 to SMS mode
  serial.println("AT\r");
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  //Serial.println("AT\r");
  serial.println("AT+CMGF=1\r");
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  // Serial.println("AT+CMGF=1\r");
  // Set module to send SMS data to serial out upon receipt
  serial.println("AT+CNMI=2,2,0,0,0\r");
  //Serial.println("AT+CNMI=2,2,0,0,0\r");

  delay(1000);
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");

  } else {
    Serial.println("Did not find fingerprint sensor :(");

    while (1) {
      delay(1);
    }
  }

}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop()
{

  if (serial.available() > 0)
  {
    Serial.write(serial.read());
    msg_receive();
  }
  lcd.setCursor(0, 0);
  lcd.print("                   ");

  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 0);
  lcd.print("place your");

  lcd.setCursor(0, 1);
  lcd.print("finger & press *");

m: f = customKeypad.getKey();

  if (f) {
    Serial.println(f);
  }
  if (f == '*')
  {
    finger.getTemplateCount();
    Serial.print("Sensor contains ");
    Serial.print(finger.templateCount);
    Serial.println(" templates");
    Serial.println("Waiting for valid finger...");
    delay(500);
    getFingerprintID();
    delay(50);
    goto m;
  }
  else if (f == '#')
  {
    Serial.println("Ready to enroll a fingerprint!");
    Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
    id = readnumber();
    if (id == 0) {// ID #0 not allowed, try again!
      return;
    }
    Serial.print("Enrolling ID #");
    Serial.println(id);

    getFingerprintEnroll();
    goto m;
  }
  else if (f == 'D')
  {
    Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");
    uint8_t id = readnumber();
    if (id == 0) {// ID #0 not allowed, try again!
      return;
    }

    Serial.print("Deleting ID #");
    Serial.println(id);

    deleteFingerprint(id);
  }

  else if (f == 'C')
  { /*

    */
    i = 0;
    lcd.setCursor(0, 0);
    lcd.print("                 ");
    lcd.setCursor(0, 1);
    lcd.print("                 ");
    lcd.setCursor(0, 0);
    lcd.print("enter otp");
    Serial.println("enter otp");
    while (i < 4)
    {
      char customKey = customKeypad.getKey();
      if (customKey) {
        Serial.println(customKey);
        Serial.flush();
        // if(k!= NO_KEY)
        otp[i++] = customKey;
      }




      delay(100);


    }
    otp[4] = '\0';
    Serial.print(otp);
    int r = atoi(otp);
    if (v == r)
    {
      Serial.println("matches");
      lcd.setCursor(0, 1);
      lcd.print(otp);
      lcd.setCursor(6, 1);
      lcd.print("matches");
      digitalWrite(relay, LOW);
      delay(3000);
      digitalWrite(relay, HIGH);
      gsm();

    }
    if (w == r)
    {
      Serial.println("matches");
      lcd.setCursor(0, 1);
      lcd.print(otp);
      lcd.setCursor(6, 1);
      lcd.print("matches");
      digitalWrite(relay, LOW);
      delay(3000);
      digitalWrite(relay, HIGH);
      gsm();
    }

    delay(1000);

  }
}




uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  int z = id;
  Serial.println(id);


  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }


  p = finger.storeModel(id);
  Serial.println("Stored!");

  if (p == FINGERPRINT_OK) {


    Serial.print("ID "); Serial.println(id);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}


uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    lcd.setCursor(0, 0);
    lcd.print("                              ");
    lcd.setCursor(0, 1);
    lcd.print("                          ");
    lcd.setCursor(0, 0);
    lcd.print("matches");
    digitalWrite(relay, LOW);
    delay(3000);
    digitalWrite(relay, HIGH);
    gsm();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    lcd.setCursor(0, 0);
    lcd.print("                              ");
    lcd.setCursor(0, 1);
    lcd.print("                          ");
    lcd.setCursor(0, 0);
    lcd.print("doesn't match");
    delay(3000);
    c = c + 1;
    if (c == 3)
    {
      v = random(1000, 9999);
      gsm();
    }
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}


uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    return p;
  }
}


void gsm()
{
  serial.println("AT+CMGF=1\r");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  serial.println("AT+CMGS=\"+919544476660\"\r"); // Replace x with mobile number
  delay(1000);
  if (c == 3)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("fail");
    delay(2000);
    serial.println("fail");
    serial.println(v);
    delay(1000);
    serial.write((char)26);// ASCII code of CTRL+Z
    delay(2000);
    lcd.setCursor(0, 0);
    lcd.print("press C &enter");
    lcd.setCursor(0, 1);
    lcd.print("otp to unlock");
    delay(4000);
    c = 0;
  }
  else if (c != 3)
  {
    serial.println("success");
    delay(1000);
    serial.write((char)26);// ASCII code of CTRL+Z
    delay(2000);
  }

}

void msg_lock()
{
  serial.println("AT+CMGF=1\r");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  serial.println("AT+CMGS=\"+919544476660\"\r"); // Replace x with mobile number
  delay(1000);
  if (m == 1)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("fail");
    delay(2000);
    serial.println(w);
    delay(1000);
    serial.write((char)26);// ASCII code of CTRL+Z
    delay(2000);
    lcd.setCursor(0, 0);
    lcd.print("press C &enter");
    lcd.setCursor(0, 1);
    lcd.print("otp to unlock");
    delay(4000);
    m = 0;
  }
  delay(2000);
}

void msg_receive()
{

  text = serial.readString();
  Serial.print(text);
  delay(10);
  text.toCharArray(textMessage, 70);
  Serial.println(textMessage);
  i = 50;
  y = 1;
  for (j = 0; j < 4; j++)
  {
    a[j] =  textMessage[i];
    //Serial.print(a[j]);
    i++;

  }
  a[j + 1] = "/0";
  Serial.print(a);
  if (strcmp(a, "0000") == 0)
  { lcd.setCursor(0, 0);
    lcd.print("                              ");
    lcd.setCursor(0, 1);
    lcd.print("                                ");
    lcd.setCursor(0, 0);
    lcd.print("stopping");
    delay(3000);
    w = random(1000, 9999);
    m = m + 1;
    msg_lock();

  }
}
