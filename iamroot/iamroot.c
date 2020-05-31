#include "iamroot.h"
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>

#include "network_communication.h"
#include "protocol.h"

//inicializa e retorna o descritor timerfd segundo o tempo indicado nos argumentos do programa
int init_timer(program_args options,struct itimerspec* p_new_value)
{
    int timerfd;
    (*p_new_value).it_value.tv_sec =options.tsecs;
    (*p_new_value).it_value.tv_nsec = 0;
    (*p_new_value).it_interval.tv_sec = 0;
    (*p_new_value).it_interval.tv_nsec = 0;
    timerfd=timerfd_create(CLOCK_REALTIME,0);
    if(timerfd==-1)
    exit(1);
    if(timerfd_settime(timerfd,0,p_new_value,NULL)==-1)
    exit(1);
    return timerfd;

}

//inicializa os argumentos do programa
void init_program_args(program_args* prog_args)
{
  prog_args->tcpsessions=1;
  prog_args->tsecs=5;
  prog_args->bestpops=1;

  prog_args->tport=NULL;
  prog_args->uport=NULL;
  prog_args->rsaddr=NULL;
  prog_args->rsport=NULL;
}

//inicializa as opções de display do programa
void init_display_options(display_options* options)
{
  options->debug=0;
  options->display=1;
  options->format=0;
  options->help=0;
}

//imprime a forma correta de chamada do programa quando é chamado de forma errada
void Usage()
{
  fprintf(stdout,"Usage: iamroot [<streamID>] [-i <ipaddr>] [-t <tport>] [-u <uport>][-s <rsaddr>[:<rsport>][-p <tcpsessions>][-n <bestpops>] [-x <tsecs>][-b] [-d] [-h]\n");
  fprintf(stdout,"\n[<streamID>] [-i <ipaddr>] must be specified if you want to run the program with options\n");
}

