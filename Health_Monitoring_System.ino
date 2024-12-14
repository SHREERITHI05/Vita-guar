#include <Adafruit_GPS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include <SoftwareSerial.h>
SoftwareSerial mySerial(6, 7); // RX, TX

const int lm35Pin = A0;
const int spo2Pin = A1;
const int bpPin = A2;
const int heartbeatPin = 2;

const String phoneNumber = "+917010653301";

byte index = 0;
char latt, longt; 
char inData[78];
char inChar = -1;
bool flag = false;

String message = "";

int t = 0, lt = 0;
int heartbeat = 0;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  pinMode(lm35Pin, INPUT);
  pinMode(spo2Pin, INPUT);
  pinMode(heartbeatPin, INPUT);
  pinMode(bpPin, INPUT);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("     WELCOME    ");
  delay(2000);

  lcd.setCursor(0, 0);
  lcd.print("HEALTH MONITORING");
  lcd.setCursor(0, 1);
  lcd.print("     SYSTEM    ");
  delay(2000);

  initializeGSM();
  lcd.clear();
}

void loop()
{
  // Read sensors
  float temperature = analogRead(lm35Pin) * 0.488; // LM35: 10mV/°C, 5V / 1024 steps
  int spo2Val = analogRead(spo2Pin);
  int spo2Value = map(spo2Val, 0, 1023, 0, 100);
  int heartbeatValue = digitalRead(heartbeatPin);
  int bpVal = analogRead(bpPin);
  int bpValue = map(bpVal, 0, 1023, 0, 200);

  if (heartbeatValue == HIGH)
  {
    heartbeat = heartbeat + 1;
  }

  if ((heartbeat < 50) && (lt >= 59) || (lt <= 59) && (heartbeat > 100))
  {
    message = "";

    if (heartbeat < 50)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" LOW HEART BEAT");
      delay(2000);
      message += "LOW HEART BEAT \n";
    }

    if (heartbeat > 100)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("HIGH HEART BEAT");
      delay(2000);
      message += "HIGH HEART BEAT \n";
    }

    flag = 1;
    Comp("$GPRMC");
    // Send SMS
    sendSMS(phoneNumber, message);
    heartbeat = 0;
  }

  if (temperature > 70)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TEMPERATURE IS ");
    lcd.setCursor(0, 1);
    lcd.print("      HIGH      ");
    delay(2000);
    message = "";
    message += "TEMPERATURE IS HIGH\n";
    flag = 1;
    Comp("$GPRMC");
    // Send SMS
    sendSMS(phoneNumber, message);
  }

  if ( spo2Value >= 50)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SPO2 IS HIGH");
    delay(2000);
    message = "";
    message += "SPO2 IS HIGH\n";
    flag = 1;
    Comp("$GPRMC");
    // Send SMS
    sendSMS(phoneNumber, message);
  }

  if (bpValue >= 130)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BP IS HIGH");
    delay(2000);
    message = "";
    message += "BP IS HIGH\n";
    flag = 1;
    Comp("$GPRMC");
    // Send SMS
    sendSMS(phoneNumber, message);
  }

  // Display data on LCD

  lcd.setCursor(0, 0); // Set cursor to column 0, row 0
  lcd.print("T: ");
  lcd.print(temperature, 1);
  lcd.print("C");
  lcd.print(" HB: ");
  lcd.print(heartbeat);
  lcd.print("  ");

  lcd.setCursor(0, 1); // Set cursor to column 0, row 1
  lcd.print("O2:");
  lcd.print(spo2Value);
  lcd.print(" BP:");
  lcd.print(bpValue);
  lcd.print("Hg ");
  lcd.print(lt);
  lcd.print("  ");

  if (lt >= 59)
  {
    // Create SMS message
    message = "Sensor Data:\n";
    message += "Temperature: " + String(temperature) + " C\n";
    message += "SPO2: " + String(spo2Value) + "\n";
    message += "Heartbeat: " + String(heartbeat) + "\n";
    message += "Blood Pressure: " + String(bpValue) + "\n";

    Serial.print(message);
    // Send SMS
    sendSMS(phoneNumber, message);
  }

  t += 1;
  lt = t / 2;
  // Delay between readings
  delay(500); // Delay for 1 sec


  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("SPO2: ");
  Serial.println(spo2Value);

  Serial.print("Heartbeat: ");
  Serial.println(heartbeat);

  Serial.print("Blood Pressure: ");
  Serial.println(bpValue);

}


// Initialize GSM module
void initializeGSM()
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("GSM INITIALIZE...");

  mySerial.println("AT");
  delay(2000);

  mySerial.println("AT+CMGF=1"); // Set SMS mode to text
  delay(2000);
}


// Send SMS
void sendSMS(String phoneNumber, String message)
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("SENDING SMS >>>");

  mySerial.println("AT+CMGS=\"" + phoneNumber + "\""); // Set recipient's phone number
  delay(2000);

  mySerial.println(message); // Send the message
  delay(2000);

  mySerial.write(26); // Send Ctrl+Z to end the message
  delay(2000);
  lcd.clear();
  heartbeat = 0;
  t = 0;
  lt = 0;
}


char Comp(char* This)
{
  while (flag)
  {
    while (mySerial.available() > 0)
    {
      inChar = mySerial.read(); // Read a character
      if (inChar == 'C')
      {
        while (flag)
        {
          inChar = mySerial.read();
          inData[index] = inChar;
          index++;
          if (index > 70)
          {
            index = 0;
            if (inData[24] == 'N')
            {
              flag = 0;
              lcd.clear();
              lcd.setCursor(1, 0);
              lcd.print("Signal Recvd...");
              //mySerial.println("Signal Recvd...");
              delay(500);
              lcd.clear();
              delay(50);
              lcd.setCursor(1, 0);
              for (index = 13; index < 26; index++)
              {
                //mySerial.print(inData[index]);
                lcd.print(inData[index]);
                delay(100);
              }
              lcd.setCursor(1, 1);
              for (index = 26; index < 40; index++)
              {
                //mySerial.print(inData[index]);
                lcd.print(inData[index]);
                delay(100);
              }

              message += "Location:";
              message += "\nLat: ";
              for (index = 13; index < 26; index++)
              {
                latt = (inData[index]);
                message += latt;
                delay(100);
              }

              message += "\nLon: ";
              for (index = 26; index < 40; index++)
              {
                longt = (inData[index]);
                message += longt;
                delay(100);
              }
            }
          }
        }
      }
    }
  }
}
