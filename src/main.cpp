#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include <Keypad.h>
#include <WiFiManager.h>

//================ TELEGRAM =================
#define BOT_TOKEN "8889153732:AAEoyP4BDjXEfMS10k21CFKpi6-maHpLjsQ"
#define CHAT_ID "6517222458"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

//================ HC-SR04 =================
#define TRIG_PIN 5
#define ECHO_PIN 18

//================ BUZZER =================
#define BUZZER_PIN 4

//================ RELAY SMART LOCK =================
#define RELAY_PIN 27

//================ GPS =================
TinyGPSPlus gps;
HardwareSerial GPSserial(2);

//================ KEYPAD =================
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
    {'1', '4', '7', '*'},
    {'2', '5', '8', '0'},
    {'3', '6', '9', '#'},
    {'A', 'B', 'C', 'D'}};

byte rowPins[ROWS] = {13, 14, 15, 19};
byte colPins[COLS] = {21, 23, 26, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//================ PASSWORD =================
String password = "1234";
String inputPassword = "";

bool lockOpened = false;
bool alarmSent = false;

unsigned long lockTimer = 0;

long duration;
float distanceCm;

unsigned long lastGPS = 0;
unsigned long lastTelegramCheck = 0;
unsigned long lastAlarmTime = 0;
unsigned long lastAlarmSent = 0;

const unsigned long TELEGRAM_INTERVAL = 1000;
const unsigned long ALARM_COOLDOWN = 6000;

//=====================================================

void sendTelegram(String msg)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    bot.sendMessage(CHAT_ID, msg, "");
  }
}

//=====================================================

void openLock()
{

lockOpened = true;

  digitalWrite(RELAY_PIN,LOW);

  lockTimer = millis();

  sendTelegram("🔓 Smart Lock Open");

  Serial.println("SMART LOCK OPEN");
}

//=====================================================

void handleTelegram()
{

  int numNewMessages =
      bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages)
  {

    for (int i = 0; i < numNewMessages; i++)
    {

      String text = bot.messages[i].text;

      // START

      if (text == "/start")
      {

        String msg;

        msg = "🤖 SMART DONATION BOX\n\n";

        msg += "/status - Status Sistem\n";

        msg += "/lokasi - Lokasi Kotak Amal\n";

        msg += "/lock - Status Smart Lock\n";

        msg += "/open - Buka Smart Lock";

        bot.sendMessage(CHAT_ID, msg, "");
      }

      // STATUS

      else if (text == "/status")
      {

        String msg;

        msg = "📦 STATUS\n\n";

        msg += "Distance : ";

        msg += String(distanceCm);

        msg += " cm\n";

        if (distanceCm >= 30)
          msg += "Alarm : AKTIF 🚨\n";
        else
          msg += "Alarm : NORMAL ✅\n";

        if (gps.location.isValid())
          msg += "GPS : FIXED\n";
        else
          msg += "GPS : WAITING\n";

        if (lockOpened)
          msg += "Lock : OPEN";
        else
          msg += "Lock : LOCKED";

        bot.sendMessage(CHAT_ID, msg, "");
      }

      // GPS

      else if (text == "/lokasi")
      {

        if (gps.location.isValid())
        {

          String maps;

          maps = "📍 Lokasi Kotak Amal\n\n";

          maps += "Latitude : ";

          maps += String(gps.location.lat(), 6);

          maps += "\nLongitude : ";

          maps += String(gps.location.lng(), 6);

          maps += "\n\n";

          maps += "https://maps.google.com/?q=";

          maps += String(gps.location.lat(), 6);

          maps += ",";

          maps += String(gps.location.lng(), 6);

          bot.sendMessage(CHAT_ID, maps, "");
        }
        else
        {

          bot.sendMessage(CHAT_ID, "❌ GPS belum mendapatkan sinyal", "");
        }
      }

      // LOCK

      else if (text == "/lock")
      {

        if (lockOpened)
        {

          bot.sendMessage(
              CHAT_ID,
              "🔓 Smart Lock OPEN",
              "");
        }
        else
        {

          bot.sendMessage(
              CHAT_ID,
              "🔒 Smart Lock LOCKED",
              "");
        }
      }

      // OPEN

      else if (text == "/open")
      {

        openLock();
      }
    }

    numNewMessages =
        bot.getUpdates(bot.last_message_received + 1);
  }
}
unsigned long buzzerTimer = 0;
      bool buzzerState = false;
      int buzzerCount = 0;
//=====================================================

void setup()
{

  Serial.begin(115200);

  // HC-SR04

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Buzzer

  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, HIGH);

  // Relay

  pinMode(RELAY_PIN, OUTPUT);

  // Solenoid terkunci

  digitalWrite(RELAY_PIN, HIGH);

  // GPS

  GPSserial.begin(
      9600,
      SERIAL_8N1,
      16,
      17);

  // WIFI

  WiFiManager wm;

  bool res;
  res = wm.autoConnect("Smart Donation Box","12345678");

