#include "llistaPDI.h"

void LLISTABID_reservar(LlistaBid l){
	SEM_wait(&l.semafor);
}

void LLISTABID_lliberar(LlistaBid l){
	SEM_signal(&l.semafor);
}

LlistaBid LLISTABID_crea(){
	LlistaBid l;
	SEM_constructor(&l.semafor);
	SEM_init(&l.semafor, 1);

	l.pri = (Node *) malloc(sizeof(Node));
	l.pri->e.nomConexio = NULL;
	if(l.pri == NULL){
		//Gestio error
		write(1,"Error en der la peticio de espai (LLISTABID)",strlen("Error en der la peticio de espai (LLISTABID)"));
	}
	else{
		l.ult = (Node *) malloc(sizeof(Node));
		l.ult->e.nomConexio = NULL;
		if(l.ult == NULL){
			//Gestio error
			write(1,"Error en der la peticio de espai (LLISTABID)",strlen("Error en der la peticio de espai (LLISTABID)"));
		}
		else{
			l.pdi = l.pri;
			l.pri -> seg = l.ult;
			l.pri -> ant = NULL;
			l.ult -> seg = NULL;
			l.ult -> ant = l.pri;
		}
	}
	return l;
}



//Inserim a la dreta de llista respecte el punt PDI

void LLISTABID_inserirEsquerra(LlistaBid * l, Connexio e){
	Node *aux;

	//Posible error: pdi = pri (fantasma)
	if(l->pri == l->pdi){
		write(1,"Error en introduir la conexio a la esquerra (LLISTABID)\n",strlen("Error en introduir la conexio a la esquerra (LLISTABID)\n"));
	}
	else {
		//demanem l'espai de memoria
		aux = (Node*) malloc(sizeof(Node));
		if(aux == NULL){
			write(1,"Error en introduir la conexio a la esquerra (LLISTABID)\n",strlen("Error en introduir la conexio a la esquerra (LLISTABID)\n"));
		}
		else {

			//apuntem els punters a les seves respectives posicions
			aux -> ant = l -> pdi -> ant;
			aux -> seg = l -> pdi;
			aux -> e = e;
			l -> pdi -> ant -> seg = aux;
			l -> pdi -> ant = aux;
		}
	}
}



//Inserim a la esquerra de llista respecte el punt PDI

void LLISTABID_inserirDreta(LlistaBid * l, Connexio e){
	Node *aux;

	//Posible error: pdi = ult (fantasma)
	if(l->ult == l->pdi){
		l-> pdi = l->pri;
	}

	//demanem l'espai de memoria
	aux = (Node *) malloc(sizeof(Node));

	if(aux == NULL){
		write(1,"Error en introduir la conexio a la dreta (LLISTABID)\n",strlen("Error en introduir la conexio a la dreta (LLISTABID)\n"));
	} else {
		aux->e = e;

		aux -> seg = l -> pdi -> seg;
		aux -> ant = l -> pdi;

		l -> pdi -> seg -> ant = aux;
		l -> pdi -> seg = aux;
	}


}

int LLISTABID_buscar_borrar(LlistaBid *l, pthread_t tit){
	if(LLISTABID_buida(*l) == 1){
			return 0;
	}

	LLISTABID_vesInici(l);
	while(!LLISTABID_fi(*l)){

		if(tit == LLISTABID_consulta(*l).tit){

			LLISTABID_esborra(l);
			return 1;

		}else{
			LLISTABID_avanca(l);
		}

	}

	return 0;

}

int LLISTABID_buscar_borrar_fd(LlistaBid *l, int sender){
	if(LLISTABID_buida(*l) == 1){
			return 0;
	}

	LLISTABID_vesInici(l);
	while(!LLISTABID_fi(*l)){

		if(sender == LLISTABID_consulta(*l).fdClient){

			LLISTABID_esborra(l);
			return 1;

		}else{
			LLISTABID_avanca(l);
		}

	}

	return 0;

}

