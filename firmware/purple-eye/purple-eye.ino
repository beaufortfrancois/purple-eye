/*********************************************************************
  This is an example for our nRF51822 based Bluefruit LE modules

  Pick one up today in the adafruit shop!

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  MIT license, check LICENSE for more information
  All text above, and the splash screen below must be included in
  any redistribution
*********************************************************************/

#include <Arduino.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Servo.h"

#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

Servo rightFoot, rightLeg, leftFoot, leftLeg;
int rightFootValue = 90, rightLegValue = 90, leftFootValue = 90, leftLegValue = 90;

void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void updateServos() {
  rightFoot.write(rightFootValue + 15);
  rightLeg.write(rightLegValue);
  leftFoot.write(leftFootValue - 10);
  leftLeg.write(leftLegValue);
}

int32_t eyebotServiceId;
int32_t eyebotServosCharId;

void setup(void)
{
  boolean success;

  rightLeg.attach(5);
  rightFoot.attach(11);
  leftFoot.attach(9);
  leftLeg.attach(10);
  updateServos();

  Serial.begin(115200);
  Serial.println("Start!");

  randomSeed(micros());

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(true) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  /* Perform a factory reset to make sure everything is in a known state */
  Serial.println(F("Performing a factory reset: "));
  if (! ble.factoryReset() ) {
    error(F("Couldn't factory reset"));
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Change the device name to make it easier to find */
  Serial.println("Setting device name");

  if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=BlueEye")) ) {
    error(F("Could not set device name?"));
  }

  /* Add the Heart Rate Service definition */
  /* Service ID should be 1 */
  Serial.println(F("Adding the service definition (UUID = 0x5100): "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x5100"), &eyebotServiceId);
  if (! success) {
    error(F("Could not add service"));
  }

  /* Chars ID for Measurement should be 1 */
  Serial.println(F("Adding the characteristic (UUID = 0x5200): "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x5200, PROPERTIES=0x0A, MIN_LEN=4, MAX_LEN=4, VALUE=5A-5A-5A-5A"), &eyebotServosCharId);
  if (! success) {
    error(F("Could not add HRM characteristic"));
  }

  /* Add the Heart Rate Service to the advertising data (needed for Nordic apps to detect the service) */
  //  Serial.print(F("Adding Heart Rate Service UUID to the advertising payload: "));
  ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-05-02-00-51-0a-18") );

  /* Reset the device for the new service setting changes to take effect */
  Serial.print(F("Performing a SW reset (service changes require a reset): "));
  ble.reset();

  Serial.println();
}

/** Send randomized heart rate data continuously **/
void loop(void)
{
  /* Command is sent when \n (\r) or println is called */
  /* AT+GATTCHAR=CharacteristicID,value */
  ble.print("AT+GATTCHAR=");
  ble.println(eyebotServosCharId);

  /* Check if command executed OK */
  char buf[100] = {0};
  ble.readline(buf, sizeof(buf));
  sscanf(buf, "%02x-%02x-%02x-%02x", &rightLegValue, &rightFootValue, &leftFootValue, &leftLegValue);
  updateServos();
}

