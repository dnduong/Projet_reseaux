#define _GNU_SOURCE

#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include "message.h"


void init_mess(char m[],int *pos){
	memset(m,0,4096);
  m[0]=93;
  m[1]=2;
  (*pos)+=4;
}

void hello_court(char m[], int *pos, char id[]){
  m[*pos]=2;
  (*pos)++;
  m[*pos]=8;
  (*pos)++;
  memcpy(&m[*pos],id,8); 
  (*pos)+=8;
}

void hello_long(char m[], int *pos, char dest_id[], char id[]){
  m[*pos]=2;
  (*pos)++;
  m[*pos]=16;
  (*pos)++;
  memcpy(&m[*pos],id,8);
  (*pos)+=8;
	memcpy(&m[*pos],dest_id,8);
  (*pos)+=8; 
}
void neighbour(char m[],int *pos, char ip[], char port[])
{
  m[*pos]=3;
  (*pos)++;
  m[*pos]=18;
  (*pos)++;
  memcpy(&m[*pos],ip,16);
  (*pos)+=16;
  memcpy(&m[*pos],port,2);
  (*pos)+=2;
}

void data(char m[], int *pos, char sender_id[], char nonce[], char type, char *mes)
{
  int taille = strlen(mes);
  m[*pos]=4;
  (*pos)++;
  m[*pos]=(char)(taille+13);
  (*pos)++;
  memcpy(&m[*pos],sender_id,8);
  (*pos)+=8;
  memcpy(&m[*pos],nonce,4);
  (*pos)+=4;
  m[*pos]=type;
  (*pos)++;
  memcpy(&m[*pos],mes,taille);
  (*pos)+=taille;

}

void ack(char m[], int *pos, char sender_id[], char nonce[])
{
  m[*pos]=5;
  (*pos)++;
  m[*pos]=12;
  (*pos)++;
  memcpy(&m[*pos],sender_id,8);
  (*pos)+=8;
  memcpy(&m[*pos],nonce,4);
  (*pos)+=4;
}

void goaway(char m[], int *pos, char code, char *mes)
{
  int taille = strlen(mes);
  m[*pos]=6;
  (*pos)++;
  m[*pos]=(char)(taille+1);
  (*pos)++;
  m[*pos]=code;
  (*pos)++;
  memcpy(&m[*pos],mes,taille);
  (*pos)+=taille;
}

int length_m(char m[])
{
  short taille;
  memcpy(&taille,&m[2],2);
  taille = ntohs(taille);
  return (int)(taille+4);
}

void create_mess(char m[], message_t tab[], int length,  char id[], char dest_id[]){
  int pos = 0;
  init_mess(m,&pos);
  for(int i = 0; i < length; i++)
  {
    switch (tab[i])
    {
      case H_COURT :
        hello_court(m,&pos,id);
        break;
      case H_LONG : 
        hello_long(m,&pos,dest_id,id);
        break;
      default :
        printf("Error!\n");
    }
  }
  short taille = htons(pos-4); 
  memcpy(&m[2],&taille,2);
}