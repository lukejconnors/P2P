#include "linkedlist.h"

Node * createnode(FileInfo data);

Node * createnode(FileInfo data){
  Node * newNode = malloc(sizeof(Node));
  newNode->data = data;
  newNode->next = NULL;
  return newNode;
}

List * emptylist(){
  List * list = malloc(sizeof(List));
  list->head = NULL;
  return list;
}

void display(List * list) {
  Node * current = list->head;
  if(list->head == NULL) 
    return;
  while(current->next != NULL){
    printf("%s,", current->data.name);
    current = current->next;
  }
  printf("%s\n", current->data.name); 
}

void add(FileInfo data, List * list){
  Node * current = NULL;
  if(list->head == NULL){
    list->head = createnode(data);
  }
  else {
    current = list->head; 
    while (current->next!=NULL){
      current = current->next;
    }
    current->next = createnode(data);
  }
}

void delete(FileInfo data, List * list){
  Node * current = list->head;            
  Node * previous = current;           
  while(current != NULL){           
    if(current->data.name == data.name){      
      previous->next = current->next;
      if(current == list->head)
        list->head = current->next;
      free(current);
      return;
    }                               
    previous = current;             
    current = current->next;        
  }                                 
} 

int search(char *fileName, List * list, FileInfo *data){
  Node * current = list->head;            
  Node * previous = current; 
  FileInfo fileInfo;
  while(current != NULL){           
    if(strcmp(current->data.name, fileName) == 0){      
      data->name = current->data.name;
      data->size = current->data.size;
      data->port = current->data.port;
      data->ip = current->data.ip;
      data->owner = current->data.owner;

      return 1;
    }                               
    previous = current;             
    current = current->next;        
  }
  return 0;
}                                  

void reverse(List * list){
  Node * reversed = NULL;
  Node * current = list->head;
  Node * temp = NULL;
  while(current != NULL){
    temp = current;
    current = current->next;
    temp->next = reversed;
    reversed = temp;
  }
  list->head = reversed;
}

void destroy(List * list){
  Node * current = list->head;
  Node * next = current;
  while(current != NULL){
    next = current->next;
    free(current);
    current = next;
  }
  free(list);
}
