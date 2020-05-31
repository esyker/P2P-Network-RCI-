#include "protocol.h"
#include "iamroot.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>



void print_string_as_hex(char* string)
{
  int i;
  for(i=0;string[i]!='\0';i++)
  {
    printf("%04x ",(unsigned char)string[i]);
    fflush(stdout);
  }
}

char* whoisroot_message (char* streamid,char* ipaddr, char* uport)
{
  char * message=(char*)malloc(128*sizeof(char));
  if(message==NULL)
  exit(-1);
  strcpy(message,"WHOISROOT ");
  strcat(message,streamid);
  strcat(message," ");
  strcat(message,ipaddr);
  strcat(message,":");
  strcat(message,uport);
  strcat(message,"\n");
  return message;
}

char* dump_message ()
{
    char * message=(char*)malloc(6*sizeof(char));
    if(message==NULL)
    exit(-1);
    strcpy(message,"DUMP\n");
    return message;
}

char* popreq_message ()
{
    char * message=(char*)malloc(8*sizeof(char));
    if(message==NULL)
    exit(-1);
    strcpy(message,"POPREQ\n");
    return message;
}

char* popresp_message (char* stream_id, char* ipaddr, char*tport)
{
    char * message=(char*)malloc(128*sizeof(char));
    if(message==NULL)
    exit(-1);
    strcpy(message,"POPRESP");
    strcat(message," ");
    strcat(message,stream_id);
    strcat(message," ");
    strcat(message,ipaddr);
    strcat(message,":");
    strcat(message,tport);
    strcat(message,"\n");
    return message;
}

char* welcome_message (char* streamid)
{
      char * message=(char*)malloc(128*sizeof(char));
      if(message==NULL)
      exit(-1);
      strcpy(message,"WE");
      strcat(message," ");
      strcat(message,streamid);
      strcat(message,"\n");
      return message;
}

char* remove_message(program_args options)
{
  char * message=(char*)malloc(128*sizeof(char));
  if(message==NULL)
  exit(-1);
  strcpy(message,"REMOVE");
  strcat(message," ");
  strcat(message,options.streamid);
  strcat(message,"\n");
  return message;
}
char* newpop_message (char* ipaddr, char * tport)
{
  char * message=(char*)malloc(128*sizeof(char));
  if(message==NULL)
  exit(-1);
  strcpy(message,"NP");
  strcat(message," ");
  strcat(message,ipaddr);
  strcat(message,":");
  strcat(message,tport);
  strcat(message,"\n");
  return message;
}

char * redirect_message(char* ipaddr, char* tport)
{
  char * message=(char*)malloc(128*sizeof(char));
  if(message==NULL)
  exit(-1);
  strcpy(message,"RE");
  strcat(message," ");
  strcat(message,ipaddr);
  strcat(message,":");
  strcat(message,tport);
  strcat(message,"\n");
  return message;
}

char* stream_flowing_message()
{
  char * message=(char*)malloc(4*sizeof(char));
  if(message==NULL)
  exit(-1);
  strcpy(message,"SP\n");
  return message;
}

char* broken_stream_message()
{
  char * message=(char*)malloc(4*sizeof(char));
  if(message==NULL)
  exit(-1);
  strcpy(message,"BS\n");
  return message;
}

char* data_message(int nbytes, char* data_content)
{
  char * message=(char*)malloc(320*sizeof(char));
  if(message==NULL)
  exit(-1);
  char hex[5];
  sprintf(hex, "%d", nbytes);
  strcpy(message,"DA");
  strcat(message," ");
  strcat(message,hex);
  strcat(message,"\n");
  strcat(message,data_content);
  return message;
}

char* popquery_message(char* _queryid, int _bestpops)
{
  char bestpops[30];
  //char queryid[30];
  sprintf(bestpops,"%d",_bestpops);
  //sprintf(queryid,"%x",_queryid);
  char * message=(char*)malloc(128*sizeof(char));
  if(message==NULL)
  exit(-1);
  strcpy(message,"PQ");
  strcat(message," ");
  strcat(message,_queryid);
  strcat(message," ");
  strcat(message,bestpops);
  strcat(message,"\n");
  return message;
}

char* popreply_message(char* _queryid,char* ipaddr, char* tport,int _avails)
{
  char avails[30];
  //char queryid[30];
  sprintf(avails,"%d",_avails);
  //sprintf(queryid,"%d",_queryid);
  char * message=(char*)malloc(128*sizeof(char));
  if(message==NULL)
  exit(-1);
  strcpy(message,"PR");
  strcat(message," ");
  strcat(message,_queryid);
  strcat(message," ");
  strcat(message,ipaddr);
  strcat(message,":");
  strcat(message,tport);
  strcat(message," ");
  strcat(message,avails);
  strcat(message,"\n");
  return message;
}

