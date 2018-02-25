#include "../include/myshell.h"

#define LINE_SIZE 512
#define ARG_SIZE  64

struct _Command{
  char *args[ARG_SIZE];

  int isAmpersand, isPipe;
};



//TODO: add flag for the situation that commadnd is left of Ampersand

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
  char input[LINE_SIZE];     

  Command commands[LINE_SIZE];

  while(1){
    printf("raptis_8467> ");

    // read line
    if (fgets(input, LINE_SIZE, stdin) == NULL) exit(EXIT_FAILURE);
    printf("\n");

    char *line = remove_whitespace(input);
    
    if (strlen(line) == 0) continue; // empty line

    // split commands 
    commandNum = split_commands("&&;",line,commands);

    // split arguments of each command
    for (i=0; i<commandNum; i++){

      // printf("comm: %s\n",commands[i].args[0]);
      // printf("isAmp: %d\n",commands[i].isAmpersand);
      commands[i].args[0] = remove_whitespace(commands[i].args[0]);
      split_args(" \t\r\n\a", commands, i);
    }

    execute(commandNum,commands);

    // delete previous line input
    memset(input, 0, LINE_SIZE);

    free_commands(commands);
    }
  }

  void free_commands(Command *commands){
    int i,j;

    for (i=0;i<LINE_SIZE;i++){
      commands[i].isAmpersand = 0;
      commands[i].isPipe =0;
      for (j=0; j<ARG_SIZE; j++)
        commands[i].args[j] = NULL;      
    }
  }


