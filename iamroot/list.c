#include "list.h"

#include <stdlib.h>
#include <string.h>

//cria o nó da lista
list_node* create_node(int fd,char* queryID, int bestpops)
{
  list_node* new_node;
  new_node=(list_node*)malloc(sizeof(list_node));
  if(new_node==NULL)
	exit(-1);
  new_node->queryID=(char*)malloc((strlen(queryID)+1)*sizeof(char));
  if(new_node->queryID==NULL)
	exit(-1);
  strcpy(new_node->queryID,queryID);
  new_node->fd=fd;
  new_node->bestpops=bestpops;
  new_node->next=NULL;
  new_node->prev=NULL;
  return new_node;
}

//insere na cabeça da lista um novo nó
void insert_head(list_node** head, list_node** new_node)
{
  if(*head==NULL)
  {
    *head=*new_node;
  }
  else
  {
      (*head)->prev=(*new_node);
      (*new_node)->next=(*head);
      (*head)=(*new_node);
  }
  return;
}

//encontra um novo nó de acordo com o query id
list_node* search_node(char* queryID, list_node* head)
{
  list_node * aux;
  for(aux=head;aux!=NULL&&strcmp(aux->queryID,queryID)!=0;aux=aux->next)
  {}
  return aux;
}

//remover um nó
void remove_node(list_node** head, list_node* curr)
{
  if(curr==NULL)
  return;
  if(*head==curr)
  *head=curr->next;
  if(curr->prev!=NULL)
  curr->prev->next=curr->next;
  if(curr->next!=NULL)
  curr->next->prev=curr->prev;
  free(curr);
}

//decrementar o numero de bestpops do nó e se chegar ao zero remover o mesmo da lista
void decrese_bestpops(list_node **head,int amount_decrease,char* queryID)
{
  list_node* aux;
  aux=search_node(queryID,*head);
  if(aux->bestpops==amount_decrease)
  {
    remove_node(head,aux);
  }
  else
  {
    aux->bestpops=aux->bestpops-amount_decrease;
  }
}
