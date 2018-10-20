#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

//constants
//Maxium length for path for Linux system
#define MAX_PATH 4096
#define ZERO 0
#define NUM_FILES 100

//globals
int kilibyte = 1024;
int arr_index = 0;

//struct holding file IDs
typedef struct file_ID{
	ino_t file_ino;
}f_id;

typedef struct file_array{
	int size;
	int cap;
	f_id **fid_arr;

}f_arr;

int scan_dir(const char* path, f_arr *file_arr, int (*fcn)(const char *, f_arr *));
int file_check(const char* path, f_arr *file_arr);
void realloc_fileList(f_arr *file_arr);
void add_fileEntry(f_arr *file_arr, dev_t dev_num, ino_t ino);

int main(int argc, char *argv[]){
	struct stat file_stat;
	const char* path = argv[1];
	f_arr file_list;

	file_list.cap = NUM_FILES;
	file_list.size = ZERO;
	file_list.fid_arr = malloc(sizeof(f_id*) * NUM_FILES);

	if(file_list.fid_arr == NULL){
		perror("Failed to allocate memory for fid_arr\n");
		exit(EXIT_FAILURE);
	}
	for(; arr_index < NUM_FILES; arr_index++){
		file_list.fid_arr[arr_index] = malloc(sizeof(f_id));
	}
	arr_index = ZERO;

	if(argc == 1)
		file_check(".\0", &file_list);

	else if(lstat(path, &file_stat) == -1){
		fprintf(stderr, "Failed to stat %s\n", path);
		exit(EXIT_FAILURE);
	}
	else if((file_stat.st_mode & S_IFREG) == S_IFREG)
		printf("%ld          [%s]\n", file_stat.st_blocks * file_stat.st_blksize / kilibyte, path);

	else
		file_check(path, &file_list);

	for(arr_index = 0; arr_index < file_list.cap; arr_index++)
		free(file_list.fid_arr[arr_index]);
	free(file_list.fid_arr);
}

//adds a file entry to fid_arr in f_arr struct and updates file_arr
void add_fileEntry(f_arr *file_arr, dev_t dev_num, ino_t ino){
	if(file_arr->size == file_arr->cap)
        realloc_fileList(file_arr);

	//adds file to fid_arr
    file_arr->fid_arr[arr_index]->file_ino = ino;
	file_arr->size++;
    arr_index++;
}

//calculates the file size and returns it to scan_dir
int file_check(const char* path, f_arr *file_list){
	struct stat file_stat;
	int dir_size = 0;
	int index = 0;

	if(lstat(path, &file_stat) == -1){
		fprintf(stderr, "Failed to stat %s how\n", path);
		exit(EXIT_FAILURE);
	}
	if((file_stat.st_mode & S_IFDIR) == S_IFDIR){
		dir_size = scan_dir(path, file_list, &file_check);
		dir_size += (file_stat.st_blocks * file_stat.st_blksize) / kilibyte;
		printf("%d			[%s]\n", dir_size, path);
	}

	//checks to see if file is unique
	else if((file_stat.st_mode & S_IFREG) == S_IFREG){
		if(file_stat.st_nlink != 1){

			//adds the file if file_list is empty
			if(file_list->size == ZERO){
				add_fileEntry(file_list, file_stat.st_dev, file_stat.st_ino);
				dir_size = (file_stat.st_blocks * file_stat.st_blksize) / kilibyte;
			}

			else{

				//searches through list for the file with the same device number and inode number
				while(file_list->fid_arr[index] != NULL	&& file_list->fid_arr[index]->file_ino != file_stat.st_ino)
						index++;

				//if the file wasn't found add it to list
				if(index == arr_index){
					add_fileEntry(file_list, file_stat.st_dev, file_stat.st_ino);
					dir_size = (file_stat.st_blocks * file_stat.st_blksize) / kilibyte;
				}
			}
		}
		else
			dir_size = (file_stat.st_blocks * file_stat.st_blksize) / kilibyte;
	}
	return dir_size;
}

//scans the files specified in path, calls file_check with path and file_list, returns dir_size as size of directory
int scan_dir(const char* path, f_arr *file_list, int (*fcn)(const char *, f_arr *)){
	DIR *dfd;
 	struct dirent *dir_stat;
	char *name = malloc(sizeof(char) * MAX_PATH);
	int dir_size = 0;
	if(name == NULL){
		perror("Failed to allocate memory for name\n");
		exit(EXIT_FAILURE);
	}

	if((dfd = opendir(path)) == NULL){
            fprintf(stderr, "Can't open: %s\n", path);
			exit(EXIT_FAILURE);
    }

    while((dir_stat = readdir(dfd)) != NULL){
		if(strcmp(dir_stat->d_name, ".") != 0 && strcmp(dir_stat->d_name, "..") != 0){
			if(strlen(dir_stat->d_name) + strlen(path) + 2 > MAX_PATH){
            	        fprintf(stderr, "Path name too long: %s", dir_stat->d_name);
                	    exit(EXIT_FAILURE);
                	}
        	else{
				sprintf(name, "%s/%s", path, dir_stat->d_name);
				dir_size += (*fcn)(name, file_list);
			}
		}
	}
	free(name);
	closedir(dfd);
	return dir_size;
}

//double the size of fid_arr
void realloc_fileList(f_arr *file_list){
	file_list->fid_arr = realloc(file_list->fid_arr, sizeof(f_id *) * (file_list->cap) * 2);

	for(;arr_index < (file_list->cap + arr_index) ;arr_index++)
		file_list->fid_arr[arr_index] = malloc(sizeof(f_id));

	file_list->cap = file_list->cap * 2;
}

