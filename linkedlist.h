#ifndef LINKEDLIST_HEADER
#define LINKEDLIST_HEADER

#include <stdio.h>
#include <stdlib.h>

typedef struct fileinfo{
  char *name;
  int size;
  char *owner;
  char *ip;
  char *port;
} FileInfo;

typedef struct node {
  FileInfo data;
  struct node * next;
} Node;

typedef struct list {
  Node * head; 
} List;

List * emptylist();
void add(FileInfo data, List * list);
void delete(FileInfo data, List * list);
int search(char *fileName, List * list, FileInfo *data);
void display(List * list);
void reverse(List * list);
void destroy(List * list);

#endif
