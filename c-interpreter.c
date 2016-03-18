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

// instructions
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };
       
void next(){
  token = *src++;
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
	cycle = 0;
	while(1){
		cycle ++;
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
		} else if (GT) {
			ax = *sp++ > ax;
		} else if (GE) {
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
  if (!(src = old_src = malloc(poolsize))) {
    printf("could not malloc(%d) for source area\n", poolsize);
    return -1;
  }
  if ((i = read(fd, src, poolsize - 1)) <= 0) {
    printf("read() returned %d\n", i);
  }
  
  if (!(text = malloc(poolsize))) {
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
  src[i] = 0;
  close(fd);
  program();
  return eval();
} 
