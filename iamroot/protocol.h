#ifndef _PROTOCOL_H
#define _PROTOCOL_H
#include "iamroot.h"

enum
{
  popreq, popresp,
  welcome,new_pop,redirect,
  stream_flowing,broken_stream,
  data,
  pop_query,pop_reply,
  tree_query, tree_reply
};

void print_string_as_hex(char* string);

char* whoisroot_message (char* streamid,char* ipaddr, char* uport);

char* dump_message ();

char* popreq_message ();

char* popresp_message (char* stream_id, char* ipaddr, char*tport);

char* welcome_message (char* streamid);

char* remove_message(program_args options);

char* newpop_message (char* ipaddr, char * tport);

char * redirect_message(char* ipaddr, char* tport);

char* stream_flowing_message();

char* broken_stream_message();

char* data_message(int nbytes, char* data);

char* popquery_message(char* _queryid, int _bestpops);

char* popreply_message(char* _queryid,char* ipaddr, char* tport,int _avails);

char * tree_query_message(char* ipaddr, char* tport);

char* tree_reply_message(char* ipaddr,char* tport,int tcpsessions_, _clients * clients);

char* error_message();

void parse_stream_id(char* streamid, char*tcp_streamserver_ipaddr,char* tcp_streamserver_port);

int parse_welcome(char* rcv_message);

int parse_rootis_urroot(char* rcv_message, char* udp_acessserver_ipaddr,char* udp_acessserver_port);

int parse_popreq(char* rcv_message);

int parse_popresp(char* rcv_message, char* tcp_p2pserver_ipaddr,char* tcp_p2pserver_tport);

int parse_newpop(char* rcv_message, char** tcp_p2pserver_client_ipaddr,char** tcp_p2pserver_client_tport);

int parse_redirect(char* rcv_message, char* tcp_p2pserver_ipaddr,char* tcp_p2pserver_tport);

int parse_popquery(char* rcv_message,int* bestpops,char** queryID);

int parse_popreply(char* rcv_message,int*avails,char** tcp_p2pstreamserver_ipaddr,char** tcp_p2pstreamserver_tcpport ,char** queryID);

int parse_data(char* rcv_message,char* data);

int parse_stream_flowing(char* rcv_message);

int parse_broken_stream(char* rcv_message);

int parse_tree_query(char* rcv_message, char* tcp_p2pserver_ipaddr,char* tcp_p2pserver_tport);

int parse_tree_reply(char* rcv_message,char* tree_node,int nbytes);

#endif
