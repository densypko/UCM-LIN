#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_LENGTH 200
#define NUM_LEDS 8
char *OFF           = "0x000000";
char *RED_BRILLANT  = "0xAA0000";
char *BLUE_BRILLANT = "0x0000AA";

const char *file="/dev/usb/blinkstick0"; //Prepared to work with one device

/*All methods implementated back*/
void usage();
int showSec(char *sec);
int clear();
int police();
int rb_cross();
int rb_cross_back();
int salad();
int change_leds(int fd, char *c0,char *c1,char *c2,char *c3,char *c4,char *c5,char *c6,char *c7);

/*Reproduce the secuence that come into argv[1] and return the result of that
  exec.
 *(input)  argc = num of strings that user write
 *(input)  argv = List of strings that user write
 *(output) ret  = 0 if ok, -1 if ocure any error
 */
int main(int argc, char **argv) {
	int ret=0;
	//only one arguments and need that argument
	if((argc <2) || (argc>2)){
		usage();
		return -1;
	}
	
	//put off the leds and go to the secuence
	ret=clear();
	ret=showSec(argv[1]);
	return ret;
}

/*Check the input sec and go to exec the secuence or return -1
 *(input) sec = secuence to parser of the program
 *(output) ret = 0 if ok, -1 if ocure any error on the secuence or not exist de secuence
 */
int showSec(char *sec) {
	int ret=0;
	if(strcmp(sec,"police")==0)
		 ret=police();
	else if(strcmp(sec,"rbcross")==0)
		 ret=rb_cross();
	else if(strcmp(sec,"rbcrossback")==0)
		 ret=rb_cross_back();
	else if(strcmp(sec,"salad")==0)
		 ret=salad();
	else if(strcmp(sec,"clear")==0)
		 ret=clear();
	else {
		usage();
		return -1;
	}
	return ret;
}

/*Help of the program*/
void usage() {
	printf("Usage: ./blink_user.c [secuencia]\n\n");
	printf("Secuencias == Este campo corresponde con la secuencia que se desea reproducir.\nEs sensible a mayusculas y minusculas, es decir, 'police' no es lo mismo que 'Police'.\nComo posible secuencia especial tenemos 'help' que muestra esta ayuda.\nLas posibles secuencias son las siguientes:\n");
	printf("\t-  police = Reproduce las luces de un coche de policia\n");
	printf("\t-  rbcross = Secuencia de rojo y azul cruzandose con retroalimentacion\n");
	printf("\t-  rbcrossback = Secuencia de rojo y azul cruzandose\n");
	printf("\t-  salad = Viaja por el rango de colores rgb en dos componentes\n");
	printf("\t-  clear = Apaga todos los leds\n");
	printf("\t-  help = Muestra la ayuda por pantalla\n");
	printf("==========================\n");
	printf("Authors: Marco & Denis\n");
	printf("==========================\n");
}

/*Change leds color to input value.
 *(input)  fd                      = file to set the value
 *(input)  c0,c1,c2,c3,c4,c5,c6,c7 = leds to set the color
 *(output) ret                    = 0 if ok, -1 if ocure any error
 */
int change_leds(int fd, char *c0,char *c1,char *c2,char *c3,char *c4,char *c5,char *c6,char *c7) {
	char *buf= (char*)malloc(sizeof(char)*BUFFER_LENGTH);
	if(!buf) {
		printf("Se ha producido un error con la reserva de memoria\n");
		return -1;
	}
	if (fd ==-1) {
		free(buf);
		printf("Se ha producido un error en la lectura del dispositivo\n");
		return -1;	
	}
	
	strcpy(buf,"0:");
	strcat(buf,c0);
	strcat(buf,",1:"); 
	strcat(buf,c1);
	strcat(buf,",2:"); 
	strcat(buf,c2);
	strcat(buf,",3:"); 
	strcat(buf,c3);
	strcat(buf,",4:");
	strcat(buf,c4);
	strcat(buf,",5:"); 
	strcat(buf,c5);
	strcat(buf,",6:"); 
	strcat(buf,c6);
	strcat(buf,",7:"); 
	strcat(buf,c7);
	write(fd,buf,strlen(buf));
	free(buf);
	return 0;
}

/*Put all leds off. clear() is considareted a secuence. Return -1 if ocure any error. Return 0 if ok*/
int clear() {
	int ret=0;
	int fd = open(file, O_WRONLY);
	ret=change_leds(fd,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF);
	close(fd);
	return ret;
}

/*simulate a light of police car. Return -1 if ocure any error. Return 0 if ok*/
int police() {
	int time=250000;
	int replay=20,i,ret=0;
	int fd = open(file, O_WRONLY);
	
	for(i=0;i<replay;i++) {
		if((ret=change_leds(fd,RED_BRILLANT,RED_BRILLANT,RED_BRILLANT,RED_BRILLANT,OFF,OFF,OFF,OFF)) == -1) break;
		usleep(time);

		if((ret=change_leds(fd,OFF,OFF,OFF,OFF,BLUE_BRILLANT,BLUE_BRILLANT,BLUE_BRILLANT,BLUE_BRILLANT)) == -1) break;
		usleep(time);
	}
	close(fd);
	clear();
	return ret;
}

