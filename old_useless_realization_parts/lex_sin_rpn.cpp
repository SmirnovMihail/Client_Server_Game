#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


enum type {number_type, string_type, variable_type, label_type,
           function_type, delimiter_type, assignment_type, key_word_type};
enum numbers {thirty_two=32, two=2};
const char *table[]={"if", "then", "goto", "print", "buy", "sell",
                     "prod", "build", "endturn"};

class Buffer
{
  int busy;
  int size;
  char *content;
  void extend();
public:
  Buffer();
  void initialize();
  void add_char(char c);
  void add_string(char *str);
  int get_busy();
  char *get_content();
  char get_char(int index);
  void cleaning();
  void update_info();
//  ~Buffer();
};


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


struct Error_list
{
  char *string;
  int string_num;
  Error_list *next;
};


class Lex_analizer
{
  int state, string_num, assignment_flag, type;
  Buffer buf;
  Buffer tmp_buf;
  Error_list *errors;
  Lexem_list *list;
  Lexem lex;
  enum state {error, home, number, identifier, key_word, string,
              assignment};
  Lexem_list *home_state(char c);
  Lexem_list *key_word_state(char c);
  Lexem_list *string_state(char c);
  Lexem_list *number_state(char c);
  Lexem_list *identifier_state(char c);
  Lexem_list *assignment_state(char c);
  Lexem_list *add_delimiter(char c);
  Lexem_list *lex_formation();
  void error_state();
  int delimiter_check(char c);
public:
  Lex_analizer();
  Error_list *check_error();
  Lexem_list *step(char c);
};


Buffer :: Buffer()
{
  initialize();
}


void Buffer :: initialize()
{
  busy=0;
  size=8;
  content=new char[size+1];
  content[size]=0;
}


void Buffer :: add_char(char c)
{
  if (busy>=size)
    extend();
  content[busy]=c;
  busy++;
  content[busy]=0;
}  


void Buffer :: add_string(char *str)
{
  int len;
  len=strlen(str);
  while (busy+len>=size)
    extend();
  for (int i=0; i<len; i++)
    content[busy+i]=str[i];
  busy=busy+len;
  content[busy]=0;
}


void Buffer :: extend()
{
  char *new_buf=new char[size*2+1];
  for (int i=0; i<busy; i++)
    new_buf[i]=content[i];
  delete[] content;
  content=new_buf;
  size=size*2;
}


int Buffer :: get_busy()
{
  return busy;
}


char* Buffer :: get_content()
{
  return content;
}


char Buffer :: get_char(int index)
{
  return content[index];
}


void Buffer :: cleaning()
{
  delete[] content;
  initialize();
}


void Buffer :: update_info()
{
  content=new char[size];
  busy=0;
}

/*
~Buffer :: Buffer()
{
  delete[] content;
}*/

void Lexem :: operator=(const Lexem &x)
{
  string=x.string;
  string_num=x.string_num;
  type=x.type;
}


char *copy_string(char *str)
{
  int len=strlen(str);
  char *copy=new char[len+1];
  for (int i=0; i<len; i++)
    copy[i]=str[i];
  copy[len]=0;
  return copy;
}


Lex_analizer :: Lex_analizer()
{
  lex.string=0;
  list=0;
  errors=0;
  string_num=1;
  state=home;
}

/*
void Lex_analizer :: read_file_in_buffer(int fd)
{
  int size=8, len;
  char *tmp_buf=new char[size+1];
  while ((len=read(fd, tmp_buf, size))!=0)
  {
    tmp_buf[len]=0;
    buf.add_string(tmp_buf);
  }
}*/


int search(char c, char *table)
{
  int i=0;
  while (table[i]!=0)
  {
    if (table[i]==c)
      return 1;
    i++;
  }
  return 0;
} 


void errors_output_and_exit(Error_list *errors)
{
  int i=1;
  while (errors!=0)
  {
    printf("%d. Error: Mistake in word '%s' in string number %d\n", i,
           errors->string, errors->string_num);
    errors=errors->next;
    i++;
  }
  exit(1);
}


Lexem_list* analize(int fd)
{
  Lexem_list *list=0, *tmp_list, *lex_pointer;
  Lex_analizer automat;
  int size=128, len;
  char *tmp_buf=new char[size+1];
  while ((len=read(fd, tmp_buf, size))!=0)
  {
    tmp_buf[len]=0;
    for (int index=0; index<len; index++)
    {
      lex_pointer=automat.step(tmp_buf[index]);
      if (lex_pointer!=0)
      {
        if (list==0)
          list=lex_pointer;
        else
        {  
          while (tmp_list->next!=0)
            tmp_list=tmp_list->next;
          tmp_list->next=lex_pointer;
        }
        tmp_list=list;
      }
    }
  }
  Error_list *errors=automat.check_error();
  if (errors!=0)
    errors_output_and_exit(errors);
  return list;
}


Error_list* Lex_analizer :: check_error()
{
  if (state!=home)
    error_state();
  return errors;
}


Lexem_list* Lex_analizer :: step(char c)
{ 
  if (state==error)
    error_state();
  if (state==home)
    return home_state(c);
  if (state==number)
    return number_state(c);
  if (state==identifier)
    return identifier_state(c);
  if (state==key_word)
    return key_word_state(c);
  if (state==string) 
    return string_state(c);
  if (state==assignment)
    return assignment_state(c);
  return 0;
}


Lexem_list* Lex_analizer :: assignment_state(char c)
{
  Lexem_list *lex_pointer=0;
  if (c=='=')
  {
    type=assignment_type;
    tmp_buf.add_char(c);
    lex_pointer=lex_formation();
    state=home;
  }
  else
    state=error;
  return lex_pointer;
}


Lexem_list* Lex_analizer :: add_delimiter(char c)
{
  char del[]="+-*/%;<>=,)([]|&";
  type=delimiter_type;
  if (search(c, del))
  {
    lex.string=new char[two];
    lex.string[0]=c;
    lex.string[1]=0;
    lex.type=type;
    lex.string_num=string_num;
    Lexem_list *lex_pointer=new Lexem_list[sizeof(Lexem_list)];
    lex_pointer->lex=lex;
    lex_pointer->next=0;
    lex.string=0;
    lex.type=0;
    lex.string_num=0;
    return lex_pointer;
  }
  return 0;
}  


Lexem_list* Lex_analizer :: home_state(char c)
{
  char del[]="+-*/%;<>=,)([]|&";
  if (c>='0' && c<='9')
  {
    state=number;
    tmp_buf.add_char(c);
  } else
  if (c=='?' || c=='@' || c=='$')
  {
    state=identifier;
    tmp_buf.add_char(c);
  } else
  if (c=='"')
  {
    state=string;
    tmp_buf.add_char(c);
  } else
  if ((c>='A' && c<='Z') || (c>='a' && c<='z'))
  {
    state=key_word;
    tmp_buf.add_char(c);
  } else
  if (search(c, del))
  {
    return add_delimiter(c);
  } else
  if (c==':')
  {
    state=assignment;
    tmp_buf.add_char(c);
  } else
  if (c=='\n')
    string_num++;
  else
  if (c!=' ' && c!='\t')
    state=error;
  return 0;
}


