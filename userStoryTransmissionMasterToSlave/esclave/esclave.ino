// ask_receiver.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to receive messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) receiver with an Rx-B1 module

#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile
#include <Wire.h>
#include <Adafruit_MotorShield.h>

//Security distance robot
#define SAFE 20

//Motor direction commands
#define BACK 4
#define FORW 8
#define LEFT 16
#define RIGHT 32

#define getH(a) (a>>1)
#define getMax(a,b) ((a) > (b) ? (a) : (b))

//Init motor
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 

Adafruit_DCMotor *motorLeft = AFMS.getMotor(1);
Adafruit_DCMotor *motorRight = AFMS.getMotor(2);

RH_ASK driver(2000,13,12); // reception sur 13 , transmission sur 12

int etat = 0;
int codeRecu;

void setup()
{
    AFMS.begin();
    motorLeft->setSpeed(170);
    motorRight->setSpeed(170);
    
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

int recv(int* receiver) // fct de reception
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
      if (i == 0) // le premier element contient l'ORDRE
      {
        *receiver = (int) buf[0]; // receiver est un pointeur sur 'codeRecu'
      }
      
      message[i] = buf[i];
      Serial.print(message[i]);
    }
    Serial.println();
    return 1;
    
  }
}

void left(){
  myStop();
  motorLeft->run(FORWARD);
  motorRight->run(BACKWARD);
  Serial.println("turn left");
}

void right()
{
  myStop();
  motorLeft->run(BACKWARD);
  motorRight->run(FORWARD);
  Serial.println("turn right");
}

void forward()
{
  motorLeft->run(FORWARD);
  motorRight->run(BACKWARD);
  Serial.println("forward");
}

void backward()
{
  motorLeft->run(BACKWARD);
  motorRight->run(FORWARD);
  Serial.println("backward");
}


void myStop()
{
  motorLeft->run(RELEASE);
  motorRight->run(RELEASE);
  Serial.println("stop");
}

void loop()
{
  while(recv(&codeRecu) != 1) // tant que on recoit pas un message
  {
    delay(500); // wait...
    if (codeRecu == BACK)
    {
      backward();
        delay(800);
        myStop();
        codeRecu = 0;
    }
    
    else if(codeRecu == FORW)
    {
      forward();
      delay(800);
      myStop();
      codeRecu = 0;
    }

    else if(codeRecu == LEFT)
    {
      left();
        delay(800);
        myStop();
        codeRecu = 0;
     }
     else if(codeRecu == RIGHT)
     {
      right();
      delay(800);
      myStop();
      codeRecu = 0;
     }

  }
  Serial.println("recu!!"); // message recu
}

