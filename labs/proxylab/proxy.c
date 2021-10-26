#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";


struct request_message {
  char method[20];
  char host[MAXLINE];
  char port[10];
  char path[MAXLINE];
  char version[20];
  struct list *header_list;
};

struct list {
  char content[MAXLINE];
  struct list *next;
};

int handle_request(int fd, struct request_message *message);
void generate_new_message(char new_message[], struct request_message *message);
int parse_uri(const char *uri, char *port, char *host, char *pathname);


void free_request_message(struct request_message *message)
{
  struct list *ptr = message->header_list;
  struct list *next_ptr = NULL;
  while (ptr != NULL) {
    next_ptr = ptr->next;
    free(ptr);
    ptr = next_ptr;
  }
  free(message);
}

int main(int argc, char *argv[])
{
  int connfd, clientfd;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  char hostname[MAXLINE], port[MAXLINE];
  char new_message[MAXLINE];
  printf("listern port in proxy: %s\n", argv[1]);
  int listenfd = open_listenfd(argv[1]);
  if (listenfd < 0) {
    fprintf(stderr, "can't listen in the localhost\n");
  }
  while (1) {
    clientlen = sizeof(clientaddr);
    printf("listening...\n");
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    printf("accept a connection\n");
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    struct request_message *message = (struct request_message *)malloc(sizeof(struct request_message));
    if (handle_request(connfd, message) < 0) {
      fprintf(stderr, "handle request failed\n");
    }
    memset(new_message, 0, sizeof(new_message)); //zero new_message when we want to build a new message.
    generate_new_message(new_message, message);
    printf("port: %s\n", message->port);
    clientfd = Open_clientfd("www.baidu.com", "80");
    if (clientfd < 0) {
      fprintf(stderr, "can't connect to the server\n");
    }
    printf("connect the server\n");
    free_request_message(message);
    Close(connfd); 
  }  
}

int handle_request(int fd, struct request_message *message)
{
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  rio_t rio;
  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("request line:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);
  strcpy(message->method, method);
  strcpy(message->version, version);
  if (parse_uri(uri, message->port, message->host, message->path) < 0) {
    return -1;
  }
  printf("request headers:\n");
  Rio_readlineb(&rio, buf, MAXLINE);
  while (strcmp(buf, "\r\n")) {
    printf("%s", buf);
    if (strncmp(buf, "Host", 4) == 0) {
      strcpy(message->host, &buf[5]);
    } else if (strncmp(buf, "Connection", 10) == 0) {
     
    } else if (strncmp(buf, "Proxy-Connection", 16) == 0) {
    
    } else if (strncmp(buf, "User-Agent", 10) == 0) {
    
    } else {
      struct list* new_header = (struct list*)malloc(sizeof(struct list));
      strcpy(new_header->content, buf);
      new_header->next = message->header_list;
      message->header_list = new_header;
    }
    Rio_readlineb(&rio, buf, MAXLINE);
  }
  return 0;
}


void generate_new_message(char new_message[], struct request_message *message)
{
  sprintf(new_message, "%s %s:%s\%s HTTP/1.0\r\n", message->method, message->host, message->port, message->path);
  sprintf(new_message, "Host: %s\r\n", message->host);
  sprintf(new_message, "%s", user_agent_hdr);
  sprintf(new_message, "Connection: close\r\n");
  sprintf(new_message, "Proxy-Connection: close\r\n");
  struct list *ptr = message->header_list;
  while (ptr != NULL) {
    sprintf(new_message, "%s", ptr->content);
    ptr = ptr->next;
  }
  sprintf(new_message, "\r\n");
}

int parse_uri(const char *uri, char *port, char *host, char *pathname)
{ 
  const char *url = strncmp(uri, "http://", 7) == 0 ? uri + 7 : uri;
  int i = 0;
  if (url[i] == '/') {  // this line only cantains path, host is in the request header.
    strcpy(pathname, &url[i + 1]);
    strcpy(port, "80");
    return 0;
  }
  while((url[i] != ':') && (url[i] != '/')) {
    host[i] = url[i];
    i++;
  }
  //extract port number.
  if (url[i] == ':') {
    i++;
    int j = i;
    while ((url[j] >= '0') && (url[j] <= '9')) {
      port[j - i] = url[j];
      j++;
    }
  } else { //this line don't cantain port number, so use default port number.
    strcpy(port, "80");
  }
  while (url[i] != '/') {
    i++;
  }
  i++;
  //extract pathname.
  strcpy(pathname, &url[i]);
  return 0;
}

