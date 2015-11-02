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

long ledctl(int num);
void use();
int isInt(char *cad);

int main(int argc, void **argv) {
	int opt, num;
	if(argc !=2) {
		use();
		return -1;	
	}

	if(!isInt(argv[1])) {
		use();
		return -1;
	}

	num=atoi(argv[1]);
	if((num<0) || (num>7)) {
		use();
		return -1;
	}
	return ledctl(num);
}



long ledctl(int num) {
	if((long)syscall(__NR_LED, num)!=0)
		return -1;
	return 0;	
}

void use() {
	printf("Use:\n");
	printf("\t./ledctl <led[0-7]\n");
	printf("\t./ledctl -h\n");
}

void help() {
	printf("This program modify the keyword's leds\n");
	printf("\t ./ledctl <led>\n");
	printf("\t ./ledctl -h\n");
}

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
