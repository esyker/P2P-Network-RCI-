#include "network_communication.h"
#include "protocol.h"
#include "iamroot.h"


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
#include <sys/select.h>


#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

//inicializa a estrutura dos clientes com a sua socket e addrinfo
_clients** init_clients(int backlog)
{
	_clients** new_array=(_clients**)malloc(backlog*sizeof(_clients*));
	if(new_array==NULL)
  exit(-1);
	for(int i=0;i<backlog;i++)
  {
		new_array[i]=(_clients*)malloc(backlog*sizeof(_clients*));
		if(new_array[i]==NULL)
	  exit(-1);
    new_array[i]->ipaddr=NULL;
    new_array[i]->tport=NULL;
    new_array[i]->fd=-1;
  }

	return new_array;
}

//aceita clientes, cria os seus descritores, envia a mensagem welcome e se não houver pontos de acesso suficientes envia a mensagem redirect
void Accept (int listenfd,fd_set* p_allset,_clients** clients,int* avails,char* streamid,
int tcp_sessions,int debug)
{
	int newfd=0;
	int i;
	struct sockaddr_in addr;
	socklen_t addrlen=sizeof(addr);

	if((newfd=accept(listenfd,(struct sockaddr*)&addr,&addrlen))==-1)
		printf("Error Creating new tcp connection!\n");
	if((*avails)>0)
	{
			for(i=0;clients[i]->fd>=0&&i<tcp_sessions;i++) //procura um espaço no array clients[] que ainda não está preenchido com um cliente
			{
		  }
			clients[i]->fd=newfd;
			FD_SET(clients[i]->fd,p_allset);
			send_tcp(newfd,welcome_message(streamid),debug,0);
			(*avails)--;
	}
	else
	{
			send_tcp(newfd,redirect_message(clients[0]->ipaddr,clients[0]->tport),debug,0); //redirect se o numero de sessões que pode aceitar chegar ao fim (avails chegar ao fim)
			close(newfd);
	}
	return;
}

//cria o cliente udp e retorna o descritor devido
int udp_client(char* ipaddr, char* udp_port, struct addrinfo **res)
{
  struct addrinfo hints;
  int udp_fd,n;
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;      //IPv4
  hints.ai_socktype=SOCK_DGRAM;//udp_socket
  hints.ai_flags=AI_NUMERICSERV;

  //standard "192.168.1.1", "59000"
  //my computer "192.168.1.80" ,"59000"
  n=getaddrinfo(ipaddr,udp_port,&hints,res);
  if(n!=0)//error
    exit(1);

  udp_fd=socket((*res)->ai_family,(*res)->ai_socktype,(*res)->ai_protocol);
  if(udp_fd==-1)//error
    exit(1);

  return udp_fd;
}

//cria um servidor udp e retorna o devido descritor
int udp_server(char* port, struct addrinfo **res)
{
  struct addrinfo hints;
  int udp_server_fd ,n;
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;      //IPv4
  hints.ai_socktype=SOCK_DGRAM;//udp_socket
  hints.ai_flags=AI_PASSIVE|AI_NUMERICSERV;//server

  n=getaddrinfo(NULL,port,&hints,res);
  if(n!=0)//error
    exit(1);

  udp_server_fd =socket((*res)->ai_family,(*res)->ai_socktype,(*res)->ai_protocol);
  if(udp_server_fd==-1)//error
	{
		printf("Could not create udp server.\n");
		exit(1);
	}

  n=bind(udp_server_fd,(*res)->ai_addr,(*res)->ai_addrlen);
  if(n==-1)/*error*/
	{
		printf("Could not create udp server.\n");
		exit(1);
	}

  return udp_server_fd;
}

