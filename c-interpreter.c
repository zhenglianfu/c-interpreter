#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

/* variables */
int token;
char *src, *old_src;
int poolsize;
int line;
int *text, *stack;
char *data;
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