//faz parse da linha de comandos e guarda na estrutura struct program_args os argumentos definidos pelo utilizador
void parse_command_line(char* argv[],program_args* args,display_options* options, int argc)
{
  int udp_rootserver_client_fd;
  struct addrinfo *udp_rootserver_client_res;
  char* rcv_message;

  //valores default do programa
  char std_tport[6] = "58000";
  char std_uport[6] = "58000";
  //char std_rsaddr[16]= "193.136.138.142";
  char std_rsaddr[16]= "192.168.1.1";
  char std_rsport[6] = "59000";

  int j,k;
  if(argc==1) //se não for indicado o streamid é imprimida a resposta à mensagem dump do servidor de raizes e o programa fecha
  {
    udp_rootserver_client_fd = udp_client(std_rsaddr,std_rsport,&udp_rootserver_client_res);
    rcv_message=send_message(udp_rootserver_client_fd,dump_message(),udp_rootserver_client_res,0);
    fprintf(stdout,"%s\n",rcv_message);
    free(rcv_message);
    close(udp_rootserver_client_fd);
    free(udp_rootserver_client_res);
    exit(1);
  }

  if(argc==2) //indica a forma correta de chamar o programa (só indicou o stream id)
  {
    Usage();
    exit(-1);
  }
  args->streamid=(char*)malloc(sizeof(char)*(strlen(argv[1])+1)); //aloca a memória para guardar o stramid
  if(args->streamid==NULL)
  exit(-1);
  strcpy((*args).streamid,(char*)argv[1]);

  for(int i=0; i< argc;i++)
  {
      // program flags for info
    if (strcmp(argv[i],"-b")==0)
      (*options).display=0;//off
    else if (strcmp(argv[i],"-d")==0)
      (*options).debug=1;
    else if (strcmp(argv[i],"-h")==0)
      (*options).help=1;

      //program flags for stream options
    else if (strcmp(argv[i],"-i")==0)//ip_adress
        {
          args->ipaddr=(char*)malloc((strlen(argv[i])+1)*sizeof(char));//aloca a memória para guardar o ip address
          if(args->ipaddr==NULL)
          exit(-1);
          strcpy((*args).ipaddr,argv[i+1]); //preenche a memória
        }
    else if (strcmp(argv[i],"-t")==0)
        {
          args->tport=(char*)malloc((strlen(argv[i])+1)*sizeof(char));
          if(args->tport==NULL)
          exit(-1);
          strcpy((*args).tport,argv[i+1]); //preenche a memória
        }

    else if (strcmp(argv[i],"-u")==0)
    {
      args->uport=(char*)malloc((strlen(argv[i])+1)*sizeof(char)); //aloca a memória para guardar o porto udp
      if(args->uport==NULL)
      exit(-1);
      strcpy((*args).uport,argv[i+1]); //preenche a memória
    }
    else if (strcmp(argv[i],"-s")==0)
    {
      args->rsaddr=(char*)malloc((strlen(argv[i+1])*sizeof(char))); //aloca a memória
      if(args->rsaddr==NULL)
      exit(-1);
      args->rsport=(char*)malloc((strlen(argv[i+1])*sizeof(char))); //aloca a memória
      if(args->rsport==NULL)
      exit(-1);
      for(j=0,k=0;argv[i+1][k]!=':';j++,k++) //preenche a memória
      {
        args->rsaddr[j]=argv[i+1][k];
      }
      (args->rsaddr)[j]='\0';
      for(j=0,k=k+1;argv[i+1][k]!='\0';j++,k++) //preenche a memória
      {
        args->rsport[j]=argv[i+1][k];
      }
      (args->rsport)[j]='\0';
    }
    else if (strcmp(argv[i],"-p")==0)
      sscanf(argv[i+1],"%d",&(*args).tcpsessions); //preenche a memória
    else if (strcmp(argv[i],"-n")==0)
      sscanf(argv[i+1],"%d",&(*args).bestpops); //preenche a memória
    else if (strcmp(argv[i],"-x")==0)
      sscanf(argv[i+1],"%d",&(*args).tsecs); //preenche a memória

  }

  //se os argumentos nao forem indicados, preenche com os valores default
  if (args->tport==NULL) {
    if((args->tport=(char*)malloc((strlen(std_tport)+1)*sizeof(char)))==NULL)
    {
      fprintf(stderr,"Error allocating memory\n");
      exit(-1);
    }
    strcpy(args->tport,std_tport);
  }

  if (args->uport==NULL) {
    if((args->uport=(char*)malloc((strlen(std_uport)+1)*sizeof(char)))==NULL)
    {
      fprintf(stderr,"Error allocating memory\n");
      exit(-1);
    }
    strcpy(args->uport,std_uport);
  }

  if (args->rsaddr==NULL) {
    if((args->rsaddr=(char*)malloc((strlen(std_rsaddr)+1)*sizeof(char)))==NULL)
    {
      fprintf(stderr,"Error allocating memory\n");
      exit(-1);
    }
    strcpy(args->rsaddr,std_rsaddr);
  }

  if (args->rsport==NULL) {
    if((args->rsport=(char*)malloc((strlen(std_rsport)+1)*sizeof(char)))==NULL)
    {
      fprintf(stderr,"Error allocating memory\n");
      exit(-1);
    }
    strcpy(args->rsport,std_rsport);
  }
  if(args->ipaddr==NULL)
  {
    printf("Specify Ip address with -i flag\n");
    Usage();
    exit(-1);
  }
  if(argv[1][0]=='-')// não foi especificado o streamid
  {
    udp_rootserver_client_fd = udp_client(args->rsaddr,args->rsport,&udp_rootserver_client_res);
    rcv_message=send_message(udp_rootserver_client_fd,dump_message(),udp_rootserver_client_res,0);
    fprintf(stdout,"%s\n",rcv_message);
    free(rcv_message);
    close(udp_rootserver_client_fd);
    free(udp_rootserver_client_res);
    exit(1);
  }
}

//imprime o streamid e envia aos clientes que estão a baixo na àrvore a mensagem tree_query
void display_tree(_clients** clients,program_args options, int debug)
{
  char* output_message;
  printf("%s\n",options.streamid);
  for(int i=0;i<options.tcpsessions;i++)
  {
    if(clients[i]->fd>=0)
    {
      output_message=tree_query_message(clients[i]->ipaddr,clients[i]->tport);
      send_tcp(clients[i]->fd,output_message,debug,0);
      free(output_message);
    }
  }
}

