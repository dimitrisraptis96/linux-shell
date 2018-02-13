#include "../include/myshell.h"

#define LINE_SIZE 512
#define ARG_SIZE  64

int main(int argc, char *argv[]){

  switch (argc){
    case 0:
    case 1:
      interactive();
      break;

    case 2:
      batch(argv[1]);
      break;

    default:
      // invalid arguments are given
      printf("[ERROR]: invalid arguments inserted\n");
      printf("\t\tinteractive mode:\t./bin/%s\n"          ,argv[0]);
      printf("\t\tbatch mode:      \t./bin/%s\n batchfile",argv[0]);
      exit(EXIT_FAILURE);
  }

  return(EXIT_SUCCESS);
}


void interactive(){
  int i;

  int commandNum; // number of commands per line

  char input[LINE_SIZE];      // origin line char array
  char *args[LINE_SIZE][ARG_SIZE];  // pointers to commands and argumentss
  char **buffer;        // temporary buffer for full commands pointers

  while(1){
    printf("raptis_8467> ");

    // read line
    if (fgets(input, LINE_SIZE, stdin) == NULL) exit(EXIT_FAILURE);
    printf("\n");

    char *line = remove_whitespace(input);
    
    if (strlen(line) == 0) continue; // empty line

    // split commands 
    commandNum = find_commandNum(line);
    HANDLE_NULL( (buffer = (char **) malloc(commandNum*sizeof(char*))) );
    split(";",line,buffer);

    // split arguments of each command
    for (i=0; i<commandNum; i++){
      args[i][0] = remove_whitespace(buffer[i]);
      split(" \t\r", args[i][0], &args[i][0]);
    }

    execute(commandNum,args);

    // delete previous line input
    memset(input, 0, LINE_SIZE);


    free(buffer);
    }
  }


void batch(char *filename){
  int i;

  int commandNum; // number of commands per line
   
  char input[LINE_SIZE];      // origin line char array
  char *args[LINE_SIZE][ARG_SIZE];  // pointers to commands and argumentss
  char **buffer;        // temporary buffer for full commands pointers

  // open batch file
  FILE *fp = fopen(filename,"r"); 
  HANDLE_NULL( fp );

  while(1){ 
    // get line
    if (fgets(input, LINE_SIZE, fp) == NULL) break;
    
    printf("\n");

    char *line = remove_whitespace(input);
    
    if (strlen(line) == 0) continue; // empty line

    // split commands
    commandNum = find_commandNum(line); // find number of commands
    HANDLE_NULL( (buffer = (char **) malloc(commandNum*sizeof(char*))) );
    commandNum = split(";",line,buffer);

    // split arguments
    for (i=0;i<commandNum;i++){
      args[i][0] = remove_whitespace(buffer[i]);
      // check if there is && seperator
      
      split(" ", args[i][0], &args[i][0]);
    }

    // execute commands
    execute(commandNum,args);

    // delete previous line input
    memset(input,0,LINE_SIZE);

    // free current buffer
    free(buffer);
  }
  // close the file
  HANDLE_EOF( fclose(fp) );
}

// remove leading and ending whitespace
char *remove_whitespace (char *line){

  int size = strlen(line);
  // remove ending whitespace
  while (line[size-1] == '\n' || line[size-1] == '\t' || line[size-1] == ' '){
    size--;
    line[size] = '\0';
  }

  // remove leading whitespace
  while (*line == ' ' || *line == '\t' || *line == '\n'){
    line++;
    if(*line == '\0') break;
  }
  return line;
}

int split(const char *delim, char *src, char **dst){
  char *token;
  int count=0;

  token = strtok(src, delim);
  while( token != NULL ) {
    // printf("%s+",token);
    dst[count] = token;
    count++;
    token = strtok(NULL, delim);
  }
  // printf("\n\n");
  return count;
}

int find_commandNum(char *line){
  int i;
  int counter=0;
  for(i=0;i<strlen(line);i++){
    if (line[i]==';')
      counter++;
  }
  if(line[strlen(line)-1] != ';') counter++;

  return counter;
}


static int *quit; // chech if quit is entered
static int *flag; // help commands' contiguous execution 

void execute(int size,char *command[LINE_SIZE][ARG_SIZE]){

  pid_t pid[size];  //array of all pids
  int status[size]; //aray of status
  int parent =0;    //parent flag
  int i;

  flag = (int *) malloc(size*sizeof(int));

  // use shared memory for global variables
  quit = mmap(NULL, sizeof (*quit),   PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  flag = mmap(NULL, size*sizeof(int), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  
  // init global variables
  *quit=0;
  for (i=0;i<size;i++) flag[i]=0;

  // command exeution loop
  for(i=0;i<size;i++){ 

    pid[i]=fork(); // fork for each command

    if (pid[i] > 0){        //PARENT process
      parent = 1;
    }
    else if (pid[i] == 0){  // CHILD processes 
      child(command,i);
    }
    else {                    // ERROR during fork
      printf("[ERROR]: Fork failed\n");
      exit(EXIT_FAILURE);
    }
  }

  // only parent here!!!
  if (parent){

    for(int i=0;i<size;i++){  
      // wait the commands execute sequently
      flag[i]=1;
      while ( ( waitpid(pid[i],&status[i],0) ) > -1 );

      printf("\n");
    }

    // check if quit is entered
    if(*quit) {
      printf("[FINAL]: 'quit' entered, exiting shell..\n");
      exit(EXIT_SUCCESS);
    }
    // free shared memory
    munmap(quit, sizeof(int *));
    munmap(flag, sizeof(int *));
  }
}

void child(char *command[LINE_SIZE][ARG_SIZE], int id){

  // wait until previous command execution
  while(!flag[id]){
    ;
  }

  // empty command
  if( strcmp(command[id][0],"") == 0){
    // printf("empty command\n");
    exit(EXIT_FAILURE);
  }
  // quit is entered
  if( strcmp(command[id][0],"quit")==0 ){
    // printf("quit entered\n");
    *quit=1;
    exit(EXIT_FAILURE);
  }

  // execute command
  if(execvp(command[id][0], command[id]) == -1){
    // invalid command   
    // printf("Unknown command: %s\n",command[id][0]);
    exit(EXIT_FAILURE);
  }
  else {
    // correct commmand execution
    // printf("Command executed: %s\n",command[id][0]);

    exit(EXIT_SUCCESS);
  }    
} 