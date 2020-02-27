#define _GNU_SOURCE

#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <endian.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include"ll.h"
#include"voisin.h"
#include "message.h"
#include <net/if.h>
#include"data.h"
#include <pthread.h>



#define SIZE_M 4096

struct node *potentiel = NULL;
struct node *symetric = NULL;
struct node *voisin = NULL;
struct node *inondation = NULL;
int quit = 0;
char id[8];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void get_time(struct tm *timeinfo){
  time_t rawtime;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
}

void recv_parser_aux(char *m,int *pos,int length, char id[],int sockfd, struct sockaddr_in6 *send_addr, socklen_t addrlen, struct sockaddr_in6 *recv_addr,char *bol){
  if((*pos) == 0){
    if(m[(*pos)]==93 && m[(*pos)+1]==2){
      (*pos)+=2;
      short taille;
      memcpy(&taille,&m[*pos],2);
      taille = ntohs(taille);
      (*pos)+=2;
      if(taille + 4 > length){
        *bol = 1;
        //printf("Ignoré!\n");
      }else{
        //printf("==Bon message!==\n");
      }
      //printf("Taille : %d octets\n",taille);
    }else{
      *bol = 1;
      //printf("Ignoré!\n");
    }
  }else{
    unsigned int taille,i;
    int posit;
    unsigned char dest_id[8];
    char *err;
    char sender_id[8];
    char nonce[4];
    char mess[SIZE_M];
    int rc;
    int is_hc;
    unsigned char type;
    switch(m[*pos]){
      case 0:
        //printf("Pad1\n");
        (*pos)++;
        break;
      case 1:
        //printf("PadN\n");
        (*pos)++;
        (*pos)+=((unsigned char)m[*pos]+1);
        break;
      case 2:
        (*pos)++;
        i = (*pos)+1;
        struct Voisin* v = malloc(sizeof(struct Voisin));
        memcpy(v->ip,&(recv_addr->sin6_addr.s6_addr),16);
        memcpy(v->port,&(recv_addr->sin6_port),2);
        memcpy(v->id,&m[i],8);
        time(&v->recv_h);
        struct Potentiel *s = malloc(sizeof(struct Potentiel));
        memcpy(s->ip,&(recv_addr->sin6_addr.s6_addr),16);
        memcpy(s->port,&(recv_addr->sin6_port),2);
        if(m[*pos]==8){
          is_hc = 1;
          //printf("Hello court\n");
        }else{
          is_hc = 0;
          i += 8;
          time(&v->recv_hl);
          //printf("Hello long\n");
          memcpy(dest_id,&m[i],8);
          if(!memcmp(dest_id,id,8)){
            //printf("Good Hello!\n");
            if(!p_exist(symetric,s)){
              push(&symetric,s,sizeof(struct Potentiel));
            }else{
              free(s);
            }
          }else{
            //printf("Bad Hello\n");
          }
        }
        if(!v_exist(voisin,v,is_hc)){
          push(&voisin,v,sizeof(struct Voisin));
        }else{
          free(v);
        }
        (*pos)+=((unsigned char)m[*pos]+1);
        break;
      case 3:
        //printf("Neigbour\n");
        (*pos)++;
        i = (*pos)+1;
        struct Potentiel* p = malloc(sizeof(struct Potentiel)); 
        memcpy(p->ip,&m[i],16);
        i+=16;
        memcpy(p->port,&m[i],2);
        if(!p_exist(potentiel,p)){
          push(&potentiel,p,sizeof(struct Potentiel));
        }else{
          free(p);
        }	
        (*pos)+=((unsigned char)m[*pos]+1);
        break;
      case 4:
        //printf("Data\n");
        (*pos)++;
        taille = (unsigned char)m[*pos];
        i = (*pos)+1;
        //printf("%u\n",taille);
        memcpy(sender_id,&m[i],8);
        i+=8;
        memcpy(nonce,&m[i],4);
        i+=4;
        struct Potentiel* sym = malloc(sizeof(struct Potentiel));
        memcpy(sym->ip,(char *)&(recv_addr->sin6_addr.s6_addr),16);
        memcpy(sym->port,(char *)&(recv_addr->sin6_port),2);
        if(p_exist(symetric,sym)){
          if(!d_exist(inondation,sender_id,nonce,(char *)&(recv_addr->sin6_addr.s6_addr),(char *)&(recv_addr->sin6_port))){
            type = m[i];
            //printf("Type :%d\n",type);
            i++;
            if (taille < 12){
              //printf("taille trop petite\n");
              *bol = 1;
              break;
            }else if (taille > 12){
              err = malloc(taille - 12);
              memcpy(err,&m[i],taille-13);
              err[taille-13]='\0';
              if(type == 0){
                printf("%s\n",err);
              }
              struct Data *d;
              d = malloc(sizeof(struct Data));
              init_data(d,sender_id,nonce,type,err,symetric);
              i_del(&(d->inonde),(char *)&(recv_addr->sin6_addr.s6_addr),(char *)&(recv_addr->sin6_port));
              push(&inondation,d,sizeof(struct Data));
              free(err);
            }     
          }
          //create ack
          posit = 0;
          init_mess(mess,&posit);
          ack(mess,&posit,sender_id,nonce);
          taille = htons(posit-4); 
          memcpy(&mess[2],&taille,2);
          //end
          memset(send_addr,0,sizeof(struct sockaddr_in6));
          send_addr->sin6_family = AF_INET6;
          send_addr->sin6_port = recv_addr->sin6_port;
          memcpy(&(send_addr->sin6_addr.s6_addr),&(recv_addr->sin6_addr.s6_addr),16);
          rc = sendto(sockfd,mess,length_m(mess),0,send_addr,addrlen);
          if(rc < 0){
            //perror("sendto ack"); 
          } 
          //send 
        }
        free(sym);
        (*pos)+=(taille+1);
        break;
      case 5:
        //printf("Ack\n");
        (*pos)++;
        i = (*pos)+1;
        memcpy(sender_id,&m[i],8);
        i+=8;
        memcpy(nonce,&m[i],4);
        i+=4;
        d_exist(inondation,sender_id,nonce,(char *)&(recv_addr->sin6_addr.s6_addr),(char *)&(recv_addr->sin6_port));
        (*pos)+=((unsigned char)m[*pos]+1);
        break;
      case 6:
        //printf("GoAway\n");
        (*pos)++;
        taille = (unsigned char)m[*pos];
        //printf("%d\n",taille);
        i = *pos;
        (*pos)+=(taille+1);
        i++;
        //printf("Code :%d\n",m[i]);
        i++;
        if (taille < 1){
          *bol = 1;
          break;
        }else if (taille > 1){
           err = malloc(taille-1);
          memcpy(err,&m[i],taille-1);
          //printf("%s\n",err);
          free(err);
        }     
        v_del(&voisin,NULL,(char *)&(recv_addr->sin6_addr.s6_addr),(char *)&(recv_addr->sin6_port));
        break;
      case 7:
        //printf("Warning\n");
        (*pos)++;
        taille = (unsigned char)m[*pos];
        //printf("%d\n",taille);
        i = *pos;
        (*pos)+=(taille+1);
        i++;
        if (taille < 0){
          *bol = 1;
          break;
        }else if (taille > 0){
          err = malloc(taille);
          memcpy(err,&m[i],taille);
          //printf("%s\n",err);
          free(err);
        }
        break;
      default:
        *bol = 1;
        //printf("Ignoré!\n");
    }
  }
}


