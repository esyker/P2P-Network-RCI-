#ifndef _LIST_H
#define _LIST_H

typedef struct list_node
{
  struct list_node *next;
  struct list_node *prev;
  char* queryID;
  int bestpops;
  int fd;
}list_node;

list_node* create_node(int fd,char* queryID, int bestpops);

void insert_head(list_node** head, list_node** new_node);

list_node* search_node(char* queryID, list_node* head);
void remove_node(list_node** head, list_node* curr);

void decrese_bestpops(list_node **head, int amount_decrease,char* queryID);

#endif
