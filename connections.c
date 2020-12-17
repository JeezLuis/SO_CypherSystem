//Lluis Farre Castan (lluis.farre)
//Joan Esteban Santaeularia (joan.esteban)

#include "connections.h"


LlistaBid lServer;
LlistaBid lClient;
pthread_t server_thread_id;
Configuration config;
int mainsocket = 0;
int exitGeneral = 0;
semaphore read_files;


void initServidorClient(Configuration confi){

  //Creem la llista on desarem els servidors dedicats
  lServer = LLISTABID_crea();

  //Creem la llista on desarem les nostres conexions actuals
  lClient = LLISTABID_crea();

  //Copiem la infromacio de la config per a no tenir colisions d'acces
  config.port = confi.port;
  config.name = (char *) malloc(strlen(confi.name) + 1);
  strcpy(config.name, confi.name);
  config.folder = (char *) malloc(strlen(confi.folder) + 1);
  strcpy(config.folder, confi.folder);
  config.ip = (char *) malloc(strlen(confi.ip) + 1);
  strcpy(config.ip, confi.ip);
  config.server = (char *) malloc(strlen(confi.server) + 1);
  strcpy(config.server, confi.server);
  config.port_begin = confi.port_begin;
  config.port_end = confi.port_end;

  //Creem el semador per a accedir als arxius
  SEM_constructor(&read_files);
  SEM_init(&read_files,1);


  //creem el listener per les escoltar conexions
  pthread_create(&server_thread_id, NULL, listener, NULL);
}

//Llegim el missatge enviat per un client
void readSay(int fd, char *nom, Packet packet){

  scrRsv();
    write(1,"\r[",2);
    write(1,nom,strlen(nom));
    write(1,"]: ",3);
    write(1,packet.data,strlen(packet.data));
    write(1,"\n",1);
    write(1,"$",1);
    write(1,config.name, strlen(config.name));
    write(1,": ",2);
  scrFree();

  sendTrama(fd, 2, MSGOK, strlen(MSGOK), 0, "");
}

//LLegim el broadcast
void readBroadcast(int fd, char *nom, Packet packet){

  scrRsv();
    write(1,"\r[",2);
    write(1,nom,strlen(nom));
    write(1,"] ",2);
    write(1,packet.data,packet.length);
    write(1,"\n",1);
    write(1,"$",1);
    write(1,config.name, strlen(config.name));
    write(1,": ",2);
  scrFree();

  sendTrama(fd, 3, MSGOK, strlen(MSGOK), 0, "");
}

//Enviem el llistat de audios que tenim
void sendAudioList(int fd){

  DIR *d;
  char *pilotes = NULL;
  struct dirent *dir;
  d = opendir(config.folder);

  //Si el directori s'ha obert correctament obrim el arxiu (Suposem que sempre ho fa be perque ens asseguren que config.dat sempre esta be)
  if (d){
      while ((dir = readdir(d)) != NULL){
          if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 ){

            pilotes = (char *) realloc(pilotes, (pilotes == NULL ? 0 : strlen(pilotes)) + strlen(dir->d_name) + 1);
            sprintf(pilotes,"%s%s\n", pilotes, dir->d_name);

          }

      }

      //Enviem el llistat de audios al client
      sendTrama(fd, 4, LIST_AUDIOS, strlen(LIST_AUDIOS), strlen(pilotes), pilotes);
      closedir(d);
      free(pilotes);
  }
}

//enviem un audio
void sendAudio(int fd, Packet packet){

  //Amb aquest semafor esperem si s'esta enviant algun audio desde un altre proces
  SEM_wait(&read_files);

  char directori[strlen(config.folder) + packet.length + 2];
  sprintf(directori,"%s/%s", config.folder, packet.data);

  //Si no troba el fitxer o no el pot obrir avisem al client
  int fd_file = open(directori, O_RDONLY);
  if(fd_file < 0){
    sendTrama(fd, 5, AUDIO_KO, strlen(AUDIO_KO), 0, "");
    return;
  }

  char data[PACKET_LEN];
  int length;

  //Anem enviant trames fins que trobem el EOF
  do{
    length = read(fd_file, data, PACKET_LEN);
    if(length > 0){
      sendTrama(fd, 5, AUDIO_RSPNS, strlen(AUDIO_RSPNS), length, data);
    }
  }while(length > 0);

  //Enviem la trama de EOF i calculem el MD5
  close(fd_file);
  char *buffer = ferMD5Sum(directori);
  sendTrama(fd, 5, FEO, strlen(FEO), strlen(buffer), buffer);
  free(buffer);

  //Esperem a la contestacio del client
  Packet p = llegirTrama(fd);
  destroyPacket(p);

  //Avisem que ja hem acabat de enviar
  SEM_signal(&read_files);

}

