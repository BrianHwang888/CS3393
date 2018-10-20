#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

char *endptr;
char *line;
int neighbor;


void input_check(char *input[], int param_size, int *num_rows, int *num_columns, FILE **fileptr, int *num_generations);
void read_file(FILE *fileptr, char **world, int *rows, int* columns);
void render_world(char **world, int row, int column, int times);

int main(int argc, char *argv[]){

	FILE *fp;
	int rows, columns = 12;
	int generations = 10;
	char *world;

	input_check(argv, argc, &rows, &columns, &fp, &generations);
	read_file(fp, &world, &rows, &columns);

	printf("Rows: %d\nColumns: %d\nGenerations: %d\n", rows, columns, generations);

	render_world(&world, rows, columns, generations);

	fclose(fp);
	return 0;
}

//checks to see if inputs given are valid
void input_check(char *input[], int num_param, int *num_rows, int *num_columns, FILE **fileptr, int *num_generations){
	/*
	checks if there are any inputs and
	if default file potato.txt is found
	*/

	if(num_param == 1){
                *fileptr = fopen("potato.txt", "r");
                if(fileptr == NULL){
                        printf("Error: default file missing\n");
                        exit(1);
		}
        }
	//checks if there is an input only for rows
	if(num_param == 2){
		printf("Error: did not specifiy column length\n");
		exit(1);
	}
	//checks if input has too much information provided
	if(num_param > 5){
		printf("Error: you provided too much information for this program\n");
		exit(1);
	}
	else{
		//checks to see if input for rows and columns are positive numbers
                if(num_param >= 3){
                        if((strtol(input[1], &endptr, 10) < 0) || (strtol(input[1], &endptr, 10) == 0 && errno == 0)){
                                printf("Error: must enter a positive number for rows\n");
                                exit(1);
                        }
                        if((strtol(input[2], &endptr, 10) < 0) || (strtol(input[2], &endptr, 10) == 0 && errno == 0)){
                                printf("Error: must enter a positive number for columns\n");
                                exit(1);
                        }
			*num_rows = strtol(input[1], &endptr, 10) + 2;
			*num_columns = strtol(input[2], &endptr, 10) + 2;

                }
		//checks to see if the file provide exists 
		if(num_param >= 4){
			if((*fileptr = fopen(input[3], "r")) == NULL){
				printf("Error: file not found\n");
				exit(1);
			}
		}
		//checks to see if input for generation is a positive number
                if(num_param == 5){
                	if((strtol(input[4], &endptr, 10) < 0) || (strtol(input[4], &endptr, 10) == 0 && errno == 0)) {
                                printf("Error: must enter a positive number for generations\n");
                                exit(1);
                        }
			*num_generations = strtol(input[4], &endptr, 10);
                }
	}
}

/*
reads in the file stored in fileptr
determines minimal rows and columns
based on file
*/
void read_file(FILE *fileptr, char **world,int *rows, int *columns){
	char *data;
	size_t len = 0;
	ssize_t buffer;
	int min_rows = 0;
	int min_col = 0;
	int curr_row = 1;
	int curr_col = 1;

	//gets maximum rows and columns of the file
	while ((buffer = getline(&data, &len, fileptr)) != -1) {
		if(min_col < buffer)
			min_col = buffer;
		min_rows++;
    	}
	free(data);
	/*
	checks to see if rows and columns specify is smaller
	than file
	*/
	if(*rows <= min_rows)
		*rows = min_rows * 2 + 2;
	if(*columns <= min_col)
		*columns = min_col * 2 + 2;

	*world = (char *)malloc((*rows) * (*columns) * sizeof(char));

	fseek(fileptr, 0, SEEK_SET);

	printf("Generation 0:\n");
	while(!feof(fileptr)){
                data = fgetc(fileptr);
               	if(data == '\n'){
			while(curr_col < *columns){
				(*world)[curr_row * *columns + curr_col] = '\0';
				curr_col++;
			}
			curr_row++;
			curr_col = 1;
		}
		(*world)[curr_row * *columns + curr_col] = data;
		curr_col++;
	}
	for(curr_row = 1; curr_row < (*rows)-1; curr_row++){
              	for(curr_col = 1; curr_col<  (*columns)-1; curr_col++)
			printf("%c", (*world)[curr_row * (*columns) + curr_col]);
	}
	printf("\n");
}

/*
creates the world based on game of life rules 
and the current state of the world
*/
void render_world(char **world, int row, int column, int times){
	int curr_row;
	int curr_col;
	int curr_gen = 0;
	char *rebirth;

	while(curr_gen < times){
		rebirth = (char *)malloc(row * column * sizeof(char));
		for(curr_row = 1; curr_row < row; curr_row++)
                	rebirth[curr_row * column + column-1] = '\n';
		printf("===================Generation %d:\n", curr_gen);
		for(curr_row = 1; curr_row < row-1; curr_row++){
                         for(curr_col = 1; curr_col<  column; curr_col++)
                                 printf("%c", (*world)[curr_row * column + curr_col]);
                }
		//checks each position of the world
		for(curr_row = 1; curr_row < row-1; curr_row++){
			for(curr_col = 1; curr_col < column-1; curr_col++){

				/*checks if index on world is dead
				checks the surrounding cells for neighbors
				if index is dead and neighbor = 3, cell is reborn ('*')
				if index is alive and neigbor = 2 || 3 cell lives o/w dead
				*/
				if((*world)[(curr_row-1) * column + curr_col - 1] == '*')
					neighbor++;

				if((*world)[(curr_row-1) * column + curr_col] == '*')
					neighbor++;

				if((*world)[(curr_row-1) * column + curr_col + 1] == '*')
                                       	neighbor++;

				if((*world)[curr_row * column + curr_col - 1] == '*')
				 	neighbor++;

				if((*world)[curr_row * column + curr_col + 1] == '*')
                                       	neighbor++;

				if((*world)[(curr_row+1) * column + curr_col - 1] == '*')
                                       	neighbor++;

				if((*world)[(curr_row+1) * column + curr_col] == '*')
                                       	neighbor++;

				if((*world)[(curr_row+1) * column + (curr_col + 1)] == '*')
				       	neighbor++;
				if((*world)[curr_row * column + curr_col] ==  '\0'){
					if(neighbor == 3)
						rebirth[(curr_row-1) * column + curr_col] = '*';
				}
				if((*world)[curr_row * column + curr_col] == '*'){
					if(neighbor == 3 || neighbor == 2)
                                                rebirth[(curr_row-1) * column + curr_col] = '*';
					else
						rebirth[(curr_row-1) * column + curr_col] = '\0';
				}
				neighbor = 0;
			}
		}
	*world = rebirth;
	free(rebirth);
	printf("\n");
	curr_gen++;
	}
}


