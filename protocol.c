#include "protocol.h"

//Envia trames
int sendTrama(int fd, int typeInt, char *header, int sizeHeader, int length, char *data){

  //Enviem el tipus de trama
  char type[1];
  type[0] = (char) typeInt;

  write(fd, type, 1);

  //Mirem si el socket esta tancat
  int error = 0;
  socklen_t len = sizeof (error);
  getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);

  if(error != 0){
    return -1;
  }

  write(fd, header, sizeHeader);

  unsigned char char_length[2];
  char_length[1] = length;
  char_length[0] = length >> 8;
  write(fd, char_length, 2);
  write(fd, data, length);

  return 0;
}

//Aixo llegeix un paquet entrant
Packet llegirTrama(int fd){
  char *header = NULL;
  unsigned char char_length[2];
  int length;
  char type[1];

  Packet packet;
  packet.data = NULL; //!!!
  //legim el tipus de paquet
  read(fd, type, 1);
  packet.type = type[0];

  //legim header
  header = readLine(header, fd, ']');
  packet.header = header;

  //llegim length
  read(fd, char_length, 2);
  packet.length = (char_length[0] << 8) + char_length[1];
  length = packet.length;

  //legim data
  if(length > 0){
    packet.data = (char*) malloc(length + 1);
    read(fd, packet.data, length);
    packet.data[length] = '\0';
  }

  return packet;

}

//Alliberem la memoria del packet indicat
void destroyPacket(Packet p){

  if (p.header != NULL){
    free(p.header);
  }

  p.header = NULL;

  if (p.data != NULL){
    free(p.data);
  }

  p.data = NULL;

}
