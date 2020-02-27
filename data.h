#ifndef DATA_H
#define DATA_H
#include"ll.h"
#include"voisin.h"
#include"message.h"
#include <time.h>

struct Data{
	char id[8];
	char nonce[4];
	char type;
	char *mess;
	struct node *inonde;
	time_t time;
};
struct Voisin_inon{
	char ip[16];
	char port[2];
	int ct;
	int timer[6];
};

extern void init_inon(struct node *symetric,struct node *inon);
extern void init_data(struct Data *d,char id[],char nonce[], char type,char *m, struct node *symetric);
extern void i_del(struct node** head_ref, char ip[], char port[]);
extern int i_del_expired(struct node** head_ref, char ip[],short* port);
extern void d_del(struct node** head_ref);
extern int d_equal(struct Data *d, char id[], char nonce[]);
extern int d_exist(struct node *node, char id[], char nonce[],char ip[], char port[]);
extern void create_data(char m[], struct Data *d);
extern int random_generate(int max, int min);
extern int puissance(int x, int y);
#endif 