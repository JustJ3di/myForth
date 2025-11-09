#include <stdio.h>
#include<stdlib.h>
#include<string.h>


//#######################UTILITIES################################

typedef enum {false,true}bool;

//wrapper malloc and realloc
void *xmalloc(size_t size){
	void *p = malloc(size);
	if(p)
		return p;
	else {
		perror("Error out of memory");
		exit(EXIT_FAILURE);
	}
}

void *xrealloc(void **p,size_t size){
	void *n = realloc(*p,size);
	if(n)
		return n;
	else {
		perror("Error out of memory");
		exit(EXIT_FAILURE);
	}

}
//#######################OBJECT TYPE################################
//
const char *math_symbol[] = {"+", "-", "/", "*"};
const char *build_in[] = {"print","dup","swap","if","jmp"};
enum{SYMBOL, NUMBER, STRING, VAR};
typedef struct myobj myobj;
typedef struct myobj{

	unsigned  type;
	union {
	 	int i;
		struct {
			size_t len;
			char *ptr;
		}str;		
	};
	
}myobj;

//object fun
myobj *create_number_obj(int number){
	myobj *o = xmalloc(sizeof(*o));
	o->type = NUMBER;
	o->i = number;
	return o;
}

//used for string and symbol , len is strlen + 1 
myobj *create_string_obj(char *s){
	myobj *o = xmalloc(sizeof(*o));
	int l = strlen(s);
	o->type = STRING;
	o->str.ptr = xmalloc(l+1);
	o->str.ptr[l] = 0;
	o->str.len = 0;
	return 0;
}


//#######################STACK################################
typedef struct {
	size_t cap; //capacity aka the current allocated memory
	size_t top; //top of the stack
	myobj **ptr;
}stack;

//stack fun
#define EMPITY -1 //top is -1 if the stack is empity
#define INIT_SIZE 2
//create stack
stack *init(){
	stack *st = xmalloc(sizeof(*st));
	st->ptr = xmalloc(sizeof(myobj *)*INIT_SIZE);
	st->cap = INIT_SIZE;
	st->top = EMPITY;
	return st;
}

void delete(stack *st){
	free(st->ptr);
	free(st);

}

//stack operation push and pop
void push(stack *st, myobj *obj){
	if (st->top >= st->cap)
	{	
		st->cap *= 2;
		st = xrealloc(st,st->cap);
	}
	st->ptr[++st->top] = obj;
}

inline bool empty(stack *st){return st->top == -1 ? true : false;}

myobj *pop(stack *st){

	if(!empty(st)){
		return st->ptr[st->top--];
	}
	return NULL;

}


//parser struct and fun
typedef struct {

	char *prg; //pointer to program
	size_t size; //file size
	char *cp;//char tok

}parser;



//Return myobj pointer the freed up of the allocated object is in the main.
myobj *compile(parser *p){

	(void)p;

	return NULL;
}

int main(int argc, char const *argv[])
{
	if(argc < 2){
		printf("No file!\n");
	}else{
		//get file
		printf("File = %s", argv[1]);
		FILE *f = fopen(argv[1], "r");
		fseek(f, 0L, SEEK_END);
		int len = ftell(f); 
		fseek(f,0L, 0);
		parser *par = malloc(sizeof(*par));	
		par->prg = malloc(len+1);
		fread(par->prg,len,1,f);
		par->prg[len] = 0;
		par->size = len;
		par->cp = par->prg;
		//printf("%s\n",par->prg);
		fclose(f);
		//RUNTIME
		stack *st = init();

		//myobj *obj = compile(st,par);
		//compile
		//exec
		delete(st);
		
		free(par->prg);
		free(par);
		

	}
	return 0;
}