/*cross the led red and blue and restart when finish. Return -1 if ocure any error. Return 0 if ok*/
int rb_cross() {
	int time=250000;
	int replay=10,i,j,ret=0;
	int fd = open(file, O_WRONLY);
	char *buf= (char*)malloc(sizeof(char)*BUFFER_LENGTH);
	
	if(!buf) {
		printf("Se ha producido un error con la reserva de memoria\n");
		return -1;
	}
	if (fd ==-1) {
		free(buf);
		printf("Se ha producido un error en la lectura del dispositivo\n");
		return -1;	
	}
	for(i=0;i<replay;i++) {
		for(j=0;j<8;j++) {
			strcpy(buf,"0:");
			if(j==0) strcat(buf,RED_BRILLANT);
			else if(j==7) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",1:"); 
			if(j==1) strcat(buf,RED_BRILLANT);
			else if(j==6) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",2:"); 
			if(j==2) strcat(buf,RED_BRILLANT);
			else if(j==5) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",3:"); 
			if(j==3) strcat(buf,RED_BRILLANT);
			else if(j==4) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",4:"); 
			if(j==4) strcat(buf,RED_BRILLANT);
			else if(j==3) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",5:"); 
			if(j==5) strcat(buf,RED_BRILLANT);
			else if(j==2) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",6:"); 
			if(j==6) strcat(buf,RED_BRILLANT);
			else if(j==1) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",7:"); 
			if(j==7) strcat(buf,RED_BRILLANT);
			else if(j==0) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			write(fd,buf,strlen(buf));
			usleep(time);
		}
	}
	free(buf);
	close(fd);
	clear();
	return ret;
}

/*led red and blue cross and back cross again. Return -1 if ocure any error. Return 0 if ok*/
int rb_cross_back() {
	int replay=10,i,j,ret=0,c;
	int fd = open(file, O_WRONLY);
	int control = 1;
	char *buf= (char*)malloc(sizeof(char)*BUFFER_LENGTH);
	
	if(!buf) {
		printf("Se ha producido un error con la reserva de memoria\n");
		return -1;
	}
	if (fd ==-1) {
		free(buf);
		printf("Se ha producido un error en la lectura del dispositivo\n");
		return -1;	
	}
	for(i=0;i<replay;i++) {
		if(i%2==1) control=1;
		else control=0;
		for(j=0;j<7;j++) {
			if(!control) {
				c=j;
				j=(NUM_LEDS-1)-j;
			}
			strcpy(buf,"0:");
			if(j==0) strcat(buf,RED_BRILLANT);
			else if(j==7) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",1:"); 
			if(j==1) strcat(buf,RED_BRILLANT);
			else if(j==6) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",2:"); 
			if(j==2) strcat(buf,RED_BRILLANT);
			else if(j==5) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",3:"); 
			if(j==3) strcat(buf,RED_BRILLANT);
			else if(j==4) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",4:"); 
			if(j==4) strcat(buf,RED_BRILLANT);
			else if(j==3) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",5:"); 
			if(j==5) strcat(buf,RED_BRILLANT);
			else if(j==2) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",6:"); 
			if(j==6) strcat(buf,RED_BRILLANT);
			else if(j==1) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);

			strcat(buf,",7:"); 
			if(j==7) strcat(buf,RED_BRILLANT);
			else if(j==0) strcat(buf,BLUE_BRILLANT);
			else strcat(buf,OFF);
			
			if(!control)
				j=c;
			write(fd,buf,strlen(buf));
			usleep(250000);
		}
	}
	free(buf);
	close(fd);
	clear();
	return ret;
}