//cria um cliente tcp e retorna o devido descritor
int tcp_client(char* ipaddr, char* tport)
{
  struct addrinfo hints, *res;
  int tcp_streamserver_client_fd,n;
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//tcp_socket
  hints.ai_flags=AI_NUMERICSERV;

  //"192.168.1", "59000"
  n=getaddrinfo(ipaddr,tport,&hints,&res);
  if(n!=0)//error
    exit(1);

  tcp_streamserver_client_fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
  if(tcp_streamserver_client_fd==-1)//error
    exit(1);

  n=connect(tcp_streamserver_client_fd,res->ai_addr,res->ai_addrlen);

  free(res);
  return tcp_streamserver_client_fd;
}

//cria um servidor tcp e retorna o devido descritor
int tcp_server(char* ipaddr,char * tport,int backlog)
{
  struct addrinfo hints, *res;
  int tcp_p2pstreamserver_client_fd,n;
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//tcp_socket
  hints.ai_flags=AI_NUMERICSERV;

  n=getaddrinfo(ipaddr,tport,&hints,&res);
  if(n!=0)//error
    exit(1);

  tcp_p2pstreamserver_client_fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);

  if(tcp_p2pstreamserver_client_fd==-1)//error
    exit(1);

  n=bind(tcp_p2pstreamserver_client_fd,res->ai_addr,res->ai_addrlen);
  if(n==-1)
		exit(1);

  n=listen(tcp_p2pstreamserver_client_fd,backlog+1);
  if(n==-1)
    exit(1);


  /*accept */

  free(res);
  return tcp_p2pstreamserver_client_fd;

}

//envia uma mensagem udp a um servidor e se não obtiver resposta fecha o programa. O timeout é feito utilizando o select e é de 5 segundos
char* send_message (int fd,char* message,struct addrinfo *res, int debug)
{
  char buffer[800];
  int n, addrlen, retval;
  char* rcv_message;
  struct sockaddr_in addr;
	struct timeval tv;
	fd_set rfds;

	/* Wait up to five seconds. */

  tv.tv_sec = 5;
  tv.tv_usec = 0;

	//debug functionality
	if(debug==1)
	printf("MESSAGE SENT UDP: %s\n", message);

  n=sendto(fd,message,strlen(message),0,res->ai_addr,res->ai_addrlen);
  //terminar a message em \0
  if(n==-1)/*error*/exit(1);
	FD_ZERO(&rfds);
	FD_SET(fd,&rfds);

  addrlen=sizeof(addr);
	retval=select(fd+1, &rfds, NULL, NULL, &tv);
	if(FD_ISSET(fd,&rfds))
	{
		n=recvfrom(fd,buffer,800,0,(struct sockaddr*) &addr, &addrlen);
		if(n==-1)
		{
			printf("Error communicating with udp Server!\n");
		}
	}
	else
	{
		printf("Timeout communicating with udp Server!\n");
		exit(-1);
	}
  rcv_message=(char*)malloc((strlen(buffer)+1)*sizeof(char));
	if(rcv_message==NULL)
	exit(-1);
  strcpy(rcv_message,buffer);

	//funcionalidade debug
	if(debug==1)
	printf("MESSAGE RECEIVED UDP: %s\n", rcv_message);

	rcv_message[n]='\0';
  return rcv_message;
}

//envia uma mensagem udp
int send_udp(char * message, int fd, struct addrinfo *res, int debug)
{
	int n;
	n=sendto(fd,message,strlen(message),0,res->ai_addr,res->ai_addrlen);
	//terminar a message em \0
	if(n==-1||n==0)/*error*/printf("Error sending udp communication message!\n");

	//debug functionality
	if(debug==1)
	printf("MESSAGE SENT UDP: %s\n", message);

	free(message);
	return n;
}

