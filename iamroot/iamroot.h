#ifndef _IAMROOT_H
#define _IAMROOT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/timerfd.h>

typedef struct _program_args
{
  char* streamid;
  char* ipaddr;//endereço IP da interface de rede usada pela aplicação
  char* tport;//porto TCP onde a aplicação aceita sessões de outros pares a jusante (por omissão 58000)
  char* uport; //porto UDP do servidor de acesso (por omissão, 58000)
  char* rsaddr;//endereço IP do servidor de raízes (por omissão, 193.136.138.142)
  char* rsport;//porto UDP do servidor de raízes (por omissão, 59000)
  int tcpsessions;
  /*o número de sessões TCP, não inferior a um,
  que esta instância da aplicação irá aceitar para a ligação de pares a jusante (por omissão, um)*/
  int bestpops;
  /*o número de pontos de acesso, não inferior a um, a recolher por esta
  instância da aplicação, quando raiz, durante a descoberta de pontos de acesso(por omissão,um)*/
  int tsecs;
  /*
  período, em segundos, associado ao registo periódico que a raiz deve fazer
  no servidor de raízes (por omissão, 5 segundos)
  */
}program_args;

typedef struct display_options
{
  int display;  /*0 off   ,  1   on*/
  int format;   /*0 ascii,   1 hex*/
  int debug;    /*0 off,     1 on */
  int help; /*0 off , 1 on */
}display_options;

typedef struct _clients
{
	char* ipaddr;
	char* tport;
	int fd;
}_clients;

enum {_root, not_root};

int init_timer(program_args options,struct itimerspec* p_new_value);
void init_program_args(program_args* prog_args);
void init_display_options(display_options* options);
void Usage();
void parse_command_line(char *argv[],program_args* args,display_options* options,int argc);

void process_user_input(char* input, display_options* options,program_args* program_args,
    int udp_rootserver_client_fd,struct addrinfo *udp_rootserver_client_res,
  int accessserver_fd,struct addrinfo* udp_accessserver_res,int tcp_streamclient_fd,int tcp_streamserver_fd
  ,_clients*** clients,int state,int avails,char*tcp_streamserver_ipaddr,char* tcp_streamserver_port);

void display_tree(_clients** clients,program_args options, int debug);

void display_status(program_args options,int state,_clients** clients,int avails
  ,char*tcp_streamserver_ipaddr,char* tcp_streamserver_port);

void display_streams(int udp_rootserver_client_fd,struct addrinfo* udp_rootserver_client_res);
#endif