/*Travel to all RGB colors into two components. Return -1 if ocure any error. Return 0 if ok*/
int salad() {
	int time=25000/2;
	int max=90,min=0;
	int ret=0,i;
	int fd = open(file, O_WRONLY);
	char *buf= (char*)malloc(sizeof(char)*BUFFER_LENGTH);
	char color[6];
	int red=min,green=min,blue=min;

	//go up red color to max
	while((red<max-1)&&(ret==0)){
		char rcolor[2],gcolor[2],bcolor[2];
		red++;
		if(red>15)sprintf(rcolor,"%x",red);
		else sprintf(rcolor,"0%x",red);

		if(green>15)sprintf(gcolor,"%x",green);
		else sprintf(gcolor,"0%x",green);
	
		if(blue>15)sprintf(bcolor,"%x",blue);
		else sprintf(bcolor,"0%x",blue);
		strcpy(color,"0x"); strcat(color,rcolor);strcat(color,gcolor);strcat(color,bcolor);
		if((ret=change_leds(fd,color,color,color,color,color,color,color,color)) == -1) break;
		usleep(time*2);
	}
	for(i=0;i<2;i++){
		//go up blue color to max
		while((blue<max-1)&&(ret==0)){
			char rcolor[2],gcolor[2],bcolor[2];
			blue++;
			if(red>15)sprintf(rcolor,"%x",red);
			else sprintf(rcolor,"0%x",red);

			if(green>15)sprintf(gcolor,"%x",green);
			else sprintf(gcolor,"0%x",green);
		
			if(blue>15)sprintf(bcolor,"%x",blue);
			else sprintf(bcolor,"0%x",blue);
			strcpy(color,"0x"); strcat(color,rcolor);strcat(color,gcolor);strcat(color,bcolor);
		
			if((ret=change_leds(fd,color,color,color,color,color,color,color,color)) == -1) break;
			usleep(time);
		}

		//go down red color to min
		while((red>min)&&(ret==0)){
			char rcolor[2],gcolor[2],bcolor[2];
			red--;
			if(red>15)sprintf(rcolor,"%x",red);
			else sprintf(rcolor,"0%x",red);

			if(green>15)sprintf(gcolor,"%x",green);
			else sprintf(gcolor,"0%x",green);
		
			if(blue>15)sprintf(bcolor,"%x",blue);
			else sprintf(bcolor,"0%x",blue);
			strcpy(color,"0x"); strcat(color,rcolor);strcat(color,gcolor);strcat(color,bcolor);
		
			if((ret=change_leds(fd,color,color,color,color,color,color,color,color)) == -1) break;
			usleep(time);
		}
		
		//go up blue color to max
		while((green<max-1)&&(ret==0)){
			char rcolor[2],gcolor[2],bcolor[2];
			green++;
			if(red>15)sprintf(rcolor,"%x",red);
			else sprintf(rcolor,"0%x",red);

			if(green>15)sprintf(gcolor,"%x",green);
			else sprintf(gcolor,"0%x",green);
		
			if(blue>15)sprintf(bcolor,"%x",blue);
			else sprintf(bcolor,"0%x",blue);
			strcpy(color,"0x"); strcat(color,rcolor);strcat(color,gcolor);strcat(color,bcolor);
		
			if((ret=change_leds(fd,color,color,color,color,color,color,color,color)) == -1) break;
			usleep(time);
		}

		//go down blue color to min
		while((blue>min)&&(ret==0)){
			char rcolor[2],gcolor[2],bcolor[2];
			blue--;
			if(red>15)sprintf(rcolor,"%x",red);
			else sprintf(rcolor,"0%x",red);

			if(green>15)sprintf(gcolor,"%x",green);
			else sprintf(gcolor,"0%x",green);
		
			if(blue>15)sprintf(bcolor,"%x",blue);
			else sprintf(bcolor,"0%x",blue);
			strcpy(color,"0x"); strcat(color,rcolor);strcat(color,gcolor);strcat(color,bcolor);
		
			if((ret=change_leds(fd,color,color,color,color,color,color,color,color)) == -1) break;
			usleep(time);
		}
	
		//go up blue color to max
		while((red<max-1)&&(ret==0)){
			char rcolor[2],gcolor[2],bcolor[2];
			red++;
			if(red>15)sprintf(rcolor,"%x",red);
			else sprintf(rcolor,"0%x",red);

			if(green>15)sprintf(gcolor,"%x",green);
			else sprintf(gcolor,"0%x",green);
		
			if(blue>15)sprintf(bcolor,"%x",blue);
			else sprintf(bcolor,"0%x",blue);
			strcpy(color,"0x"); strcat(color,rcolor);strcat(color,gcolor);strcat(color,bcolor);
			if((ret=change_leds(fd,color,color,color,color,color,color,color,color)) == -1) break;
			usleep(time);
		}

		//go down green color to min
		while((green>min+1)&&(ret==0)){
			char rcolor[2],gcolor[2],bcolor[2];
			green--;
			if(red>15)sprintf(rcolor,"%x",red);
			else sprintf(rcolor,"0%x",red);

			if(green>15)sprintf(gcolor,"%x",green);
			else sprintf(gcolor,"0%x",green);
		
			if(blue>15)sprintf(bcolor,"%x",blue);
			else sprintf(bcolor,"0%x",blue);
			strcpy(color,"0x"); strcat(color,rcolor);strcat(color,gcolor);strcat(color,bcolor);
		
			if((ret=change_leds(fd,color,color,color,color,color,color,color,color)) == -1) break;
			usleep(time);
		}
	}

	//go down red color to min
	while((red>min)&&(ret==0)){
		char rcolor[2],gcolor[2],bcolor[2];
		red--;
		if(red>15)sprintf(rcolor,"%x",red);
		else sprintf(rcolor,"0%x",red);

		if(green>15)sprintf(gcolor,"%x",green);
		else sprintf(gcolor,"0%x",green);
		
		if(blue>15)sprintf(bcolor,"%x",blue);
		else sprintf(bcolor,"0%x",blue);
		strcpy(color,"0x"); strcat(color,rcolor);strcat(color,gcolor);strcat(color,bcolor);
		
		if((ret=change_leds(fd,color,color,color,color,color,color,color,color)) == -1) break;
		usleep(time*2);
	}
	//free resources
	close(fd);
	clear();
	return ret;
}