//recebe uma mensagem tcp, com um timeout de 5 segundos, que se chegar ao final faz a função retornar NULL
char* receive_udp(int fd, struct addrinfo *res,int*nbytes, int debug)
{
	int retval;
	struct timeval tv;
	fd_set rfds;

	/* Wait up to five seconds. */
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	char * rcv_message=(char*)malloc(128*sizeof(char));
	if(rcv_message==NULL)
	exit(-1);

	FD_ZERO(&rfds);
	FD_SET(fd,&rfds);
	retval=select(fd+1, &rfds, NULL, NULL, &tv);
	if(FD_ISSET(fd,&rfds))
	{
		*nbytes=recvfrom(fd,rcv_message,128,0,res->ai_addr,&res->ai_addrlen);
		if(*nbytes==0||*nbytes==-1) //erro ao receber a mensagem
			return NULL;
	}
	else
	{
		return NULL;
	}
	rcv_message[*nbytes]='\0';
	//funcionalidade debug
	if(debug==1)
	printf("MESSAGE RECEIVED UDP: %s\n", rcv_message);

	return rcv_message;
}

//envia uma mensagem tcp
void send_tcp(int sockfd,char* message,int debug,int data)
{
		char* ptr=message;
		int nleft=strlen(message);
		int nwritten=0;
		while(nleft>0) //ciclo para enviar a mensagem toda, só termina de enviar quando a mensagem chegar ao final
		{
			nwritten=write(sockfd,ptr,nleft);
			if(nwritten==-1)
			{/*error*/
				printf("Erro a enviar mensagem TCP\n");
				exit(1);
			}
			nleft=nleft-nwritten;
			ptr=ptr+nwritten;
		}

		//funcionalidade debug. A variavel data serve para impedir que imprima o stream
		if((debug==1)&&(data==0))
		printf("MESSAGE SENT TCP: %s\n", message);
}

//recebe uma mensagem tcp
char* receive_tcp(int sockfd,int* nbytes ,int debug,int data)
{
  char* message=(char*)malloc((300*sizeof(char)));
	if(message==NULL)
	exit(-1);
	*nbytes=read(sockfd,message,299);
	if(*nbytes==-1||*nbytes==0) //erro ao receber a mensagem
	{
		//error/connection lost
		return NULL;
	}
	else
	{
		message[*nbytes]='\0';
		//funcionalidade debug. A variavel data serve para impedir que imprima o stream
		if((debug==1)&&(data==0))
		printf("MESSAGE RECEIVED TCP: %s\n", message);

		return message;
	}
}

//obtem o ponto de acesso e se for bem sucedido retorna o descritor do cliente tcp que recebe o stream
int init_tcpclientp2p(	int udp_accessserver_client_fd,struct addrinfo* udp_accessserver_client_res,
char*tcp_streamserver_ipaddr,char*tcp_streamserver_port,int debug)
{
	int nbytes;
	char* rcv_message;

	//enviar popreq ao servidor de acesso para obter um ponto de acesso
	char* output_message=popreq_message();
	if(send_udp(output_message,udp_accessserver_client_fd,udp_accessserver_client_res,debug)==-1)
	{
		//erro ao comunicar com o servidor de acesso
		return -1;
	}
	else //comunicação bem sucedida
	{
		//recebe a mensagem de resposta do servir de acesso
		rcv_message=receive_udp(udp_accessserver_client_fd,udp_accessserver_client_res,&nbytes,debug);
		if(nbytes==0||nbytes==-1)
		{
			//erro ao não receber nada
			return -1;
		}
		//se receber a mensagem faz parse e retorna o descritor que liga com o stream
		else if(parse_popresp(rcv_message,tcp_streamserver_ipaddr,tcp_streamserver_port)==popresp)
		{
			free(rcv_message);
			//criar cliente tcp do stream
			return tcp_client(tcp_streamserver_ipaddr,tcp_streamserver_port);
		}
		else
		{
			return -1;
		}
	}
}

