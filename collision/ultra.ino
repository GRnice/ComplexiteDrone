/****************************************************************************
   HC-SR04 : test program for ultrasonic module

   Author: Enrico Formenti & Lavoisier Clément & Audibert Julien
   Permissions: MIT licence & UNS licence

   Remarks:
*****************************************************************************/
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
#include <com.cpp>

enum {
  ETAT_INIT,
  ETAT_STOP,
  ETAT_GO
}

Adafruit_MotorShield AFMS1 = Adafruit_MotorShield();
Adafruit_MotorShield AFMS2 = Adafruit_MotorShield();
Adafruit_DCMotor *myMotor1 = AFMS1.getMotor(1);
Adafruit_DCMotor *myMotor2 = AFMS2.getMotor(2);

const uint8_t trigPin1 = 3;
const uint8_t echoPin1 = 2;

const uint8_t trigPin2 = 6;
const uint8_t echoPin2 = 5;

const uint8_t trigPin3 = 9;
const uint8_t echoPin3 = 8;

Com com;
uint8_t etat = ETAT_INIT;

void rotation_droite()
{
  uint8_t i;
  myMotor1->run(FORWARD);
  myMotor2->run(BACKWARD);

 // for (i=155; i<255; i++)
  //{
    myMotor1->setSpeed(150);
    myMotor2->setSpeed(150);
 // }
  //for (i=255; i!=155; i--)
  //{
   // myMotor1->setSpeed(i);
    //myMotor2->setSpeed(i);
  //}
  delay(500);
    myMotor1->run(RELEASE);
    myMotor2->run(RELEASE);
}

void rotation_gauche()
{
  uint8_t i;
  myMotor1->run(BACKWARD);
  myMotor2->run(FORWARD);

  //for (i=155; i<255; i++)
  //{
    myMotor1->setSpeed(150);
    myMotor2->setSpeed(150);
  //}
  //for (i=255; i!=155; i--)
  //{
   // myMotor1->setSpeed(i);
    //myMotor2->setSpeed(i);
  //}
  delay(500);
    myMotor1->run(RELEASE);
    myMotor2->run(RELEASE);
}

void deplacement_avant()
{
  uint8_t i;
  myMotor1->run(FORWARD);
  myMotor2->run(FORWARD);

  //for (i=155; i<255; i++)
  //{
    myMotor1->setSpeed(150);
    myMotor2->setSpeed(150);
  //}
  //for (i=255; i!=155; i--)
  //{
    //myMotor1->setSpeed(i);
    //myMotor2->setSpeed(i);
  //}
  delay(500);
    myMotor1->run(RELEASE);
    myMotor2->run(RELEASE);
}

void deplacement_droite()
{
  uint8_t i;
  myMotor1->run(FORWARD);
  myMotor2->run(FORWARD);

  //for (i=155; i<255; i++)
  //{
    myMotor1->setSpeed(100);
    myMotor2->setSpeed(200);
  //}
  //for (i=255; i!=155; i--)
  //{
    //myMotor1->setSpeed(i);
    //myMotor2->setSpeed(i);
  //}
  delay(500);
    myMotor1->run(RELEASE);
    myMotor2->run(RELEASE);
}

void deplacement_gauche()
{
  uint8_t i;
  myMotor1->run(FORWARD);
  myMotor2->run(FORWARD);

  //for (i=155; i<255; i++)
  //{
    myMotor1->setSpeed(200);
    myMotor2->setSpeed(100);
  //}
  //for (i=255; i!=155; i--)
  //{
    //myMotor1->setSpeed(i);
    //myMotor2->setSpeed(i);
  //}
  delay(500);
    myMotor1->run(RELEASE);
    myMotor2->run(RELEASE);
}

void deplacement_arriere()
{
  uint8_t i;
  myMotor1->run(BACKWARD);
  myMotor2->run(BACKWARD);

  //for (i=155; i<255; i++)
  //{
    myMotor1->setSpeed(150);
    myMotor2->setSpeed(150);
 // }
  //for (i=255; i!=155; i--)
 // {
   // myMotor1->setSpeed(i);
   // myMotor2->setSpeed(i);
  //}
  delay(500);
    myMotor1->run(RELEASE);
    myMotor2->run(RELEASE);
}