char * tree_query_message(char* ipaddr, char* tport)
{
    char * message=(char*)malloc(128*sizeof(char));
    if(message==NULL)
    exit(-1);
    strcpy(message,"TQ ");
    strcat(message,ipaddr);
    strcat(message,":");
    strcat(message,tport);
    strcat(message,"\n");
    return message;
}

char* tree_reply_message(char* ipaddr,char* tport,int tcpsessions_, _clients * clients)
{
  char tcpsessions[10];
  char * message=(char*)malloc(128*sizeof(char));
  if(message==NULL)
  exit(-1);
  strcpy(message,"TR ");
  strcat(message,ipaddr);
  strcat(message,":");
  strcat(message,tport);
  strcat(message," ");
  sprintf(tcpsessions,"%d",tcpsessions_);
  strcat(message,tcpsessions);
  strcat(message,"\n");
  for(int i=0;i<=tcpsessions_;i++)
  {
    if(clients[i].fd > 0)
    {
      strcat(message,clients[i].ipaddr);
      strcat(message,":");
      strcat(message,clients[i].tport);
      strcat(message,"\n");
    }
  }

  return message;
}

char* error_message()
{
  char * message=(char*)malloc(7*sizeof(char));
  if(message==NULL)
  exit(-1);
  strcpy(message,"ERROR\n");
  return message;
}
////////////////////////////////////////////////////////////////////////////////////////////
///////////////////    PARSE DAS MENSAGENS             /////////////////////////////////////

void parse_stream_id(char* streamid, char*tcp_streamserver_ipaddr,char* tcp_streamserver_port)
{
  int i, j;
  for(i=0;streamid[i]!=':';i++)
  {
  }
  for(i=i+1,j=0;streamid[i]!=':';i++,j++)
  {
    tcp_streamserver_ipaddr[j]=streamid[i];
  }
  tcp_streamserver_ipaddr[j]='\0';

  for(i=i+1,j=0;streamid[i]!='\0';i++,j++)
  {
    tcp_streamserver_port[j]=streamid[i];
  }
  tcp_streamserver_port[j]='\0';

  return;
}

int parse_welcome(char* rcv_message)
{
  int i=0, j=0;

//  printf("%s\n", rcv_message);
  if(rcv_message[0]=='W')
  {
    if(rcv_message[1]=='E')
    {
      if(rcv_message[2]==' ')
      {
        return welcome;
      }
    }
  }
  return -1;
}

int parse_rootis_urroot(char* rcv_message, char* udp_acessserver_ipaddr,char* udp_acessserver_port)
{
  int i=0, j=0;

//  printf("%s\n", rcv_message);
  if(rcv_message[0]=='U')
  {
    if(rcv_message[1]=='R')
    {
      if(rcv_message[2]=='R')
      {
        if(rcv_message[3]=='O')
        {
          if(rcv_message[4]=='O')
          {
            if(rcv_message[5]=='T')
            {
                return _root;
            }
          }
        }
      }
    }
  }

  else if (rcv_message[0]=='R')
  {
    if(rcv_message[1]=='O')
    {
      if(rcv_message[2]=='O')
      {
        if(rcv_message[3]=='T')
        {
          if(rcv_message[4]=='I')
          {
            if(rcv_message[5]=='S')
            {
                if(rcv_message[6]==' ')
                {
                    for(i=7;rcv_message[i]!=' ';i++)
                    {
                    }
                    for(i=i+1,j=0;rcv_message[i]!=':';i++,j++)
                    {
                      udp_acessserver_ipaddr[j]=rcv_message[i];
                      //printf("i=%d, m[i]=%c\n", i,rcv_message[i]);
                      //printf("j=%d , ip[i]=%c\n",j,udp_acessserver_ipaddr[j]);
                    }
                    udp_acessserver_ipaddr[j]='\0';
                    //printf("%s",udp_acessserver_ipaddr);
                    for(i=i+1,j=0;rcv_message[i]!='\n';i++,j++)
                    {
                      udp_acessserver_port[j]=rcv_message[i];
                    //  printf("i=%d\n", i);
                    //  printf("j=%d \n",j);
                    }
                    udp_acessserver_port[j]='\0';
                    //printf("%s",udp_acessserver_port);
                    return not_root;
                }
            }
          }
        }
      }
    }
  }
  return -1;
}

int parse_popreq(char* rcv_message)
{
  int i=0, j=0;

  if (rcv_message[0]=='P')
  {
    if(rcv_message[1]=='O')
    {
      if(rcv_message[2]=='P')
      {
        if(rcv_message[3]=='R')
        {
          if(rcv_message[4]=='E')
          {
            if(rcv_message[5]=='Q')
            {
                return popreq;
            }
          }
        }
      }
    }
  }
  return -1;
}