void recv_parser(char *m,int length, char id[],int sockfd, struct sockaddr_in6 *send_addr, socklen_t addrlen, struct sockaddr_in6 *recv_addr){
  if(length <= SIZE_M){
    int pos = 0;
    char bol = 0;
    while(pos < length && !bol){
      char ip[16];
      memcpy(&ip[0],&(recv_addr->sin6_addr.s6_addr),16);       
      char str[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, &ip[0], str, INET6_ADDRSTRLEN);
      //printf("ip: %s\n",str);
      recv_parser_aux(m,&pos,length,id,sockfd,send_addr,addrlen,recv_addr,&bol);
    }
    //printf("end parse\n");
  } 
}

void init_jul(){
  struct Potentiel* jul = malloc(sizeof(struct Potentiel));
  short jul_port = htons(1212);
  char jul_ip[16];
  inet_pton(AF_INET6,"::ffff:81.194.27.155",&jul_ip[0]);
  //inet_pton(AF_INET6,"::ffff:78.113.15.178",&jul_ip[0]);
  memcpy(jul->ip,&jul_ip[0],16);
  memcpy(jul->port,&jul_port,2);
  push(&potentiel,jul,sizeof(struct Potentiel));
}

void init_socket(int *s,int *rc){
  *s = socket(AF_INET6, SOCK_DGRAM, 0);
  if(*s < 0){
    //perror("socket");
    exit(1);
  }
  int val_1 = 1;
  if(setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, &val_1, sizeof(val_1)) < 0){
    //perror("setsockopt");
  }
  *rc = fcntl(*s, F_GETFL);
  if(*rc < 0){
    //perror("fcntl")
  }
  *rc = fcntl(*s,F_SETFL,*rc | O_NONBLOCK);
  if(*rc < 0){
  	//perror ("fcntl");	
  } 
}

