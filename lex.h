struct Lexem
{
  int type;
  int string_num;
  char *string;
  void operator=(const Lexem &x);
};

struct Lexem_list
{
  Lexem lex;
  Lexem_list *next;
};

extern Lexem_list* analize(int fd);

extern void print_list(Lexem_list *list);