//Thread que escolta activament el canal per a la comunicacio amb algun client
void* acceptConnection(void *arg){
  NewUser user = *((NewUser*) arg);
  free(arg);

  //Confirmem la conexio i enviem el nom del Servidor
  sendTrama(user.noumitjo, 1, CON_OK, strlen(CON_OK), strlen(config.name), config.name);

  int exit = 0;

  do{

    Packet packet = llegirTrama(user.noumitjo);
    switch(packet.type){
      //SAY
      case 2:
        readSay(user.noumitjo, user.nom, packet);
        break;

      //BROADCAST
      case 3:
        readBroadcast(user.noumitjo, user.nom, packet);
        break;

      //SHOW AUDIOS
      case 4:
        sendAudioList(user.noumitjo);
        break;

      //DOWNLOAD AUDIO
      case 5:
        sendAudio(user.noumitjo, packet);
        break;

      //EXIT
      case 6:
        sendTrama(user.noumitjo,6,CON_OK,strlen(CON_OK),0,"");
        exit = 1;
        break;

      default:
        close(user.noumitjo);
        exit = 1;
        break;

    }

    destroyPacket(packet);

  }while(exit == 0);

  //Esborrem l'usuari de la nostra llista
  LLISTABID_reservar(lServer);
  LLISTABID_buscar_borrar(&lServer, pthread_self());
  LLISTABID_lliberar(lServer);

  free(user.nom);
  return NULL;
}

//esperem noves conexions i mirem si
void* listener(){

  //Creem el socket on escoltarem
  int noumitjo = 0;
  mainsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(mainsocket < 0){
    writeSynced("Error En crear el socket\n",strlen("Error En crear el socket\n"));
    return NULL;
  }

  //Fem el bind del socket
  struct sockaddr_in s_addr;
  memset (&s_addr, 0, sizeof (s_addr));
  s_addr.sin_family = AF_INET;
  s_addr.sin_port = htons (config.port);
  s_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(mainsocket, (void *) &s_addr, sizeof (s_addr)) < 0){
    writeSynced("Error: Bind erroni.\n",strlen("Error: Bind erroni.\n"));
    return NULL;
  }

  int exit = 0;

  //Indiquem a quin socket estarem escoltant
  listen(mainsocket, 1);

  do{

    Connexio con;
    con.nomConexio = NULL;
    socklen_t len = sizeof(s_addr);

    //Acceptarem les noves conexions
    noumitjo = accept(mainsocket, (void *) &s_addr, &len);

    //Mirem que la conexio sigui valida i si ho es creem el thread
    if(noumitjo < 0){
      exit = 1;
    }
    else{

      Packet packet = llegirTrama(noumitjo);

      if(packet.type != 1){
        close(noumitjo);
        destroyPacket(packet);
      }
      else{
        if(strcmp(packet.header, "[TR_NAME") != 0){
          sendTrama(noumitjo, 1, KO_CONEXIO, strlen(KO_CONEXIO), 0, "");
          close(noumitjo);
          destroyPacket(packet);
          return NULL;
        }
        else{
          //On desarem la informació de la nova conexio
          NewUser *arg = malloc(sizeof(*arg));

          arg->nom = (char *) malloc(packet.length);
          strcpy(arg->nom, packet.data);

          arg->noumitjo = noumitjo;

          destroyPacket(packet);



          con.fdClient = noumitjo;

          //Creem el seu thread
          pthread_create(&con.tit, NULL, acceptConnection, arg);

          //Desem la informacio de l'usuari en la estructura de dades
          LLISTABID_reservar(lServer);
          LLISTABID_inserirDreta(&lServer, con);
          LLISTABID_lliberar(lServer);
        }
      }
    }
  }while(exit == 0 && exitGeneral == 0);

  return NULL;
}