//quando pedido pelo utilizador, imprime o estado do programa
void display_status(program_args options,int state,_clients** clients,int avails
  ,char*tcp_streamserver_ipaddr,char* tcp_streamserver_port)
{
  int occupied_sessions=options.tcpsessions-avails;
  printf("\n");
  printf("STATUS\n");
  printf("streamid:%s\n",options.streamid);
  if(state==_root)
  {
    printf("state:root\n");
    printf("Root ipaddr:%s Root uport:%s\n",options.ipaddr,options.uport);
  }
  else if(state==not_root)
  {
    printf("state:not root\n");
    printf("P2pserver ipaddr:%s P2pserver tport:%s\n",tcp_streamserver_ipaddr,tcp_streamserver_port);
  }
  printf("My Access point ipaddr:%s tport:%s\n",options.ipaddr,options.tport);
  printf("Number of access points supported:%d Number of access points occupied:%d\n",options.tcpsessions,occupied_sessions);
  printf("Clients' List:\n");
  for(int i=0;i<options.tcpsessions;i++)
  {
    if(clients[i]->fd>=0)
    {
      printf("Client %d ",i+1);
      if(clients[i]->ipaddr!=NULL)
      {
        printf("ipaddr:%s ",clients[i]->ipaddr);
      }
      if(clients[i]->tport!=NULL)
      {
        printf("tport:%s ",clients[i]->tport);
      }
    }
  }
}

//imprime a resposta ao dump do servidor de raizes
void display_streams(int udp_rootserver_client_fd,struct addrinfo* udp_rootserver_client_res)
{
  char* rcv_message;
  char* output_message=dump_message();
  rcv_message=send_message(udp_rootserver_client_fd,
    output_message,udp_rootserver_client_res,1);
  printf("%s\n",rcv_message);
  free(output_message);
  free(rcv_message);
}

//faz parse e processa o input do utilizador
void process_user_input(char* input, display_options* options,program_args* program_args,
    int udp_rootserver_client_fd,struct addrinfo *udp_rootserver_client_res,
  int accessserver_fd,struct addrinfo* udp_accessserver_res,int tcp_streamclient_fd,int tcp_streamserver_fd
  ,_clients*** clients,int state,int avails,char*tcp_streamserver_ipaddr,char* tcp_streamserver_port)
{
    char* output_message;
    if (strcmp(input,"streams\n")==0)
      {
          display_streams(udp_rootserver_client_fd,udp_rootserver_client_res);
      }
    else if (strcmp(input,"status\n")==0)
      {
          display_status(*program_args,state,*clients,avails,tcp_streamserver_ipaddr,tcp_streamserver_port);
      }
    else if (strcmp(input,"display on\n")==0)
      {
        (*options).display=1;
      }
    else if (strcmp(input,"display off\n")==0)
      {
        (*options).display=0;
        printf("\n");
      }
    else if (strcmp(input,"format ascii\n")==0)
      {
        (*options).format=0;
      }
    else if (strcmp(input,"format hex\n")==0)
      {
        (*options).format=1;
      }
    else if (strcmp(input,"debug on\n")==0)
      {
        (*options).debug=1;
      }
    else if (strcmp(input,"debug off\n")==0)
      {
        (*options).debug=0;
      }
    else if (strcmp(input,"tree\n")==0 && state==_root)
      {
        display_tree(*clients,*program_args,(*options).debug);
      }
    else if (strcmp(input,"exit\n")==0)
      {
          //termina graciosamente o programa, libertando memória alocada e fechando as sockets
          if(state==_root)
          {
            output_message=remove_message(*program_args);
            send_udp(output_message,udp_rootserver_client_fd,udp_rootserver_client_res,0);
          }
          //exit after closing all sockets and freeing allocated memory
          for (int i = 0; i<program_args->tcpsessions; i++)
          {
            if((*clients)[i]->fd>=0)
            {
              close((*clients)[i]->fd);
              if((*clients)[i]->ipaddr!=NULL)
                free((*clients)[i]->ipaddr);
              if((*clients)[i]->tport!=NULL)
                free((*clients)[i]->tport);
              free((*clients)[i]);
            }
          }
          free(*clients);
          close(udp_rootserver_client_fd);
          close(accessserver_fd);
          close(tcp_streamclient_fd);
          close(tcp_streamserver_fd);
          free(udp_accessserver_res);
          free(udp_rootserver_client_res);
          free(input);
          printf("\n\nProgram succesfully closed!\n");
          exit(1);
      }
    else
      {
        fprintf(stderr, "\n\nInput not recognized!\n");
      }
}
