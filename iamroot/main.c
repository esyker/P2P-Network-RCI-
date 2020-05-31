#include "iamroot.h"
#include "network_communication.h"
#include "protocol.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <getopt.h>

//Para fazer data process ver se o tipo de mensagem do tcp_client é data_message
// e ler a data para um buffer para imprimir se display_options estiver on, se não estiver não imprimir
//MUDAR A FUNÇÂO TCP_CLIENT

int main(int argc, char *argv[], program_args* args) {

  //inicialização de variáveis relacionadas com opções do programa
  program_args program_options;
  display_options screen_options;

  //inicialização dos argumentos do programa
  init_program_args(&program_options);
  //inicialização das opções do ecrã do utilizador
  init_display_options(&screen_options);
  //parse dos argumentos do programa e preenchimento da estutura struct program_args
  parse_command_line(argv,&program_options,&screen_options,argc);

  //inicialização de variáveis
  int udp_rootserver_client_fd=0, udp_accessserver_client_fd=0 , udp_accessserver_fd=0;
  int tcp_p2pstreamserver_client_fd=0,tcp_p2pstreamserver_fd=0, maxfd=0,set_maxfd=0;

  char udp_accessserver_ipaddr[46];
  char udp_accessserver_port[10];
  char tcp_streamserver_ipaddr[46];
  char tcp_streamserver_port[10];

  int state,ret;
  fd_set rset;

  int avails= program_options.tcpsessions;
  int queryID=0;

  list_node * pop_query_list_head=NULL;
  _clients** clients=init_clients(program_options.tcpsessions);

  char* rcv_message=(char*)malloc(128*sizeof(char));
  if(rcv_message==NULL)
  exit(-1);
  char* output_message;
  char* buffer=(char*)malloc(128*sizeof(char));
  if(buffer==NULL)
  exit(-1);
  int nbytes;
  int i;

  struct addrinfo *udp_rootserver_client_res, *udp_accessserver_client_res, *udp_accessserver_res;

  //timerfd funciona como um descritor que quando chega ao fim do tempo defenido ativa o select
  struct itimerspec new_value;
  int timerfd=0;
  timerfd=init_timer(program_options,&new_value);

  //saber se é raiz, inicializar concexao com root server
  udp_rootserver_client_fd = udp_client(program_options.rsaddr,program_options.rsport,&udp_rootserver_client_res);
  //enviar a mensagem who is root ao servidor de raizes
  output_message=whoisroot_message(program_options.streamid,program_options.ipaddr,program_options.tport);
  rcv_message=send_message(udp_rootserver_client_fd,output_message,udp_rootserver_client_res,screen_options.debug);
  printf("%s\n",rcv_message);

  //parse da mensagem de resposta do servidor de raizes para saber se é raiz da árvore ou não
  state=parse_rootis_urroot(rcv_message,udp_accessserver_ipaddr,udp_accessserver_port);
  free(output_message);
  free(rcv_message);
  //printf("your state is:%d\n",state);
  //printf("%s %s\n",udp_accessserver_ipaddr,udp_accessserver_port);


  //inicialização dos sockets dos clientes e servidores respetivos para cada situação (root ou não root)
  if(state==not_root)
  {
    //cria o cliente udp para comunicar com o servidor de acesso
    udp_accessserver_client_fd=udp_client(udp_accessserver_ipaddr,udp_accessserver_port,&udp_accessserver_client_res);
    //comunica com o servidor de acesso, obtem um ponto de acesso e retorna o descritor do cliente tcp que recebe o stream
    tcp_p2pstreamserver_client_fd=init_tcpclientp2p(udp_accessserver_client_fd,udp_accessserver_client_res,
      tcp_streamserver_ipaddr,tcp_streamserver_port,screen_options.debug);
    if(tcp_p2pstreamserver_client_fd==-1)
    {
      //se não for bem sucedido
      free(udp_accessserver_client_res);
      free(udp_rootserver_client_res);
      close(udp_accessserver_client_fd);
      close(udp_rootserver_client_fd);
      printf("Error communicationg with access server to get acess point!\n");
      printf("Program successfully closed!\n");
      exit(-1);
    }
      //criar servidor tcp que aceita ligações de clientes
      tcp_p2pstreamserver_fd=tcp_server(program_options.ipaddr,program_options.tport,program_options.tcpsessions);
    }
  else if (state==_root)
  {
    //criar servidor de acesso udp
    udp_accessserver_fd=udp_server(program_options.uport,&udp_accessserver_res);

    //parse do stream id para retirar o ip address e o porto de onde pode descarregar o stream
    parse_stream_id(program_options.streamid,tcp_streamserver_ipaddr,tcp_streamserver_port);

    //criar cliente que vai descarregar o stream
    tcp_p2pstreamserver_client_fd=tcp_client(tcp_streamserver_ipaddr,tcp_streamserver_port);

    //criar servidor tcp que aceita ligações de clientes
    tcp_p2pstreamserver_fd=tcp_server(program_options.ipaddr,program_options.tport,program_options.tcpsessions);
  }
  while(1) //ciclo de while exterior que serve para quando a raiz deixa de ser raiz
  {
      //inicializar o valor maximo dos descritores estáticos, com o STDIN_FILENO, conexão p2p  e cliente do servidor de raízes
    if(state==not_root)
    {
      if(STDIN_FILENO > maxfd) set_maxfd = STDIN_FILENO;
      if(tcp_p2pstreamserver_fd > set_maxfd) set_maxfd = tcp_p2pstreamserver_fd;
      if(tcp_p2pstreamserver_client_fd > set_maxfd) set_maxfd = tcp_p2pstreamserver_client_fd;
      if(udp_rootserver_client_fd > set_maxfd) set_maxfd = udp_rootserver_client_fd;
      if(udp_accessserver_client_fd > set_maxfd) set_maxfd = udp_accessserver_client_fd;

      while(1) //ciclo do select
      {
        //inicializar o fd_set rset
        FD_ZERO(&rset);
        FD_SET(STDIN_FILENO,&rset);
        FD_SET(tcp_p2pstreamserver_fd, &rset);
        FD_SET(tcp_p2pstreamserver_client_fd,&rset);
        FD_SET(udp_rootserver_client_fd,&rset);
        FD_SET(udp_accessserver_client_fd,&rset);
        FD_SET(timerfd,&rset);
        maxfd=set_maxfd;
        //adicionar ao set dos descritores os clientes
        for(i=0;i<program_options.tcpsessions;i++)
        {
          if(clients[i]->fd>=0)
          {
            FD_SET(clients[i]->fd,&rset);
            maxfd=max(maxfd,clients[i]->fd);
          }
        }
        //select
        if ((ret = select(maxfd+1,&rset,NULL,NULL,NULL)) == -1)
        {
          printf("erro no select\n");
          exit(1);
        }
        if(FD_ISSET(STDIN_FILENO,&rset))
        {
          //lê o input do utilizador, faz parse do mesmo e processa as ações devidas para cada comando escrito
          fgets(buffer,128,stdin);
          process_user_input(buffer,&screen_options,&program_options,
              udp_rootserver_client_fd,udp_rootserver_client_res,
            udp_accessserver_client_fd,udp_accessserver_client_res,tcp_p2pstreamserver_client_fd,tcp_p2pstreamserver_fd,
            &clients,state,avails,tcp_streamserver_ipaddr,tcp_streamserver_port);
        }
        if(FD_ISSET(tcp_p2pstreamserver_fd,&rset)) //aceita conexões de clientes tcp
        {
          Accept (tcp_p2pstreamserver_fd,&rset,clients,&avails,program_options.streamid,program_options.tcpsessions,screen_options.debug);
        }
        for (int i = 0; i<program_options.tcpsessions; i++)
        {
          if(clients[i]->fd>=0) //se o descritor do cliente estiver iniciado deixa de ser -1
          {
            if(FD_ISSET(clients[i]->fd,&rset))
            {
              //faz parse da mensagem recebida do cliente, processa-a e respode adequadamente
              process_message_tcp_server(&clients[i]->fd,&tcp_p2pstreamserver_client_fd,state,&clients,&avails,
                &program_options,&pop_query_list_head,NULL,&rset,i,screen_options.debug);
            }
          }
        }

        if(FD_ISSET(tcp_p2pstreamserver_client_fd,&rset))
        {
          //processa a mensagem vinda do iamroot a que está ligado, faz o parse e responde adequadamente
          if(process_message_tcp_client(&tcp_p2pstreamserver_client_fd,&state,&clients,&avails,&screen_options,
            &program_options,&pop_query_list_head,&udp_accessserver_client_res,&udp_accessserver_client_fd
              ,udp_rootserver_client_fd,udp_rootserver_client_res,&udp_accessserver_fd,&udp_accessserver_res,
              tcp_streamserver_ipaddr,tcp_streamserver_port)==-1)
          {
          udp_accessserver_client_fd=udp_client(udp_accessserver_ipaddr,udp_accessserver_port,&udp_accessserver_client_res);
          tcp_p2pstreamserver_client_fd=init_tcpclientp2p(udp_accessserver_client_fd,udp_accessserver_client_res,
            tcp_streamserver_ipaddr,tcp_streamserver_port,screen_options.debug);
          }
        }
        if(state==_root) //no caso de passar a ser a raiz da árvore
          break;
        }
      }

      if (state==_root) //se for a raiz
      {
        //inicializar o valor maximo dos descritores estáticos, com o STDIN_FILENO, conexão p2p  e cliente do servidor de raízes
        if(STDIN_FILENO > set_maxfd) set_maxfd = STDIN_FILENO;
        if(tcp_p2pstreamserver_fd > set_maxfd) set_maxfd = tcp_p2pstreamserver_fd;
        if(tcp_p2pstreamserver_client_fd > set_maxfd) set_maxfd = tcp_p2pstreamserver_client_fd;
        if(udp_rootserver_client_fd > set_maxfd) set_maxfd = udp_rootserver_client_fd;
        if(udp_accessserver_fd > set_maxfd) set_maxfd = udp_accessserver_fd;

        while(1) //ciclo do select
        {
          //inicializar o fd_set rset
          FD_ZERO(&rset);
          FD_SET(STDIN_FILENO,&rset);
          FD_SET(tcp_p2pstreamserver_fd, &rset);
          FD_SET(tcp_p2pstreamserver_client_fd,&rset);
          FD_SET(udp_rootserver_client_fd,&rset);
          FD_SET(udp_accessserver_fd,&rset);
          FD_SET(timerfd,&rset);
          maxfd=set_maxfd;
          //adicionar ao set dos descritores os clientes
          for(i=0;i<program_options.tcpsessions;i++)
          {
            if(clients[i]->fd>=0)
            {
              FD_SET(clients[i]->fd,&rset);
              maxfd=max(maxfd,clients[i]->fd);
            }
          }

          if ((ret = select(maxfd+1,&rset,NULL,NULL,NULL)) == -1)
          {
            printf("erro no select\n");
            exit(1);
          }
          if(FD_ISSET(STDIN_FILENO,&rset)!=0)
          {
            //lê o input do utilizador, faz parse do mesmo e processa as ações devidas para cada comando escrito
            fgets(buffer,128,stdin);
            process_user_input(buffer,&screen_options,&program_options,
            udp_rootserver_client_fd,udp_rootserver_client_res,
            udp_accessserver_fd,udp_accessserver_res,tcp_p2pstreamserver_client_fd,tcp_p2pstreamserver_fd,
            &clients,state,avails,tcp_streamserver_ipaddr,tcp_streamserver_port);
          }

          if(FD_ISSET(udp_accessserver_fd,&rset)!=0) //recebe, faz parse e responde aos clientes que comunicam com o servidor de acesso via udp
          {
            process_message_udp_server(udp_accessserver_fd,udp_accessserver_res,&pop_query_list_head,
            program_options,clients,avails,&queryID,screen_options.debug);
          }

          if(FD_ISSET(tcp_p2pstreamserver_fd,&rset)) //aceita conexões de clientes tcp
          {
            Accept (tcp_p2pstreamserver_fd,&rset,clients,&avails,program_options.streamid,program_options.tcpsessions,screen_options.debug);
          }
          if(FD_ISSET(tcp_p2pstreamserver_client_fd,&rset)!=0) //recebe o stream e processa o seu conteudo
          {
            process_message_stream(clients,tcp_p2pstreamserver_client_fd,program_options,screen_options);
          }
          if(FD_ISSET(timerfd,&rset)) //timer, quando termina o tempo defenido volta a perguntar ao servidor de raizes se é raiz ou não
          {
            output_message=whoisroot_message(program_options.streamid,program_options.ipaddr,program_options.tport);
            rcv_message=send_message(udp_rootserver_client_fd,output_message,udp_rootserver_client_res,screen_options.debug);
            if(timerfd_settime(timerfd,0,&new_value,NULL)==-1)
            exit(1);
          }
          for (int i = 0; i<program_options.tcpsessions; i++)
          {
            if(clients[i]->fd>=0) //se o descritor do cliente estiver iniciado deixa de ser -1
            {
              if(FD_ISSET(clients[i]->fd,&rset))
              {
                //faz parse da mensagem recebida do cliente, processa-a e respode adequadamente
                process_message_tcp_server(&clients[i]->fd,&tcp_p2pstreamserver_client_fd,state,&clients,&avails,
                &program_options,&pop_query_list_head,udp_accessserver_res,&rset,i,screen_options.debug);
              }

            }
          }
        }
      }
    }
}