void setup() {
  // initialize serial communication:
  Serial.begin(9600);

  AFMS1.begin();  // create with the default frequency 1.6KHz
  AFMS2.begin();  // create with the default frequency 1.6KHz

  // Set the speed to start, from 0 (off) to 255 (max speed)
  myMotor1->setSpeed(150);
  myMotor2->setSpeed(150);

  myMotor1->run(FORWARD);
  myMotor2->run(FORWARD);

  myMotor1->run(RELEASE);
  myMotor2->run(RELEASE);

  com_setup();
}

void go(uint32_t cm1, uint32_t cm2, uint32_t cm3) { // procédure master
  if(cm1>30|cm2>30|cm3>30)
  {
    if (cm2>30)
    {
      deplacement_avant();
    }
    else
    {
      if((cm1>cm2) & (cm1>cm3))
      {
        rotation_droite();
      }

      if((cm3>cm1) & (cm3>cm2))
      {
        rotation_gauche();
      }
    }
  }
  // else
  // {
  //   deplacement_arriere();
  // }
  myMotor1->run(RELEASE);
  myMotor2->run(RELEASE);
}

void suivre(uint32_t cm1, uint32_t cm2, uint32_t cm3) { // procédure esclave
  if(cm1>30|cm2>30|cm3>30)
  {
    if (cm2>30)
    {
      deplacement_avant();
    }
    else
    {
      if((cm1>cm2) & (cm1>cm3))
      {
        deplacement_gauche();
      }

      if((cm3>cm1) & (cm3>cm2))
      {
        deplacement_droite();
      }
    }
  }
  else
  {
    deplacement_arriere();
  }
  myMotor1->run(RELEASE);
  myMotor2->run(RELEASE);
}

void loop()
{
  if(etat == ETAT_INIT) {
    com.init();
    etat = ETAT_GO;
  } else {
    uint32_t duration1; // duration of the round trip
    uint32_t duration2; // duration of the round trip
    uint32_t duration3; // duration of the round trip

    uint32_t cm1;        // distance of the obstacle
    uint32_t cm2;        // distance of the obstacle
    uint32_t cm3;        // distance of the obstacle

    pinMode(trigPin1, OUTPUT);
    digitalWrite(trigPin1, LOW);
    delayMicroseconds(3);
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin1, LOW);
    pinMode(echoPin1, INPUT);
    duration1 = pulseIn(echoPin1, HIGH);
    cm1 = (uint32_t)((duration1<<4)+duration1)/1000.0; // cm = 17 * duration/1000
    Serial.print(cm1);
    Serial.print(" centimeters for duration 1");
    Serial.println();

    pinMode(trigPin2, OUTPUT);
    digitalWrite(trigPin2, LOW);
    delayMicroseconds(3);
    digitalWrite(trigPin2, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin2, LOW);
    pinMode(echoPin2, INPUT);
    duration2 = pulseIn(echoPin2, HIGH);
    cm2 = (uint32_t)((duration2<<4)+duration2)/1000.0; // cm = 17 * duration/1000
    Serial.print(cm2);
    Serial.print(" centimeters for duration 2");
    Serial.println();

    pinMode(trigPin3, OUTPUT);
    digitalWrite(trigPin3, LOW);
    delayMicroseconds(3);
    digitalWrite(trigPin3, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin3, LOW);
    pinMode(echoPin3, INPUT);
    duration3 = pulseIn(echoPin3, HIGH);
    cm3 = (uint32_t)((duration3<<4)+duration3)/1000.0; // cm = 17 * duration/1000
    Serial.print(cm3);
    Serial.print(" centimeters for duration 3");
    Serial.println();

    com.com(&etat);

    switch(etat) {
      case ETAT_GO :
      {
        if(com.isChef()) {
          go(cm1, cm2, cm3);
        }
        else {
          suivre(cm1, cm2, cm3);
        }
        break;
      }
      case ETAT_STOP :
      {
        // TODO : rien
        break;
      }
    }
  }
}
