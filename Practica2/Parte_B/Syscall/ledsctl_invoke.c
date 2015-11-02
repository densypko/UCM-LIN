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
int isInt(char *cad);
void use();
void help();

int main(int argc, char **argv) {
	int opt, num, len;
	char *aux;
	//check num of arguments
	if(argc !=2) {
		use();
		return -1;	
	}
	//parser arguments
	while((opt=getopt(argc,argv,"h:"))!=-1) {
		switch(opt) {
			case 63: help(); return 0; break; //help option
		}	
	}
	
	//remove the x
	if(strlen(argv[1])>2) {
		if((argv[1][1] == 'x') || (argv[1][1] == 'X')) {
			char subbuff[5];
			len = strlen(argv)-2 ;
			memcpy(aux, &argv[1][2], len);
			aux[len] = '\0';
		}
	}
	//check the argument(char) is int
	if(!isInt(aux)) {
		use();
		return -1;
	}
	//convert argument(char) into int
	num=atoi(aux);
	if((num<0) || (num>7)) {
		use();
		return -1;
	}
	return ledctl(num);
}


/*call the syscall 316(ledctl) return 0 if ok and return -1 if any error*/
long ledctl(int num) {
	if((long)syscall(__NR_LED, num)!=0)
		return -1;
	return 0;	
}

/*when exist any error of the use, show the method to use this program*/
void use() {
	printf("Use:\n");
	printf("\t./ledctl <led[0-7]\n");
	printf("\t./ledctl -h\n");
}

/*show a short documentation of this program*/
void help() {
	printf("==============\n");
	printf("Instructions\n");
	printf("==============\n");
	printf("This program modify the keyword's leds.To use this program, it should be included as a parameter the combination of LEDs to be recreated. The leds encoding is as follow:\n");
	printf("\t [0] --  ALL OFF\n");
	printf("\t [1] --  Scroll\n");
	printf("\t [2] --  Caps\n");
	printf("\t [3] --  Scroll+Caps\n");
	printf("\t [4] --  Num\n");
	printf("\t [5] --  Num+Scroll\n");
	printf("\t [6] --  Num+Caps\n");
	printf("\t [7] --  ALL ON\n");
	printf("If you use the option -h, you can view this help.\n");
	printf("==========================\n");
	printf("Authors: Marco & Denys\n");
	printf("==========================\n");
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