void generate_id(char *id){
  int fd = open("/dev/urandom",O_RDONLY);
  if(fd < 0){
  	//perror("open");
  }
  if(read(fd,id,8)<0){
  	//perror("read id:");
  }
  close(fd);
}

void generate_nonce(short *nonce){
  int fd = open("/dev/urandom",O_RDONLY);
  if(fd < 0){
  	//perror("open");
  }
  if(read(fd,nonce,4)<0){
  	//perror("read nonce:");
  }
  close(fd);
}

void create_neighbour(char m[]){
  int pos = 0;
  init_mess(m,&pos);
  struct node *new = symetric;
  while(new != NULL){
    struct Potentiel *s = (struct Potentiel*)(new->data);
    neighbour(m,&pos,s->ip,s->port);
    new = new->next;
  }
  short taille = htons(pos-4); 
  memcpy(&m[2],&taille,2);
}

void send_all_hc(int s, char id[], struct sockaddr_in6 *send_addr){
  struct node* new = potentiel;
  char m[SIZE_M];
  message_t tab[1] = {H_COURT}; 
  create_mess(m,tab,1,id,NULL);
  while(new != NULL){
    //printf("send hello court\n");
    memset(send_addr,0,sizeof(struct sockaddr_in6));
    send_addr->sin6_family = AF_INET6;
    short port;
    struct Potentiel *p = new->data;
    memcpy(&port,p->port,2);
    send_addr->sin6_port = port;
    memcpy(&(send_addr->sin6_addr.s6_addr),p->ip,16);
    if (sendto(s, m, length_m(m), 0, send_addr, sizeof(struct sockaddr_in6)) <0) {
      //perror("sendto: ");
    }
    new = new->next;
  }
}

void send_all_hl(int s, char id[], struct sockaddr_in6 *send_addr){
  struct node *new = voisin;
  char m[SIZE_M];
  message_t tab[1] = {H_LONG};
  while(new != NULL){
    //printf("send hello long\n");
    memset(send_addr,0,sizeof(struct sockaddr_in6));
    send_addr->sin6_family = AF_INET6;
    short port;
    struct Voisin *v = new->data;
    memcpy(&port,v->port,2);
    send_addr->sin6_port = port;
    memcpy(&(send_addr->sin6_addr.s6_addr),v->ip,16);
    create_mess(m,tab,1,id,v->id);
    if(sendto(s,m,length_m(m),0,send_addr,sizeof(struct sockaddr_in6))< 0){
      //perror("sendto:");
    }
    new = new->next;
  }
}

void send_all_nb(int s, struct sockaddr_in6 *send_addr){
  struct node *new = voisin;
  char m[SIZE_M];
  create_neighbour(m);
  while(new != NULL){
    //printf("send hello neighbour\n");
    memset(send_addr,0,sizeof(struct sockaddr_in6));
    send_addr->sin6_family = AF_INET6;
    short port;
    struct Voisin *v = new->data;
    memcpy(&port,v->port,2);
    send_addr->sin6_port = port;
    memcpy(&(send_addr->sin6_addr.s6_addr),v->ip,16);
    if(sendto(s,m,length_m(m),0,send_addr,sizeof(struct sockaddr_in6))< 0){
      //perror("sendto:");
    }
    new = new->next;
  }
}
void send_quit(int s, struct sockaddr_in6 *send_addr){
  struct node *new = symetric;
  char m[SIZE_M];
  int pos = 0;
  init_mess(m,&pos);
  goaway(m,&pos,1,"Bye bye");
  short taille = htons(pos-4); 
  memcpy(&m[2],&taille,2);
  while(new != NULL){
    //printf("send quit\n");
    memset(send_addr,0,sizeof(struct sockaddr_in6));
    send_addr->sin6_family = AF_INET6;
    short port;
    struct Voisin *v = new->data;
    memcpy(&port,v->port,2);
    send_addr->sin6_port = port;
    memcpy(&(send_addr->sin6_addr.s6_addr),v->ip,16);
    if(sendto(s,m,length_m(m),0,send_addr,sizeof(struct sockaddr_in6))< 0){
      //perror("sendto:");
    }
    new = new->next;
  }
}


