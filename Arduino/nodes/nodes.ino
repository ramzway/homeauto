/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 */

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

// Pin configurations - From DuinodeV3
const int led_red = 3;
const int led_yellow = 4;
const int led_green = 5;
const int button_a = 6;

const int rf_ce = 8;
const int rf_csn = 7;

const int temp_pin = 2; // analog
const int voltage_pin = 3; // analog

// 1.1V internal reference after 1M/470k divider, in 8-bit fixed point
const unsigned voltage_reference = 0x371;

const unsigned num_measurements = 8;

// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(rf_ce, rf_csn);

// Network uses that radio
RF24Network network(radio);

// Address of our node
const uint16_t this_node = 0;

// Address of the other node
const uint16_t other_node = 1;

unsigned long currenttemperature;
unsigned long currentvoltage;
unsigned long currentpinstatus;
unsigned long currenttime;

// Structure of command
struct command_t
{
  unsigned long messageid;
  unsigned int commandtype;
  unsigned int commanddata;
};

// Structure of response
struct response_t
{
  unsigned long messageid;
  unsigned long temperature;
  unsigned long battvoltage;
  unsigned int pinstatus; 
};

unsigned long getTemperature() {
    int i = num_measurements;
    uint32_t reading = 0;
    while(i--)
      reading += analogRead(temp_pin);

    // Convert the reading to celcius*256
    // This is the formula for MCP9700.
    // C = reading * 1.1
    // C = ( V - 1/2 ) * 100
    return ( ( ( reading * 0x120 ) - 0x800000 ) * 0x64 ) >> 16;
}

unsigned long getVoltage() { 
      // Take the voltage reading 
    int i = num_measurements;
    uint32_t reading = 0;
    while(i--)
      reading += analogRead(voltage_pin);

    // Convert the voltage reading to volts*256
    return ( reading * voltage_reference ) >> 16; 
}

unsigned int getPinStatus() {
  int idx = 0 ;
  idx = digitalRead(led_red);
  idx = digitalRead(led_yellow) << 2 + idx;
  idx = digitalRead(led_green) << 4 + idx;
  idx = digitalRead(button_a) << 8 + idx;
}

void setup(void)
{
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  currenttemperature = getTemperature();
  currentvoltage = getVoltage();
  
  pinMode(led_red, OUTPUT);
  pinMode(led_yellow, OUTPUT);
  pinMode(led_green, OUTPUT);
  digitalWrite(led_red, HIGH);
  digitalWrite(led_yellow, HIGH);
  digitalWrite(led_green, HIGH);
  delay(250);
  digitalWrite(led_red, LOW);
  digitalWrite(led_yellow, LOW);
  digitalWrite(led_green, LOW);
  delay(250);
  digitalWrite(led_red, HIGH);
  digitalWrite(led_yellow, HIGH);
  digitalWrite(led_green, HIGH);
  delay(500);
  digitalWrite(led_red, LOW);
  digitalWrite(led_yellow, LOW);
  digitalWrite(led_green, LOW);
  delay(500);

}

void loop(void)
{
  int currentmessageid = 0;
  // Pump the network regularly
  network.update();


  // Is there anything ready for us?
  while ( network.available() )
  {
    // If so, grab it and print it out
    RF24NetworkHeader header;
    network.read(header,&command,sizeof(command));
    response_t response;
    command_t command; 
    if (command.commandtype == 7) // Probe 
    {
      response.messageid = currentmessageid;
      currentmessageid = currentmessageid + 1;
    }
    RF24NetworkHeader responseheader(/*to node*/ 1);
    response.temperature = currenttemperature;
    response.battvoltage = currentvoltage;
    bool ok = network.write(responseheader, &response, sizeof(response));
  }
  
  if (millis() > currenttime + 3600000) // Update every minute or so
  {
    currenttemperature = getTemperature();
    currentvoltage = getVoltage(); 
    currenttime = millis();
  }
}