//Ens intentem conectar a un nou usuari
int createConnection(int port){

  //Mirem que no ens estem intentant conectar a nosaltres mateixos o que estigui dintre del rang de ports
  if(port == config.port || port > config.port_end || port < config.port_begin ){
    writeSynced("Error, port invalid\n", strlen("Error, port invalid\n"));
    return -1;
  }

  //Mirem que no tinguem una conexio amb aquest usuari
  LLISTABID_reservar(lClient);
  if(LLISTABID_buscar_port(lClient, port) != NULL){
    writeSynced("Error, usuari ja conectat\n", strlen("Error, usuari ja conectat\n"));
    return -1;
  }
  LLISTABID_lliberar(lClient);
  writeSynced( "Connecting...\n", strlen("Connecting...\n"));

  //Creem el socket
  int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sockfd < 0){
    writeSynced("Error En crear el socket\n",strlen("Error En crear el socket\n"));
    return -1;
  }

  struct sockaddr_in s_addr;
  memset(&s_addr, 0, sizeof(s_addr));
  s_addr.sin_family = AF_INET;
  s_addr.sin_port = htons(port);

  if(inet_aton(config.ip, &s_addr.sin_addr) == 0){
    writeSynced("Error: No es una adreça valida.\n",strlen("Error: No es una adreça valida.\n"));
    return -1;
  }

  //Ens connectem
  if (connect(sockfd, (void*) &s_addr, sizeof(s_addr)) < 0){
    writeSynced("Error: No s'ha pogut connectar.\n",strlen("Error: No s'ha pogut connectar.\n"));
    return -1;
  }

  //CONEXIO FETA VEIEM TRAMES
  sendTrama(sockfd, 1, TR_NAME, strlen(TR_NAME), strlen(config.name), config.name);

  //Llegim el nom del servidor
  Packet p = llegirTrama(sockfd);

  //Si header incorrecte o error fotem el camp
  if(strcmp(p.header, "[CONOK") != 0){
    //Cas en el que la conexio es rebutji
    writeSynced("Conexio rebutjada per el servidor\n",strlen("Conexio rebutjada per el servidor\n"));
    close(sockfd);
    destroyPacket(p);
    return -1;
  }

  //Inserim la conexio en la estructura de dades
  Connexio aux;
  aux.port = port;
  aux.fdClient = sockfd;
  aux.nomConexio = (char *) malloc(p.length+1);
  strcpy(aux.nomConexio, p.data);
  aux.nomConexio[p.length] = '\0';

  LLISTABID_reservar(lClient);
  LLISTABID_inserirDreta(&lClient, aux);
  LLISTABID_lliberar(lClient);
  char aux_string[19];

  //Mostrem per panrtalla la informacio de l'usuari
  sprintf(aux_string, "%d connected :", port);

  scrRsv();
    write(1, aux_string, strlen(aux_string));
    write(1, p.data, p.length);
    write(1,"\n", 1);
  scrFree();
  destroyPacket(p);

  return 0;
}

//Enviar un missatge al servidor demanat
int sendMessage(char *name, char *message){
  int sender;

  //Busquem l'usuari en la nostra estructura de dades i ens retorna el seu FD per a parlar amb ell, si no el troba retornara un 0 (que sabem que mai s'assignara a cap usuari perque )
  LLISTABID_reservar(lClient);
  sender = LLISTABID_buscar_fd(lClient, name);
  LLISTABID_lliberar(lClient);

  if(sender != 0){

    //Enviaem el missatge
    int error = sendTrama(sender, 2, MSG, strlen(MSG), strlen(message), message);

    //Si el missatge no es pot enviar significa que l'usuari s'ha desconnectat
    if(error < 0){
      writeSynced("Error, usuari s'ha desconetat\n",strlen("Error, usuari s'ha desconetat\n"));
      LLISTABID_reservar(lClient);
      LLISTABID_buscar_borrar_fd(&lClient, sender);
      LLISTABID_lliberar(lClient);
      close(sender);
      return 0;
    }

    //LLegim el ACK
    Packet p = llegirTrama(sender);
    destroyPacket(p);
  }
  else{
    writeSynced("Error, no existeix cap usuari amb aquest nom\n",strlen("Error, no existeix cap usuari amb aquest nom\n"));
  }

  return 0;

}

//Enviem un missatge a tots els usuaris que tenim connectats
void sendBroadcast(char *message){

  //Reservem la estructura de dades
  LLISTABID_reservar(lClient);
  LLISTABID_vesInici(&lClient);

  while(!LLISTABID_fi(lClient)){

    Connexio con = LLISTABID_consulta(lClient);
    int error = sendTrama(con.fdClient, 3, BROADCAST, strlen(BROADCAST), strlen(message), message);

    //Si l'usuari ja no esta conectat l'esborrem
    if(error < 0){
      LLISTABID_reservar(lClient);
      LLISTABID_buscar_borrar_fd(&lClient, con.fdClient);
      LLISTABID_lliberar(lClient);
      close(con.fdClient);
      return;
    }

    //LLegim al response i printem el ACK
    Packet p = llegirTrama(con.fdClient);
    destroyPacket(p);
    scrRsv();
      write(1,"[",1);
      write(1,con.nomConexio, strlen(con.nomConexio));
      write(1,"] Cool!\n",8);
    scrFree();
    LLISTABID_avanca(&lClient);

  }

  //Els altres processos ja poden modificar la estructura de dades
  LLISTABID_lliberar(lClient);

}

