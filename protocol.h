#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "utilities.h"

#define KO_CONEXIO "[CONKO]"
#define TR_NAME "[TR_NAME]"
#define CON_OK  "[CONOK]"
#define MSGOK "[MSGOK]"
#define MSG "[MSG]"
#define BROADCAST "[BROADCAST]"
#define SHOW_AUDIOS "[SHOW_AUDIOS]"
#define LIST_AUDIOS "[LIST_AUDIOS]"
#define AUDIO_RQST "[AUDIO_RQST]"
#define AUDIO_KO "[AUDIO_KO]"
#define AUDIO_RSPNS "[AUDIO_RSPNS]"
#define FEO "[EOF]"
#define MD5OK "[MD5OK]"
#define MD5KO "[MD5KO]"
#define SORTIR "[]"

#define PACKET_LEN 512

//Estructura que defineix un paquet
typedef struct{

  char type;
  char *header;
  unsigned int length;
  char *data;

}Packet;

//Envia una trama de informacio usannt el protocol
int sendTrama(int fd, int typeInt, char *header, int sizeHeader, int length, char *data);

//Llegeix una trama d'informacio usant el protocol
Packet llegirTrama(int fd);

//Llibera la memoria utilitzada per un paquet
void destroyPacket(Packet p);


#endif
