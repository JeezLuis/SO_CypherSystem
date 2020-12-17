#ifndef _utilities_h_
#define _utilities_h_

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "semaphore_v66.h"
#include <sys/wait.h>

#define MD5_PATH "/usr/bin/md5sum"
#define MD5_NAME "md5sum"
#define STDOUT_FD 1

//Estrucutra de configuracio
typedef struct{
  char* name;
  char* folder;
  char* ip;
  int port;
  char* server;
  int port_begin;
  int port_end;
}Configuration;

//llegeix una linia de text fins el delimitador suministrat a el fd suministrat. Guarda la informacio de manera dinamica a buffer. Buffer ha de esta igualat a NULL o ja inicialitzat. Retorna el punter de buffer
char* readLine(char* buffer,int fd, char delimiter);

//passa una string a uppercase
char* stringToUpper(char* string, char* new_string);

//passa una string a lowercase
void stringToLower(char* string);

//Retorna la string result, aquesta conte tots els caracters que coincideixen entre string 1 i string 2 de manera consecutiva
char* inString(char* string1, char* string2, char* result);

//Mirarem si el port esta dintre del rang correcte
int portTest(char* port);

//intercambia dos variables entre elles
void swap(char *x, char *y);

//inverteix una string
char* reverse(char *buffer, int i, int j);

//integer to ascii
char* itoa(int value, char *buffer, int base);

//Inicialitza el semafor de sincronitzacio descriptura per pantalla
void initSemWriteSync();

//Destrueix el semafor de sincronitzacio descriptura per pantalla
void destructorSemWrite();

//Escriu per pantalla de manera sincronitzada
void writeSynced(char *msg, int length);

//Reserva la escriptura per pantalla fins el porxi scrRsv
void scrFree();

//Llibera la escriptura per pantalla a el proxim a la cua
void scrRsv();

//Calcula el md5 de un fitxe usant execl
char* ferMD5Sum(char* dir);


#endif //_utilities_h_
