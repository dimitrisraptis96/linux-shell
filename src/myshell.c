/*
 * Author:    Dimitrios Raptis 8467
 *
 * Purpose:   A command line linux shell implementing interactive and batch mode
 *
 * Language:  C
 */


#include "../include/myshell.h"

#define LINE_SIZE 512
#define ARG_SIZE  64

struct _Command {
  char *args[ARG_SIZE]; // command plus arguments
  int isAmpersand;      // true if command is right before '&&'
  int isPipe;           // true if command includes pipeline
};


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

    char *line = remove_whitespace(input);
    
    if (strlen(line) == 0) continue; // empty line

    // split commands 
    commandNum = split_commands("&&;",line,commands);

    // split arguments of each command
    for (i=0; i<commandNum; i++){

      commands[i].args[0] = remove_whitespace(commands[i].args[0]);
      split_args(" \t\r\n\a", &commands[i]);
    }

    execute(commandNum,commands);

    // delete previous line input
    memset(input, 0, LINE_SIZE);

    free_commands(commands);
    printf("\n");
    }
  }



void batch(char *filename){
  int i;

  int commandNum; // number of commands per line
  char input[LINE_SIZE];     

  Command commands[LINE_SIZE];

  // open batch file
  FILE *fp = fopen(filename,"r"); 
  HANDLE_NULL( fp );

  while(fgets(input, LINE_SIZE, fp) != NULL){
    
    char *line = remove_whitespace(input);
    
    if (strlen(line) == 0) continue; // empty line
    if (line[0] == '#')    continue;    // comment line

    commandNum = split_commands("&&;",line,commands);

    // split arguments of each command
    for (i=0; i<commandNum; i++){
      commands[i].args[0] = remove_whitespace(commands[i].args[0]);
      split_args(" \t\r\n\a", &commands[i]);;
    }

    execute(commandNum,commands);

    free_commands(commands);
    }

  // close the file
  HANDLE_EOF( fclose(fp) );
}

