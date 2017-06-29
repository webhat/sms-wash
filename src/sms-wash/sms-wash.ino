/* Copyright (c) 2013 Daniël W. Crompton <daniel@specialbrands.net>,
  *     Special Brands <info@specialbrands.net>
  */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_FONA.h>

#include "fona.h"

int ledPin = 13;

void setup() {
#if (DEBUG >= 1000)
  while (!Serial);
#endif /* DEBUG */

  Serial.begin(115200);

  // Define the LED pin as Output
  pinMode (ledPin, OUTPUT);

  // FONA Setup
  fona_setup();
}

bool callback(char *replybuffer) {
    Serial.println(replybuffer);

    return true;
}

void loop() {
  fona_read_sms(callback);
  delay(10);
}