void inonder(int s, struct sockaddr_in6 *send_addr){
  struct node *new = inondation;
  char m[SIZE_M];
  time_t now;
  time(&now);
  while(new != NULL){
    struct Data *d = new->data;
    struct node *v = d->inonde;
    time_t t = d->time;
    int dif = (int)difftime(now,t);
    create_data(m,d);
    while(v != NULL){
      struct Voisin_inon *vi = v->data;
      if(vi->ct <=5){
        if(dif >= (vi->timer)[vi->ct]){ 
          //printf("%d\n",vi->ct);
          send_addr->sin6_family = AF_INET6;
          short port;
          memcpy(&port,vi->port,2);
          send_addr->sin6_port = port;
          memcpy(&(send_addr->sin6_addr.s6_addr),vi->ip,16);
          if(sendto(s,m,length_m(m),0,send_addr,sizeof(struct sockaddr_in6))< 0){
            //perror("sendto:");
          }
          (vi->ct)++;
          //printf("SENDDDDDDDDDDDD§§§§§§§§§§§\n");
        }
      } 
      v = v->next;
    }
    new = new->next;
  }
}

void update_inonde(int s, struct sockaddr_in6 *send_addr){
  struct node *new = inondation;
  char ip[16];
  short port;
  while(new != NULL){
    struct Data *d = new->data;
    if(i_del_expired(&(d->inonde),ip,&port)){
      char m[SIZE_M];
      int pos = 0;
      init_mess(m,&pos);
      goaway(m,&pos,2,"You are so slow!!");
      short taille = htons(pos-4);
      memcpy(&m[2],&taille,2);
      memset(send_addr,0,sizeof(struct sockaddr_in6));
      send_addr->sin6_family = AF_INET6;
      send_addr->sin6_port = port;
      memcpy(&(send_addr->sin6_addr.s6_addr),ip,16);
      if(sendto(s,m,length_m(m),0,send_addr,sizeof(struct sockaddr_in6))< 0){
        //perror("sendto:");
      }
      v_del(&voisin,NULL,ip,(char*)&port);
      p_del(&symetric,ip,(char *)&port);
    }
    new = new->next;
  }
}

void update_inondation(){
  for(int i = 0; i < lengthList(inondation);i++)
    d_del(&inondation);
}

void update_voisin(int s, struct sockaddr_in6 *send_addr){
  struct node *new = voisin;
  time_t now;
  time(&now);
  while(new != NULL){
    if(difftime(now,((struct Voisin*)(new->data))->recv_h) > 120){
      //printf("delete voisin\n");
      char m[SIZE_M];
      int pos = 0;
      init_mess(m,&pos);
      goaway(m,&pos,2,"You did'nt send hello recently");
      short taille = htons(pos-4); 
      memcpy(&m[2],&taille,2);
      struct Voisin *v = (struct Voisin*)(new->data);
      struct Potentiel *p = malloc(sizeof(struct Potentiel));
      memcpy(p->ip,v->ip,16);
      memcpy(p->port,v->port,2);
      if(!p_exist(potentiel,p)){
        push(&potentiel,p,sizeof(struct Potentiel));
      }else{
        free(p);
      }   
      memset(send_addr,0,sizeof(struct sockaddr_in6));
      send_addr->sin6_family = AF_INET6;
      short port;
      memcpy(&port,v->port,2);
      send_addr->sin6_port = port;
      memcpy(&(send_addr->sin6_addr.s6_addr),v->ip,16);
      if(sendto(s,m,length_m(m),0,send_addr,sizeof(struct sockaddr_in6))< 0){
        //perror("sendto:");
      }
      v_del(&voisin,v,NULL,NULL);
      new = NULL;
    }else{
      new = new->next;
    }
  } 
}

