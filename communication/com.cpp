#include <stdio.h>
#include <string.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile
#include <cstdlib>
#include "com.h"

#define TIMEOUT_CHEF 2000 //secondesss
#define TIMEOUT_INIT 60000 //secondes

bool Com::chefRequete(int ID, int *num) {
    char buffer[RH_ASK_MAX_MESSAGE_LEN];
    for (int i = 0; i < RH_ASK_MAX_MESSAGE_LEN; ++i) {
        buffer[i] = 0;
    }
    sprintf(buffer, "00");
    sprintf(buffer + strlen(buffer), "%02d", TYPE_CHEF_REQ);
    sprintf(buffer + strlen(buffer), "%010d", ID);
    sendMsg(buffer);

    unsigned long start = millis();
    unsigned long end;
    do {
        recvMsg(buffer); // Lance recv en boucle jusqu'au timeout
        end = millis();

        if (end - start >= TIMEOUT_CHEF) break;
    } while (!strlen(buffer));

    if (strlen(buffer) > 0) {
        msg_header_t header;
        getHeaderMessage(&header, buffer);

        if (header.type == TYPE_CHEF_REP) {

            if (ID == getID(buffer)) {
                *num = header.dest;
                return true;
            }
        }
    }
    return false;
}

void Com::chefReponse(unsigned long id) {
    char buffer[RH_ASK_MAX_MESSAGE_LEN];

    sprintf(buffer, "%02d", numAssigne++);
    sprintf(buffer + strlen(buffer), "%02d", TYPE_CHEF_REP);
    sprintf(buffer + strlen(buffer), "%010d", id);

    sendMsg(buffer);
}

void Com::setup() {
    Serial.begin(9600);  // Debugging only
    driver.setModeRx();
    if (!driver.init()) {
        Serial.println("init failed");
    }
    randomSeed(analogRead(0));
}

void Com::init() {
    char buffer[RH_ASK_MAX_MESSAGE_LEN];
    char isChef = false;
    ID = genID();

    if (!chefRequete(ID, &num)) {
        isChef = true;
        unsigned long start = millis();
        unsigned long end;
        do {
            do {
                recvMsg(buffer); // Lance recv en boucle jusqu'au timeout
                end = millis();

                if (end - start >= TIMEOUT_INIT) break;
            } while (!strlen(buffer));
            end = millis();

            msg_header_t header;
            getHeaderMessage(&header, buffer);
            if(header.type == TYPE_CHEF_REQ) {
                chefReponse(getID(buffer));
            }
        } while (end - start < TIMEOUT_INIT);
    }
}

void Com::marche_avant(int vitesse) {
    char buffer[100] = {0};
    sprintf(buffer, "0");
    sprintf(buffer + strlen(buffer), "%02d", TYPE_AVANT);
    sprintf(buffer + strlen(buffer), "%02d", vitesse);
    sendMsg(buffer);
}

void Com::stop() {
    char buffer[100] = {0};
    sprintf(buffer, "0");
    sprintf(buffer + strlen(buffer), "%02d", TYPE_STOP);
    sendMsg(buffer);
}

void Com::droite() {
    char buffer[100] = {0};
    sprintf(buffer, "0");
    sprintf(buffer + strlen(buffer), "%02d", TYPE_DROITE);
    sendMsg(buffer);
}

void Com::gauche() {
    char buffer[100] = {0};
    sprintf(buffer, "0");
    sprintf(buffer + strlen(buffer), "%02d", TYPE_GAUCHE);
    sendMsg(buffer);
}

void Com::com(uint8_t *type) {
    char buffer[RH_ASK_MAX_MESSAGE_LEN];

    if (recvMsg(buffer)) {
        msg_header_t header;
        getHeaderMessage(&header, buffer);
        if (header.dest == num || header.dest == 0) {
            switch (header.type) {
                case TYPE_AVANT:
                case TYPE_STOP:
                case TYPE_DROITE:
                case TYPE_GAUCHE:
                    *type = header.type;
            }
        }
    }
}
