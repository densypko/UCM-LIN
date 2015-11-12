#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NR_LEDS 8
int main(int argc, char **argv) {
	int i,j=0,len=0, k=0;
	int pl=0;
	char *input;
	char **buf;
	int led[NR_LEDS],color[NR_LEDS];

	
	for(i=1;i<argc;i++)
		len += strlen(argv[i]);

	/*if(len > BUFFER_LENGTH)
		return -ENOSPC;*/
	
	input = (char*)malloc(sizeof(char)*len);
	if(!input) 
		return -1;
	buf = (char**)malloc(NR_LEDS*sizeof(char*));
	buf[0] = (char*)malloc(sizeof(char)*len);
	

	for(i=1;i<argc;i++)
		strcat(input,argv[i]);
	/*if(copy_from_user(input,user_buffer,len)
		return -EFAULT;*/

	input[len] = '\n';
	for(i=0;i<strlen(input);i++){
		if(pl+1>=NR_LEDS) {
			for(k=0;k<pl+1; k++) {
				free(buf[k]);
			}
			free(buf);
			printf("No existen tantos leds\n");
			return -1;
		}
		if(input[i]==',') {
			memcpy(buf[pl],&input[j], i-j);
			pl++;
			buf[pl] = (char*)malloc(sizeof(char)*len);
			j=i+1;	
		}
	}

	memcpy(buf[pl],&input[j], len-j);
	
	free(input);
	for(i=0;i<pl+1;i++){
		if(sscanf(buf[i],"%d:0x%d", &led[i], &color[i]) == 1) {
			for(k=0;k<pl+1; k++) {
				free(buf[k]);
			}
			free(buf);
			printf("La cadena introducida no cumple los objetivos\n");
			return -1;
		} 
	}
	for(i=0;i<pl+1;i++){
		printf("Led: %d ==> Color: %d\n",led[i],color[i]);
	}

	for(k=0;k<pl+1; k++) {
		free(buf[k]);
	}
	free(buf);
	return 0;
}