//Demanem al servidor el llistat de audios
void showAudios(char *name){

  int sender;
  LLISTABID_reservar(lClient);
  sender = LLISTABID_buscar_fd(lClient, name);
  LLISTABID_lliberar(lClient);

  //Mirem si l'usuari s'ha desconectat
  if(sender != 0){
    int error = sendTrama(sender, 4, SHOW_AUDIOS, strlen(SHOW_AUDIOS), 0, "");

    if(error < 0){
      writeSynced("Error, usuari s'ha desconetat\n",strlen("Error, usuari s'ha desconetat\n"));
      LLISTABID_reservar(lClient);
      LLISTABID_buscar_borrar_fd(&lClient, sender);
      LLISTABID_lliberar(lClient);
      close(sender);
      return;
    }


    Packet packet = llegirTrama(sender);

    scrRsv();
      write(1,"[",1);
      write(1,name,strlen(name));
      write(1,"] Audios\n",9);
      write(1,packet.data, packet.length);
    scrFree();
    destroyPacket(packet);
  }
  else{
    writeSynced("Error, no existeix cap usuari amb aquest nom\n",strlen("Error, no existeix cap usuari amb aquest nom\n"));
  }

  return;

}

//Descarreguem un audio desde el servidor
void downloadAudio(char* name, char* audio){
  int sender;
  LLISTABID_reservar(lClient);
  sender = LLISTABID_buscar_fd(lClient, name);
  LLISTABID_lliberar(lClient);

  Packet packet;

  //Mirem que l'usuari no s'hagi desconectat
  if(sender != 0){
    int error = sendTrama(sender, 5, AUDIO_RQST, strlen(AUDIO_RQST), strlen(audio), audio);

    if(error < 0){
      writeSynced("Error, usuari s'ha desconetat\n",strlen("Error, usuari s'ha desconetat\n"));
      LLISTABID_reservar(lClient);
      LLISTABID_buscar_borrar_fd(&lClient, sender);
      LLISTABID_lliberar(lClient);
      close(sender);
      return;
    }

    packet = llegirTrama(sender);

    //Mirem si el audio existeix
    if(strcmp(packet.header, "[AUDIO_KO") == 0){
        writeSynced("El audio solicitat no existeix\n",strlen("El audio solicitat no existeix\n"));
        destroyPacket(packet);
    }
    else{
      char directori[strlen(config.folder)+strlen(audio)+2];
      sprintf(directori,"%s/%s",config.folder, audio);

      //Creem el fitxer amb els permisos corresponents
      int fd_file = open(directori, O_WRONLY | O_TRUNC | O_CREAT, 0644);
      if(fd_file < 0){
        writeSynced("Error creant el arxiu\n",strlen("Error creant el arxiu\n"));
        destroyPacket(packet);
        return;
      }

      //Mirem si es el final de l'arxiu
      while(strcmp(packet.header, "[EOF") != 0){

        write(fd_file,packet.data,packet.length);
        destroyPacket(packet);
        packet = llegirTrama(sender);

      }
      close(fd_file);
      char *md5Local = ferMD5Sum(directori);
      char *md5Remot = packet.data;

      scrRsv();
        write(1,"[",1);
        write(1,name,strlen(name));
        write(1,"] ",2);

        //Mirem si coincideixen els MD5, si no coincidexem avisem al usuari de que els arxius poden estar corruptes
        if(strcmp(md5Local, md5Remot) == 0){
          write(1, audio, strlen(audio));
          write(1," downloaded\n", strlen(" downloaded\n"));

          sendTrama(sender, 5, MD5OK, strlen(MD5OK), 0, "");
        }
        else{
          write(1,"Error en descarregar el fitxer ", strlen("Error en descarregar el fitxer "));
          write(1, audio, strlen(audio));
          write(1,", checksum incorrecte (dades corruptes).\n",strlen(", checksum incorrecte (dades corruptes).\n"));
          sendTrama(sender, 5, MD5KO, strlen(MD5OK), 0, "");
        }
      scrFree();
      //lliberem memoria
      free(md5Local);
      destroyPacket(packet);

    }

  }
  else{
    writeSynced("Error, no existeix cap usuari amb aquest nom\n",strlen("Error, no existeix cap usuari amb aquest nom\n"));
  }

}

