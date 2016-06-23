#ifndef COM_H
#define COM_H

#include <RH_ASK.h>
#include <SPI.h>

typedef struct {
    uint8_t dest;
    uint8_t type;
} msg_header_t;

class Com {
    RH_ASK driver(2000,13,12); // reception sur 13 , transmission sur 12
    int ID = -1;
    int num = -1;
    int numAssigne = 1; // seulement pour le chef
    bool chef = false;

    /*!
     * \fn private  char Com::getHeaderMessage(msg_header_t * header, char * buffer);
     * \brief Extrait le Header du message.
     *
     * \param header : Valeur de retour contenant les information du header
     * \param buffer : buffer contenant le massage
     * \return char
     */
    char getHeaderMessage(msg_header_t *header, char *buffer) {
        char strType[3];
        char strDest[3];
        strncpy(strDest, buffer, 2);
        strncpy(strType, buffer + 2, 2);
        header->dest = atoi(strDest);
        header->type = atoi(strType);
    }

    unsigned long getID(char * buffer) {
        char strID[11] = {0};
        strncpy(strID, buffer + 4, 10);
        return strtoul(strID, NULL, 0);
    }


    /*!
     * \fn private  void Com::sendMsg(char* msg);
     * \brief Envoie un message en boardcast
     *
     * \param msg : le contenue du message à envoyer
     * \return void
     */
    void sendMsg(char *msg) {
        driver.setModeTx();
        driver.send((uint8_t *) msg, strlen(msg));
        driver.waitPacketSent();
    }


    /*!
     * \fn private  uint8_t Com::recvMsg(char * buffer)
     * \brief Ecoute et reçois les messages
     * la taille du buffer ne doit pas dépasser RH_ASK_MAX_MESSAGE_LEN
     *
     * \param buffer : le buffer au sera stocker le contenue du message reçus
     * \return uint8_t la taille du contenue du message
     */
    uint8_t recvMsg(char *buffer) // la taille du buffer ne doit pas exceder RH_ASK_MAX_MESSAGE_LEN
    {
        driver.setModeRx();
        uint8_t buflen = -1;
        char message[8];
        if (driver.recv((uint8_t *) buffer, &buflen)) // Non-blocking
        {
            for (int i = 0; i < buflen; i++) {
                message[i] = buf[i];
                Serial.print(message[i]);
            }
            Serial.println();
        }
        return buflen;
    }

    /*!
     * \fn private  unsigned long Com::genID();
     * \brief sert à generé un ID unique qui identifira le robot
     *
     * \return unsigned long un nombre compris entre 0 et 30000
     */
    unsigned long genID() {
        return millis();
    }

    /*!
     * \fn private  boolean Com::chefRequete(int ID, int * num);
     * \brief Envoie un message pour savoir si un chef à déjà était élu, et si ou, récupére le nouvelle ID unique
     *
     * \param ID : l'ID unique du robot lors de son lancement
     * \param num : l'ID unique du robot assignée par le chef
     * \return boolean true si un chef est déjà élus
     */
    bool chefRequete(int ID, int *num);


    /*!
     * \fn private  void Com::chefReponse();
     * \brief La réponse du chef à chefRequete, contient le nouvelle ID unique
     *
     * \return void
     */
    void chefReponse();

public:
    enum {
        TYPE_CHEF_REQ,
        TYPE_CHEF_REP,
        TYPE_AVANT,
        TYPE_STOP,
        TYPE_DROITE,
        TYPE_GAUCHE
    };

    Com();

    /*!
     * \fn public  void Com::setup();
     * \brief Instancie le driver du communication
     *
     * \return void
     */
    void setup();

    /*!
     * \fn public  void Com::init();
     * \brief Initialise les ids des robot et permet d'élire un chef
     *
     * \return void
     */
    void init();

    /*!
     * \fn public  void Com::marche_avant(int vitesse);
     * \brief Envoie un message pour ordonner d'avancer au robot
     *
     * \param vitesse :
     * \return void
     */
    void marche_avant(int vitesse);

    /*!
     * \fn public  void Com::stop();
     * \brief Ordonne l'arrêt des robot
     *
     * \return void
     */
    void stop();

    /*!
     * \fn public  void Com::droite();
     * \brief Ordonne de tourner à droite
     *
     * \return void
     */
    void droite();

    /*!
     * \fn public  void Com::gauche();
     * \brief Odonne de tourner à gauche
     *
     * \return void
     */
    void gauche();

    /*
     * \fn public  void Com::com(uint8_t * type);
     * \brief Reçois les message contenant les ordres de marche
     *
     * \param type :
     * \return void
     */
    void com(uint8_t *type);

    /*!
     * \fn public  boolean Com::isChef();
     * \brief Permet de savoit si le robot est le chef
     *
     * \return boolean
     */
    bool isChef() { return chef; }
}

#endif // COM_H
