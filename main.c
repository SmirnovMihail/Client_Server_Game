#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "lex.h"
#include "sin.h"


int main(int argc, char **argv)
{
  int fd=0;
  Lexem_list *list;
  if (argc<1)
  {
    perror("You didn't entered file name\n");
    exit(1);
  }
  else
    fd=open(argv[1], O_RDONLY);
  list=analize(fd);
  sintaxis_analizer(list);
  print_list(list);
  return 0;
}