//processa as mensagens enviadas ao servidor de acesso. Só é chamado se o programa for a raiz da árvore
void process_message_udp_server(int fd,struct addrinfo *res,list_node ** pop_query_list_head
	,program_args program_options,_clients** clients,int avails,int* _queryID,int debug)
{
	int nbytes;
	list_node* new_node;
  int message_type=0;
  char* output_message;
	char queryID[9];
	int n;
	char * rcv_message=(char*)malloc(128*sizeof(char));
	if(rcv_message==NULL)
	exit(-1);
	char * message;

	rcv_message=receive_udp(fd,res,&nbytes,debug);
	if(nbytes==0||nbytes==-1)
		printf("Error receiving udp message!\n");

  message_type=parse_popreq(rcv_message); //faz parse da mensagem

  if(message_type==popreq) //se for pop request
  {
		if(avails>0) //se ainda tiver espaço livre para mais conexões de clientes envia pop response com o seu ponto de acesso
		{
			output_message=popresp_message (program_options.streamid,program_options.ipaddr,program_options.tport);
			send_udp(output_message,fd,res,debug);
		}
		else //se não tiver espaço disponivel envia pop query aos seus clientes e incrementa o query id
		{
			//POP_QUERY
			sprintf(queryID, "%.4x",*_queryID);
			printf("\nQUERYID: %s\n",queryID);
			(*_queryID)++;
			new_node=create_node(fd,queryID,program_options.bestpops);
			insert_head(pop_query_list_head,&new_node);
			output_message=popquery_message(new_node->queryID,program_options.bestpops);
			for(int i=0;i<program_options.tcpsessions;i++)
			{
				if(clients[i]->fd>=0)
				{
					send_tcp(clients[i]->fd,output_message,debug,0);
				}
			}
		}
  }
  else //enviar mensagem de erro ao cliente que pediu um ponto de acesso se não tiver possibilidade de o adicionar
  {
    output_message=error_message();
		send_udp(output_message,fd,res,debug);
  }
	return;
}

//processa a mensagem recebida do servidor fonte
int process_message_stream(_clients** clients,int tcp_p2pstreamserver_client_fd,program_args options
	,display_options screen_options)
{
	int nbytes;
	int i;
	char* output_message;
	char*rcv_message=receive_tcp(tcp_p2pstreamserver_client_fd,&nbytes,screen_options.debug,1);
	if(screen_options.display==1 &&screen_options.format==0) //display das mensagens do servidor fonte
	{
		printf("\n\n");
		printf("%.*s",nbytes,rcv_message);
	}
	else if(screen_options.display==1 &&screen_options.format==1)
	{
		print_string_as_hex(rcv_message);
	}
	output_message=data_message(nbytes,rcv_message);
	for ( i = 0; i<options.tcpsessions; i++) //envia aos clientes a mensagem recebida pelo servidor fonte do stream
	{
		if(clients[i]->fd>=0)
		{
				send_tcp(clients[i]->fd,output_message,screen_options.debug,1);
		}
	}
	free(rcv_message);
	free(output_message);
}