int parse_popresp(char* rcv_message, char* tcp_p2pserver_ipaddr,char* tcp_p2pserver_tport)
{
  int i=0, j=0;

  if (rcv_message[0]=='P')
  {
    if(rcv_message[1]=='O')
    {
      if(rcv_message[2]=='P')
      {
        if(rcv_message[3]=='R')
        {
          if(rcv_message[4]=='E')
          {
            if(rcv_message[5]=='S')
            {
                if(rcv_message[6]=='P')
                {
                  if(rcv_message[7]==' ')
                  {
                      for(i=8;rcv_message[i]!=' ';i++)
                      {
                      }

                      for(i=i+1,j=0;rcv_message[i]!=':';i++,j++)
                      {
                        tcp_p2pserver_ipaddr[j]=rcv_message[i];
                      }
                      tcp_p2pserver_ipaddr[j]='\0';
                      for(i=i+1,j=0;rcv_message[i]!='\n';i++,j++)
                      {
                        tcp_p2pserver_tport[j]=rcv_message[i];
                      }
                      tcp_p2pserver_tport[j]='\0';
                      return 1;
                  }
                }
            }
          }
        }
      }
    }
  }
  return -1;
}

int parse_newpop(char* rcv_message, char** tcp_p2pserver_client_ipaddr,char** tcp_p2pserver_client_tport)
{
  int i=0, j=0;
  (*tcp_p2pserver_client_ipaddr)=(char*)malloc(47*sizeof(char));
  if(*tcp_p2pserver_client_ipaddr==NULL)
    exit(-1);
  (*tcp_p2pserver_client_tport)=(char*)malloc(6*sizeof(char));
  if(*tcp_p2pserver_client_tport==NULL)
    exit(-1);
  if (rcv_message[0]=='N')
  {
    if(rcv_message[1]=='P')
    {
          if(rcv_message[2]==' ')
          {

              for(i=3,j=0;j<47&&rcv_message[i]!=':';i++,j++)
              {
                (*tcp_p2pserver_client_ipaddr)[j]=rcv_message[i];
              }
                (*tcp_p2pserver_client_ipaddr)[j]='\0';

              for(i=i+1,j=0;j<6&&rcv_message[i]!='\n';i++,j++)
              {
                (*tcp_p2pserver_client_tport)[j]=rcv_message[i];
              }
                (*tcp_p2pserver_client_tport)[j]='\0';

              return new_pop;
            }
      }
  }
    free(*tcp_p2pserver_client_ipaddr);
    free(*tcp_p2pserver_client_tport);
    return -1;
}

int parse_redirect(char* rcv_message, char* tcp_p2pserver_ipaddr,char* tcp_p2pserver_tport)
{
  int i=0, j=0;

  if (rcv_message[0]=='R')
  {
    if(rcv_message[1]=='E')
    {
          if(rcv_message[2]==' ')
          {

              for(i=3,j=0;rcv_message[i]!=':';i++,j++)
              {
                tcp_p2pserver_ipaddr[j]=rcv_message[i];
              }
                tcp_p2pserver_ipaddr[j]='\0';

              for(i=i+1,j=0;rcv_message[i]!='\n';i++,j++)
              {
                tcp_p2pserver_tport[j]=rcv_message[i];
              }
                tcp_p2pserver_tport[j]='\0';
        return 1;
              }
      }
    }
    return -1;
}

int parse_popquery(char* rcv_message,int* bestpops,char** queryID)
{
  int i=0, j=0;
  char bestpops_str[20];
  (*queryID)=(char*)malloc(5*sizeof(char));
  if(*queryID==NULL)
    exit(-1);
  if (rcv_message[0]=='P')
  {
    if(rcv_message[1]=='Q')
    {
          if(rcv_message[2]==' ')
          {

              for(i=3,j=0;rcv_message[i]!=' ';i++,j++)
              {
                (*queryID)[j]=rcv_message[i];
              }
              (*queryID)[j]='\0';

              for(i=i+1,j=0;rcv_message[i]!='\n';i++,j++)
              {
                  bestpops_str[j]=rcv_message[i];
              }
              bestpops_str[j]='\0';
              if((sscanf(bestpops_str,"%d",bestpops))!=1)
              {
                free(*queryID);
                return -1;
              }
            return pop_query;
          }
    }
  }
    free(*queryID);
    return -1;
}

