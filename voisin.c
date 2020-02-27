#define _GNU_SOURCE
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "voisin.h"
#include <stdlib.h>

void print_potentiel(void* p)
{
	short port;
	memcpy(&port,((struct Potentiel*)p)->port,2);
	port = ntohs(port);
	char ip[16];
	memcpy(&ip[0],((struct Potentiel*)p)->ip,16);
	char str[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &ip[0], str, INET6_ADDRSTRLEN);
	printf("ip: %s\n",str);
	printf("port:%d\n", port);
}

void print_voisin(void *v){
	short port;
	memcpy(&port,((struct Voisin*)v)->port,2);
	port = ntohs(port);
	char ip[16];
	memcpy(&ip[0],((struct Voisin*)v)->ip,16);
	char str[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &ip[0], str, INET6_ADDRSTRLEN);
	printf("ip: %s\n",str);
	printf("port:%d\n", port);
	printf ( "%ld\n", ((struct Voisin*)v)->recv_h);
	printf ( "%ld\n", ((struct Voisin*)v)->recv_hl);
}

int p_equal(struct Potentiel *p1, struct Potentiel *p2){
	return (memcmp(p1->ip,p2->ip,16)==0 && 
			memcmp(p1->port,p2->port,2)==0);
}

int p_exist(struct node* node, struct Potentiel *p){
	while(node != NULL)
	{
		if(p_equal(node->data,p)) return 1;
		node = node->next;
	}
	return 0;
} 

int v_equal(struct Voisin *p1, struct Voisin *p2){
	return (memcmp(p1->ip,p2->ip,16)==0 && 
			memcmp(p1->port,p2->port,2)==0 &&
			memcmp(p1->id,p2->id,8)==0);
}

int v_exist(struct node* node, struct Voisin *p, int is_hc){
	while(node != NULL)
	{
		if(v_equal(node->data,p)){
			if(is_hc){
				time(&(((struct Voisin*)(node->data))->recv_h));
			}else{
				time(&(((struct Voisin*)(node->data))->recv_h));
				time(&(((struct Voisin*)(node->data))->recv_hl));
			}		
			return 1;
		}
		node = node->next;
	}
	return 0;
} 

void p_del(struct node** head_ref, char ip[], char port[]){
	struct node* temp = *head_ref, *prev;
	if (temp == NULL) return;
	int cmp;
	cmp = (memcmp(((struct Potentiel*)(temp->data))->ip,ip,16) == 0 && 
				memcmp(((struct Potentiel*)(temp->data))->port,port,2) == 0);
  
    if (temp != NULL && cmp) 
    { 
        *head_ref = temp->next;   
        free(temp);                
        return; 
    } 
  
    while (temp != NULL && !cmp) 
    { 
        prev = temp; 
        temp = temp->next;
        if(temp != NULL)
	        cmp = (memcmp(((struct Potentiel*)(temp->data))->ip,ip,16) == 0 && 
					memcmp(((struct Potentiel*)(temp->data))->port,port,2) == 0); 
    } 
  
    if (temp == NULL) return; 
  
    prev->next = temp->next; 
  
    free(temp);  
}

void v_del(struct node** head_ref, struct Voisin *v, char ip[16], char port[2]){
	struct node* temp = *head_ref, *prev;
	if (temp == NULL) return;
	int cmp;
	if(v != NULL){
		cmp = (memcmp(((struct Voisin*)(temp->data))->ip,v->ip,16) == 0 && 
				memcmp(((struct Voisin*)(temp->data))->port,v->port,2) == 0);
	}else{
		cmp = (memcmp(((struct Voisin*)(temp->data))->ip,ip,16) == 0 && 
				memcmp(((struct Voisin*)(temp->data))->port,port,2) == 0);
	}
	
    if (temp != NULL && cmp) 
    { 
        *head_ref = temp->next;
        free(temp);            
        return; 
    } 
  
    while (temp != NULL && !cmp) 
    { 
        prev = temp; 
        temp = temp->next; 
        if(temp != NULL){
        	if(v != NULL){
				cmp = (memcmp(((struct Voisin*)(temp->data))->ip,v->ip,16) == 0 && 
						memcmp(((struct Voisin*)(temp->data))->port,v->port,2) == 0);
			}else{
				cmp = (memcmp(((struct Voisin*)(temp->data))->ip,ip,16) == 0 && 
						memcmp(((struct Voisin*)(temp->data))->port,port,2) == 0);
			}
        }
    } 
  
    if (temp == NULL) return; 
  
    prev->next = temp->next; 
  
    free(temp);   
}

