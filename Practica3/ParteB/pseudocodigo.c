mutex mtx;                        /*Garantiza exclusion mutua del buffer*/
condvar prod,cons;                /*Bloquea o desbloque los productores o consumidores segun sea necesario*/
int prod_count=0,cons_count=0;    /*Numero de productores y consumidores actualmente*/
cbuffer_t* cbuffer;               /*Recurso compartido implementado como buffer circular*/

/*
 *Abre un /proc en el modo especificado por el parametro de entrada
 *bool abre_para_lectura ==> 1 abre en modo lectura, 0 abre en modo escritura
 */
void fifoproc_open(bool abre_para_lectura) {
	lock(mtx);
		/*Si es para lectura, aumentamos el numero de consumidores,
		 *mandamos un signal a los productores para despertarlos 
		 *en el caso de que estuvieran bloqueados por falta de consumidores
		 *y en el caso de que no haya productores bloqueamos el proceso
		 */
		if(abre_para_lectura) {
			cons_count++;
			cond_signal(prod);
			while(prod_count==0)
				cond_wait(cons,mtx);
		}
		/*Realizamos el mismo algoritmo previamente explicado intercambiando
		 *los productores por los consumidores en el modo de operacion
		 */
		else {
			prod_count++;
			cond_signal(cons);
			while(cons_count==0)
				cond_wait(prod,mtx);
		}
	unlock(mtx);
}

void fifoproc_write(char* buff, int len) {
	char kbuffer[MAX_KBUF];

	if(len > MAX_CBUFFER_LEN || len>MAX_KBUF) { return Error;}
	if(copy_from_user(kbuffer, buff, len)) {return Error;}

	lock(mtx);
		/*Esperar hasta que haya hueco para insertar (debe haber consumidores)*/
		while(nr_gaps_cbuffer_t(cbuffer)<len && cons_count>0) {
			cond_wait(prod,mtx);
		}

		/*Detectar fin de comunicacion por error (consumidor cierrar FIFO antes)*/
		if (cons_count==0) {unlock(mtx); return -EPIPE;}

		insert_items_cbuffer_t(cbuffer,kbuffer,len);

		/*Despertar a posible consumidor bloqueado*/
		cond_signal(cons);
	unlock(mtx);
	return len;
}

void fifoproc_read(const char* buff, int len) {
	char kbuffer[MAX_KBUF];
	
	if(len > MAX_CBUFFER_LEN || len>MAX_KBUF) { return Error;}
	lock(mtx);
		/*Esperar hasta que haya elementos que leer*/
		while(size_cbuffer_t(cbuffer)==0 && prod_count>0) {
			cond_wait(cons,mtx);
		}
		
		/*En caso de no existir productores finaliza con EOF*/
		if (prod_count==0) {unlock(mtx); return 0;}

		/*Si no hay suficientes elementos para satisfacer la demanda de lectura,
		 *lee los elementos que haya en el buffer*/
		if (size_cbuffer_t(cbuffer)<len)
			memcpy(kbuffer,head_cbuffer_t(cbuffer),size_cbuffer_t(cbuffer));
		/*Si hay suficientes elementos, los lee*/
		else
			memcpy(kbuffer,head_cbuffer_t(cbuffer),len);

		/*Elimina los elementos leidos del buffer y manda una señal a los productores
		 *por si alguno dormido*/
		remove_items_cbuffer_t(cbuffer, head_cbuffer_t(cbuffer), len);
		cond_signal(prod);
	unlock(mtx);
	return len;
}

/*
 *Cierra un /proc en el modo especificado por el parametro de entrada
 *bool lectura ==> 1 cierra en modo lectura, 0 cierra en modo escritura
 */
void fifoproc_release(bool lectura) {
	lock(mtx);
		/*Si queremos cerrar un fichero en modo lectura, reducimos
		 *el numero de consumidores en una unidad y, en caso de que
		 *ya no existan productores, limpiamos el buffer*/
		if(lectura) {
			cons_count--;
			if(prod_count ==0)
				clear_cbuffer_t(cbuffer);
				
		}

		/*Si queremos cerrar un fichero en modo escritura, reducimos
		 *el numero de productores en una unidad y, en caso de que
		 *ya no existan consumidores, limpiamos el buffer*/
		else {
			prod_count--;
			if(cons_count ==0)
				clear_cbuffer_t(cbuffer);
		}
	unlock(mtx);
}
