#ifndef COM_H
#define COM_H


typedef struct {
  uint8_t type;
  uint8_t dest;
} msg_header_t;

class Com
{
  RH_ASK driver(2000,13,12); // reception sur 13 , transmission sur 12
  int ID = -1;
  int num = -1;
  int numAssigne = 1; // seulement pour le chef
  boolean chef = false;

  char getHeaderMessage(msg_header_t * header, char * buffer) {
    char strType[3];
    char strDest[3];
    strncpy(strDest, buffer, 2);
    strncpy(strType, buffer+2, 2);
    header->dest = atoi(strDest);
    header->type = atoi(strType);
  }

  // Fonction de transmission
  void sendMsg(char* msg)
  {
    driver.setModeTx();
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
  }

  // Fonction de reception
  uint8_t recvMsg(char * buffer) // la taille du buffer ne doit pas exceder RH_ASK_MAX_MESSAGE_LEN
  {
    driver.setModeRx();
    uint8_t buflen = -1;
    char message[8];
    if (driver.recv((uint8_t *)buffer, &buflen)) // Non-blocking
    {
      for (int i = 0 ; i < buflen ; i++)
      {
        message[i] = buf[i];
        Serial.print(message[i]);
      }
      Serial.println();
    }
    return buflen;
  }

  unsigned long genID() {
    return random(30000);
  }

  boolean chefRequete(int ID, int * num);
  void chefReponse();
public:

  enum {
    TYPE_AVANT,
    TYPE_STOP,
    TYPE_DROITE,
    TYPE_GAUCHE
  }

  Com();

  void setup();
  void init();
  void marche_avant(int vitesse);
  void stop();
  void droite();
  void gauche();
  void com(uint8_t * type);

  boolean isChef() { return chef; }
}

#endif // COM_H
