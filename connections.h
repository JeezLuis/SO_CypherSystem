#ifndef _connections_h_
#define _connections_h_

#include <string.h>
#include <stdio.h>
#include "utilities.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "llistaPDI.h"
#include <pthread.h>
#include "protocol.h"
#include <dirent.h>
#include <fcntl.h>
#include <sys/wait.h>

#define STDOUT_FD 1
#define STDERR_FD 2
#define BASH_NAME "show_connections_v2.sh"
#define BASH_PATH "./show_connections_v2.sh"

typedef struct{
  char port[6];
}Ports;

typedef struct{
  int noumitjo;
  char* nom;
}NewUser;

//Inicia tots els components necesaris per el servidor
void initServidorClient();

//Crea una conexio amb un altre programa a traves del port suministrat
int createConnection(int port);

//Accepta y filtra una conexio entrant
void* acceptConnection(void *arg);  //THREAD

//Escolta conexions entrants, y en cas darribar una la llença en el process acceptConection
void* listener();         //THREAD

//Envia un missatge a un programa amb el que ja tenim una conexio
int sendMessage(char *name, char *message);

//Mostra les conexions posibles
int showConnections();

//Llegeix el missatge que algu ja conectat a mi m'ha enviat
void readSay(int fd, char *nom, Packet packet);

//Llegeix el missatge de tipus broadcast que algu ja conectat a mi m'ha enviat
void readBroadcast(int fd, char *nom, Packet packet);

//Envia un missatge a tots els programes que estic conectat
void sendBroadcast(char *message);

//Envia la llista de audios que tenim en la carpeta audios suministrada per el configx.dat i lenvia a la maquina que la solicitat
void sendAudioList(int fd);

//Envia un audio a la maquina que la solicitat
void sendAudio(int fd, Packet packet);

//Demana el llistat de audios a l amaquina amb non suministrat, y mostra la llista rebuda per pantalla
void showAudios(char *name);

//Demana l'audio amb el nom suministrat de la maquina amb nom d'usuari suministrat i el descarrega
void downloadAudio(char* name, char* audio);

//Tanca totes les conexions del servidor;
void closeConnections();


#endif //_connections_h_
