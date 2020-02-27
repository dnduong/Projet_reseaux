#define _GNU_SOURCE
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include "data.h"
#include <math.h>
void init_data(struct Data *d,char id[],char nonce[], char type,char *m, struct node *symetric)
{
	memcpy(d->id,id,8);
	memcpy(d->nonce,nonce,4);
	d->mess = malloc(strlen(m)+1);
	memcpy(d->mess,m,strlen(m)+1);
	d->type = type;
    d->inonde = NULL;
    time(&(d->time));
	struct node *new = symetric;
    while(new != NULL){
        struct Voisin_inon *i = malloc(sizeof(struct Voisin_inon));
        struct Potentiel *p = (struct Potentiel *)new->data;
        memcpy(i->ip,p->ip,16);
        memcpy(i->port,p->port,2);
        i->ct = 0;
        (i->timer)[0]=0;
        for(int j = 1; j<=5; j++){
            (i->timer)[j]=random_generate(puissance(2,j),puissance(2,j-1));
        }
        push(&(d->inonde),i,sizeof(struct Voisin_inon));
        new = new->next;
    }
}

void i_del(struct node** head_ref, char ip[], char port[]){
	struct node* temp = *head_ref, *prev;
	if (temp == NULL) return;
	int cmp;
    cmp = (memcmp(((struct Voisin_inon*)(temp->data))->ip,ip,16) == 0 && 
				memcmp(((struct Voisin_inon*)(temp->data))->port,port,2) == 0);

    if (temp != NULL && cmp) 
    { 
        *head_ref = temp->next;   // Changed head 
        free(temp);               // free old head 
        return; 
    } 
  
    while (temp != NULL && !cmp) 
    { 
        prev = temp; 
        temp = temp->next; 
        if(temp!= NULL)
            cmp = (memcmp(((struct Voisin_inon*)(temp->data))->ip,ip,16) == 0 && 
                    memcmp(((struct Voisin_inon*)(temp->data))->port,port,2) == 0);
    } 
   
    if (temp == NULL) return; 
   
    prev->next = temp->next; 
  
    free(temp); 
}

int i_del_expired(struct node** head_ref, char ip[],short *port){
	struct node* temp = *head_ref, *prev;
	if (temp == NULL) return 0;
	int cmp;
    cmp = (((struct Voisin_inon*)(temp->data))->ct > 5);
  
    if (temp != NULL && cmp) 
    { 
        *head_ref = temp->next;
        memcpy(ip,((struct Voisin_inon*)(temp->data))->ip,16);
        memcpy(port,((struct Voisin_inon*)(temp->data))->port,2);
        free(temp);              
        return 1;
    } 

    while (temp != NULL && !cmp) 
    { 
        prev = temp; 
        temp = temp->next;
        if(temp!= NULL)
            cmp = (((struct Voisin_inon*)(temp->data))->ct > 5); 
    } 
  
 
    if (temp == NULL) return 0;

    prev->next = temp->next; 
    memcpy(ip,((struct Voisin_inon*)(temp->data))->ip,16);
    memcpy(port,((struct Voisin_inon*)(temp->data))->port,2);
    free(temp);
    return 1;
}

void d_del(struct node** head_ref){
	struct node* temp = *head_ref, *prev;
    time_t now;
    time(&now);  
	if (temp == NULL) return;
	int cmp;
    cmp = (((struct Data *)(temp->data))->inonde == NULL
                && difftime(now,((struct Data *)(temp->data))->time) > 60);
  
    if (temp != NULL && cmp) 
    {   
        *head_ref = temp->next;
        free(((struct Data *)(temp->data))->inonde);
        free(((struct Data *)(temp->data))->mess);
        free(temp);                  
        return; 
    } 
   
    while (temp != NULL && !cmp) 
    { 
        prev = temp; 
        temp = temp->next;
        if(temp!= NULL)
            cmp = (((struct Data *)(temp->data))->inonde == NULL
                && difftime(now,((struct Data *)(temp->data))->time) > 60); 
    } 
  
 
    if (temp == NULL) return; 
  

    prev->next = temp->next; 
    free(((struct Data *)(temp->data))->inonde);
    free(((struct Data *)(temp->data))->mess);
    free(temp);  
}

int d_equal(struct Data *d, char id[], char nonce[])
{
	return (memcmp(d->id,id,8)==0 && 
			memcmp(d->nonce,nonce,4)==0);
}

int d_exist(struct node *node, char id[], char nonce[], char ip[], char port[])
{
	while(node != NULL)
	{
		if(d_equal(node->data,id,nonce)){
			i_del(&(((struct Data*)(node->data))->inonde),ip,port);
			return 1;
		} 
		node = node->next;
	}
	return 0;
}
void create_data(char m[], struct Data *d){
	int pos = 0;
    init_mess(m,&pos);
   	data(m,&pos,d->id,d->nonce,d->type,d->mess);
    short taille = htons(pos-4); 
    memcpy(&m[2],&taille,2);
}

int random_generate(int max, int min){
    srand(time(NULL));
    return (rand()%(max - min + 1))+min;
}

int puissance(int x, int y){
    int res = 1;
    for(int i = 0; i < y ; i++){
        res = res*x;
    }
    return res;
}