//HEM TRET CONFIG
int showConnections(){

  pid_t pid;
  int pipe_fd[2];

  pipe(pipe_fd);
  pid = fork();

  if(pid < 0){
    writeSynced("Error en Show Connections\n", strlen("Error en Show Connections\n"));
    //ERROR
    close(pipe_fd[0]);
    close(pipe_fd[1]);
  }
  else if(pid == 0){
    //CHILD
    dup2(pipe_fd[1], STDOUT_FD);
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    char port1[5];
    char port2[5];
    itoa(config.port_begin, port1, 10);
    itoa(config.port_end, port2, 10);
    execl(BASH_PATH, BASH_NAME, port1, port2, config.server, NULL);
    exit(1);

  }
  else if(pid > 0){
    //PARENT
    close(pipe_fd[1]); //no el necesitem, per tant el tanquem
    int sortir = 0;
    int con_count = 0;
    Ports *ports = NULL;

    wait(NULL);

    do{

      char *buffer = NULL;
      buffer = readLine(buffer, pipe_fd[0], '\n');

      if (strlen(buffer) == 0){
        sortir = 1;
      }else{

        con_count++;
        ports = realloc(ports, con_count * sizeof(Ports));
        int aux_count = 0;

        //recorrem el buffer buscant els numeros
        for (int i = 0; buffer[i] != '\0'; i++) {

          //si es un numero el guardem sino carrer
          if(buffer[i] >= '0' && buffer[i] <= '9'){

            ports[con_count - 1].port[aux_count] = buffer[i];
            aux_count++;

          }

        }

        ports[con_count - 1].port[aux_count] = '\0';
      }

      //lliberem memoria
      free(buffer);

    }while(sortir != 1);

    close(pipe_fd[0]);

    char aux[50];
    int con_count_aux = con_count;
    if(config.port >= config.port_begin && config.port <= config.port_end){
      con_count--;
    }

    sprintf(aux,"%d connections availables\n", con_count);
    scrRsv();
      write(1,aux,strlen(aux));

      //mostrem els ports disponibles
      for(int i = 0; i < con_count_aux; i++){
        if(config.port != atoi(ports[i].port)){
          write(1, ports[i].port, strlen(ports[i].port));
          LLISTABID_reservar(lClient);
          char * nom_aux = LLISTABID_buscar_port(lClient, atoi(ports[i].port));
          LLISTABID_lliberar(lClient);
          if(nom_aux != NULL){
            write(1," ", 1);
            write(1, nom_aux, strlen(nom_aux));
          }

          write(1, "\n", 1);

        }

      }
    scrFree();

    free(ports);

  }

  return 0;

}

void closeConnections(){

  LLISTABID_reservar(lClient);
  LLISTABID_vesInici(&lClient);

  while(!LLISTABID_fi(lClient) && !LLISTABID_buida(lClient)){

    Connexio con = LLISTABID_consulta(lClient);
    sendTrama(con.fdClient, 6, SORTIR, strlen(SORTIR), 0, "");

    Packet p = llegirTrama(con.fdClient);
    destroyPacket(p);

    shutdown(con.fdClient, SHUT_RDWR);
    close(con.fdClient);

    LLISTABID_avanca(&lClient);

  }

  LLISTABID_reservar(lServer);
  LLISTABID_vesInici(&lServer);

  void *res;

  while(!LLISTABID_buida(lServer)){

    LLISTABID_vesInici(&lServer);

    Connexio con = LLISTABID_consulta(lServer);

    //Tancar el socket del ususari
    //esperar a que aquell thread fagi join

    exitGeneral = 1;

    LLISTABID_lliberar(lServer);
    shutdown(con.fdClient, SHUT_RDWR);

    pthread_join(con.tit, &res);
    LLISTABID_reservar(lServer);

    close(con.fdClient);
  }

  shutdown(mainsocket, SHUT_RDWR);
  close(mainsocket);


  pthread_join(server_thread_id, &res);

  LLISTABID_destrueix(&lServer);
  LLISTABID_destrueix(&lClient);

  SEM_destructor(&read_files);

  //lliberem copida de config
  free(config.name);
  free(config.folder);
  free(config.ip);
  free(config.server);

}
