/* Copyright (c) 2013 Daniël W. Crompton <daniel@specialbrands.net>,
 *     Special Brands <info@specialbrands.net>
 *
 * Includes code written by Limor Fried/Ladyada for Adafruit Industries.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <Adafruit_FONA.h>

#include "fona.h"

#include <SoftwareSerial.h>

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

void fona_setup() {
  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }

  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

  fona_enable_gprs();
}

void fona_send(char *message) {
  char sendto[21];
  Serial.println(EEPROM.get(1000, sendto));
  if (!fona.sendSMS(sendto, message)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("Sent!"));
  }
}

void fona_set_number(char * number) {
  EEPROM.put(1000, number);

}

void fona_enable_gprs() {
  if (!fona.enableGPRS(true)) {
    Serial.println(F("Failed to turn on GPRS"));
  }
}

void fona_read_sms(bool (*callback)(char * replybuffer)) {
  // read all SMS
  int8_t smsnum = fona.getNumSMS();
  uint16_t smslen;
  int8_t smsn;
  char replybuffer[255];
  char sender[128];

  smsn = 1; // zero indexed

  for ( ; smsn <= smsnum; smsn++) {
    Serial.print(F("\n\rReading SMS #")); Serial.print(smsn);
    Serial.print(F(" of ")); Serial.println(smsnum);
    if (!fona.readSMS(smsn, replybuffer, 250, &smslen)) {  // pass in buffer and max len!
      Serial.println(F("Failed!"));
      break;
    }
    // if the length is zero, its a special case where the index number is higher
    // so increase the max we'll look at!
    if (smslen == 0) {
      Serial.println(F("[empty slot]"));
      smsnum++;
      continue;
    }

    Serial.print(F("***** SMS #")); Serial.print(smsn);
    Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
    if(callback(replybuffer) == true) {
      // delete SMS
      Serial.println(F("Delete SMS"));
      if(fona.getSMSSender(smsn, sender, 120)) {
        if( strcmp(sender, "65@6461666@6+656") == 0) {
          fona.deleteSMS(smsn);
          continue;
        }
      }
#if 1
      if (fona.deleteSMS(smsn)) {
        Serial.println(F("Delete OK!"));
      } else {
        Serial.println(F("Couldn't delete"));
      }
#endif
    } else {
      // delete skipped
      Serial.println(F("Skip SMS"));
    }
    Serial.println(F("*****"));
  }
}

bool fona_post_http(char *url, char *data) {
  // Post data to website
  uint16_t statuscode;
  int16_t length;

  int fona_ret = fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length);
  if (!fona_ret) {
    Serial.println("Failed!");
    return false;
  }
  while (length > 0) {
    while (fona.available()) {
      char c = fona.read();

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
      loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
      UDR0 = c;
#else
# if (DEBUG >= 100)
      Serial.write(c);
# endif
#endif

      length--;
      if (! length) break;
    }
  }
  fona.HTTP_POST_end();
}