int parse_popreply(char* rcv_message,int*avails,char** tcp_p2pstreamserver_ipaddr,char** tcp_p2pstreamserver_tcpport ,char** queryID)
{
  int i=0, j=0;
  char avails_str[20];

  if (rcv_message[0]=='P')
  {
    if(rcv_message[1]=='R')
    {
          if(rcv_message[2]==' ')
          {
            *tcp_p2pstreamserver_ipaddr=(char*)malloc(47*sizeof(char));
            if(*tcp_p2pstreamserver_ipaddr==NULL)
              exit(-1);
            *tcp_p2pstreamserver_tcpport=(char*)malloc(7*sizeof(char));
            if(*tcp_p2pstreamserver_tcpport==NULL)
              exit(-1);
            *queryID=(char*)malloc(7*sizeof(char));
            if(*queryID==NULL)
              exit(-1);
            for(i=3,j=0;rcv_message[i]!=' ';i++,j++)
            {
              (*queryID)[j]=rcv_message[i];
            }
            (*queryID)[j]='\0';

            for(i=i+1,j=0;rcv_message[i]!=':';i++,j++)
            {
              (*tcp_p2pstreamserver_ipaddr)[j]=rcv_message[i];
            }
            (*tcp_p2pstreamserver_ipaddr)[j]='\0';

              for(i=i+1,j=0;rcv_message[i]!=' ';i++,j++)
              {
                (*tcp_p2pstreamserver_tcpport)[j]=rcv_message[i];
              }
              (*tcp_p2pstreamserver_tcpport)[j]='\0';

              for(i=i+1,j=0;rcv_message[i]!='\n';i++,j++)
              {
                  avails_str[j]=rcv_message[i];
              }
              avails_str[j]='\0';
              if((sscanf(avails_str,"%d",avails))!=1)
                return -1;
              return pop_reply;
              }
          }
    }
    return -1;
}

int parse_data(char* rcv_message,char* data_content)
{
  int i=0, j=0;
  char _nbytes[10];
  int nbytes;
  if (rcv_message[0]=='D')
  {
    if(rcv_message[1]=='A')
    {
      if(rcv_message[2]==' ')
      {
        for(i=3,j=0;rcv_message[i]!='\n';i++,j++)
        {
          _nbytes[j]=rcv_message[i];
        }
        _nbytes[j]='\0';
        sscanf(_nbytes,"%d",&nbytes);
        for(i=i+1,j=0;j!=nbytes;i++,j++)
        {
          data_content[j]=rcv_message[i];
        }
        data_content[j]='\0';
        return data;
      }
    }
  }
  return -1;
}

int parse_stream_flowing(char* rcv_message)
{
  if (rcv_message[0]=='S')
  {
    if(rcv_message[1]=='F')
    {
      if(rcv_message[2]=='\n')
      {
        return stream_flowing;
      }
    }
  }
  return -1;
}

int parse_broken_stream(char* rcv_message)
{
  if (rcv_message[0]=='B')
  {
    if(rcv_message[1]=='S')
    {
      if(rcv_message[2]=='\n')
      {
        return broken_stream;
      }
    }
  }
  return -1;
}

int parse_tree_query(char* rcv_message, char* tcp_p2pserver_ipaddr,char* tcp_p2pserver_tport)
{
  int i=0, j=0;

  if (rcv_message[0]=='T')
  {
    if(rcv_message[1]=='Q')
    {
      for(i=3,j=0;rcv_message[i]!=':';i++,j++)
      {
        tcp_p2pserver_ipaddr[j]=rcv_message[i];
      }
      tcp_p2pserver_ipaddr[j]='\0';
      for(i=i+1,j=0;rcv_message[i]!='\n';i++,j++)
      {
        tcp_p2pserver_tport[j]=rcv_message[i];
      }
      tcp_p2pserver_tport[j]='\0';
      return tree_query;
    }
  }

  return -1;
}

int parse_tree_reply(char* rcv_message,char* tree_node,int nbytes)
{
  int i=0, j=0;

  if (rcv_message[0]=='T')
  {
    if(rcv_message[1]=='R')
    {
      if(rcv_message[2]==' ')
      {
          for(i=3,j=0;rcv_message[i]!=' ';i++,j++)
          {
            tree_node[j]=rcv_message[i];
          }
          tree_node[j]=' ';
          j++;
          tree_node[j]='(';
          j++;
          for(i=i+1;rcv_message[i]!='\n';i++,j++)
          {
            tree_node[j]=rcv_message[i];
          }
          tree_node[j]=' ';
          j++;
          for(i=i+1;i<nbytes;i++,j++)
          {
            if(rcv_message[i]=='\n')
             tree_node[j]=' ';
            else
              tree_node[j]=rcv_message[i];
          }
          tree_node[j]=')';
          tree_node[j+1]='\0';
          return tree_reply;
      }
    }
  }
  return -1;
}
