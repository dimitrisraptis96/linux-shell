# linux-shell
A command line linux shell. The user can choose between __interactive__ and __batch version__. Inserted commands can be seperated using __';'__ delimiter for each line.

### How to run the project:

### INTERACTIVE
Instantly execution of commands after return character is pressed. 

    ```
    $ cd /linux-shell   //change to root directory
    
    $ make              //compile source code
    
    $ ./bin/myshell     //start interactive shell
    ```

### BATCH
Create a batch file with your commands and pass it as argument. 

    ```
    $ cd /linux-shell             //change to root directory
    
    $ make                        //compile source code
    
    $ ./bin/myshell batchfile     //start batch shell
    ```
