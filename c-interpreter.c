#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

/* variables */
int token;
char *src, *old_src;
int poolsize;
int line;
// 指令和栈 
int *text, *old_text, *stack;
// 字符串存放区域 
char *data;

int *pc, // 指向下一条指令 
 	*bp, // 保存一个指向栈的地址 
 	*sp, // 永远指向栈顶 
	ax,  // 通用寄存器 
	cycle; 

int token_val;
int *current_id, *symbols;
int *idmain; // main function

// instructions
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };
// 支持的标记 
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

// fields of identifier
enum {
	Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize
}; 
// typeof variable/function
enum{
	CHAR, INT, PTR
};

void next(){
  char *last_pos;
  int hash;
  while(token = *src){
  	++src;
  	// parse token here
  	if (token == '\n') {
  		++line;
	} else if (token == '#') {
		// 跳过宏和引用
		while (*src != 0 && *src != '\n') {
			src ++;
		} 
	} else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || token == '_') {
		last_pos = src - 1;
		hash = token;
		while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || *src == '_') {
			hash = 147 * hash + *src;
			src ++;
		}
		// looking for existing identifier, linear search
		current_id = symbols;
		while (current_id[token]) {
			if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
				token = current_id[Token];
				return;
			}
			current_id = current_id + IdSize;
		}		
		// store new ID
		current_id[Name] = (int)last_pos;
		current_id[Hash] = hash;
		token = current_id[Token] = Id;
		return;
	} else if (token >= '0' && token <= '9') {
		token_val = token - '0';
		if (token_val > 0) {
			while (*src >= '0' && *src <= '9') {
				token_val = token_val * 10 + (*src - '0');
				src ++;
			}
		} else {
			if (*src == 'x' || *src == 'X') {
				token = *++src;
				while((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')) {
					token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
					token = *++src;
				}
			} else {
				while(*src >= '0' && *src <= '7'){
					token_val = token_val * 8 + (*src++ - '0');
				}
			}
		}
		token = Num;
		return;
	} else if (token == '"' || token == '\'') {
		last_pos = data;
		while(*src != 0 && *src != token){
			token_val = *src++;
			if (token_val == '\\') {
				token_val = *src++;
				if (token_val == 'n') {
					token_val = '\n';
				}
			}
			if (token == '"') {
				// 如果是字符串保存每个字符，单字符忽略 
				*data++ = token_val;
			}
		}
		if (token == '"') {
			token_val = (int)last_pos;
		} else {
			// char 
			token = Num;
		}
		return;
	} else if (token == '/') {
		// 只支持单行注释，不支持/**/ 
		if (*src == '/') {
			while (*src != 0 && *src != '\n') {
				src ++;
			}
		} else {
			token = Div;
			return;
		}
	} else if (token == '=') {
		if (*src == '=') {
			src ++;
			token = Eq;
		} else {
			token = Assign;
		}
		return;
	} else if (token == '+') {
		if (*src == '+') {
			src ++;
			token = Inc;
		} else {
			token = Add;
		}
		return;
	} else if (token == '-') {
		if (*src == '-') {
			src ++;
			token = Dec;
		} else {
			token = Sub;
		}
		return;
	} else if (token == '!') {
		if (*src == '=') {
			src ++;
			token = Ne;
		}
		return;
	} else if (token == '<') {
		if (*src == '=') {
			src ++;
			token = Le;
		} else if (*src == '<') {
			src ++;
			token = Shl;
		} else {
			token = Lt;
		}
		return;
	} else if (token == '>') {
		if (*src == '=') {
			src ++;
			token = Ge;
		} else if (token == '>') {
			src ++;
			token = Shr;
		} else {
			token = Gt;
		}
		return;
	} else if (token == '|') {
		if (*src == '|') {
			src ++;
			token = Lor;
		} else {
			token = Or;
		}
		return;
	} else if (token == '&') {
		if (*src == '&') {
			src ++;
			token = Lan;
		} else {
			token = And;
		}
		return;
	} else if (token == '^') {
		token = Xor;
		return;
	} else if (token == '*') {
		token = Mul;
		return;
	} else if (token == '[') {
		token = Brak;
		return;
	} else if (token == '?') {
		token = Cond;
		return;
	} else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ';' || token == ':') {
		return;
	}
  }	
  return;
}

void expression(int level){
  // do somthing later
}

void program(){
  next();
  while(token > 0){                                                                                     
    printf("token is: %c\n", token);
    next();
  }
}

