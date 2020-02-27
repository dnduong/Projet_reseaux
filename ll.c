#include<stdio.h> 
#include<stdlib.h> 
#include"ll.h"
  
void push(struct node** head_ref, void *new_data, size_t data_size) 
{ 
    struct node* new_node = (struct node*)malloc(sizeof(struct node)); 
  
    new_node->data  = malloc(data_size); 
    new_node->next = (*head_ref); 
    int i; 
    for (i=0; i<data_size; i++) 
        *(char *)(new_node->data + i) = *(char *)(new_data + i); 
    (*head_ref)    = new_node; 
} 
void printList(struct node *node, void (*fptr)(void *)) 
{ 
    while (node != NULL) 
    { 
        (*fptr)(node->data); 
        node = node->next; 
    } 
} 
int lengthList(struct node *node){
    int i = 0;
    while (node != NULL){
        i++;
        node = node->next;
    }
    return i;
}

void deleteList(struct node** head_ref)  
{  
      
struct node* current = *head_ref;  
struct node* next;  
  
while (current != NULL)  
{  
    next = current->next;  
    free(current);  
    current = next;  
}  
      
*head_ref = NULL;  
}  


