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

// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(9,10);

// Network uses that radio
RF24Network network(radio);

// Address of our node
const uint16_t this_node = 1;

// Address of the other node
const uint16_t other_node = 0;

// How often to send 'hello world to the other unit
const unsigned long interval = 2000; //ms

// When did we last send?
unsigned long last_sent;

// How many have we sent already
unsigned long packets_sent;

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

void setup(void)
{
  Serial.begin(57600);
  Serial.println("Mini BaseStation");
 
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  Serial.println("Radio Ready");

 
}

void loop(void)
{
  delay(500);
  // Pump the network regularly
  network.update();

    while (network.available() ) 
      {
       Serial.println("Got message");
       RF24NetworkHeader responseheader;
       response_t response;
       network.read(responseheader, &response, sizeof(response)); 
       Serial.print("\nTemperature : ");
       Serial.print(response.temperature);
       Serial.print("\nVoltage : ");
       Serial.print(response.battvoltage);
       Serial.print("\ninStatus : ");
       Serial.print(response.pinstatus);
      }

// Test code : send a getStatus command
    Serial.print("Sending...");
    command_t command;
    command.messageid = 99;
    command.commandtype = 7;
    command.commanddata = 8;
    
    RF24NetworkHeader commandheader(other_node);
    bool ok = network.write(commandheader,&command,sizeof(command));
    if (ok) {
      Serial.println("ok.");
    }
    else
    {
      Serial.println("failed.");
      delay(250);
    }

    
    
}
// vim:ai:cin:sts=2 sw=2 ft=cpp
