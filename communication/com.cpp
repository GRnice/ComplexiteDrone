#include <stdio.h>
#include <string.h>
#include <time.h>
#include <uuid/uuid.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile
#include "com.h"

#define TAILLE_TRAME_MAX 32;
#define TIMEOUT_CHEF 2000; //secondes
#define TIMEOUT_INIT 60000; //secondes

boolean Com::chefRequete(int ID, int * num) {
  sprintf(buffer, "%02d", TYPE_CHEF_REQ);
  sprintf(buffer+strlen(buffer), "%05d", ID);
  sendMsg(buffer);

  unsigned long start = millis();
  unsigned long end;
  do {
    recvMsg(buffer); // Lance recv en boucle jusqu'au timeout
    end = millis();

    if(end - start >= TIMEOUT_CHEF) break;
  } while(!strlen(buffer));

  if(strlen(buffer) > 0) (
    char strNum[3];
    char strType[3];
    strncpy(strNum, buffer, 2);
    strncpy(strType, buffer+2, 2);

    if(atoi(strType) == TYPE_CHEF_REP) {
      char strID[6];
      strncpy(strID, buffer+4, 5);

      if(ID == atoi(id)) {
        *num = atoi(strNum);
        return true;
      }
    }
  }
  return false;
}

void Com::chefReponse() {
  sprintf(buffer, "%02d", numAssigne++);
  sprintf(buffer+strlen(buffer), "%02d", TYPE_CHEF_REP);

  sendMsg(buffer);
}

void Com::setup()
{
  Serial.begin(9600);  // Debugging only
  driver.setModeRx();
  if (!driver.init())
  {
    Serial.println("init failed");
  }
  randomSeed(analogRead(0));
}

void Com::init() {
  ID = genID();
  if(!chefRequete(ID)) {
    isChef = true;
    unsigned long start = millis();
    unsigned long end;
    do{
      do {
        recvMsg(buffer); // Lance recv en boucle jusqu'au timeout
        end = millis();

        if(end - start >= TIMEOUT_CHEF) break;
      } while(!strlen(buffer));
      end = millis();
      chefReponse();
    } while(end - start < TIMEOUT_CHEF);
  }
}

void Com::marche_avant(int vitesse) {
  char buffer[100] = {0};
  sprintf(buffer, "0");
  sprintf(buffer+strlen(buffer), "%02d", TYPE_AVANT);
  sprintf(buffer+strlen(buffer), "%02d", vitesse);
  sendMsg(buffer);
}

void Com::stop() {
  char buffer[100] = {0};
  sprintf(buffer, "0");
  sprintf(buffer+strlen(buffer), "%02d", TYPE_STOP);
  sendMsg(buffer);
}

void Com::droite() {
  char buffer[100] = {0};
  sprintf(buffer, "0");
  sprintf(buffer+strlen(buffer), "%02d", TYPE_DROITE);
  sendMsg(buffer);
}

void Com::gauche() {
  char buffer[100] = {0};
  sprintf(buffer, "0");
  sprintf(buffer+strlen(buffer), "%02d", TYPE_GAUCHE);
  sendMsg(buffer);
}

void Com::com(uint8_t * etat) {
  char buffer[RH_ASK_MAX_MESSAGE_LEN];

  if(recvMsg(&buffer)) {
    msg_header_t header;
    getHeaderMessage(&header, buffer);
    if(header.dest == num || header.dest == 0) {
      switch(header.type) {
        case TYPE_AVANT:
        case TYPE_STOP:
        case TYPE_DROITE:
        case TYPE_GAUCHE:
          *etat = header.type;
      }
    }
  }
}