char* LLISTABID_buscar_port(LlistaBid l, int port){
	if(LLISTABID_buida(l) == 1){
			return NULL;
	}

	LLISTABID_vesInici(&l);
	while(!LLISTABID_fi(l)){

		if(port == LLISTABID_consulta(l).port){
			char *aux = LLISTABID_consulta(l).nomConexio;
			return aux;

		}else{
			LLISTABID_avanca(&l);
		}
	}
	return NULL;
}

int LLISTABID_buscar_fd(LlistaBid l, char *user){
		if(LLISTABID_buida(l) == 1){
				return 0;
		}

	LLISTABID_vesInici(&l);

	while(!LLISTABID_fi(l)){

		if(strcmp(user, LLISTABID_consulta(l).nomConexio) == 0){
			return LLISTABID_consulta(l).fdClient;

		}else{
			LLISTABID_avanca(&l);
		}
	}
	return 0;
}


//Esborrem el valor al que apunta PDI
void LLISTABID_esborra(LlistaBid * l){
	Node *aux;

	if(l -> pdi == l -> ult || l -> pdi == l -> pri){
		//gestio error: PDI es troba al inici o final de la llista, es a dir, als fantasmes
		write(1,"Error en  esborrar la conexio (LLISTAPDI)\n",strlen("Error en  esborrar la conexio (LLISTAPDI)\n"));
	} else {

		//Asignem les noves direccions dels punters
		l -> pdi -> ant -> seg = l -> pdi -> seg;
		l -> pdi -> seg -> ant = l -> pdi -> ant;
		aux = l->pdi;
		l -> pdi = l -> pdi -> seg;



		//alliberem la memoria
		if(aux->e.nomConexio != NULL){
			free(aux->e.nomConexio);
		}

		free(aux);
	}
}



//Retornem el valor de la casella a la que apunta PDI

Connexio LLISTABID_consulta (LlistaBid l){
    Connexio e;
		e = l.pdi -> e;
	//cas en que PDI es trobi en algun dels fantasmes, retorno una Conexio falsa.
    return e;
}



//Apuntem PDI al primer valor de la llista

void LLISTABID_vesInici(LlistaBid *l){
	if(l->pri->seg != l->ult){

		l->pdi = l->pri->seg;

	}
}



//Apuntem PDI al ultim valor de la llista

void LLISTABID_vesFinal(LlistaBid *l){
	l->pdi = l->ult->ant;
}



//Fem avancar el PDI cap a la dreta

void LLISTABID_avanca(LlistaBid *l){
	if(l->pdi->seg != NULL){
		l->pdi = l->pdi->seg;
	}
}



//Fem retrocedir el PDI cap a la esquerra

void LLISTABID_retrocedeix(LlistaBid *l){
	if(l->pdi->ant != l->pri){
		l->pdi = l->pdi->ant;
	}
}



//Retorna un enter en forma binaria per a saber si ens trobem al inici de la llista

int LLISTABID_inici(LlistaBid l){
	int i = 0;
	//Comprovo si PDI es troba al davant del primer fantasma.
	if(l.pdi == l.pri){
		i = 1;
	}
	return i;
}



//Retorna un enter en forma binaria per a saber si ens trobem al final de la llista

int LLISTABID_fi(LlistaBid l){
	int i = 0;
	//Comprovo si PDI es troba al davant del ultim fantasma.
	if(l.pdi == l.ult){
		i = 1;
	}
	return i;
}



//Retorna un enter en forma binaria per a saber si la llista es buida

int LLISTABID_buida(LlistaBid l){
	int i = 0;
	//Comprovo si els dos fantasmes estan al costat
	if(l.pri -> seg == l.ult){
		i = 1;
	}
	return i;
}



//Alliberem tots els espais de memoria i eliminem tota la informacio.

void LLISTABID_destrueix(LlistaBid *l){

	Node *aux;

	//Avancem PDI fins arribar al final de la llista mentres aliberem memoria.
	while (l -> pri != NULL) {
		aux = l->pri;

		if(aux->e.nomConexio != NULL){
			free(aux->e.nomConexio);
		}

		l -> pri = l -> pri -> seg;
		free (aux);
	}

	SEM_destructor(&l->semafor);

}