int eval(){
	int op, *tmp;
	while(1){
		op = *pc++;
		if (op == IMM) {
			ax = *pc++;
		} else if (op == LC) {
			ax = *(char *)ax;
		} else if (op == LI) {
			ax = *(int *)ax;
		} else if (op == SC) {
			ax = *(char *)*sp++ = ax;
		} else if (op == SI) {
			*(int *)*sp++ = ax;
		} else if (op == PUSH) {
			*--sp = ax;
		} else if (op == JMP) {
			// 强制转为指针并指向该位置 
			pc = (int *)*pc; 
		} else if (op == JZ) {
			// 判断语句， ax为条件值 
			pc = ax ? pc + 1 : (int *)*pc;
		} else if (op == JNZ) {
			pc = ax ? (int *)*pc : pc + 1;
		} else if (op == CALL) {
			*--sp = (int)(pc + 1); // 保存位置信息到栈 记住函数返回后的下一条语句位置 
			pc = (int *)*pc; // JMP 
		} else if (op == ENT) {
			*--sp = (int)bp;
			bp = sp;
			sp = sp - *pc++;
		} else if (op == ADJ) {
			sp = sp + *pc++;
		} else if (op == LEV) {
			sp = bp;
			bp = (int *)*sp++;
			pc = (int *)*sp++;
		} else if (op == LEA) {
			ax = (int)(bp + *pc++);
		} else if (op == OR) {
			ax = *sp++ | ax;
		} else if (op == XOR) {
			ax = *sp++ ^ ax;
		} else if (op == AND) {
			ax = *sp++ & ax;
		} else if (op == EQ) {
			ax = *sp++ == ax;
		} else if (op == NE) {
			ax = *sp++ != ax;
		} else if (op == LT) {
			ax = *sp++ < ax;
		} else if (op == LE) {
			ax = *sp++ <= ax;
		} else if (op == GT) {
			ax = *sp++ > ax;
		} else if (op == GE) {
			ax = *sp++ >= ax;
		} else if (op == SHL) {
			ax = *sp++ << ax;
		} else if (op == SHR) {
			ax = *sp++ >> ax;
		} else if (op == ADD) {
			ax = *sp++ + ax;
		} else if (op == SUB) {
			ax = *sp++ - ax;
		} else if (op == MUL) {
			ax = *sp++ * ax;
		} else if (op == DIV) {
			ax = *sp++ / ax;
		} else if (op == MOD) {
			ax = *sp++ % ax;
		} else if (op == EXIT) {
			printf("exitd(%d)", *sp);
			return *sp;
		} else if (op == OPEN) {
			ax = open((char *)sp[1], sp[0]); // 参数倒序进入列表 
		} else if (op == CLOS) {
			ax = close(*sp);
		} else if (op == READ) {
			ax = read(sp[2], (char *)sp[1], *sp);
		} else if (op == PRTF) {
			tmp = sp + pc[1];	
			ax = printf((char *)(tmp[-1], tmp[-2], tmp[-3],tmp[-4],tmp[-6])); 
		} else if (op == MALC) {
			ax = (int)(malloc(*sp));
		} else if (op == MSET) {
			ax = (int)memset((char *)sp[2], (char *)sp[1], *sp);
		} else if (op == MCMP) {
			ax= memcmp((char *)sp[2], (char *)sp[1], *sp);
		} else {
			printf("unknown instruction: %d\n", op);
			return -1;
		}
	} 
	return 0;
}

int main(int argc, char **argv){
  int i, fd;
  argc--;
  argv++;
  poolsize = 256 * 1024;
  line = 1;
  if ((fd = open(*argv, 0)) < 0) {
    printf("cloud not open(%s)\n", *argv);
    return -1;
  }
  
  // allocate memory for virtual machine
  if (!(text = old_text = malloc(poolsize))) {
  	printf("could not malloc(%d) for text\n", poolsize);
  	return -1;
  } 
  if (!(data = malloc(poolsize))) {
  	printf("could not malloc(%d) for data\n", poolsize);
  	return -1;
  }
  if (!(stack = malloc(poolsize))) {
  	printf("could not malloc(%d) for stack\n", poolsize);
  	return -1;
  }
  if (!(symbols = malloc(poolsize))) {
  	printf("could not malloc(%) for symbols\n", poolsize);
  	return -1;
  }
  
  memset(text, 0, poolsize);
  memset(data, 0, poolsize);
  memset(stack, 0, poolsize);
  bp = sp = (int *)((int)stack + poolsize);
  ax = 0;
  // keywords / buildin functions 
  src = "char else enum if int return sizeof while"
  		"open read close printf malloc memset memcmp exit void main";
  i = Char;
  while(i <= While){
  	next();
  	current_id[Token] = i++;
  }	
  i = OPEN;
  while(i <= EXIT){
  	next();
  	current_id[Class] = Sys;
  	current_id[Type] = INT;
  	current_id[Value] = i++;
  }
  
  next();
  current_id[Token] = Char; // void type
  next();
  idmain = current_id; // main
  
  if (!(src = old_src = malloc(poolsize))) {
    printf("could not malloc(%d) for source area\n", poolsize);
    return -1;
  }
  if ((i = read(fd, src, poolsize - 1)) <= 0) {
    printf("read() returned %d\n", i);
  }
  src[i] = 0;
  close(fd);	
  
  pc = text; // no code in text
  
  program();
  return eval();
} 
