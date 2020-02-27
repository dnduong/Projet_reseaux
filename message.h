#ifndef MESSAGE_H
#define MESSAGE_H
typedef enum message_t message_t;
enum message_t
{
  H_COURT, H_LONG, NEIGHBOUR, DATA, ACK, GOAWAY
};
extern void init_mess(char m[],int *pos);
extern void hello_court(char m[],int *pos, char id[]);
extern void	hello_long(char m[],int *pos, char dest_id[], char id[]);
extern void create_mess(char m[], message_t tab[], int length, char id[], char dest_id[]);
extern void neighbour(char m[],int *pos, char ip[], char port[]);
extern void data(char m[], int *pos, char sender_id[], char nonce[], char type, char *mes);
extern void ack(char m[], int *pos, char sender_id[], char nonce[]);
extern void goaway(char m[], int *pos, char code, char *mes);
extern int length_m(char m[]);

#endif 