void batch(char *filename){
  int i;

  int commandNum; // number of commands per line
   
  char input[LINE_SIZE];      // origin line char array
  char *args[LINE_SIZE][ARG_SIZE];  // pointers to commands and argumentss
  char **buffer;        // temporary buffer for full commands pointers
  char *commands;

  // open batch file
  FILE *fp = fopen(filename,"r"); 
  HANDLE_NULL( fp );

  while(1){ 
    // get line
    if (fgets(input, LINE_SIZE, fp) == NULL) break;
    
    char *line = remove_whitespace(input);
    
    if (strlen(line) == 0) continue; // empty line
    if (line[0] = '#') continue;   // comment line
    
    printf("\n");

    // split commands
    commandNum = find_commandNum(line); // find number of commands
    HANDLE_NULL( (buffer = (char **) malloc(commandNum*sizeof(char*))) );
    // commandNum = split(";",line,buffer);
    
    // split arguments
    for (i=0;i<commandNum;i++){
      args[i][0] = remove_whitespace(buffer[i]);
      // check if there is && seperator
      
      // split(" ", args[i][0], &args[i][0]);
    }

    // execute commands
    // execute(commandNum,args);

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

int split_args(const char *delim, Command *commands, int id){
  char *token;
  char *copy = strdup(commands[id].args[0]);
  int count=0;

  commands[id].isPipe = has_pipe(copy,strlen(copy));

  token = strtok(commands[id].args[0], delim);
  while( token != NULL ) {
    commands[id].args[count] = token;
    count++;
    token = strtok(NULL, delim);
  }
  return count;
}

int split_commands(const char *delim, char *src, Command *commands){
  char *token;
  char *copy = strdup(src);
  int count=0;

  token = strtok(src, delim);
  while( token != NULL ) {
    commands[count].args[0] = token;

    if (copy[token-src+strlen(token)] == '&')
      commands[count].isAmpersand = 1;
    
    count++;
    token = strtok(NULL, delim);
  }

  free(copy);

  return count;
}

int find_commandNum(char *line){
  int i;
  int counter=1;
  for(i=0;i<strlen(line)-1;i++)
  if (line[i]==';' || (line[i]=='&' && line[i+1]=='&'))
      counter++;

  // if(line[strlen(line)-1] != ';') counter++;
    if(line[i]==';') counter++;
  return counter;
}

void sigquit_handler (int sig) {
    assert(sig == SIGQUIT);
    pid_t self = getpid();
    _exit(0);
}

static int *quit; // chech if quit is entered
static int *flag; // help commands' contiguous execution 

void execute(int size,Command *commands){

  pid_t pid[size];  //array with all the pids
  int status[size]; //array with the status
  int parent =0;    //parent flag
  int i;
  int fd[2];

  pipe(fd);

  signal(SIGQUIT, sigquit_handler);

  flag = (int *) malloc(size*sizeof(int));

  // use shared memory for global variables
  quit = mmap(NULL, sizeof (*quit),   PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  flag = mmap(NULL, size*sizeof(int), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  
  // init global variables
  *quit=0;
  for (i=0;i<size;i++) flag[i]=0;

  // command execution loop
  for(i=0;i<size;i++){ 

    pid[i]=fork(); // fork for each command

    if (pid[i] > 0){        //PARENT process
      parent = 1;
    }
    else if (pid[i] == 0){  // CHILD processes 
      child(commands[i], i);
      exit(EXIT_SUCCESS);
    }
    else {                  // ERROR during fork
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

      // printf("status[%d]: %d\n",i,status[i]);
      
      // while(WIFEXITED(status));

      if (WEXITSTATUS(status[i]) == EXIT_FAILURE && commands[i].isAmpersand){
        printf("[ERROR]: command %s FAILED\n",commands[i].args[0]);
        kill_children(pid,i,size);
        return EXIT_FAILURE;
      }
      else{
        // printf("Everything FINE\n");
      }
      // if (WISIGNALED(status[i])) 
     //   printf("Child exited via signal %d\n",WTERMSIG(status[i]));
  
  
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

void kill_children(pid_t * pid, int start, int size){
  int i;
  for(i=start;i<size;i++)
    kill(pid[i], SIGQUIT);
}

/*int
spawn_proc (int in, int out, struct command *cmd)
{
  pid_t pid;

  if ((pid = fork ()) == 0)
    {
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }

      return execvp (cmd->argv [0], (char * const *)cmd->argv);
    }

  return pid;
}*/

void child(Command cmd,int id){

  // wait until previous command execution
  while(!flag[id]);
  

  // char copy[LINE_SIZE];
  // int size = get_command_copy(copy,command[id],id);  
  // int isPipe = has_pipe(copy,size);

  // if(isPipe){

   //  char **dst = (char**) malloc(isPipe*sizeof(char*);
    //   int num = split_old("|", copy, dst);
   //  int i,in, fd [2];

   //  /* The first process should get its input from the original file descriptor 0.  */
   //  in = 0;

   //  /* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
   //  for (i = 0; i < isPipe - 1; ++i)
   //    {
   //      pipe (fd);

   //       f [1] is the write end of the pipe, we carry `in` from the prev iteration.  
   //      spawn_proc (in, fd [1], cmd + i);

   //      /* No need for the write end of the pipe, the child will write here.  */
   //      close (fd [1]);

   //      /* Keep the read end of the pipe, the next child will read from there.  */
   //      in = fd [0];
   //    }

   //  /* Last stage of the pipeline - set stdin be the read end of the previous pipe
   //     and output to the original file descriptor 1. */  
   //  if (in != 0)
   //    dup2 (in, 0);
  // }

  // empty command
  if( strcmp(cmd.args[0],"") == 0){
    // printf("empty command\n");
    exit(EXIT_SUCCESS);
  }
  // quit is entered
  if( strcmp(cmd.args[0],"quit")==0 ){
    // printf("quit entered\n");
    *quit=1;
    exit(EXIT_FAILURE);
  }

  // execute command
  if(execvp(cmd.args[0], cmd.args) == -1){
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

int checkPipe(){


  return 1;
}

int get_command_copy (char *copy, char *command[ARG_SIZE], int id){
  int j=0, pos=0, size;

  // loop arguments
  while(command[j] != NULL){
    size = strlen(command[j]);
    memcpy(&copy[pos], command[j], size);
    pos += size;
    j++;
  }

  copy[pos] = '\0'; 
  // printf("neww --%s--\n",copy);

  return pos;
}

int has_pipe(char *command, int size){
  int i=0;
  int count=0;
  for(i=0;i<size;i++)
    if (command[i] == '|')
      count++;

  return count;
}