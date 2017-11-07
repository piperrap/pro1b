/*
    strtokeg - skeleton shell using strtok to parse command line
    usage:
      ./a.out
      reads in a line of keyboard input at a time, parsing it into
      tokens that are separated by white spaces (set by #define
      SEPARATORS).
      can use redirected input
      if the first token is a recognized internal command, then that
      command is executed. otherwise the tokens are printed on the
      display.

	internal commands:
        clr - clears the screen
	dir <directory> - list the current directory contents
	environ - list all environment strings 
        quit - exits from the program
	cd - change directory
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BUFFER 1024		//max line buffer
#define MAX_ARGS 64		//max # args
#define MAX_ARG_LEN 64		//max length for an arg
#define SEPARATORS " \t\n"	//token separators

extern char **environ;

int redirect(int argc, char * args[]);
void remove2(int argc, char * args[], int index);

//char * args[MAX_ARGS];	//pointers to arg strings

int main (int argc, char ** argv)
{
	char buf[MAX_BUFFER];	//line buffer
//	char * args[MAX_ARGS];	//pointers to arg strings
	char ** arg;		//working pointer thru args
	char * prompt = "==>" ;	//shell prompt
	char * argAdd; 		//the address of args[0]
	char * args[MAX_ARGS];	//pointers to arg strings
	int numArgs;
	//char * cmd = (char *)malloc(MAX_BUFFER+4); //place to build the command that will be exec-ed
/* keep reading input until "quit" command or eof of redirected input */
	while (!feof(stdin)) {
		char * cmd = (char *) malloc(MAX_BUFFER+4); //cmd holds the name of the program to be exec-ed
		
		numArgs = 0;
		/* get command line from input */
		prompt = (char*)malloc(MAX_BUFFER+4);
		char * buf = (char*)malloc(MAX_BUFFER);
		prompt = getcwd(buf, MAX_BUFFER);
		free(buf);
		strcat(prompt, "==>");
		fputs (prompt, stdout);                // write prompt
		if (fgets (buf, MAX_BUFFER, stdin )) { // read a line
/* tokenize the input into args array */
			arg = args;
			*arg++ = strtok(buf,SEPARATORS); numArgs++;  // tokenize input
			while ((*arg++ = strtok(NULL,SEPARATORS))) numArgs++;
					// last entry will be NULL
			if (args[0]) {                     // if there's anything there
/* check for internal/external command */
				argAdd = *args;
				if (!strcmp(args[0],"clr")) { 	// "clr" command
					//system("clear");
					strcpy(cmd, "clear");	
				//	continue;
				}
				else if(!strcmp(args[0],"dir")){	// "dir" command
										
					strcpy (cmd, "ls"); //set the command to ls
					strcpy(args[0], "-la"); //set first argument to -la
				}
				else if (!strcmp(args[0],"quit"))   // "quit" command
					exit(EXIT_SUCCESS);	// break out of 'while' loop
				else if(!strcmp(args[0],"environ")){ // "environ" command
					int cpid, status;
					switch (cpid = fork()){
						case -1: //errof
							fprintf(stderr, "ERROR: fork\n");
							exit(EXIT_FAILURE);
							break;
						case 0: //child
							redirect(numArgs, args);	
							char** env = environ;
							while(*env) printf("%s\n",*env++);
							exit(EXIT_SUCCESS);
						default:
							waitpid(cpid, &status, WUNTRACED);
							continue;
					}

				}
				else if(!strcmp(args[0], "cd")){
					buf = (char*)malloc(MAX_BUFFER);
					if(args[1]){
						if(!chdir(args[1]) && !setenv("PWD", getcwd(buf, MAX_BUFFER), 1)){
							continue;
						}else{
						       	fprintf(stderr, "ERROR: cd\n");
							continue;
						}
					}else{
						fprintf(stdout, "PWD=%s\n", getcwd(buf, MAX_BUFFER));
						continue;
					}
					free(buf);
				}
				else{
					strcpy(cmd, args[0]);
				}
/*for and exec command with args in child process*/
				int pid, status;
				switch (pid = fork())
				{
					case -1://error
						fprintf(stderr, "ERROR: fork\n");
						exit(EXIT_FAILURE);
						break;
					case 0:;//child
						redirect(numArgs, args);
						execvp(cmd, args);
						fprintf(stderr, "ERROR: exec\n");
						exit(EXIT_FAILURE);
						break;
					default:
						waitpid(pid, &status, WUNTRACED);
				
				}
				free(cmd);
				continue;
			}

		}

	}
	return 0; 
}

/*reads through the args array to find < > or >> tokens
 * if they are present at index i it will open the appropriate file 
 * from the path at index i+1
 * returns number of redirects successfully opened, 0 for none*/
int redirect(int argc, char * args[]){
	int redirects = 0; //number of redirects successfully opened
	for(int i=1; i<argc; i++){
		if(!strcmp(args[i], "<")){ //input redirection
			//file path will be at index i+1
			char * path = strdup(args[i+1]);
				
			//remove rediretion args from arg array
			args[i] = NULL;
			args[i++] = NULL;
			
			//if the file exists and is readable
			if(!access(path, R_OK)){
			//open file and replace stdin
				freopen(path, "r", stdin); 
				free(path);
				redirects++;//increment redirects
			}else{
				fprintf(stderr, "ERROR: Input Redirect\n");
			}
		//output redirection with truncation
		}else if(!strcmp(args[i], ">")){
		//file path will be at index i+1
			char * path = strdup(args[i+1]);
		
			//remove rediretion args from arg array
			args[i] = NULL;
			args[i++] = NULL;

			//if the file exists and is writeabel or the file does not exist
			if(!access(path, W_OK) || access(path, F_OK))
			//open or create, truncate, and replace stdout
			{
				freopen(path, "w", stdout);
				free(path);
				redirects++;//increment redirects
			}
			else{
				fprintf(stderr, "ERROR: Output Redirect\n");
			}
			//appended output redirection 
		}else if(!strcmp(args[i], ">>")){
			//file path will be at index i+1
//			outIndex = i+1;	
			char * path = strdup(args[i+1]);
			
			//remove rediretion args from arg array
			args[i] = NULL;
			args[i++] = NULL;
						
			//if the file exists and is writable or the file does not exist
			if(!access(path, W_OK) || access(path, F_OK)){
				//open or create for writing at the end of the file
				freopen(path, "a", stdout);
				free(path);
				redirects++;//increment redirects
			}else{
				fprintf(stderr, "ERROR: Output Redirect");
			}
		}
						
	}
	return redirects;
}
