#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//#######################UTILITIES################################


//wrapper malloc and realloc
void *xmalloc(size_t size){
	void *p = malloc(size);
	if(p) {
		memset(p,0,size);
		return p;
	}
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
#define SYMBOLS 20 //number of symbol
typedef enum {ADD, SUB ,DIV ,MUL, MOD, ABS,PRINT, DUP, SWAP, CR, OVER, ROT, DROP, EMIT , DOT , IF, ELSE, THEN, LT, GT, MIN, MAX} tsymbol; 
const char *build_in[] = {"+", "-", "/", "*", "mod", "abs","print", "dup", "swap", "cr", "over", "rot", "drop", "emit", ".", "if", "else", "then", "<", ">", "min", "max"};
enum{SYMBOL, NUMBER, STRING, VAR};

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
	o->str.len = l;
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

	//search symbol
	tsymbol s = -1;
	int build_in_count = SYMBOLS; 
    
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

//parse string
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
		while(isspace(p->cp[0]))p->cp++;
		//parse number
		if(isdigit(p->cp[0]) || (p->cp[1] !=  0 && p->cp[0]=='-' && isdigit(p->cp[1]))) {
			
			int number = parse_number(p);
			myobj *o =  create_number_obj(number);
			push(st,o);
			//puts("PARSE NUMBER\n");

		}
		if(isalpha(p->cp[0])) {
			
			tsymbol symbol = parse_symbol(p);
			if(symbol == -1){ //error
				return st;
			}
			
			myobj *o = create_symbol_obj(symbol);
			push(st, o);
			
		}
		if(p->cp[0] == '.'){
			
			myobj *o = create_symbol_obj(DOT);
			push(st, o);

		}
		if(p->cp[0] == '+'){

			myobj *o = create_symbol_obj(ADD);
			push(st, o);

		}
		if(p->cp[0] == '-'){
			myobj *o = create_symbol_obj(SUB);
			push(st, o);
		}
		if(p->cp[0] == '*'){
			myobj *o = create_symbol_obj(MUL);
			push(st, o);
		}
		if(p->cp[0] == '/'){
			myobj *o = create_symbol_obj(DIV);
			push(st, o);
		}
		if(p->cp[0] == '<'){
			myobj *o = create_symbol_obj(LT);
			push(st, o);
		}
		if(p->cp[0] == '>'){
			myobj *o = create_symbol_obj(GT);
			push(st, o);
		}
		if(p->cp[0] == '\"') {
			
			char *string = parse_string(p);
			
			if(string == NULL) {
				return st;
			}
			
			myobj *o = create_string_obj(string);
			push(st,o);
			free(string);

		}
		p->cp++;
	}
	
	return st;
}



//#################################EXEC################################################

//utilities print stack 
void print_stack(stack *st) { 
	for (size_t i = 0; i <=  st->top ; i++) //for each element in the stack
	{
		printf("at position %ld I ", i);
		switch (st->ptr[i]->type)
		{
		case STRING:
			printf("found string = %s\n", st->ptr[i]->str.ptr);
			break;
		case NUMBER:
			printf("found number = %d\n",st->ptr[i]->i);
			break;
		case SYMBOL:
			printf("found symbol = %s\n",build_in[st->ptr[i]->i]);
			break;
		default:
			break;
		}
	}
}