// set last commands struct to zero
void free_commands(Command *commands){
  int i,j;

  for (i=0;i<LINE_SIZE;i++){
    commands[i].isAmpersand = 0;
    commands[i].isPipe =0;
    for (j=0; j<ARG_SIZE; j++)
      commands[i].args[j] = NULL;      
  }
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

// split a command into arguments and return the number of them
int split_args(const char *delim, Command *command){
  char *token;
  char *copy = strdup(command->args[0]);
  int count=0;

  // check for pipeline symbol
  command->isPipe = has_pipe(copy,strlen(copy));

  token = strtok(command->args[0], delim);
  while( token != NULL ) {
    command->args[count] = token;
    count++;
    token = strtok(NULL, delim);
  }
  return count;
}

// split a string into commands according to delimiters
int split_commands(const char *delim, char *src, Command *commands){
  char *token;
  char *copy = strdup(src);
  int count=0;

  token = strtok(src, delim);
  while( token != NULL ) {
    commands[count].args[0] = token;

    // check for commands seperated by &&
    if (copy[token-src+strlen(token)] == '&')
      commands[count].isAmpersand = 1;
    
    count++;
    token = strtok(NULL, delim);
  }
  return count;
}

// check if the string-command includes the pipeline symbol
int has_pipe(char *command, int size){
  int i=0;
  int count=0;
  for(i=0;i<size;i++)
    if (command[i] == '|')
      count++;

  return count;
}


// get the full command from the struct Command
int get_command_copy (char *copy, Command command){
  int i=0, pos=0, size;

  // loop arguments
  while(command.args[i] != NULL){
    size = strlen(command.args[i]);
    memcpy(&copy[pos], command.args[i], size);
    pos += size;
    i++;
    // add whitespace to seperate arguments
    copy[pos] = ' ';
    pos++; 
  }

  copy[pos] = '\0';

  return pos;
}

// find number of commands into the line
int find_commandNum(char *line){
  int i;
  int counter=1;
  for(i=0;i<strlen(line)-1;i++)
    if (line[i]==';' || (line[i]=='&' && line[i+1]=='&'))
        counter++;
  
  if(line[i]==';') counter++;
  return counter;
}

// signal handler for killing child processes
void sigquit_handler (int sig) {
    assert(sig == SIGQUIT);
    _exit(0);
}

static int *quit; // chech if quit is entered
static int *flag; // help commands execute sequentially 

void execute(int size,Command *commands){
  pid_t pid[size];  //array with all the pids
  int status[size]; //array with the status
  int parent =0;    //parent flag
  int i;

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

      // check if the command includes pipeline
      (commands[i].isPipe) ? pipeline(commands[i],i): child(commands[i], i);
      exit(EXIT_SUCCESS);
    }
    else {                  // ERROR during fork
      
      printf("[ERROR]: Fork failed\n");
      exit(EXIT_FAILURE);
    }
  }

  // only parent goes here!!!
  if (parent){

    for(int i=0;i<size;i++){  
      // wait the commands execute sequently
      flag[i]=1;
      while ( ( waitpid(pid[i],&status[i],0) ) > -1 );

      // check if previous command executed successfully (if '&&' is used)
      if (WEXITSTATUS(status[i]) == EXIT_FAILURE && commands[i].isAmpersand){
        printf("Rest of commands abandoned!\n");
        kill_children(pid,i,size);
        break;
      }
    }

    // check if quit is entered
    if(*quit) {
      printf("'quit' entered, exiting shell..\n");
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


void child(Command cmd,int id){

  // wait until previous command execution
  while(!flag[id] || cmd.isPipe);

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
    printf("Unknown command: %s\n",cmd.args[0]);
    exit(EXIT_FAILURE);
  }
  else {
    // correct commmand execution
    // printf("\t\t\t\tCommand executed: %s\n",cmd.args[0]);
    exit(EXIT_SUCCESS);
  }    
}

// execute pipeline commands
void pipeline(Command cmd, int id){
  int i, num;
  char copy[LINE_SIZE];
  Command *commands;
  
  // get the full command string  
  get_command_copy(copy,cmd);

  // temporary struct with pipeline command
  Command tmp;
  tmp.args[0] = copy;

  // split pipeline to seperated commands
  num = split_args("|",&tmp);
  HANDLE_NULL( ( commands = (Command *) malloc (num*sizeof(Command)) ) );

  // split arguments for each command
  for(i=0; i<num; i++){
    commands[i].args[0] = tmp.args[i]; // copy tmp commands to the final struct
    commands[i].args[0] = remove_whitespace(commands[i].args[0]);
    commands[i].isPipe =1;
    split_args(" \t\r\n\a", &commands[i]);
  }

  // wait until previous command execution
  while(!flag[id]);

  /*The following link was useful here:
    https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell*/
  
  int in, fd [2];
 
  pid_t pid;
  int status;  

  // expect last command
  for (i=0; i<(num-1); i++){
    pipe (fd);
     // fd[1] is the write end of the pipe, we carry `in` from the prev iteration.  
    pid = spawn_proc (in, fd [1], commands[i], id);
    close (fd [1]);   // no need for the write end of the pipe, the child will write here.  
    
    in = fd [0];      // keep the read end of the pipe, the next child will read from there. 
    
    //wait child to finish
    while ( ( waitpid(pid,&status,0) ) > -1 );
  }
  
  // Last stage of the pipeline - set stdin be the read end of the previous pipe
  // and output to the original file descriptor 1.   
  if (in != 0) dup2 (in, 0);

  // execute last commands of pipeline
  child(commands[num-1], id);
}

// execute commands within a pipeline forking a new child for each command(expept last one)
int spawn_proc (int in, int out, Command cmd, int id){
  pid_t pid;

  if ((pid = fork ()) == 0){

    if (in != 0) {
      dup2 (in, 0);
      close (in);
    }
    if (out != 1){
      dup2 (out, 1);
      close (out);
    }

     child(cmd, id);
  }

  return pid;
}