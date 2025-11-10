#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

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

void *xrealloc(void *p,size_t size){
	void *n = realloc(p,size);
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
typedef enum{PRINT,DUP,SWAP,IF,JMP}tsymbol; 
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

//used for string, len is strlen + 1 
myobj *create_string_obj(char *s){
	myobj *o = xmalloc(sizeof(*o));
	int l = strlen(s);
	o->type = STRING;
	o->str.ptr = xmalloc(l+1);
	memcpy(o->str.ptr, s, l);
	o->str.ptr[l] = 0;
	o->str.len = 0;
	return o;
}

myobj *create_symbol_obj(tsymbol symbol){
	myobj *o = xmalloc(sizeof(*o));
	o->type = SYMBOL;
	o->i = symbol;
	return o;
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
	while(st->top != EMPITY){ 
		if(st->ptr[st->top]->type == STRING)
			free(st->ptr[st->top]->str.ptr);	
		free(st->ptr[st->top--]); 
	}
		
	free(st->ptr); 

	free(st);

}

//stack operation push and pop
void push(stack *st, myobj *obj){
	if (st->top + 1 >= st->cap)
	{	
		st->cap = st->cap * 2;
		st->ptr = xrealloc(st->ptr,sizeof(myobj *) * st->cap);
	}
	st->ptr[++st->top] = obj;
}


myobj *pop(stack *st){

	if(st->top != EMPITY ){
		return st->ptr[st->top--];
	}
	return NULL;

}


//#######################PARSER################################
typedef struct {

	char *prg; //pointer to program
	size_t size; //file size
	size_t line;
	char *cp;//char tok


}parser;

//argument file name , return if the file exist, return the parser object else print error and return NULL
parser *init_parser(const char *filename){
	FILE *f = fopen(filename, "r");
	if (!f)
	{
		perror("Error file not found\n");
		return NULL;
	}
	
	fseek(f, 0L, SEEK_END);
	int len = ftell(f); 
	fseek(f,0L, 0);
	parser *par = xmalloc(sizeof(*par));	
	par->prg = xmalloc(len+1);
	fread(par->prg,len,1,f);
	fclose(f);
	par->prg[len] = 0;
	par->size = len;
	par->cp = par->prg;
	
	return par;
}

int parse_number(parser *p){
	char *startp = p->cp;
	char* endp = NULL;  // This will point to end of string.
	while(isdigit(p->cp[0]))p->cp++;

	endp = p->cp;

	size_t len = endp-startp;
	char *number = malloc(len+1);
	memcpy(number,startp,len);

	number[len] = 0;

	int i = atol(number);
	free(number);
	return i;
		

}


//this funcion retun the symbol, symbol_type, it is a index of the array of the build in
tsymbol parse_symbol(parser *p){

	char *startp = p->cp;
	char* endp = NULL;  // This will point to end of string.
	while(isalpha(p->cp[0]))p->cp++;

	endp = p->cp;

	size_t len = endp-startp;
	char *symbol = malloc(len+1);

	
	memcpy(symbol,startp,len);
	symbol[len] = 0;
	printf("\n\n symbol = %s\n\n",symbol);
	//search symbol
	tsymbol s = -1;
	int build_in_count = 5; 
    
	//check symbol
    for(int i = 0; i < build_in_count; i++){
        

        if (strcmp(build_in[i], symbol) == 0){
            s = i;
            break; 
        }
    }

    if (s == -1) {
        printf("Error: Symbol '%s' not recognized at line '%ld'\n", symbol, p->line);
    }
    
    free(symbol); // free symbol
    
    return s;

}

char *parse_string(parser *p) {

	p->cp++; 
    char *startp = p->cp; 
    
    while(p->cp[0] != 0 && p->cp[0] != '\"') {
        if(p->cp[0] == '\n') p->line++; // newline
        p->cp++;
    }

    if(p->cp[0] == 0){
        printf("Error: String not close started at line : %ld\n", p->line);
        return NULL;
    }
    
    char *endp = p->cp;
    
    size_t len = endp - startp; 

    p->cp++; 
    

    char *string = malloc(len + 1);
    if (!string) {
         perror("Error allocating string");
         return NULL;
    }
    memcpy(string, startp, len);
    string[len] = 0;
    
    return string;
}

//Return stack pointer the freed up of the allocated object is in the main.
stack *compile(parser *p){
	stack *st = init();
	printf("----file content = [%s] ----file size = %ld\n",p->prg,p->size);
	printf("Start parsing\n");
	while (p->cp[0] != 0) 
	{	
		if(p->cp[0]  == '\n')p->line++; //special case
		//skipSpace
		if(isspace(p->cp[0]))p->cp++;
		//parse number
		if(isdigit(p->cp[0]) || (p->cp[1] !=  0 && p->cp[0]=='-' && isdigit(p->cp[1]))) {
			
			int number = parse_number(p);
			myobj *o =  create_number_obj(number);
			push(st,o);
			//puts("PARSE NUMBER\n");

		}else if(isalpha(p->cp[0])) {
			tsymbol symbol = parse_symbol(p);
			if(symbol == -1){ //error
				return st;
			}
			myobj *o = create_symbol_obj(symbol);
			push(st,o);
			
		}else if(p->cp[0] == '\"') {
			char *string = parse_string(p);
			if(string == NULL) {
				return st;
			}
			myobj *o = create_string_obj(string);
			push(st,o);
			free(string);
		}
			
	}
	
	return st;
}



int main(int argc, char const *argv[])
{
	if(argc < 2){
		printf("No file!\n");
	}else{
		//get file
		parser *par = init_parser(argv[1]);
		if(!par)//check error
			return 0;
		//RUNTIME
		stack *st;
		st = compile(par);
		if(!st){//some error occurr.
			return 0;
		}

		//compile
		//exec

		delete(st);

		free(par->prg);

		free(par);

	}
	
	return 0;
}