void exec(stack *st){

	//print_stack(st);// call utilities
	int *data_stack = xmalloc(sizeof(*data_stack) *(st->top+1)); //stack of integer to perform operation.
	memset(data_stack,0,st->top);
	int sp = -1;
	int if_count = 0;
	for (size_t i = 0; i <= st->top ; i++)
	{
		switch (st->ptr[i]->type)
		{
		case SYMBOL:
			switch (st->ptr[i]->i)
			{
			case ADD:
				{
					int n = data_stack[sp--] + data_stack[sp--];
					data_stack[++sp] = n;
				}
				break;
			case SUB:
				{
					int n1 = data_stack[sp--];
					int n2 = data_stack[sp--] - n1;
					data_stack[++sp] = n2;	
				}
				break;
			case DIV:
				{
					int n1 = data_stack[sp--];
					if(n1 == 0){
						printf("Error: divison by zero\n");
						return;
					}
					int n2 = data_stack[sp--] / n1;
					data_stack[++sp] = n2;
				}
				break;
			case MUL:
				{
					int a = data_stack[sp--] * data_stack[sp--];
					data_stack[++sp] = a;
				}
				break;
			case MOD:
				int m = data_stack[sp--];
					if(m == 0){
						printf("Error: divison by zero\n");
						return;
					}
				int mod = data_stack[sp--]%m;
				data_stack[++sp] = mod;
				break;
			case ABS:
				{
					int a = data_stack[sp--];
					data_stack[++sp] =  (a > 0 ? a : -1* a);
				}
				break;
			case DOT:
				printf("%d",data_stack[sp--]);
				break;
			case SWAP:
				{
				int a = data_stack[sp--],b = data_stack[sp--];
				data_stack[++sp] = b;
				data_stack[++sp] = a;
				}
				break;
			case DUP:
				{
					int a = data_stack[sp--];
					data_stack[++sp] = a;
					data_stack[++sp] = a;
				}
				break;
			case OVER:
				{
					int a = data_stack[sp--],b = data_stack[sp--];
					data_stack[++sp] = a;data_stack[++sp] = b;data_stack[++sp] = a;
				}
				break;
			case EMIT:
				{
					char ch = data_stack[sp--];
					printf("%c",ch);
				}
				break;
			case CR:
				putchar('\n');
				break;
			case DROP:
				sp--;
				break;
			            case LT: // < : n1 n2 -- flag (True if n1 < n2)
                { 
                    if (sp < 1) { printf("Error: Stack underflow for <\n"); return; }
                    int n2 = data_stack[sp--]; // TOS
                    int n1 = data_stack[sp--]; // NOS
                    data_stack[++sp] = n1 < n2 ? 1 : 0; // 1 for True, 0 for False
                }
                break;
            case GT: // > : n1 n2 -- flag (True if n1 > n2)
                { 
                    if (sp < 1) { printf("Error: Stack underflow for >\n"); return; }
                    int n2 = data_stack[sp--]; // TOS
                    int n1 = data_stack[sp--]; // NOS
                    data_stack[++sp] = n1 > n2 ? 1 : 0; // 1 for True, 0 for False
                }
                break;
 			case IF: {
                if (sp < 0) { printf("Error: Stack underflow before IF\n"); return; }
                int cond = data_stack[sp--];
                if (cond == 0) {
                    // jump to else or then
                    int depth = 1;
                    while (++i <= st->top && depth > 0) {
                        if (st->ptr[i]->type == SYMBOL) {
                            if (st->ptr[i]->i == IF) depth++;
                            else if (st->ptr[i]->i == ELSE && depth == 1) break;
                            else if (st->ptr[i]->i == THEN && depth == 1) { depth = 0; break; }
                            else if (st->ptr[i]->i == THEN) depth--;
                        }
                    }
                }
            } break;

            case ELSE: {
                // jump on then
                int depth = 1;
                while (++i <= st->top && depth > 0) {
                    if (st->ptr[i]->type == SYMBOL && st->ptr[i]->i == THEN && depth == 1)
                        break;
                    else if (st->ptr[i]->type == SYMBOL && st->ptr[i]->i == IF)
                        depth++;
                    else if (st->ptr[i]->type == SYMBOL && st->ptr[i]->i == THEN)
                        depth--;
                }
            } break;

            case THEN:
                //do nothing
                break;
			case MIN:
				if(sp<0){printf("Stack underflow on min");}
				{
					int top = data_stack[sp--],tos = data_stack[sp--];
					data_stack[++sp] = (top < tos ? top : tos);
				}
				break;
			case MAX:
				if(sp<0){printf("Stack underflow on max");}
				{
					int top = data_stack[sp--],tos = data_stack[sp--];
					data_stack[++sp] = (top > tos ? top : tos);
				}
				break;
			case PRINT:
				{	
					if(st->ptr[i-1]->type == STRING){
						puts(st->ptr[i-1]->str.ptr);
					}
					else{
						printf("ERROR No string to print\n");
					}
				}
				break;
			default:
				break;
			}
			break;
		case NUMBER:
			data_stack[++sp] = st->ptr[i]->i ;
			break;
		default:

			break;
		} 
		
	}
		
	free(data_stack);
	
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
			free(par->prg);
			free(par);
			return 0;
		}

		print_stack(st);

		exec(st);

		delete(st);

		free(par->prg);

		free(par);

	}
	
	return 0;
}
