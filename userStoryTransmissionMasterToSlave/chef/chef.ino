// ask_receiver.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to receive messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) receiver with an Rx-B1 module

#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile

RH_ASK driver(2000,13,12); // reception sur 13 , transmission sur 12

int etat = 0;

void setup()
{
    
    pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);
    Serial.begin(9600);  // Debugging only
    driver.setModeRx();
    if (!driver.init())
    {
      Serial.println("init failed");
    }
}

void sendMsg(char* msg)
{
  driver.setModeTx(); // mode transmission
  driver.send((uint8_t *)msg, strlen(msg));
  driver.waitPacketSent();
}

int recv() // fct de reception
{
  driver.setModeRx();
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);
  char message[8];
  if (driver.recv(buf, &buflen)) // Non-blocking
  {
    // Message with a good checksum received, dump it.
    for (int i = 0 ; i < buflen ; i++)
    {
      
      message[i] = buf[i];
      Serial.print(message[i]);
    }
    Serial.println();
    return 1;
    
  }
}



void loop()
{
  //int bcl = 0;
  char msg[1];
  msg[0] = 8;
  sendMsg(msg); // forward
  delay(500);
  
  /*while(recv() != 1) // tant que on recoit pas un message
  {
    delay(500); // wait...


   
    if (bcl == 3)
    {
      //sendMsg("hello"); // timeout , on renvoi un message
      Serial.println("retry");
      bcl = -1;
    }
    bcl++;
  }*/
  
  Serial.println("envoye forward!!"); // message recu
}