Lexem_list* Lex_analizer :: lex_formation()
{
  lex.string=tmp_buf.get_content();
  lex.string_num=string_num;
  lex.type=type;
  Lexem_list *lex_pointer=new Lexem_list[sizeof(Lexem_list)];
  lex_pointer->lex=lex;
  lex.string=0;
  lex.type=0;
  tmp_buf.update_info();
  return lex_pointer;
}


Lexem_list* Lex_analizer :: number_state(char c)
{
  type=number_type;    
  Lexem_list* lex_pointer=0;
  if (c>='0' && c<='9')
  {
    tmp_buf.add_char(c);
  }
  else
  {
    if (delimiter_check(c))
    {
      lex_pointer=lex_formation();
      lex_pointer->next=add_delimiter(c);
      state=home;
    } else
    if (c==':')
    {
      lex_pointer=lex_formation();
      tmp_buf.add_char(c);
      state=assignment;
    } 
    else
      state=error;
  }
  return lex_pointer;
}   


Lexem_list* Lex_analizer :: identifier_state(char c)
{
  Lexem_list *lex_pointer=0;
  if (tmp_buf.get_char(0)=='$')
    type=variable_type;
  if (tmp_buf.get_char(0)=='@')
    type=label_type;
  if (tmp_buf.get_char(0)=='?')
    type=function_type;
  if ((c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') ||
      (c==':' && type==label_type) || c=='_')
  {
    tmp_buf.add_char(c);
  } 
  else
  {
    if (delimiter_check(c))
    {
      lex_pointer=lex_formation();
      lex_pointer->next=add_delimiter(c);
      state=home;
    } else
    if (c==':')
    {
      lex_pointer=lex_formation();
      tmp_buf.add_char(c);
      state=assignment;
    } 
    else
      state=error;
  }
  return lex_pointer;
}    


int string_compare(const char *str1,const char *str2)
{
  int len1=strlen(str1), len2=strlen(str2);
  if (len1==len2)
  {
    for (int i=0; i<len1; i++)
    {
      if (str1[i]!=str2[i])
        return 0;
    }
    return 1;
  }
  return 0;
}


Lexem_list* Lex_analizer :: key_word_state(char c)
{
  int f=0, i=0;
  type=key_word_type;
  Lexem_list* lex_pointer=0;
  if ((c>='A' && c<='Z') || (c>='a' && c<='z') || c=='_')
  {
    tmp_buf.add_char(c);
  }
  else
  { 
    while (table[i]!=0)
    {
      if (string_compare(tmp_buf.get_content(), table[i]))
        f=1;
      i++;
    }
    if (delimiter_check(c) && f==1)
    { 
      lex_pointer=lex_formation();
      lex_pointer->next=add_delimiter(c);
      state=home;
    } else
    if (c==':')
    {
      lex_pointer=lex_formation();
      tmp_buf.add_char(c);
      state=assignment;
    } 
    else
      state=error;
  }
  return lex_pointer;
}   


Lexem_list* Lex_analizer :: string_state(char c)
{
  tmp_buf.add_char(c);
  type=string_type;
  if (c=='"')
  {
    Lexem_list* lex_pointer=lex_formation();
    lex_pointer->next=add_delimiter(c);
    state=home;
    return lex_pointer;
  }
  return 0;
}

void Lex_analizer :: error_state()
{
  Error_list *tmp=errors;
  if (errors==0)
  {
    errors=new Error_list[sizeof(Error_list)];
    errors->string=tmp_buf.get_content();
    errors->string_num=string_num;
    errors->next=0;
  }
  else
  {
    while(tmp->next!=0)
      tmp=tmp->next;
    tmp->next=new Error_list[sizeof(Error_list)];
    tmp=tmp->next;
    tmp->string=tmp_buf.get_content();
    tmp->string_num=string_num;
    tmp->next=0;
  }
  tmp_buf.update_info();
  state=home;
}


int Lex_analizer :: delimiter_check(char c)
{
  char del[]="+-*/%;<>=,)([]|& \n\t";
  if (c=='\n')
    string_num++;
  if (search(c, del))
  {
    return 1;
  }
  return 0;
}
 

void print_list(Lexem_list *list)
{
  char type1[]="Const string", type2[]="Const number", type3[]="Label",
       type4[]="Variable", type5[]="Function", type6[]="Delimiter",
       type7[]="Key word", type8[]="Assignment";
  char *final_type=0;
  while (list!=NULL)
  {
    printf("%s ",list->lex.string);
    for (int i=0; i+strlen(list->lex.string)<thirty_two; i++)
      putchar(' ');
    if (list->lex.type==string_type)
      final_type=type1;
    if (list->lex.type==number_type)
      final_type=type2;
    if (list->lex.type==label_type)
      final_type=type3;
    if (list->lex.type==variable_type)
      final_type=type4;
    if (list->lex.type==function_type)
      final_type=type5;
    if (list->lex.type==delimiter_type)
      final_type=type6;
    if (list->lex.type==key_word_type)
      final_type=type7;
    if (list->lex.type==assignment_type)
      final_type=type8;
    printf("%d  %s\n\n", list->lex.string_num, final_type);
    list=list->next;
  }
}


//end of second part

//forth part

int rpn_number=0;



class RPN_ex_not_int;      
class RPN_ex_not_label;
class RPN_ex_not_str_not_int;
class RPN_ex_not_var_not_int;
class RPN_ex_not_var_addr;
class RPN_ex_zero_division;
class RPN_ex_unitialized_variable;
class RPN_ex_unitialized_label;
class RPN_ex_double_lab_initialization;

class RPN_element;


struct RPN_item
{
  RPN_item() {rpn_count++;}
  int num;
  static int rpn_count;
  RPN_element *elem;
  RPN_item *next;
  ~RPN_item() {rpn_count--;}
};
  

int RPN_item :: rpn_count=0;


class RPN_element
{
public: 
  virtual ~RPN_element() {}
  virtual void evaluate(RPN_item **stack, RPN_item **curr_elem) const = 0;
protected:
  static void push(RPN_item **stack, RPN_element *elem);
  static RPN_element* pop(RPN_item **stack);
}; 


void RPN_element :: push(RPN_item **stack, RPN_element *elem)
{
  RPN_item *tmp=new RPN_item;
  tmp->elem=elem;
  if (*stack==0)
  {
    *stack=tmp;
    tmp->next=0;
  }
  else
  {
    tmp->next=*stack;
    *stack=tmp;
  }
}


RPN_element* RPN_element :: pop(RPN_item **stack)
{
  if (stack!=0)
  {
    RPN_element *tmp=(*stack)->elem;
    *stack=(*stack)->next;
    return tmp;
  }
  else
    printf("Empty stack!\n");
  return 0;
}


class RPN_const : public RPN_element
{
public:
  virtual RPN_element* clone() const = 0;
  void evaluate(RPN_item **stack, RPN_item **curr_elem) const
  {
    push(stack, clone());
    *curr_elem=(*curr_elem)->next;
  }
  virtual ~RPN_const() {}
};


class RPN_function : public RPN_element
{
public:
  virtual RPN_element* evaluate_fun(RPN_item **stack) const = 0; 
  void evaluate(RPN_item **stack, RPN_item **curr_elem) const
  {
    printf("evaluate function\n");
    RPN_element* res=evaluate_fun(stack);
    if (res) 
      push(stack, res);
    *curr_elem=(*curr_elem)->next;
  }
  virtual ~RPN_function() {}
};


class RPN_int : public RPN_const
{
  int value;
public:
  RPN_int(int a) { value=a; }
  virtual RPN_element *clone() const {return new RPN_int(value);}
  int get() const {return value;}
  virtual ~RPN_int() {}
};


class RPN_label : public RPN_const
{
  RPN_item *value;
  char *name;
public:
  RPN_label(RPN_item* a) { value=a; }
  RPN_label(char *lab_name)
  {
    int len=strlen(lab_name);
    name=new char[len+1];
    for (int i=0; i<len; i++)
      name[i]=lab_name[i];
    name[len]=0;
  }
  void set_label(RPN_item* curr_rpn);
  virtual RPN_element* clone() const {return new RPN_label(value);}
  RPN_item* get() const {return value;}
  char *get_name() const {return name;}
  virtual ~RPN_label() {}
};


void RPN_label :: set_label(RPN_item *curr_rpn)
{
  printf("label adr num = %d\n", curr_rpn->num);
  value=curr_rpn;
}


class RPN_string : public RPN_const
{
  char *value; 
public:
  RPN_string(char *a);
  virtual RPN_element *clone() const;
  char *get() const {return value;}
  virtual ~RPN_string() {delete[] value;}
};


RPN_element* RPN_string :: clone() const
{
  int len=strlen(value);
  char *copy=new char[len+1];
  for (int i=0; i<len; i++)
    copy[i]=value[i];
  copy[len]=0;
  return new RPN_string(copy);
}


RPN_string ::RPN_string(char *a)
{
  int len=strlen(a);
  value=new char[len+1];
  for (int i=0; i<len; i++)
    value[i]=a[i];
  value[len]=0;
}
 

struct Var_table
{
  char *name;
  int value;
  Var_table *next;
};  


class RPN_var_addr : public RPN_const
{
  int *value;
  char *name;
  bool index;
// here put var table
  RPN_var_addr* change_name(int i) const;
public:
  static Var_table *table;
  RPN_var_addr(char *var_name, bool indexing=false)  
  {
    index=indexing;
    value=new int;
    value=0;
    int len=strlen(var_name);
    name=new char[len+1];
    for (int i=0; i<len; i++)
      name[i]=var_name[i];
    name[len]=0;
  }
  virtual RPN_element* clone() const 
  {
    return new RPN_var_addr(name, index);
  }
  virtual void evaluate(RPN_item **stack, RPN_item **curr_elem) const;
  int *get() const {return value;}
  char *get_name() const {return name;}
  void put_variable_in_table(RPN_int *i);
  RPN_int* table_search();
  virtual ~RPN_var_addr() 
  {
    delete value;
    delete[] name;
  }
};


Var_table* RPN_var_addr :: table=0;


RPN_int* RPN_var_addr :: table_search()
{
  Var_table *tmp=table;
  while (tmp!=0)
  {
    if (string_compare(tmp->name, this->name))
      return new RPN_int(tmp->value);
    tmp=tmp->next;
  }
  printf("Can't find variable\n");
  return 0;
} 


void RPN_var_addr :: put_variable_in_table(RPN_int *i)
{
  Var_table *tmp=table;
  int f=0;
  while (tmp!=0 && f==0)
  {
    if (string_compare(tmp->name, this->name))
      f=1;
    else
      tmp=tmp->next;
  }
  if (f==0) 
  {  
    tmp=new Var_table;
    int len=strlen(this->name);
    tmp->name=new char[len+1];
    for (int j=0; j<len; j++)
      tmp->name[j]=this->name[j];
    tmp->name[len]=0;
    tmp->value=i->get();
    tmp->next=table;
    table=tmp;
  }
  else
    tmp->value=i->get();
  printf("add %s %d\n", tmp->name, tmp->value);
}


RPN_var_addr* RPN_var_addr :: change_name(int i) const
{
  int len=0, name_len=strlen(name);
  for (int num=i; num!=0; num=num/10)
    len++;
  char *new_name=new char[name_len+len+1];
  sprintf(new_name, "%s%d", name, i);
  new_name[name_len+len]=0;
  return new RPN_var_addr(new_name);
}

 
void RPN_var_addr :: evaluate(RPN_item **stack,
                              RPN_item **curr_elem) const
{
  if (index)
  {
    printf("index\n");
    RPN_var_addr *tmp;
    RPN_element *operand1=pop(stack);
    RPN_int *i1=dynamic_cast<RPN_int*> (operand1);
    if (i1==0)
      throw new RPN_ex_not_int(operand1);
    tmp=change_name(i1->get());
    push(stack, tmp);
    delete operand1;
  }
  else
  {
    push(stack, clone());
    printf("without index\n");
  }
  *curr_elem=(*curr_elem)->next;
}    
    

class RPN_go : public RPN_element
{
public:
  RPN_go() {}
  void evaluate(RPN_item **stack, RPN_item **curr_elem) const
  {
    RPN_element *operand1=pop(stack);
    RPN_label *lab=dynamic_cast<RPN_label*> (operand1);
    if (lab==0)
      throw new RPN_ex_not_label(operand1);
    RPN_item* addres=lab->get();
    *curr_elem=addres;
    delete operand1;
  }
  virtual ~RPN_go() {}
};


class RPN_go_false : public RPN_element
{
public:
  RPN_go_false() {}
  void evaluate(RPN_item **stack, RPN_item **curr_elem) const
  {
    RPN_element *operand1=pop(stack);
    RPN_label *lab=dynamic_cast<RPN_label*> (operand1);
    if (lab==0)
      throw new RPN_ex_not_label(operand1);
    RPN_element *operand2=pop(stack);
    RPN_int *exp=dynamic_cast<RPN_int*> (operand2);
    if (exp==0)
      throw new RPN_ex_not_int(operand2);
    if (exp->get()==0)
    {
      printf("lab points on %d\n", lab->get()->num);
      RPN_item *addres=lab->get()->next;
      *curr_elem=addres;
    }
    else
      *curr_elem=(*curr_elem)->next;
    delete operand1;
    delete operand2;
  }
  virtual ~RPN_go_false() {}
};


class RPN_print : public RPN_function
{
public:
  RPN_print() {}
  virtual RPN_element* evaluate_fun(RPN_item **stack) const
  {
    printf("try to print\n");
    RPN_element *operand=pop(stack);
    RPN_int *exp=dynamic_cast<RPN_int*> (operand);
    if (exp==0)
    {
      RPN_string *str=dynamic_cast<RPN_string*> (operand);
      if (str==0)
      {
        RPN_var_addr *var=dynamic_cast<RPN_var_addr*> (operand);
        if (var==0)
          throw new RPN_ex_not_str_not_int(operand);
        else
        {
          RPN_int *i=var->table_search();
          printf("%d", i->get());
        }
      }
      else
        printf("%s", str->get());
    }
    else
      printf("%d", exp->get());
    delete operand;
    return 0;
  }
  virtual ~RPN_print() {}     
};   


class RPN_assign : public RPN_function
{
public:
  RPN_assign() {}
  void put_varaible_in_table(Var_table *table, 
                             RPN_var_addr *variable,
                             RPN_int *num);
  virtual RPN_element* evaluate_fun(RPN_item **stack) const
  {
    RPN_element *operand2=pop(stack);
    RPN_int *exp=dynamic_cast<RPN_int*> (operand2);
    if (exp==0)
      throw new RPN_ex_not_int(operand2);
    RPN_element *operand1=pop(stack);
    RPN_var_addr *var=dynamic_cast<RPN_var_addr*> (operand1);
    if (var==0)
      throw new RPN_ex_not_var_addr(operand1);
    var->put_variable_in_table(exp);
    delete operand1;
    delete operand2;
    return 0;
  }
  virtual ~RPN_assign() {}
};


class RPN_sign : public RPN_function
{
public:
  RPN_sign() {}
  RPN_int* convert_to_int(RPN_element *operand) const;
  virtual RPN_int* evaluate_sign(RPN_int *int1, RPN_int *int2) const =0;
  virtual RPN_element *evaluate_fun(RPN_item **stack) const
  {
    RPN_element *operand2=pop(stack);
    RPN_int *i2=convert_to_int(operand2);
 //   printf("m1\n");
    RPN_element *operand1=pop(stack);
   // printf("m2\n");
    RPN_int *i1=convert_to_int(operand1);
   // printf("m3\n");
    RPN_int* res=evaluate_sign(i1, i2);
   // printf("m4\n");
    delete operand1;
    delete operand2;
    return res;
  }  
  virtual ~RPN_sign() {}
};


RPN_int* RPN_sign :: convert_to_int(RPN_element* operand) const
{
  RPN_int *i=dynamic_cast<RPN_int*> (operand);
  if (i==0)
  {
    RPN_var_addr *var=dynamic_cast<RPN_var_addr*> (operand);
    if (var==0)
      throw new RPN_ex_not_var_not_int(operand);
    else
    {
      i=var->table_search();
      if (i==0)
        throw new RPN_ex_unitialized_variable(var);
    }
  } 
  return i;
}


class RPN_plus : public RPN_sign
{
public:
  RPN_plus() {}
  virtual RPN_int* evaluate_sign(RPN_int *int1, RPN_int *int2) const
  {
    int res=int1->get()+int2->get();
    return new RPN_int(res);
  }
  virtual ~RPN_plus() {}
};


class RPN_minus : public RPN_sign
{
public:
  RPN_minus() {}
  virtual RPN_int* evaluate_sign(RPN_int *int1, RPN_int *int2) const
  {
    int res=int1->get()-int2->get();
    return new RPN_int(res);
  }
  virtual ~RPN_minus() {}
};


class RPN_mult : public RPN_sign
{
public:
  RPN_mult() {}
  virtual RPN_int* evaluate_sign(RPN_int *int1, RPN_int *int2) const
  {
    int res=int1->get()*int2->get();
    return new RPN_int(res);
  }
  virtual ~RPN_mult() {}
};


class RPN_div : public RPN_sign
{
public:
  RPN_div() {}
  virtual RPN_int* evaluate_sign(RPN_int *int1, RPN_int *int2) const
  {
    int res, op2=int2->get();
    if (op2)
      res=int1->get()/op2;
    else
      throw new RPN_ex_zero_division();
    return new RPN_int(res);
  }
  virtual ~RPN_div() {}
};


class RPN_more : public RPN_sign
{
public:
  RPN_more() {}
  RPN_int* evaluate_sign(RPN_int *int1, RPN_int *int2) const
  {
    int res;
    if (int1->get()>int2->get())
      res=1;
    else
      res=0;
    return new RPN_int(res);
  }
  virtual ~RPN_more() {}
};


class RPN_less : public RPN_sign
{
public:
  RPN_less() {}
  virtual RPN_int* evaluate_sign(RPN_int *int1, RPN_int *int2) const
  {
    int res;
    if (int1->get()<int2->get())
      res=1;
    else
      res=0;
    return new RPN_int(res);
  }
  virtual ~RPN_less() {}
};


class RPN_equal : public RPN_sign
{
public:
  RPN_equal() {}
  virtual RPN_int* evaluate_sign(RPN_int *int1, RPN_int *int2) const
  {
    int res;
    if (int1->get()==int2->get())
      res=1;
    else
      res=0;
    return new RPN_int(res);
  }
  virtual ~RPN_equal() {}
};


class RPN_or : public RPN_sign
{
public:
  RPN_or() {}
  virtual RPN_int* evaluate_sign(RPN_int *int1, RPN_int *int2) const
  {
    int res=(int1->get() || int2->get());
    return new RPN_int(res);
  }
  virtual ~RPN_or() {}
};


class RPN_and : public RPN_sign
{
public:
  RPN_and() {}
  RPN_int* evaluate_sign(RPN_int *int1, RPN_int *int2) const
  {
    int res=(int1->get() && int2->get());
    return new RPN_int(res);
  }
  virtual ~RPN_and() {}
};


class RPN_buy : public RPN_function
{
public:
  RPN_buy() {}
  virtual RPN_element* evaluate_fun(RPN_item **stack) const
  {
    RPN_element *operand2=pop(stack);
    RPN_int *i2=dynamic_cast<RPN_int*> (operand2);
    if (i2==0)
      throw new RPN_ex_not_int(operand2);
    RPN_element *operand1=pop(stack);
    RPN_int *i1=dynamic_cast<RPN_int*> (operand1);
    if (i1==0)
      throw new RPN_ex_not_int(operand1);
    printf("buy %d %d\n", i1->get(), i2->get());
    return 0;
  }
  virtual ~RPN_buy() {}
};


class RPN_sell : public RPN_function
{
public:
  RPN_sell() {}
  virtual RPN_element* evaluate_fun(RPN_item **stack) const
  {
    RPN_element *operand2=pop(stack);
    RPN_int *i2=dynamic_cast<RPN_int*> (operand2);
    if (i2==0)
      throw new RPN_ex_not_int(operand2);
    RPN_element *operand1=pop(stack);
    RPN_int *i1=dynamic_cast<RPN_int*> (operand1);
    if (i1==0)
      throw new RPN_ex_not_int(operand1);
    printf("sell %d %d\n", i1->get(), i2->get());
    return 0;
  }
  virtual ~RPN_sell() {}
};


class RPN_prod : public RPN_function
{
public:
  RPN_prod() {}
  virtual RPN_element* evaluate_fun(RPN_item **stack) const
  {
    RPN_element *operand1=pop(stack);
    RPN_int *i1=dynamic_cast<RPN_int*> (operand1);
    if (i1==0)
      throw new RPN_ex_not_int(operand1);
    printf("prod %d\n", i1->get());
    return 0;
  }
  virtual ~RPN_prod() {}
};


class RPN_build : public RPN_function
{
public:
  RPN_build() {}
  virtual RPN_element* evaluate_fun(RPN_item **stack) const
  {
    RPN_element *operand1=pop(stack);
    RPN_int *i1=dynamic_cast<RPN_int*> (operand1);
    if (i1==0)
      throw new RPN_ex_not_int(operand1);
    printf("build %d\n", i1->get());
    return 0;
  }
  virtual ~RPN_build() {}
};


class RPN_endturn : public RPN_function
{
public:
  RPN_endturn() {}
  virtual RPN_element* evaluate_fun(RPN_item **stack) const
  { 
    printf("endturn\n");
    return 0;
  }
  virtual ~RPN_endturn() {}
};


class RPN_embedded_fun : public RPN_function
{
  bool param;
  char *name;
public:
  RPN_embedded_fun(char *fun, bool info) 
  {
    param=info;
    int len=strlen(fun);
    name=new char[len+1];
    for (int i=0; i<len; i++)
      name[i]=fun[i];
    name[len]=0;
  }
  virtual RPN_element* evaluate_fun(RPN_item **stack) const
  { 
    if (param)
    {
      RPN_element *operand1=pop(stack);
      RPN_int *i1=dynamic_cast<RPN_int*> (operand1);
      if (i1==0)
        throw new RPN_ex_not_int(operand1);
    }
    printf("function\n");
    return 0;
  }
  virtual ~RPN_embedded_fun() {}
};


class RPN_ex
{
public:
  virtual ~RPN_ex() {};
  virtual void ex_processing() const = 0;
};


class RPN_ex_not_int : public RPN_ex
{
  RPN_element *value;
public:
  RPN_ex_not_int(RPN_element *elem) {value=elem;}
 // RPN_ex_not_int(const RPN_ex_not_int& other) {}
  virtual void ex_processing() const
  {
    RPN_label *lab=dynamic_cast<RPN_label*> (value);
    if (lab==0)
    {
      RPN_var_addr *var=dynamic_cast<RPN_var_addr*> (value);
      if (var==0)
      {
        RPN_string *str=dynamic_cast<RPN_string*> (value);
        if (str==0)
          printf("Smth strange in stack, when there should be number\n");
        else
          printf("String <%s> in stack, when there should be number\n",
                 str->get());
      }
      else
        printf("Variable <%s> in stack, when there should be number\n",
               var->get_name());
    }
    else
      printf("Label <%s> in stack, when there should be number\n",
             lab->get_name());
  }
  virtual ~RPN_ex_not_int() {}
};


class RPN_ex_not_label : public RPN_ex
{
  RPN_element *value;
public:
  RPN_ex_not_label(RPN_element *elem) {value=elem;}
  virtual void ex_processing() const
  {
    RPN_int *i=dynamic_cast<RPN_int*> (value);
    if (i==0)
    {
      RPN_var_addr *var=dynamic_cast<RPN_var_addr*> (value);
      if (var==0)
      {
        RPN_string *str=dynamic_cast<RPN_string*> (value);
        if (str==0)
          printf("Smth strange in stack, when there should be label\n");
        else
          printf("String <%s> in stack, when there should be label\n",
                 str->get());
      }
      else
        printf("Variable <%s> in stack, when there should be label\n",
               var->get_name());
    }
    else
      printf("Number <%d> in stack, when there should be label\n",
             i->get());
  }
  virtual ~RPN_ex_not_label() {}
};


class RPN_ex_not_var_not_int : public RPN_ex
{
  RPN_element *value;
public:
  RPN_ex_not_var_not_int(RPN_element *elem) {value=elem;}
  virtual void ex_processing() const
  {
    RPN_label *lab=dynamic_cast<RPN_label*> (value);
    if (lab==0)
    {
      RPN_string *str=dynamic_cast<RPN_string*> (value);
      if (str==0)
        printf("Smth strange in stack, when there should be variable or "
               "number\n");
      else
        printf("String <%s> in stack, when there should be variable or "
               "number\n", str->get());
    }
    else
      printf("Label <%s> in stack, when there should be variable or "
             "number\n", lab->get_name());
  }
  virtual ~RPN_ex_not_var_not_int() {}
};


class RPN_ex_not_string_not_int : public RPN_ex
{
  RPN_element *value;
public:
  RPN_ex_not_string_not_int(RPN_element *elem) {value=elem;}
  virtual void ex_processing() const
  {
    RPN_label *lab=dynamic_cast<RPN_label*> (value);
    if (lab==0)
    {
      RPN_var_addr *var=dynamic_cast<RPN_var_addr*> (value);
      if (var==0)
        printf("Smth strange in stack, when there should be string or "
               "number\n");
      else
        printf("variable <%s> in stack, when there should be string or "
               "number\n", var->get_name());
    }
    else
      printf("Label <%s> in stack, when there should be string or "
             "number\n", lab->get_name());
  }
  virtual ~RPN_ex_not_string_not_int() {}
};


class RPN_ex_unitialized_label : public RPN_ex
{
  RPN_label *value;
public:
  RPN_ex_unitialized_label(RPN_label *lab) {value=lab;}
  virtual void ex_processing() const
  {
    printf("Label <%s> is not initialized\n", value->get_name());
  }
  virtual ~RPN_ex_unitialized_label() {}
};


class RPN_ex_unitialized_variable : public RPN_ex
{
  RPN_var_addr *value;
public:
  RPN_ex_unitialized_variable(RPN_var_addr *var) {value=var;}
  virtual void ex_processing() const
  {
    printf("Variable <%s> is not initialized\n", value->get_name());
  }
  virtual ~RPN_ex_unitialized_variable() {}
};


class RPN_ex_zero_division : public RPN_ex
{
public:
  RPN_ex_zero_division() {}
  virtual void ex_processing() const
  {
    printf("Try to divide by zero\n");
  }
  virtual ~RPN_ex_zero_division() {}
};


class RPN_ex_double_lab_initialization : public RPN_ex
{
  char *value;
public:
  RPN_ex_double_lab_initialization(char *lab)
  {
    int len=strlen(lab);
    char *value=new char[len+1];
    for (int i=0; i<len; i++)
      value[i]=lab[i];
    value[len]=0;
  }
  virtual void ex_processing() const
  {
    printf("Label <%s> initialized twice\n", value);
  }
  virtual ~RPN_ex_double_lab_initialization() {delete[] value;}
};
  
//end of fourth part

//third part


struct Label_table
{
  char *name;
  RPN_item *addres;
  Label_table *next;
};


class Sintax_analizer
{
  Lexem current_lex;
  Lexem_list *list;
  int end_str_flag, change_str;
  void OPER();
  bool MASS();
  void COLON();
  void COMMA();
  void ELEM();
  void LIST();
  void GAME();
  void EXP();
  void E1();
  void E2();
  void E3();
  int next_lexem();
  void add_to_rpn(RPN_element *elem);
  int label_flag;
  Label_table *lab_ptr;
  void add_label_to_table(char *name);
  void initialize_label(RPN_item *adr);
  RPN_item *lab_search(char *name);
  RPN_item *rpn; 
  RPN_item *curr_rpn_ptr;
public:
  Sintax_analizer(Lexem_list *list);
  RPN_item* get_rpn() {return rpn->next;} //first elem is empty by default
  void purpose();
};


Label_table *tmp_lab=0; //delete glob var


RPN_item* Sintax_analizer :: lab_search(char *name)
{
  Label_table *tmp=lab_ptr;
  while (tmp!=0)
  {
    if (string_compare(tmp->name, name))
      return tmp->addres;
    tmp=tmp->next;
  }
  return 0;
}


void Sintax_analizer :: initialize_label(RPN_item *adr)
{
  lab_ptr->addres=adr;
  label_flag=0;
}


void Sintax_analizer :: add_label_to_table(char *name)
{
  label_flag=1;
  Label_table *tmp=lab_ptr;
  while (tmp!=0)
  {
    if (string_compare(name, tmp->name))
    {
      printf("double label intialization\n");
      throw new RPN_ex_double_lab_initialization(name);
    }
    tmp=tmp->next;
  }
  tmp=new Label_table;
  int len=strlen(name);
  tmp->name=new char[len+1];
  tmp->name[len]=0;
  for (int i=0; i<len; i++)
    tmp->name[i]=name[i];
  tmp->next=lab_ptr;
  lab_ptr=tmp;
}
    


void Sintax_analizer :: add_to_rpn(RPN_element *elem)
{
  curr_rpn_ptr->next=new RPN_item;
  curr_rpn_ptr=curr_rpn_ptr->next;
  if (label_flag)
    initialize_label(curr_rpn_ptr);
  curr_rpn_ptr->next=0;
  curr_rpn_ptr->elem=elem;
  rpn_number++;//delete global
  curr_rpn_ptr->num=rpn_number;//-||-
}


class Exception
{
  enum status {dependent=0, independent=1};
  char *lex;
  char *message;
  int string_num, status;
public:
  Exception(Lexem curr_lex, const char *mes); 
  Exception(Lexem curr_lex, const char *mes, int change_str);
  Exception(int num, const char *mes);
  Exception(const Exception& other); 
  char *get_str() const;
  int get_str_num() const;
  char *get_mes() const;
  int get_status() const;
  ~Exception();
}; 
 

Exception :: Exception(Lexem curr_lex, const char *mes)
{ 
  lex=new char[strlen(curr_lex.string)];
  strcpy(lex, curr_lex.string);
  string_num=curr_lex.string_num;
  message=new char[strlen(mes)];
  strcpy(message, mes);
  status=dependent;
}


Exception :: Exception(Lexem curr_lex, const char *mes, int change_str)
{ 
  lex=new char[strlen(curr_lex.string)];
  strcpy(lex, curr_lex.string);
  if (change_str)
    string_num=curr_lex.string_num-1;
  else
    string_num=curr_lex.string_num;
  message=new char[strlen(mes)];
  strcpy(message, mes);
  status=dependent;
}

/*
Exception :: Exception(char* string, int num, const char *mes)
{
  lex=new char[strlen(string)];
  strcpy(lex, string);
  string_num=num;
  message=new char[strlen(mes)];
  strcpy(message, mes);
  status=dependent;
}*/


Exception :: Exception(int num, const char *mes)
{
  string_num=num;
  lex=0;
  message=new char[strlen(mes)];
  strcpy(message, mes);
  status=independent;
}


Exception :: Exception(const Exception& other)
{
  if (other.lex!=0)
  {
    lex=new char[strlen(other.lex)];
    strcpy(lex, other.lex);
  }
  string_num=other.string_num;
  printf("!!%d\n",string_num);
  message=new char[strlen(other.message)];
  strcpy(message, other.message);
  status=other.status;
}


Exception :: ~Exception()
{
  if (message!=0)
    delete[] message;
  if (lex!=0)
    delete[] lex;
}
  

int Exception :: get_status() const
{
  return status;
}


char* Exception :: get_str() const
{
  if (status==dependent)
    return lex;
  return 0;
}
  

int Exception :: get_str_num() const
{
    return string_num;
}


char* Exception :: get_mes() const
{
  return message;
}


Sintax_analizer :: Sintax_analizer(Lexem_list *lexems)
{
  label_flag=0;
  lab_ptr=0;
  rpn=new RPN_item;
  rpn->next=0;
  rpn->elem=0;
  curr_rpn_ptr=rpn;
  end_str_flag=0;
  change_str=0;
  list=lexems;
}


int sign_search(char *string, char *table)
{
  if (strlen(string)!=1)
    return 0;
  int len=strlen(table);
  for (int i=0; i<len; i++)
    if (string[0]==table[i])
      return 1;
  return 0;
}
 

int Sintax_analizer :: next_lexem()
{
  if (list==0)
  { 
    if (end_str_flag==0)
  //  printf("%d\n",current_lex.string_num);
      throw Exception(current_lex.string_num,
                      "Next lexem expected, but the list is empty");
    return 0;
  }
  int num=current_lex.string_num;
  current_lex=list->lex;
  list=list->next;
  if (num==current_lex.string_num)
    change_str=0;
  else
    change_str=1;
  return 1;
}



void Sintax_analizer :: purpose()
{
  while (next_lexem())
  {
    end_str_flag=0; 
    OPER();
  }
  tmp_lab=lab_ptr; //delete
  curr_rpn_ptr=rpn;
  while (curr_rpn_ptr!=0)
  {
    RPN_label *lab=dynamic_cast<RPN_label*> (curr_rpn_ptr->elem);
    if (lab!=0)
    {
      if (lab->get()==0)
      {
        RPN_item *ptr=lab_search(lab->get_name());
        if (ptr==0)
          throw new RPN_ex_unitialized_label(lab);
        else
          lab->set_label(ptr);
      }
    }
    curr_rpn_ptr=curr_rpn_ptr->next;
  }
  printf("Everything is OK:)\n");
}


void Sintax_analizer :: OPER()
{ 
  char go[]="goto", print[]="print", if_lex[]="if", then_lex[]="then",
       assignment[]=":="; 
  if (current_lex.type==label_type)
  {
    add_label_to_table(current_lex.string);
    next_lexem();
    OPER();
  } else
  if (string_compare(current_lex.string, go))
  {
    next_lexem();
    if (current_lex.type==label_type)
    {
      RPN_label *lab=new RPN_label(current_lex.string);
      add_to_rpn(lab);
      RPN_go *op_go=new RPN_go;
      add_to_rpn(op_go);
//printf("%s\n", current_lex.string);
//printf("GOTO\n");
      next_lexem();
    }
    else
      throw Exception(current_lex, "Label expected");
    COLON();
  } else
  if (string_compare(current_lex.string, print))
  {
    next_lexem();
    LIST();
    COLON();
  } else
  if (string_compare(current_lex.string, if_lex))
  {
    next_lexem();
    EXP();
    RPN_label *lab=new RPN_label(rpn);
    add_to_rpn(lab);
    RPN_go_false *go_false=new RPN_go_false;
    add_to_rpn(go_false);
//printf("Sys label\n");
//printf("False GOTO\n");
    if (string_compare(current_lex.string, then_lex))
      next_lexem();
    else
      throw Exception(current_lex, "Then expected");
    OPER();
    //if (curr_rpn_ptr->next==0)
      //printf("error\n");
    lab->set_label(curr_rpn_ptr); //assignming curr_rpn to label  
    //if (lab==0)
      //printf("label not alive\n");
    //printf("lab recived adr num %d\n", lab->get()->num); 
  } else
  if (current_lex.type==variable_type)
  {
    char *name=current_lex.string;
//printf("%s\n", current_lex.string);
    next_lexem();
    bool index=MASS();
    RPN_var_addr *var=new RPN_var_addr(name, index);
    add_to_rpn(var);
    if (string_compare(current_lex.string, assignment))
      next_lexem();
    else
      throw Exception(current_lex, "Assignment expected");
    EXP();
    RPN_assign *assign=new RPN_assign;
    add_to_rpn(assign);
    COLON();
  } else
  if (current_lex.type==key_word_type)
  {
    GAME();
    COLON();
  }
  else
    throw Exception(current_lex, "Operand expected");
}


void Sintax_analizer :: COLON()
{    
  char semicolon[]=";";
  end_str_flag=1; 
  if (string_compare(current_lex.string, semicolon))
  {}//    next_lexem();
  else
  {
      throw Exception(current_lex, "Semicolon expected", change_str);
  }
}


void Sintax_analizer :: COMMA()
{    
  char comma[]=","; 
  if (string_compare(current_lex.string, comma))
    next_lexem();
  else
    throw Exception(current_lex, "Comma expected");
}


bool Sintax_analizer :: MASS()
{
  char bracket1[]="[", bracket2[]="]";
  if (string_compare(current_lex.string, bracket1))
  {
    next_lexem();
    EXP();
    if (string_compare(current_lex.string, bracket2))
      next_lexem();
    else 
      throw Exception(current_lex, "Close of square bracket expected");
    return true;
  }
  return false;
}

 
void Sintax_analizer :: GAME()
{
  char endturn[]="endturn", prod[]="prod", build[]="build", buy[]="buy",
       sell[]="sell";
  char *lex;
  if (string_compare(current_lex.string, endturn))
  {
//printf("Endturn\n");
    RPN_endturn *rpn=new RPN_endturn;
    add_to_rpn(rpn);
    next_lexem(); 
  } else
  if (string_compare(current_lex.string, prod) ||
      string_compare(current_lex.string, build))
  {
    lex=current_lex.string;
    next_lexem();
    EXP();
    if (string_compare(lex, prod))
    {
      RPN_prod *rpn=new RPN_prod;
      add_to_rpn(rpn);
    } 
    else
    {
      RPN_build *rpn=new RPN_build;
      add_to_rpn(rpn);
    }
//printf("Prod/build\n");
  } else 
  if (string_compare(current_lex.string, buy) ||
      string_compare(current_lex.string, sell))
  { 
    lex=current_lex.string;
    next_lexem();
    EXP();
    //COMMA();
    EXP();
    if (string_compare(lex, buy))
    {
      RPN_buy *rpn=new RPN_buy;
      add_to_rpn(rpn);
    } 
    else
    {
      RPN_sell *rpn=new RPN_sell;
      add_to_rpn(rpn);
    }
//printf("Buy/Sell\n");
  }
  else
    throw Exception(current_lex, "Action expected");
}


void Sintax_analizer :: LIST()
{
  char comma[]=",";
  ELEM();
  RPN_print *print=new RPN_print;
  add_to_rpn(print);
//printf("Print\n");
  if (string_compare(current_lex.string, comma))
  {
    next_lexem();
    LIST();
  } else
  if (current_lex.string[0]!=';')
    throw Exception(current_lex, "Comma expected");
}

void Sintax_analizer :: ELEM()
{
  if (current_lex.type==string_type)
  {
    RPN_string *string=new RPN_string(current_lex.string);
    add_to_rpn(string);
    next_lexem();
//printf("String\n");
  }
  else
    EXP();
}


void Sintax_analizer :: EXP()
{
  E1();
  char not_equal[]="!=", compare_sign[]="<>=";
  if (sign_search(current_lex.string, compare_sign))
  {
    char *lex=current_lex.string;
    next_lexem();
    E1();
    if (*lex=='<')
    {
      RPN_less *sign=new RPN_less;
      add_to_rpn(sign);
    }
    else if (*lex=='>')
    {
      RPN_more *sign=new RPN_more;
      add_to_rpn(sign);
    }
    else
    {
      RPN_equal *sign=new RPN_equal;
      add_to_rpn(sign);
    }
//printf("Compare\n");
  } else
  if (string_compare(current_lex.string, not_equal))
  {
    next_lexem();
    E1();
//printf("Compare\n");
  }
}


void Sintax_analizer :: E1()
{
  char low_priority[]="+-|";
  E2();
  while (sign_search(current_lex.string, low_priority))
  {
    char *lex=current_lex.string;
    next_lexem();
    E2();
    if (*lex=='+')
    {
      RPN_plus *sign=new RPN_plus;
      add_to_rpn(sign);
    }
    else if (*lex=='-')
    {
      RPN_minus *sign=new RPN_minus;
      add_to_rpn(sign);
    }
    else
    {
      RPN_or *sign=new RPN_or;
      add_to_rpn(sign);
    }
//printf("low\n");
  }
}


void Sintax_analizer :: E2()
{
  E3();
  char high_priority[]="*/&";
  while (sign_search(current_lex.string, high_priority))
  {
    char *lex=current_lex.string;
    next_lexem();
    E3();
    if (*lex=='*')
    {
      RPN_mult *sign=new RPN_mult;
      add_to_rpn(sign);
    }
    else if (*lex=='/')
    {
      RPN_div *sign=new RPN_div;
      add_to_rpn(sign);
    }
    else
    {
      RPN_and *sign=new RPN_and;
      add_to_rpn(sign);
    }
//printf("high\n");
  } 
}
  

void Sintax_analizer :: E3()
{
  if (current_lex.type==number_type)
  {
    RPN_int *i=new RPN_int(atoi(current_lex.string));
    add_to_rpn(i);
//printf("%s\n", current_lex.string);
    next_lexem();
  } else
  if (current_lex.type==variable_type)
  {
    char *lex=current_lex.string;
//printf("%s\n", current_lex.string);
    next_lexem();
    bool index=MASS();
    RPN_var_addr *var=new RPN_var_addr(lex, index);
    add_to_rpn(var);
  } else
  if (current_lex.type==function_type)
  {
//printf("%s\n", current_lex.string);
    char *name=current_lex.string;
    bool param=false;
    next_lexem();
    if (current_lex.string[0]=='(')
    {
      next_lexem();
      EXP();
      if (current_lex.string[0]==')')
        next_lexem();
      param=true;
    }
    RPN_embedded_fun *fun=new RPN_embedded_fun(name, param);
    add_to_rpn(fun);
  } else
  if (current_lex.string[0]=='(')
  {
    next_lexem();
    EXP();
    if (current_lex.string[0]==')')
      next_lexem();
    else
      throw Exception(current_lex, "Close of round bracket expected");
  } else
    throw Exception(current_lex,
                    "Expected number, variable or function in expression");
}


RPN_item* sintaxis_analizer(Lexem_list *list)
{
  int status=0;
  Sintax_analizer analize(list);
  try
  {
    analize.purpose();
    return analize.get_rpn();
  }
  catch (const Exception &ex)
  {
    if ((status=ex.get_status()))
      fprintf(stderr, "String number %d: %s\n", 
              ex.get_str_num(),
              ex.get_mes());
    else
      fprintf(stderr, "String number %d: %s, but <<%s>> met\n",
              ex.get_str_num(),
              ex.get_mes(),   
              ex.get_str());
  }
  return 0;
}    


void print_type(RPN_element *elem)
{
  RPN_int *i1=dynamic_cast<RPN_int*> (elem);
  if (i1!=0)
  {
    printf("number <%d>\n", i1->get());
    return;
  }
  RPN_label *i2=dynamic_cast<RPN_label*> (elem);
  if (i2!=0)
  {
    printf("label <%d>\n", i2->get()->num);
    return;
  }
  RPN_string *i3=dynamic_cast<RPN_string*> (elem);
  if (i3!=0)
  {
    printf("string <%s>\n", i3->get());
    return;
  }
  RPN_var_addr *i4=dynamic_cast<RPN_var_addr*> (elem);
  if (i4!=0)
  {
    printf("variable addr <%s>\n", i4->get_name());
    return;
  }
  RPN_print *i5=dynamic_cast<RPN_print*> (elem);
  if (i5!=0)
  {
    printf("print\n");
    return;
  }
  RPN_assign *i6=dynamic_cast<RPN_assign*> (elem);
  if (i6!=0)
  {
    printf("assign\n");
    return;
  }
  RPN_embedded_fun *i7=dynamic_cast<RPN_embedded_fun*> (elem);
  if (i7!=0)
  {
    printf("function\n");
    return;
  }
  RPN_buy *i8=dynamic_cast<RPN_buy*> (elem);
  if (i8!=0)
  {
    printf("buy\n");
    return;
  }
  RPN_sell *i9=dynamic_cast<RPN_sell*> (elem);
  if (i9!=0)
  {
    printf("sell\n");
    return;
  }
  RPN_prod *i10=dynamic_cast<RPN_prod*> (elem);
  if (i10!=0)
  {
    printf("prod\n");
    return;
  }
  RPN_build *i11=dynamic_cast<RPN_build*> (elem);
  if (i11!=0)
  {
    printf("build\n");
    return;
  }
  RPN_endturn *i12=dynamic_cast<RPN_endturn*> (elem);
  if (i12!=0)
  {
    printf("endturn\n");
    return;
  }
  RPN_plus *i13=dynamic_cast<RPN_plus*> (elem);
  if (i13!=0)
  {
    printf("plus\n");
    return;
  }
  RPN_minus *i14=dynamic_cast<RPN_minus*> (elem);
  if (i14!=0)
  {
    printf("minus\n");
    return;
  }
  RPN_go *i15=dynamic_cast<RPN_go*> (elem);
  if (i15!=0)
  {
    printf("operator go\n");
    return;
  }
  RPN_go_false *i16=dynamic_cast<RPN_go_false*> (elem);
  if (i16!=0)
  {
    printf("operator go false\n");
    return;
  }
  RPN_mult *i17=dynamic_cast<RPN_mult*> (elem);
  if (i17!=0)
  {
    printf("multiplication\n");
    return;
  }
  RPN_div *i18=dynamic_cast<RPN_div*> (elem);
  if (i18!=0)
  {
    printf("division\n");
    return;
  }
  RPN_or *i19=dynamic_cast<RPN_or*> (elem);
  if (i19!=0)
  {
    printf("or\n");
    return;
  }
  RPN_and *i20=dynamic_cast<RPN_and*> (elem);
  if (i20!=0)
  {
    printf("and\n");
    return;
  }
  RPN_more *i21=dynamic_cast<RPN_more*> (elem);
  if (i21!=0)
  {
    printf("more\n");
    return;
  }
  RPN_equal *i22=dynamic_cast<RPN_equal*> (elem);
  if (i22!=0)
  {
    printf("equal\n");
    return;
  }
  RPN_less *i23=dynamic_cast<RPN_less*> (elem);
  if (i23!=0)
  {
    printf("less\n");
    return;
  }
}
  

void RPN_interpretation(RPN_item *rpn)
{
  RPN_item *curr=rpn, *stack=0;
  while (curr!=0)
  {
    printf("m0\n");
    printf("%d. current=", curr->num);
    printf("m4\n");
    print_type(curr->elem);
    curr=curr->next;
    printf("m1\n");
  }
  putchar('\n');
  curr=rpn;
    printf("m2\n");
  while (curr!=0)
  {
    printf("m3\n");
    printf("current=");
    print_type(curr->elem);
    printf("rpn number=%d\n",curr->num);
    curr->elem->evaluate(&stack, &curr);
    //printf("rpn number after evaluate=%d\n",curr->num);
    if (stack!=0)
    {
      printf("stack top=");
      print_type(stack->elem);
    }
    Var_table *tmp=RPN_var_addr :: table;
    int i=0;
    printf("\nvariable table:\n");
    while(tmp!=0)
    {
      i++;
      printf("%d. %s %d\n", i, tmp->name, tmp->value);
      tmp=tmp->next;
    }
    Label_table *ptr=tmp_lab;
    i=0;
    printf("\nlabel table:\n");
    while(ptr!=0)
    {
      i++;
      printf("%d. %s %d\n", i, ptr->name, ptr->addres->num);
      ptr=ptr->next;
    }
  }
} 
    

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
  print_list(list);
  try
  {
    RPN_item *rpn=sintaxis_analizer(list);
    RPN_interpretation(rpn);
  } 
  catch (RPN_ex *ex)
  {
    ex->ex_processing();
    delete ex;
  }
  return 0;
}
