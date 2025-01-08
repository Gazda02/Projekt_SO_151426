#include <stdlib.h>
#include <stdio.h>
#include "semafor.h"

int main(int argc, char *argv[]) {
  float masa_bagazu;
  int isVip;

  if(argc != 2){
    perror("pasazer.c | innit | ");
    exit(1);
  }
  isVip = atoi(argv[1]);
  printf("VIP -> %s\n", isVip ? "YES" : "NO");
}