//processa a mensagem recebida pelos clientes ligados a si mesmo
void process_message_tcp_server(int* clientfd,int*serverfd,int state,_clients*** clients,int* avails,program_args *program_options
	,list_node** p_head,struct addrinfo* udp_acessserver_res,fd_set* p_allset,int client_numb,int debug)
	{
		char* rcv_message;
		//variables used for parsing
		char *tcp_p2pserver_client_ipaddr, *tcp_p2pserver_client_tport;//new_pop
		char* new_tcp_p2pserver_ipaddr, * new_tcp_p2pserver_tport;
		int nbytes;
		//parse pop_query
		int bestpops;
		char* queryID;
		list_node* new_node;
		//parse pop_reply
		int new_pop_avails;
		char* new_pop_ipaddr,*new_pop_tcpport,* new_pop_queryID;
		//parse tree_reply
		char* tree_node;

		list_node* aux_node;
		int message_type;
	  char* output_message;
		//auxiliary variables
		int i;

		rcv_message=receive_tcp((*clients)[client_numb]->fd,&nbytes,debug,0);
		if (nbytes == 0|| nbytes==-1)
		{
				// client is closed, remove clients[i] from clients array
				FD_CLR((*clients)[client_numb]->fd,p_allset);
				close((*clients)[client_numb]->fd);
				free((*clients)[client_numb]->ipaddr);
				free((*clients)[client_numb]->tport);
				(*clients)[client_numb]->fd=-1;
				(*avails)++;
				free(rcv_message);
				return;
		}
	  else if(parse_newpop(rcv_message,&tcp_p2pserver_client_ipaddr,&tcp_p2pserver_client_tport)==new_pop)
	  {
			//adiciona à estrutura dos clientes o seu ip address e porto
			(*clients)[client_numb]->ipaddr=tcp_p2pserver_client_ipaddr;
			(*clients)[client_numb]->tport=tcp_p2pserver_client_tport;
	  }
	  else if(parse_popreply(rcv_message,&new_pop_avails,&new_pop_ipaddr,&new_pop_tcpport ,&new_pop_queryID)==pop_reply)
	  {
			if(state==not_root)
			{			//redirecionar o pop_reply para o servidor p2p
						send_tcp(*serverfd,rcv_message,debug,0);
			}
			else
			{
				//popresp response to popreq after using popquery and popreply for finding access points
				aux_node=search_node(new_pop_queryID,*p_head);
				if(aux_node!=NULL)
				{
						output_message=popresp_message (program_options->streamid,new_pop_ipaddr,new_pop_tcpport);
						send_udp(output_message,aux_node->fd,udp_acessserver_res,debug);
						decrese_bestpops(p_head,new_pop_avails,new_pop_queryID);//update query list
				}
			}

	  }
		else if(parse_tree_reply(rcv_message,tree_node,nbytes)==tree_reply)//se receber a mensagem de resposta tree reply imprime-a na forma adequada
	  {
			if(state==_root)
			{
					printf("\n%s\n",tree_node);
			}
			if(state==not_root)
			{
				send_tcp(*serverfd,rcv_message,debug,0);
			}
	  }
		else
		{
				output_message=error_message();
				send_tcp(*clientfd,output_message,0,0);
		}

		free(rcv_message);
		return;
	}

