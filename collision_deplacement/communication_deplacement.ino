/****************************************************************************
HC-SR04 : test program for ultrasonic module

Author: Enrico Formenti & Lavoisier Clément & Audibert Julien
Permissions: MIT licence & UNS licence

Remarks:
*****************************************************************************/

/*!
* \file communication_deplacement.ino
* \brief Fichier
* \author hiko-seijuro
* \version 0.1
*/
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
#include "com.h"

/**
* \enum
* \brief Etats de la machine à états.
* La machine a état permet d'adapter le comportement du drone en fonction du
* dernier message qu'il a reçu. Tant qu'il ne reçoit pas un message contradictoire
* il garde le même comportement.
*/
enum {
    ETAT_INIT,
    ETAT_STOP,
    ETAT_GO
}

        Adafruit_MotorShield
AFMS1 = Adafruit_MotorShield();
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
/*!< Objet de communication*/
uint8_t etat = ETAT_INIT;

/**
* \fn void rotation_droite()
* \brief Rotation sur place du drone vers la droite.
* Utilisée par la procédure maitre dans le cas de l'évitement d'un obstacle.
*/
void rotation_droite() {
    uint8_t i;
    myMotor1->run(FORWARD);
    myMotor2->run(BACKWARD);

    myMotor1->setSpeed(150);
    myMotor2->setSpeed(150);
}

/**
* \fn void rotation_gauche()
* \brief Rotation sur place du drone vers la gauche.
* Utilisée par la procédure maitre dans le cas de l'évitement d'un obstacle.
*/
void rotation_gauche() {
    uint8_t i;
    myMotor1->run(BACKWARD);
    myMotor2->run(FORWARD);

    myMotor1->setSpeed(150);
    myMotor2->setSpeed(150);
}

/**
* \fn void deplacement_avant()
* \brief Déplacement en marche avant.
* Utilisée par les procédures maitre et esclave.
*
* \param uint32_t range : Pourcentage de la vitesse maximal (range 0..100), calculée en fonction de de la distance du drone par par rapport à son précédent dans cas où c'est un suiveur. 100 sinon.
*/
void deplacement_avant(uint32_t range) {
    uint8_t i;
    myMotor1->run(FORWARD);
    myMotor2->run(FORWARD);
    myMotor1->setSpeed((uint32_t)(150 * range / 100));
    myMotor2->setSpeed((uint32_t)(150 * range / 100));
}

/**
* \fn void deplacement_droite(uint32_t range)
* \brief Rotation droite en marche avant du drone vers la droite.
* Utilisée par la procédure esclave dans la poursuite du drône précédent.
*
* \param uint32_t range : Pourcentage de la vitesse maximal (range 0..100), calculée en fonction de de la distance du drone par par rapport à son précédent dans cas où c'est un suiveur.
*/
void deplacement_droite(uint32_t range) {
    uint8_t i;
    myMotor1->run(FORWARD);
    myMotor2->run(FORWARD);

    myMotor1->setSpeed((uint32_t)(100 * range / 100));
    myMotor2->setSpeed((uint32_t)(200 * range / 100));
}

/**
* \fn void deplacement_gauche(uint32_t range)
* \brief Rotation gauche en marche avant du drone vers la droite.
* Utilisée par la procédure esclave dans la poursuite du drône précédent.
*
* \param uint32_t range : Pourcentage de la vitesse maximal (range 0..100), calculée en fonction de de la distance du drone par par rapport à son précédent dans cas où c'est un suiveur.
*/
void deplacement_gauche(uint32_t range) {
    uint8_t i;
    myMotor1->run(FORWARD);
    myMotor2->run(FORWARD);

    myMotor1->setSpeed((uint32_t)(200 * range / 100));
    myMotor2->setSpeed((uint32_t)(100 * range / 100));

    delay(500);
    myMotor1->run(RELEASE);
    myMotor2->run(RELEASE);
}

/**
* \fn void deplacement_arriere()
* \brief Marche arrière
* Non utilisée.
*/
void deplacement_arriere() {
    uint8_t i;
    myMotor1->run(BACKWARD);
    myMotor2->run(BACKWARD);

    myMotor1->setSpeed(150);
    myMotor2->setSpeed(150);
}

/**
* \fn void setup()
* \brief Fonction d'initialisation Arduino.
*/
void setup() {
    // initialize serial communication:
    Serial.begin(9600);

    AFMS1.begin();  // create with the default frequency 1.6KHz
    AFMS2.begin();  // create with the default frequency 1.6KHz

    // Set the speed to start, from 0 (off) to 255 (max speed)
    myMotor1->setSpeed(150);
    myMotor2->setSpeed(150);

    myMotor1->run(RELEASE);
    myMotor2->run(RELEASE);

    com_setup();
}


enum {
    GO_INIT,
    GO_DROITE,
    GO_GAUCHE,
    GO_TOUT_DROIT
};

uint8_t etat_go = GO_TOUT_DROIT;

