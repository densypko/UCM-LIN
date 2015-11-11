#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/errno.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

#ifdef __i386__
#define __NR_LED 353
#else
#define __NR_LED 316
#endif

#define buff 25

long ledctl(int num);
int isInt(char *cad);
void use();

int main(int argc, char **argv) {
	int num, len;
	char aux2[buff];
	char *aux;
	
	//check num of arguments
	if(argc !=2) {
		fprintf(stderr,"El nº de argumentos es incorrecto.\n");
		use();
		return -EXIT_FAILURE;	
	}

	
	aux=argv[1];
	//remove the 0x
	if(strlen(argv[1])>2) {
		if( argv[1][0] == '0'  && ( (argv[1][1] == 'x') || (argv[1][1] == 'X') ) ) {
			len = strlen(argv[1])-2 ;
			memcpy(aux2, &argv[1][2], len);
			aux2[len] = '\0';
			aux = aux2;
			
		}
		
		
	}
	//check the argument(char) is int
	if(!isInt(aux)) {
		fprintf(stderr,"El arguemnto es incorrecto.\n");
		use();
		return -EXIT_FAILURE;
	}
	//convert argument(char) into int
	num=atoi(aux);
	if(ledctl(num) == -1) {
		perror("Se ha producido un error");
		return -EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;

}


/*call the syscall 316(ledctl) return 0 if ok and return -1 if any error*/
long ledctl(int num) {	
	return (long)syscall(__NR_LED, num);	
}

/*when exist any error of the use, show the method to use this program*/
void use() {
	printf("Use:\n");
	printf("\t./ledctl < 0-7 || 0x0-0x7 >\n");
	printf("\t./ledctl -h\n");
}

/*return 1 when cad is int and return 0 in the other case*/
int isInt(char *cad) {
   int i, valor;

   for(i=0; i < strlen(cad); i++){
      valor = cad[i]-'0';

      if(valor < 0 || valor > 9) {
          if(i==0 && valor==-3) 
		continue;
          return 0;
      }
   }
   return 1;
}




























