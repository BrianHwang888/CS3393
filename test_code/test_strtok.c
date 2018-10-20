#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]){
	char message[] = "   the potato =is =a weird thingy";
	char message2[] = "you what potato";
	char *s = strtok(message, " ");
	printf("%s \n", s);
	int len;
	while(s != NULL){
		for(int i = 0; s[i] != '\0'; i++)
			len++;
		printf("token: %s\nlen: %d\n", s, len);
		s = strtok(NULL, " ");
		len = 0;
	}
	char *g = strtok(message2, "=");
	printf("g: %s %p\n", g);
	while(g != NULL){
		for(int i = 0; g[i] != '\0'; i++)
                        len++;
		printf("token: %s\nlen: %d\n", g, len);
		g = strtok(NULL, "=");
		printf("%p %d\n", g, g == NULL);
		len = 0;
	}
	char *war = "wap";
	char *we = "wap";
	printf("%d\n", war == we);
}