/**
* \fn void go(uint32_t cm1, uint32_t cm2, uint32_t cm3)
* \brief Déplacement et évitement des obstacles. Cette procédure permet
* également au chef de file d'envoyer des ordres aux suiveurs.
* Cette fonction de comportement est exéutée en continu par le chef.
*
* \param uint32_t cm1 : Distance de l'objet détectée par le capteur gauche.
* \param uint32_t cm2 : Distance de l'objet détectée par le capteur central.
* \param uint32_t cm3 : Distance de l'objet détectée par le capteur droit.
*/
void go(uint32_t cm1, uint32_t cm2, uint32_t cm3) { // procédure master
    uint32_t distance_min = 60;


    if (cm1 < distance_min | cm2 < distance_min | cm3 < distance_min) {
        switch (etat_go) {
            case GO_DROITE:
            case GO_GAUCHE: {
                if (etat_go != GO_GAUCHE) {
                    if ((cm3 > cm1) & (cm3 > cm2)) {
                        rotation_gauche();
                        etat_go = GO_GAUCHE;
                    }
                } else { // etat_go != GO_DROITE
                    if ((cm1 > cm2) & (cm1 > cm3)) {
                        rotation_droite();
                        etat_go = GO_DROITE;
                    }
                }
                break;
            }
            case GO_INIT:
            case GO_TOUT_DROIT: {
                com.stop();

                if ((cm1 > cm2) & (cm1 > cm3)) {
                    rotation_droite();
                    etat_go = GO_DROITE;
                } else if ((cm3 > cm1) & (cm3 > cm2)) {
                    rotation_gauche();
                    etat_go = GO_GAUCHE;
                } else {
                    rotation_gauche();
                    etat_go = GO_GAUCHE;
                }
                break;
            }
        }
    }
    else {
        if(etat_go != GO_TOUT_DROIT) {
            com.marche_avant();
            deplacement_avant(100);
            etat_go = GO_TOUT_DROIT;
        }
    }
}

/**
* \fn void arret()
* \brief Arrêt des moteurs du drône.
*/
void arret() {
    myMotor1->run(RELEASE);
    myMotor2->run(RELEASE);
}

/**
* \fn uint32_t calcule_pourcentage_vitesse(uint32_t cm, uint32_t max, uint32_t min)
* \brief Calcule le pourcentage de la vitesse, en fonction de la distance de
* l'objet poursuivi et de l'intervalle de distance dans lequel il doit adapter sa vitesse.
*
* \param uint32_t cm : Distance de l'objet détectée par le capteur correspondant au mouvement.
*/
uint32_t calcule_pourcentage_vitesse(uint32_t cm, uint32_t max, uint32_t min) {
    if (cm >= max) {
        return 100;
    } else if (cm <= min) {
        return 0;
    } else {
        return (uint32_t)(cm - min * 100 / (max - min));
    }
}

/**
* \fn void suivre(uint32_t cm1, uint32_t cm2, uint32_t cm3)
* \brief Déplacement et poursuite de l'objet le plus proche. Procédure esclave
* utilisée par les suiveurs pour se diriger vers l'objet le plus proche.
* Cette fonction de comportement est exéutée en continu par le chef.
*
* \param uint32_t cm1 : Distance de l'objet détectée par le capteur gauche.
* \param uint32_t cm2 : Distance de l'objet détectée par le capteur central.
* \param uint32_t cm3 : Distance de l'objet détectée par le capteur droit.
*/
void suivre(uint32_t cm1, uint32_t cm2, uint32_t cm3) { // procédure esclave
    uint32_t distance_min = 5;
    uint32_t distance_max = 30;

    if (cm2 < cm1 && cm2 < cm3) {
        deplacement_avant(calcule_pourcentage_vitesse(cm2, distance_max, distance_min));
    }
    else {
        if ((cm1 < cm2) & (cm1 < cm3)) {
            deplacement_gauche(calcule_pourcentage_vitesse(cm1, distance_max, distance_min));
        }

        if ((cm3 < cm1) & (cm3 < cm2)) {
            deplacement_droite(calcule_pourcentage_vitesse(cm3, distance_max, distance_min));
        }
    }
}

void voir(uint32_t *cm1, uint32_t *cm2, uint32_t *cm3) {
    uint32_t duration1; // duration of the round trip
    uint32_t duration2; // duration of the round trip
    uint32_t duration3; // duration of the round trip

    pinMode(trigPin1, OUTPUT);
    digitalWrite(trigPin1, LOW);
    delayMicroseconds(3);
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin1, LOW);
    pinMode(echoPin1, INPUT);

    *cm1 = (uint32_t)((duration1 << 4) + duration1) / 1000.0; // cm = 17 * duration/1000
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
    *cm2 = (uint32_t)((duration2 << 4) + duration2) / 1000.0; // cm = 17 * duration/1000
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
    *cm3 = (uint32_t)((duration3 << 4) + duration3) / 1000.0; // cm = 17 * duration/1000
    Serial.print(cm3);
    Serial.print(" centimeters for duration 3");
    Serial.println();
}

/**
* \fn void loop()
* \brief Fonction principale de l'Arduino, exécutée en tant que tâche de fond.
*/
void loop() {
    if (etat == ETAT_INIT) {
        com.init();
        etat = ETAT_GO;
    } else {
        uint32_t cm1;        // distance of the obstacle
        uint32_t cm2;        // distance of the obstacle
        uint32_t cm3;        // distance of the obstacle

        voir(&cm1, &cm2, &cm3);
        com.com(&etat);

        switch (etat) {
            case ETAT_GO : {
                if (com.isChef()) {
                    go(cm1, cm2, cm3);
                }
                else {
                    suivre(cm1, cm2, cm3);
                }
                break;
            }
            case ETAT_STOP : {
                arret();
                break;
            }
        }
    }
}