//processa a mensagem tcp recebida pelo iamroot que está a cima. Só é chamado quando não é a raiz da árvore
int process_message_tcp_client(int* fd,int* state,_clients*** clients,int* avails,display_options* display,
	program_args *program_options,list_node** p_head,struct addrinfo** udp_accessserver_client_res,
	int* udp_accessserver_client_fd,int udp_rootserver_client_fd,struct addrinfo* udp_rootserver_client_res,
	int* udp_accessserver_fd,struct addrinfo** udp_accessserver_res,char* tcp_streamserver_ipaddr,
		char* tcp_streamserver_port )
{
	//variables used for parsing
	char *tcp_p2pserver_client_ipaddr, *tcp_p2pserver_client_tport;//new_pop
	int nbytes;
	char ipaddr[46];
	char tport[10];
	char new_tcp_p2pserver_ipaddr [46];
	char new_tcp_p2pserver_tport [10];
	//parse pop_query
	int bestpops;
	char* queryID;
	list_node* new_node;
	//parse data
	char data_content[400];
	//parse broken_stream
	char udp_accessserver_ipaddr[46];
	char udp_accessserver_port[10];
	char  tcp_p2pstreamserver_ipaddr[46];
	char tcp_p2pstreamserver_port[10];

	list_node* aux_node;
	int message_type;
  char* output_message;
  char* rcv_message=receive_tcp(*fd,&nbytes,display->debug,0);


	//auxiliary variables
	int i;

	//servidor tcp a montante saiu
	if(nbytes==0||nbytes==-1)
	{
			if(*state==not_root)
			{
				output_message=broken_stream_message();
				for(i=0;i<program_options->tcpsessions;i++)
				{
					if((*clients)[i]->fd>=0)
					{
						send_tcp((*clients)[i]->fd,output_message,display->debug,0); //envia indicação de broken stream para os clientes
					}
				}
				free(output_message);
				close(*fd);
				//pergunta novamente ao servidor de raizes quem é a raiz
				output_message=whoisroot_message(program_options->streamid,program_options->ipaddr,program_options->tport);
			  rcv_message=send_message(udp_rootserver_client_fd,output_message,udp_rootserver_client_res,display->debug);
				*state=parse_rootis_urroot(rcv_message,udp_accessserver_ipaddr,udp_accessserver_port);
				if(*state==_root) //se ficar a raiz
				{
					close(*fd);
					close(*udp_accessserver_client_fd);
					free(*udp_accessserver_client_res);
					//cria um servidor de acesso
					*udp_accessserver_fd=udp_server(program_options->uport,udp_accessserver_res);
					parse_stream_id(program_options->streamid,tcp_p2pstreamserver_ipaddr,tcp_p2pstreamserver_port);
					//ligar cliente tcp do stream server ao servidor fonte
					*fd=tcp_client(tcp_p2pstreamserver_ipaddr,tcp_p2pstreamserver_port);
				}
				else if(*state==not_root)
				{
					close(*udp_accessserver_client_fd);
					free(*udp_accessserver_client_res);
					//ligar ao servidor de acesso novo
					*udp_accessserver_client_fd=udp_client(udp_accessserver_ipaddr,udp_accessserver_port,udp_accessserver_client_res);
					close(*fd);
					//pedir ao servidor de acesso um pop e ligar-se
					memset(tcp_streamserver_ipaddr,0,strlen(tcp_streamserver_ipaddr));
					memset(tcp_streamserver_port,0,strlen(tcp_streamserver_port));
					*fd=init_tcpclientp2p(*udp_accessserver_client_fd,*udp_accessserver_client_res
						,tcp_streamserver_ipaddr,tcp_streamserver_port,display->debug);
					if(*fd==-1)//error
					{
						return -1;
					}

				output_message=stream_flowing_message();
				for(i=0;i<program_options->tcpsessions;i++)
				{
					if((*clients)[i]->fd>=0)
					{
						send_tcp((*clients)[i]->fd,output_message,display->debug,1); //envia aos clientes a indicação do restabelecimento do stream
					}
				}
				free(output_message);
				return 1;
			}
		}
	}
	else if(parse_data(rcv_message,data_content)==data) //mensagem de dados
	{
		if(display->display==1&&display->format==0)//display on, ascii format
		{
			printf("\n%s\n",data_content);
			//printf("%.*s",nbytes,data_content);
			fflush(stdout);
		}
		else if(display->display==1&&display->format==1)//display on, hex format
		{
			print_string_as_hex(data_content);
		}
		//send stream to clients
		for(i=0;i<program_options->tcpsessions;i++)
		{
			if((*clients)[i]->fd>=0)
			{
				send_tcp((*clients)[i]->fd,rcv_message,display->debug,1);
			}
		}
	}
  else if(parse_welcome(rcv_message)==welcome) //envia a resposta do welcome new pop
  {
    output_message=newpop_message(program_options->ipaddr,program_options->tport);
		printf ("WELCOME");
		send_tcp(*fd,output_message,display->debug,0);
	  free(output_message);
	}
  else if(parse_redirect(rcv_message,new_tcp_p2pserver_ipaddr,new_tcp_p2pserver_tport)==redirect) //fechar o cliente e abrir um novo com novos dados
  {
		close(*fd);
		memset(tcp_streamserver_ipaddr,0,strlen(tcp_streamserver_ipaddr));
		memset(tcp_streamserver_port,0,strlen(tcp_streamserver_port));
		strcpy(tcp_streamserver_ipaddr,new_tcp_p2pserver_ipaddr);
		strcpy(tcp_streamserver_port,new_tcp_p2pserver_tport);
		*fd=tcp_client(tcp_streamserver_ipaddr,tcp_streamserver_port);
		while(*fd==-1)
		{
			close(*fd);
			memset(tcp_streamserver_ipaddr,0,strlen(tcp_streamserver_ipaddr));
			memset(tcp_streamserver_port,0,strlen(tcp_streamserver_port));
			*fd=tcp_client(tcp_streamserver_ipaddr,tcp_streamserver_port);
		}
  }
	else if(parse_popquery(rcv_message,&bestpops,&queryID)==pop_query)
  {
		if(*avails<=0)
		{
			//mandar a mesma mensagem para baixo
			for(i=0;i<program_options->tcpsessions;i++)
			{
				if((*clients)[i]->fd>=0)
				{
					send_tcp((*clients)[i]->fd,rcv_message,display->debug,0);
				}
			}
		}
		else
		{
			if(bestpops<=*avails) //enviar pop reply para montante
			{
				output_message=popreply_message(queryID,program_options->ipaddr,program_options->tport,bestpops);
				send_tcp(*fd,output_message,display->debug,0);//send message back to the server that asked for the message
				free(output_message);
			}

			else//bestpops>avails
			{
				//envia pop reply para montante e pop query para jusante
				output_message=popreply_message(queryID,program_options->ipaddr,program_options->tport,*avails);
				send_tcp(*fd,output_message,display->debug,0);//send message back to the server that asked for the message
				free(output_message);
				output_message=popquery_message(queryID,bestpops-*avails);
				for(i=0;i<program_options->tcpsessions;i++)
				{
					if((*clients)[i]->fd>=0)
					{
						send_tcp((*clients)[i]->fd,output_message,display->debug,0);
					}
				}
				free(output_message);
			}

		}
  }
  else if(parse_stream_flowing(rcv_message)==stream_flowing)//
  {
		output_message=stream_flowing_message();
		for(i=0;i<program_options->tcpsessions;i++)
		{
			if((*clients)[i]->fd>=0)
			{
				send_tcp((*clients)[i]->fd,output_message,display->debug,0); //envia indicação de broken stream para os clientes
			}
		}
		free(output_message);
  }
  else if(parse_broken_stream(rcv_message)==broken_stream)//
  {
		output_message=broken_stream_message();
		for(i=0;i<program_options->tcpsessions;i++)
		{
			if((*clients)[i]->fd>=0)
			{
				send_tcp((*clients)[i]->fd,output_message,display->debug,0); //envia indicação de broken stream para os clientes
			}
		}
		free(output_message);
  }
	else if(parse_tree_query(rcv_message, ipaddr,tport)==tree_query)
	{
		//envia para os clientes a mensagem tree query e envia para montante a mensagem tree reply
		if(((strcmp(program_options->ipaddr,ipaddr)==0))&&((strcmp(program_options->tport,tport))==0))
		{
				output_message=tree_reply_message(program_options->ipaddr,program_options->tport,program_options->tcpsessions ,**clients);
				send_tcp(*fd,output_message,display->debug,0);
				free(output_message);
				for(i=0;i<program_options->tcpsessions;i++)
			  {
			    if((*clients)[i]->fd>=0)
			    {
			      output_message=tree_query_message((*clients)[i]->ipaddr,(*clients)[i]->tport);
			      send_tcp((*clients)[i]->fd,output_message,display->debug,0);
			      free(output_message);
			    }
			  }
		}
	}
	else
	{
		output_message=error_message();
		for(i=0;i<program_options->tcpsessions;i++)
		{
			if((*clients)[i]->fd>=0)
			{
				send_tcp((*clients)[i]->fd,output_message,display->debug,0); //envia indicação de broken stream para os clientes
			}
		}
		free(output_message);
	}

	free(rcv_message);
	return 1;
}
