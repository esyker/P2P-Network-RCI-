#ifndef _NETWORK_COMMUNICATION_H
#define _NETWORK_COMMUNICATION_H

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "iamroot.h"
#include "list.h"
#include "protocol.h"


_clients** init_clients(int backlog);

void Accept (int listenfd,fd_set* p_allset,_clients** clients,int* avails,char* streamid,int tcp_sessions,int debug);

int udp_client(char* ipaddr, char* udp_port, struct addrinfo **res);

int udp_server(char* port, struct addrinfo **res);

int tcp_client(char* ipaddr, char* tport);

int tcp_server(char* ipaddr,char * tport,int backlog);

char* send_message (int fd,char* message,struct addrinfo *res,int debug);

int send_udp(char * message, int fd, struct addrinfo *res,int debug);

char* receive_udp(int fd, struct addrinfo *res,int*nbytes, int debug);

void send_tcp(int sockfd,char* message,int debug,int data);

char* receive_tcp(int sockfd,int* nbytes,int debug,int data);

int init_tcpclientp2p(	int udp_accessserver_client_fd,struct addrinfo* udp_accessserver_client_res,
char*tcp_streamserver_ipaddr,char*tcp_streamserver_port,int debug);

void process_message_udp_server(int fd,struct addrinfo *res,list_node ** pop_query_list_head
	,program_args program_options,_clients** clients,int avails,int* _queryID,int debug);

	int process_message_stream(_clients** clients,int tcp_p2pstreamserver_client_fd,program_args options
		,display_options screen_options);

void process_message_tcp_server(int* clientfd,int*serverfd,int state,_clients*** clients,int* avails,program_args *program_options
		,list_node** p_head,struct addrinfo* udp_acessserver_res,fd_set* p_allset,int client_numb,int debug);

		int process_message_tcp_client(int* fd,int* state,_clients*** clients,int* avails,display_options* display,
			program_args *program_options,list_node** p_head,struct addrinfo** udp_accessserver_client_res,
			int* udp_accessserver_client_fd,int udp_rootserver_client_fd,struct addrinfo* udp_rootserver_client_res,
			int* udp_accessserver_fd,struct addrinfo** udp_accessserver_res,char* tcp_p2pstreamserver_ipaddr,
				char* tcp_p2pstreamserver_port );

#endif
