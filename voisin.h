#include <time.h>
#ifndef VOISIN_H
#define VOISIN_H
#include "ll.h"
struct Potentiel{
  char ip[16];
  char port[2];
};

struct Voisin{
  char ip[16];
  char port[2];
  char id[8];
  time_t recv_h;
  time_t recv_hl;
};
extern void print_potentiel(void *p);
extern void print_voisin(void *p);
extern int p_equal(struct Potentiel *p1, struct Potentiel *p2);
extern int p_exist(struct node* node, struct Potentiel *p);
extern int v_equal(struct Voisin *p1, struct Voisin *p2);
extern int v_exist(struct node* node, struct Voisin *p, int is_hc);
extern void p_del(struct node** head_ref, char ip[], char port[]);
extern void v_del(struct node** head_ref, struct Voisin *v, char ip[16], char port[2]);
#endif 