if(!res){

  Serial.println("WiFi Failed");

}
else{

  Serial.println("WiFi Connected");

  client.setInsecure();

  client.setTimeout(100);

  delay(500);

  sendTelegram("✅ Smart Donation Box Online");

  Serial.println("SYSTEM READY");
}
}

//=====================================================

void loop()
{
  if(buzzerState){

  if(millis() - buzzerTimer >= 1000){

    digitalWrite(BUZZER_PIN, HIGH); // buzzer OFF

    buzzerState = false;
  }
}
  // GPS

  while (GPSserial.available())
  {

    gps.encode(
        GPSserial.read());
  }

  // TELEGRAM

  if (millis() > lastTelegramCheck + TELEGRAM_INTERVAL)
  {

    handleTelegram();

    lastTelegramCheck = millis();
  }

  // ULTRASONIC

  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(
      ECHO_PIN,
      HIGH,
      10000);

  distanceCm =
      duration * 0.0343 / 2;

  Serial.print("Distance : ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  // ALARM

  // ALARM PERGESERAN KOTAK AMAL

if(distanceCm >= 30){

    digitalWrite(BUZZER_PIN, LOW);

    if(!alarmSent &&
   millis() - lastAlarmTime > ALARM_COOLDOWN){

        String msg;

        msg = "🚨 PERINGATAN KEAMANAN!\n\n";

        msg += "Kotak Amal terindikasi mengalami perpindahan tanpa izin.\n\n";

        msg += "📏 Jarak Pergeseran : ";

        msg += String(distanceCm,1);

        msg += " cm\n\n";

        if(gps.location.isValid()){

            msg += "📍 Lokasi Terkini :\n";

            msg += "https://maps.google.com/?q=";

            msg += String(gps.location.lat(),6);

            msg += ",";

            msg += String(gps.location.lng(),6);

            msg += "\n\nSegera lakukan pemeriksaan.";
        }

        sendTelegram(msg);

        lastAlarmSent = millis();
        alarmSent = true;
    }

}else{

    digitalWrite(BUZZER_PIN, HIGH);

    alarmSent = false;
}

  // KEYPAD

  char key = keypad.getKey();

if(key){

  Serial.print("Key : ");
  Serial.println(key);

  if(key == '#'){

    Serial.print("Input Password : ");
    Serial.println(inputPassword);

    if(inputPassword == password){

      Serial.println("ACCESS GRANTED");

      // buzzer 2x

      
      // buka smart lock
      if(buzzerState){

      if(millis()-buzzerTimer > 100){

          buzzerTimer = millis();

          digitalWrite(BUZZER_PIN,!digitalRead(BUZZER_PIN));

          buzzerCount++;

          if(buzzerCount>=4){

              digitalWrite(BUZZER_PIN,HIGH);

              buzzerState=false;

              buzzerCount=0;
          }
      }
    }
      buzzerState = true;
      lockOpened = true;

      digitalWrite(RELAY_PIN, LOW);

      sendTelegram("🔓 Smart Lock Open");

      Serial.println("SMART LOCK OPEN");


      // tutup lagi

      digitalWrite(RELAY_PIN, HIGH);

      lockOpened = false;

      sendTelegram("🔒 Smart Lock Locked");

      Serial.println("SMART LOCK LOCKED");

    }

    else{

      Serial.println("ACCESS DENIED");

      digitalWrite(BUZZER_PIN, LOW);

      buzzerTimer = millis();
      buzzerState = true;

      sendTelegram("❌ Password Salah");
    }

    inputPassword = "";
  }

  else if(key=='*'){

    inputPassword = "";

    Serial.println("PASSWORD CLEARED");
  }

  else{

    inputPassword += key;

    Serial.print("Password : ");

    Serial.println(inputPassword);
  }
}

  // GPS INFO

  if (millis() - lastGPS > 100)
  {

    lastGPS = millis();

    Serial.println("\n===== GPS =====");

    Serial.print("Satellites : ");

    Serial.println(
        gps.satellites.value());

    if (gps.location.isValid())
    {

      Serial.print("Latitude : ");

      Serial.println(
          gps.location.lat(),
          6);

      Serial.print("Longitude : ");

      Serial.println(
          gps.location.lng(),
          6);

      Serial.print("Google Maps : ");

      Serial.print(
          "https://maps.google.com/?q=");

      Serial.print(
          gps.location.lat(),
          6);

      Serial.print(",");

      Serial.println(
          gps.location.lng(),
          6);
    }

    else
    {

      Serial.println(
          "Waiting GPS Fix...");
    }
  }

  // AUTO LOCK SETELAH 5 DETIK

  if(lockOpened){

    if(millis() - lockTimer >= 5000){

      digitalWrite(RELAY_PIN, HIGH);

      lockOpened = false;

      sendTelegram("🔒 Smart Lock Locked");

      Serial.println("SMART LOCK LOCKED");
    }

  }

  delay(100);

}