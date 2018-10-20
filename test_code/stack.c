#include <stdio.h>
#include <stdlib.h>


typedef struct Node{
    struct Node *next;
    struct stat file_stats;

}node;

struct Node* build_node(struct stat file);
struct stat* pop_node(struct Node **top);
void push_node(struct Node **top, struct stat file);
int is_empty(struct Node *struc);

struct Node* build_node(struct stat file){
    struct Node *new_node = malloc(sizeof(node));
    new_node->file_stats = file;
    new_node->next = NULL;
    return new_node;
}

void push_node(struct Node **top, struct stat file){
    struct Node *new_node = build_node(file);
    new_node->next = *top;
    *top = new_node;
}

struct stat *pop_node(struct Node **top){
    struct stat *file_top;
    struct Node *temp = *top;
    if(is_empty(*top))
        return NULL;
    *top = (*top)->next;
    *file_top = temp->file_stats;
    free(temp);

    return file_top;
}

int is_empty(struct Node *struc){
    return !struc;
}