void update_symetric(){
  struct node *new = voisin;
  time_t now;
  time(&now);
  while(new != NULL){
    if(difftime(now,((struct Voisin*)(new->data))->recv_hl) > 120){
      //printf("delete symetric\n");
      struct Voisin *v = (struct Voisin*)(new->data);
      p_del(&symetric,v->ip,v->port);
      new = NULL;
    }else{
      new = new->next;
    }
  } 
}

void* get_data(void* arg){
  short nonce;
  generate_nonce(&nonce);
  int DATA_MAX = 242;
  //printf("%d\n",DATA_MAX );
  char buff[DATA_MAX+1];
  char *mes = malloc(DATA_MAX);
  int n = 0;
  printf("Set nickname : ");
  while((buff[n++] = getchar()) != '\n');
  n = n-1;
  buff[n++]=':';
  buff[n++]=' ';
  int m = n;
  printf("Nickname set!\n");
  while(!quit){
    update_inondation();
    memset(&buff[m],0,DATA_MAX+1-m);
    memset(mes,0,DATA_MAX);
    n = m;
    while((buff[n++] = getchar()) != '\n' && n < DATA_MAX);
    memcpy(mes,buff,n-1);
    //printf("%d\n",strlen(mes));
    pthread_mutex_lock (&mutex);
    if(memcmp(&buff[m],"quit",4)==0){
      quit = 1;
      printf("Disconnected, Bye Bye!\n");
    }else{
      struct Data *d;
      d = malloc(sizeof(struct Data));
      init_data(d,id,(char *)&nonce,0,mes,symetric);
      push(&inondation,d,sizeof(struct Data));
      nonce++;
      //printf("%d\n", lengthList(inondation));
      //printf("%d\n", lengthList(d->inonde));
      //printf("%s\n", d->mess);
    }
    pthread_mutex_unlock (&mutex);
  }
  free(mes);
  pthread_exit(NULL);
}

void* treat_data(void* arg){
  int counter = 30;
  int rc;
  int s;
  init_jul();
  init_socket(&s,&rc); 
  struct sockaddr_in6 send_addr;
  memset(&send_addr,0,sizeof(send_addr));
  struct sockaddr_in6 recv_addr;
  memset(&recv_addr,0,sizeof(recv_addr));  
  
  char answer[SIZE_M];
  memset(answer, 0, SIZE_M);
  socklen_t addrlen = sizeof(struct sockaddr_in6);

  while(1){
    if(!quit){
      counter++;
      struct timeval *time_out = malloc(sizeof(struct timeval));
      time_out->tv_sec = 1;
      time_out->tv_usec = 0;
      fd_set readfds;
      FD_ZERO(&readfds);
      FD_SET(s, &readfds);
      rc = select(s + 1, &readfds, NULL, NULL, time_out);
      pthread_mutex_lock (&mutex);
      if(FD_ISSET(s, &readfds)){
        rc = recvfrom(s, answer, SIZE_M, 0, &recv_addr, &addrlen);
        if(rc < 0){
          //perror("recvfrom: ");
          close(s);
          exit(1);
        }else{
          recv_parser(answer,rc,id,s,&send_addr,addrlen,&recv_addr);
          //printf("Potentiel: \n");
          //printList(potentiel,print_potentiel);
          //printf("Voisin: \n");
          //printList(voisin,print_voisin);
          //printf("Symetric: \n");
          //printList(symetric,print_potentiel);
          //printf("\n");
        }
      }
      free(time_out);
      if(counter >= 30){
        if(lengthList(symetric) < 8){
          send_all_hc(s,id,&send_addr);
        }
        send_all_nb(s,&send_addr);
        send_all_hl(s,id,&send_addr);
        counter = 0;
      }
      update_voisin(s,&send_addr);
      update_symetric();
      update_inonde(s,&send_addr);
      update_inondation();
      inonder(s,&send_addr);
      pthread_mutex_unlock (&mutex);
    }else{
      send_quit(s,&send_addr);
      deleteList(&symetric);
      deleteList(&potentiel);
      deleteList(&voisin);
      deleteList(&inondation);
      break;
    }
  }
  pthread_exit(NULL);
}

int main(){
  generate_id(id);
  pthread_t get;
  pthread_t treat;

  pthread_create (&get, NULL, get_data, (void*)NULL);
  pthread_create (&treat, NULL, treat_data, (void*)NULL);

  pthread_join (get, NULL);
  pthread_join (treat, NULL); 
  
  return 0;
}
  
  
