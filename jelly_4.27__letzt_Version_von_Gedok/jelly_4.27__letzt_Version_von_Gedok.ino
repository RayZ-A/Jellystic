#include "mp3Player.h"

const int VENTILATOR_PIN = A5; // 风扇
const int PIR_PIN = 8; // 红外热释电

enum PirStates
{
  PIR_STATE_LOW = 0, // first pir state
  PIR_STATE_HIGH,    // second pir state
  PIR_STATES         // number of pir states
};

const int MAX_LEDS = 2;
const int USED_LEDS = 2;
const int LED_PIN[MAX_LEDS] = { 9, 10 }; // led灯
const int LED_UPDATE_INTERVAL[MAX_LEDS][PIR_STATES] = { { 12, 53 }, { 15, 41 } }; // change LED every 15 ms on low pin, every 5ms on high pin
const int MIN_LUMINANCE[MAX_LEDS][PIR_STATES] = { { 1, 190 }, { 2, 189 } };  // minimum luminance per LED per PIR pin value
const int MAX_LUMINANCE[MAX_LEDS][PIR_STATES] = { { 2, 250 }, { 3, 254 } }; // maximum luminance per LED per PIR pin value
const int VENTILATOR_UPDATE_INTERVAL[PIR_STATES] = { 8000, 15000 }; // in ms
const int DEBUG_UPDATE_INTERVAL = 500; // status messages sent every half second

unsigned long lastTime, ledTime[MAX_LEDS], ventilatorTime, debugTime;
int lastPirState = PIR_STATE_LOW;
byte luminance[MAX_LEDS] = { 0, 0 };
int changeLight[MAX_LEDS] = { 1, 1 };

void setup() {
  Serial.begin(9600);

  pinMode(PIR_PIN, INPUT); // Setzen Sie den PIR-Pin als Eingang
  pinMode(LED_PIN, OUTPUT); // Setzen Sie den LED-Pin als Ausgang
  pinMode(VENTILATOR_PIN, OUTPUT);

  // initiailize values of our output pins
  for (int led = 0; led < USED_LEDS; led++)
  {
    analogWrite(LED_PIN[led], luminance[led]);
    ledTime[led] = 0;
  }
  digitalWrite(VENTILATOR_PIN, HIGH);

  setupMp3();

  lastTime = millis();
  ventilatorTime = debugTime = 0;
}


void loop() {
  int elapsedTime = millis() - lastTime;
  lastTime = millis();
  // simply discard loop if negative value
  if (elapsedTime < 0) return;

  ventilatorTime += elapsedTime;
  debugTime += elapsedTime;
  for (int led = 0; led < USED_LEDS; led++)
    ledTime[led] += elapsedTime;

  // read the state of the PIR-sensor
  int pirState;
  if (digitalRead(PIR_PIN) == LOW)
    pirState = PIR_STATE_LOW; // 0
  else
    pirState = PIR_STATE_HIGH; // 1

  for (int led = 0; led < USED_LEDS; led++)
  {
    while (ledTime[led] >= LED_UPDATE_INTERVAL[led][pirState]) {
      ledTime[led] -= LED_UPDATE_INTERVAL[led][pirState];

      // bounce LED brightness between MIN_LUMINANCE and MAX_LUMINANCE
      if (luminance[led] >= MAX_LUMINANCE[led][pirState]) {
        changeLight[led] = -1;
      } else if (luminance[led] <= MIN_LUMINANCE[led][pirState]) {
        changeLight[led] = 1;
      }

      luminance[led] += changeLight[led];
      analogWrite(LED_PIN[led], luminance[led]);

    } // while elapsed time is bigger than led update interval
  } // for all leds

  if (lastPirState != pirState ||
      ventilatorTime >= VENTILATOR_UPDATE_INTERVAL[pirState]) {
    
    if (lastPirState != pirState)
      ventilatorTime = 0;
    else
      ventilatorTime %= VENTILATOR_UPDATE_INTERVAL[pirState];

    if (pirState == PIR_STATE_LOW)
    {
      // flip ventilator output
      if (digitalRead(VENTILATOR_PIN))
        digitalWrite(VENTILATOR_PIN, LOW);
      else
        digitalWrite(VENTILATOR_PIN, HIGH);
    } else {
      // flip ventilator output
      // if (digitalRead(VENTILATOR_PIN))
      //   digitalWrite(VENTILATOR_PIN, LOW);
      // else
      //   digitalWrite(VENTILATOR_PIN, HIGH);
     
      // set ventilator to high
      digitalWrite(VENTILATOR_PIN, HIGH);
    }
  }

  if (debugTime >= DEBUG_UPDATE_INTERVAL) {
    debugTime %= DEBUG_UPDATE_INTERVAL;
    
    if (digitalRead(PIR_PIN)) {
      Serial.print("人体检测传感器:高电平");
    } else {
      Serial.print("人体检测传感器:低电平");
    }
    Serial.print("   ");
    if (digitalRead(VENTILATOR_PIN)) {
      Serial.print("继电器:高电平");
    } else {
      Serial.print("继电器:低电平");
    }
    Serial.print("   ");
    Serial.print("led亮度:");
    for (int led = 0; led < USED_LEDS; led++)
    {
      if (led == USED_LEDS-1)
      {
        Serial.println(luminance[led]);
      }
      else
      {
        Serial.print(luminance[led]);
        Serial.print(", ");
      }
    }
  }

  if (pirState != lastPirState)
  {
    playMp3(pirState);
  }

  lastPirState = pirState;

  loopMp3();
}
