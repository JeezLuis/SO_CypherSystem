//Lluis Farre Castan (lluis.farre)
//Joan Esteban Santaeularia (joan.esteban)

#include "utilities.h"
#include <stdio.h>

//Semafor de sincronitzacio per escriure per pantalla
semaphore semWrite;


char* readLine(char* buffer,int fd,char delimiter){
  char data[1];
  int index = 0;

  //check si el buffer ha sigut inicilitzat o no
  if(buffer != NULL){
    free(buffer);
    buffer = NULL;
  }

  int aux;
  do{

    aux = read(fd,data,1);

    //llegim caracter a caracter copiant si no es el delimitador
    if(data[0] != delimiter){

      buffer = (char *) realloc(buffer, index+1);
      buffer[index] = data[0];
      index++;

    }

  }while (data[0] != delimiter && aux != 0);  //llegim fins el delimitador

  //realloc per ficar \0 final de la cadena
  buffer = (char *) realloc(buffer, index+1);
  buffer[index] = '\0';

  return buffer;
}


char* stringToUpper(char* string, char* new_string){

  new_string = (char *) realloc(new_string, strlen(string)+1);
  new_string[strlen(string)] = '\0';

  for(int i = 0; i < (int) strlen(string); i++){
    if(string[i] >= 'a' && string[i] <= 'z'){
      new_string[i] = 'A' + (string[i] - 'a');
    }
    else{
      new_string[i] = string[i];
    }
  }

  return new_string;
}

char* inString(char* string1, char* string2, char* result){
  int max_len;
  int i;

  if(strlen(string1) > strlen(string2)){
    max_len = strlen(string2);
  }
  else{
    max_len = strlen(string1);
  }

  for (i = 0; i < max_len; i++) {
    if(string1[i] != string2[i]){
      result[i] = '\0';
      return result;
    }
    else{
      result[i] = string1[i];
      result = (char *) realloc(result, i+2);
    }
  }
  result[i] = '\0';

  return result;
}

int portTest(char* port){

  //mirem que el port no tingui mes de 5 caracters
  if(strlen(port) > 5){
    return -1;
  }

  //mirem que el port no siguiu NULL
  if(port == NULL){
    return -2;
  }

  //mirem que el port estigui dins el rang indicat al arxiu configuracio
  for(int i = 0; i < (int) strlen(port); i++){
    if(port[i] <= '0' && port[i] >= '9'){
      return -1;
    }
  }

  return atoi(port);
}

void swap(char *x, char *y) {
	char t = *x;
  *x = *y;
  *y = t;
}

char* reverse(char *buffer, int i, int j){
	while (i < j){
		swap(&buffer[i++], &buffer[j--]);
  }

	return buffer;
}

// Iterative function to implement itoa() function in C
char* itoa(int value, char* buffer, int base)
{
	// invalid input
	if (base < 2 || base > 32)
		return buffer;

	// consider absolute value of number
	int n = abs(value);

	int i = 0;
	while (n)
	{
		int r = n % base;

		if (r >= 10)
			buffer[i++] = 65 + (r - 10);
		else
			buffer[i++] = 48 + r;

		n = n / base;
	}

	if (i == 0)
		buffer[i++] = '0';

	if (value < 0 && base == 10)
		buffer[i++] = '-';

	buffer[i] = '\0';

	return reverse(buffer, 0, i - 1);
}



void initSemWriteSync(){
  SEM_constructor(&semWrite);
  SEM_init(&semWrite, 1);
}

void destructorSemWrite(){
  SEM_destructor(&semWrite);
}



void writeSynced(char *msg, int length){

  SEM_wait(&semWrite);
  write(1, msg, length);
  SEM_signal(&semWrite);

}

//Per a fer multiples writes a pantalla en el thread principal, reservem el semafior
void scrFree(){
  SEM_signal(&semWrite);
}

//Desbloqueja el semafor
void scrRsv(){
  SEM_wait(&semWrite);
}

//Dintre un fork executem usant un bash la comanda de MD5SUM
char* ferMD5Sum(char* dir){

  pid_t pid;
  int pipe_fd[2];

  pipe(pipe_fd);
  pid = fork();

  if(pid < 0){
    //ERROR
    close(pipe_fd[1]);
  }
  else if(pid == 0){
    //CHILD
    dup2(pipe_fd[1], STDOUT_FD);
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    execl(MD5_PATH, MD5_NAME, dir, NULL);
    exit(1);

  }
  else if(pid > 0){
    //PARENT
    close(pipe_fd[1]); //no el necesitem, per tant el tanquem
    char *buffer = NULL;
    wait(NULL);
    buffer = readLine(buffer, pipe_fd[0], ' ');
    return buffer;

  }

  return NULL;

}
