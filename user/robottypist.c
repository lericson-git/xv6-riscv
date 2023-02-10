
/*
With this program, user type texts line by line, where each line has a maximum length of 127 characters.  
Each line is considered as "entered" (i.e., finalized) when the ENTER or RETURN key is clicked.
The program counts the number of characters entered for each minute.
When user types ":exit", the program exits.  
The Makefile should be changed to include this program and thus it can be run from the shell of xv6.
*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_LINE_SIZE 128
#define MAX_BUF_SIZE 1280
char buf[MAX_BUF_SIZE];


void
panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(1);
}

//create a new process
int
fork1(void)
{
  int pid;
  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

//create a pipe
void 
pipe1(int fd[2])
{
 int rc = pipe(fd);
 if(rc<0){
   panic("Fail to create a pipe.");
 }
}


//pull everything from pipe and return the size of the content
int
read1(int *fd)
{
 char ch='a';
 write(fd[1],&ch,1); //write something to the pipe so the next read will not block (because read operation is a blocking operation - if the pipe is empty, read will wait till something is put in)
 int len = read(fd[0],buf,MAX_BUF_SIZE)-1;
 return len;
}


int
main(int argc, char *argv[])
{

    //create two pipe to share with child
    int fd1[2], fd2[2];
    pipe1(fd1); //a pipe from child to parent - child sends entered texts to parent for counting
    pipe1(fd2); //a pipe from child to parent - child lets parent stop (when user types :exit)

    //Test input A: running time (sec) B: time interval (sec), A should be multiple of B
    if (argc != 3) { printf("robottypist requires 2 parameters: running time and time interval (in seconds)\n"); exit(0); }
    int runTime = atoi(argv[1]);
    int timeInterval = atoi(argv[2]);

    //Make sure parameters are multiple of each other
    if(runTime % timeInterval != 0) {
        printf("Runtime: %d, Time Interval: %d, argc: %d", runTime, timeInterval, argc);
        printf("runtime must be a multiple of time interval parameter\n");
        exit(0);
    }




    int result = fork1(); //create child process

    //Vars for child process
    char const str[] = "Hello!";
    int len = strlen(str) - 1;
    int iterations = runTime / timeInterval;
    int i = 0;

    if(result == 0){

        //child process:

        close(fd1[0]);
        close(fd2[0]);
        //char buf[MAX_LINE_SIZE];
        while(1) {
            printf("%s\n", str);
            write(fd1[1], &len, 1);
            sleep(timeInterval);
            i++;

            //check for exit
            if(i == iterations) {
                char ch = 'a';
                write(fd2[1], &ch, 1);
                exit(0);
            }     
        }
    } else {

        //parent process:

        while(1){
            sleep(60);

            int len = read1(fd1); 	
            printf("\nIn last minute, %d characters were entered.\n", len);

            len = read1(fd2);
            if(len > 0){
            //now to terminate
            wait(0);
            exit(0);
            }
        }

    }    
}
