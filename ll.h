#ifndef LL_H
#define LL_H
struct node 
{ 
    void  *data;   
    struct node *next; 
}; 
extern void push(struct node** head_ref, void *new_data, size_t data_size);
extern void printList(struct node *node, void (*fptr)(void *));
extern int lengthList(struct node *node);
extern void deleteList(struct node** head_ref);
#endif 