#include <stdio.h>
#include <stdlib.h>

int b(int y, void (*fcn)(int, int *z));

//int *c = malloc(sizeof(int) * 10);
int i = 0;
void a(int x, int *c){
	c[i] = x;
	i++;
	if(i == 10)
		printf("Done\n");
	else
		b(i, &a(x, c));
}

int b(int y, void (*fcn)(int, int*)){
	y++;
	(*fcn)(y, int*);
	return y;
}

int main(){
	int *c = malloc(sizeof(int) * 10);
	int x = 10;
	a(x, c);
}
