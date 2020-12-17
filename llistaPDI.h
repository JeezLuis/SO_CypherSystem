#ifndef _LLISTA_BID_H_
#define _LLISTA_BID_H_

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "utilities.h"
#include "semaphore_v66.h"

//Estructura que defineix uan conexio
typedef struct{
  pthread_t tit;
  int port;
  char *nomConexio;
  int fdClient;
}Connexio;

typedef struct _Node{
	Connexio e;
	struct _Node * seg;
	struct _Node * ant;
}Node;

typedef struct {
	Node * pri;
	Node * ult;
	Node * pdi;
  semaphore semafor;
}LlistaBid;

void LLISTABID_reservar(LlistaBid l);                       //Reserva la llista per poderla usar de manera sincronitzada
void LLISTABID_lliberar(LlistaBid l);                       //Llibera el us de la llista a el proxim a la cua que vulgui utilitzarla
LlistaBid LLISTABID_crea();                                 //Creem la llista bidireccional i demanant els espais de memoria i indicant els punters a les seves direccions pertinents
void LLISTABID_inserirDreta(LlistaBid *l, Connexio e);			//Inserim a la dreta de llista respecte el punt PDI
void LLISTABID_inserirEsquerra(LlistaBid *l, Connexio e);		//Inserim a la esquerra de llista respecte el punt PDI
Connexio LLISTABID_consulta(LlistaBid l);						        //Retornem el valor de la casella a la que apunta PDI
void LLISTABID_esborra(LlistaBid *l);						            //Esborrem el valor al que apunta PDI
void LLISTABID_vesInici(LlistaBid *l);						          //Apuntem PDI al primer valor de la llista
void LLISTABID_vesFinal(LlistaBid *l);						          //Apuntem PDI al ultim valor de la llista
void LLISTABID_avanca(LlistaBid *l);						            //Fem avancar el PDI cap a la dreta
void LLISTABID_retrocedeix(LlistaBid *l);					          //Fem retrodedir el punter PDI cap a l'esquerra
int LLISTABID_fi(LlistaBid l);								              //Retorna un enter en forma binaria per a saber si ens trobem al final de la llista
int LLISTABID_buida(LlistaBid l);							              //Retorna un enter en forma binaria per a saber si la llista es buida
int LLISTABID_inici(LlistaBid l);							              //Retorna un enter en forma binaria per a saber si ens trobem al inici de la llista
void LLISTABID_destrueix(LlistaBid *l);						          //Alliberem tots els espais de memoria i eliminem tota la informació.
int LLISTABID_buscar_borrar(LlistaBid *l, pthread_t tit);	  //Borra y retorna 1 o 0 en funcio de si sha eliminat be 1, o manlament 0
int LLISTABID_buscar_borrar_fd(LlistaBid *l, int sender);   //Borra y retorna 1 o 0 en funcio de si sha eliminat be 1, o manlament 0
char* LLISTABID_buscar_port(LlistaBid l, int port);         //Busca si un port existeix i retorna el usuari al que esta asignat
int LLISTABID_buscar_fd(LlistaBid l, char *user);           //Busca un ususari, i retorna el seu file descriptor corresponent

#endif //_LLISTA_BID_H_
