//Lluis Farre Castan (lluis.farre)
//Joan Esteban Santaeularia (joan.esteban)

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include "commands.h"
#include "utilities.h"

#define FD_CLOSED -1    //Indica que el FD esta tancat

Configuration configs;
int fd_file = FD_CLOSED;
char* buffer_bis = NULL;


void closingProtocol(){

  //lliberem memoria configs
  free(configs.name);
  free(configs.folder);
  free(configs.ip);
  free(configs.server);

  //Tanquem totes les conexions experant-nos a que acabin tots els threads oberts
  closeConnections();

  //Destruim el semaor que fem servir per a escriure
  destructorSemWrite();

  if(fd_file != FD_CLOSED){
    close(fd_file);
  }

}


void exitProgram(){
  //Desconectem ctrl + C per a que no intentin tancar quan ja estem tancant amb un exit
  signal(SIGINT,SIG_IGN);
  writeSynced("\nDisconnecting Trinity...\n",strlen("\nDisconnecting Trinity...\n"));
  closingProtocol();
  exit(0);
}

//Legim la informacio del fitxer de configuracio
int readFile(char* fitxer){

  fd_file = open(fitxer,O_RDONLY);
  if(fd_file < 0){
    return -1;
  }

  char* buffer = NULL;

  configs.name = NULL;
  configs.folder = NULL;
  configs.ip = NULL;
  configs.server = NULL;

  //Amb la funcio read line, llegim tots els parametres i els posem dins de les sevees respectives variabless
  configs.name = readLine(configs.name, fd_file,'\n');
  configs.folder = readLine(configs.folder, fd_file,'\n');
  configs.ip = readLine(configs.ip, fd_file,'\n');
  buffer = readLine(buffer,fd_file,'\n');
  configs.port = atoi(buffer);
  configs.server = readLine(configs.server, fd_file,'\n');
  buffer = readLine(buffer,fd_file,'\n');
  configs.port_begin = atoi(buffer);
  buffer = readLine(buffer,fd_file,'\n');
  configs.port_end = atoi(buffer);

  close(fd_file);
  fd_file = FD_CLOSED;

  free(buffer);

  return 0;

}

int main(int argc, char const *argv[]) {

  //Comprovem que passem tots els parametres
  if (argc < 2){
    writeSynced("Error: Parameters not correct\n",strlen("Error: Parameters not correct\n"));
    //Falten parametres
    return -1;
  }

  writeSynced("\nStarting Trinity...\n\n", strlen("\nStarting Trinity...\n\n"));
  if(readFile((char*) argv[1]) < 0){
    writeSynced("Error: Problem opening config file.\n", strlen("Error: Problem opening config file.\n"));
    //Error en obrir arxiu
    return -1;
  }

  //Configurem el signal de Ctrl+C per a sortir del programa.
  signal(SIGINT,exitProgram);

  int exit = 0;
  initServidorClient(configs);
  do{
    scrRsv();
      write(1,"$",1);
      write(1,configs.name, strlen(configs.name));
      write(1,": ",2);
    scrFree();

    buffer_bis = NULL;
    buffer_bis = readLine(buffer_bis,0,'\n');
    exit = commandManager(buffer_bis);
    free(buffer_bis);

  }while(exit != 1);

  exitProgram();

  return 0;

}
