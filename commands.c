//Lluis Farre Castan (lluis.farre)
//Joan Esteban Santaeularia (joan.esteban)

#include "commands.h"

#define COMMAND_AMOUNT 7

char* commands[] = {"SHOW CONNECTIONS", "CONNECT ", "SAY ", "BROADCAST ", "SHOW AUDIOS ", "DOWNLOAD ", "EXIT"};

int commandManager(char* currentCommand){

  int port;
  char* user;
  char* text;
  char* audio;
  char* token;

  char current_comand2[strlen(currentCommand)];

  strcpy(current_comand2, currentCommand);

  int commandId = -1;
  int num_param = 0;

  const char delimiter[2] = " ";
  const char delimiter_quotation_marks[2] = "\0";

  char* new_string = (char *) malloc(sizeof(char));
  new_string = stringToUpper(currentCommand,new_string);  //make all caps
  char* aux = (char*) malloc(1);

  for (int i = 0; i < COMMAND_AMOUNT; i++) {

    aux = inString(new_string, commands[i], aux);

    //Mirem que el que ha introduit l'usuari no estigui buit
    if (strcmp(aux,"\0") != 0){

      //Mirem si correspon a alguna comanda i quin id te
      if (strcmp(aux, commands[i]) == 0) { //check if command exists in string

        commandId = i;
        break;

      }

    }

  }

  free(new_string);
  free(aux);

  num_param = 0;

  switch (commandId) {

    case 0:
      //Command SHOW CONNECTIONS

      writeSynced("Testing...\n",strlen("Testing...\n"));
      showConnections();
      break;

    case 1:

      //Command CONNECT
      token = strtok(current_comand2, delimiter);
      num_param++;
      token = strtok(NULL, delimiter);

      //Mirem que hi hagin parametres
      if(token == NULL){
        writeSynced("Missing parameters\n",strlen("Missing parameters\n"));
        break;
      }


      port = portTest(token);

      if(port == -2){
        writeSynced("Missing parameters\n",strlen("Missing parameters\n"));
        break;
      }

      if(port == -1){
        writeSynced("Wrong port format\n",strlen("Wrong port format\n"));
        break;
      }

      /* walk through other tokens */
      while( token != NULL ) {
        token = strtok(NULL, delimiter);
        num_param++;
      }

      if(num_param != 2){
        writeSynced("Too many parameters\n",strlen("Too many parameters\n"));
        break;
      }
      else{
        //Ens conectem
        createConnection(port);
      }

      break;

    case 2:

      //Command SAY
      token = strtok(current_comand2, delimiter);
      num_param++;
      user = strtok(NULL, delimiter);

      if(user == NULL){
        writeSynced("Missing parameters\n",strlen("Missing parameters\n"));
        break;
      }

      num_param++;
      text = strtok(NULL, delimiter_quotation_marks);

      //Falten parametres
      if(text == NULL){
        writeSynced("Missing parameters\n",strlen("Missing parameters\n"));
        break;
      }

      if((text[0] != '\"') || (text[strlen(text)-1] != '\"')){
        writeSynced("Incorrect message format\n",strlen("Incorrect message format\n"));
        break;
      }

      /* walk through other tokens */
      while( token != NULL ) {
        token = strtok(NULL, delimiter);
        num_param++;
      }

      if(num_param != 3){
        writeSynced("Too many parameters\n",strlen("Too many parameters\n"));
      }
      else{
        sendMessage(user,text);
      }
      break;

    case 3:

      //Command BROADCAST
      token = strtok(current_comand2, delimiter);
      num_param++;
      text = strtok(NULL, delimiter_quotation_marks);
      if(text == NULL){
        writeSynced("Missing parameters\n",strlen("Missing parameters\n"));
        break;
      }

      if((text[0] != '\"') || (text[strlen(text)-1] != '\"')){
        writeSynced("Incorrect message format\n",strlen("Incorrect message format\n"));
        break;
      }

      /* walk through other tokens */
      while( token != NULL ) {
        token = strtok(NULL, delimiter);
        num_param++;
      }

      if(num_param != 2){
        writeSynced("Too many parameters\n",strlen("Too many parameters\n"));
      }
      else{
        //do whatever
        sendBroadcast(text);
      }
      break;

    case 4:

      //Command SHOW AUDIOS
      token = strtok(current_comand2, delimiter);
      token = strtok(NULL, delimiter);
      num_param++;
      user = strtok(NULL, delimiter);

      if(user == NULL){
        writeSynced("Missing parameters\n",strlen("Missing parameters\n"));
        break;
      }

      /* walk through other tokens */
      while( token != NULL ) {
        token = strtok(NULL, delimiter);
        num_param++;
      }

      if(num_param != 2){
        writeSynced("Too many parameters\n",strlen("Too many parameters\n"));
      }
      else{
        //do whatever
        showAudios(user);
      }

      break;

    case 5:

      //Command DOWNLOAD
      token = strtok(current_comand2, delimiter);
      num_param++;
      user = strtok(NULL, delimiter);
      if(user == NULL){
        writeSynced("Missing parameters\n",strlen("Missing parameters\n"));
        break;
      }
      num_param++;
      audio = strtok(NULL, delimiter);
      if(audio == NULL){
        writeSynced("Missing parameters\n",strlen("Missing parameters\n"));
        break;
      }

      /* walk through other tokens */
      while( token != NULL ) {
        token = strtok(NULL, delimiter);
        num_param++;
      }

      //Mes parametres dels esperats
      if(num_param != 3){
        writeSynced("Too many parameters\n",strlen("Too many parameters\n"));
      }
      else{
        downloadAudio(user,audio);
      }
      break;

    case 6:

      //Miro que no hi hagin mes parametres de la conta
      token = strtok(current_comand2, delimiter);
      num_param++;
      token = strtok(NULL, delimiter);

      //Mirem que hi hagin parametres
      if(token != NULL){
        writeSynced("Too many parameters\n",strlen("Too many parameters\n"));
        break;
      }

      //Command EXIT
      return 1;

      break;

    default:
      writeSynced("Syntax error\n",strlen("Syntax error\n"));

  }
  return 0;
}
