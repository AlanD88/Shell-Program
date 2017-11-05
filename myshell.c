/*
 * Alan Duncan - aljdunca
 * Asgn1
 * myshell program
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

extern char **getline();

#define READ 0
#define WRITE 1

#define TRUE 0
#define FALSE 1


int Check_Input(char **);
int Check_Output(char **);
void run_cmd(int, int pipepos[], char **);
int main() {
  int i;
  char **input;
  char **cmd_set[255];
  int pipe_pos[50];

  while (1) {
    input = getline();
    if (strcmp("exit", input[0]) == 0){
	exit(0);
    }
    else if (strcmp("cd", input[0]) == 0){
	if (input[1] == NULL || (strcmp("home", input[1]) == 0) || (strcmp("HOME", input[1]) == 0) || (strcmp("Home", input[1]) == 0))
	{
		if (chdir(getenv("HOME")) != 0){
			perror("Error");
		}
	}
	else {
		if (chdir(input[1]) == 0){
		}
		else {
			perror("Error");
		}
	}
    }
    else if (input[0] != NULL){
	int tmp = 0;
	int numcommands = 0;
	int out_bool = FALSE;
	int in_bool = FALSE;
	//Find Number of commands and if performing any input/output operations	
	for (int i = 0; input[i] != NULL; i++) {
		if (strcmp("|", input[i]) == 0){
			pipe_pos[numcommands] = i;
			numcommands++;
		}

		if (strcmp(">", input[i]) == 0){
			out_bool = TRUE;
		}
		if (strcmp("<", input[i]) == 0) {
			in_bool = TRUE;			
		}
		if (input[i + 1] == NULL){
			pipe_pos[numcommands] = i+1;
		}
	}
	
	numcommands++;
	// If no commands at all have been entered.
	if (input[0] == NULL) {
		numcommands = 0;
	}


	pid_t pid;
	int status;
	int in, out, pos;

	if (numcommands == 1) {
		pid = fork();
		
		if (pid == 0){
			if (in_bool == TRUE)
			{
				pos = Check_Input(input);
				in = open(input[pos+1], O_RDONLY);
				if (in < 0){
					perror("Error");
					exit(1);
				}
				// Add in error checking
				dup2(in, 0);
				close(in);
			}
			if (out_bool == TRUE) {
				pos = Check_Output(input);
				out = open(input[pos+1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
				if(out < 0){
					perror("Error");
				}
				// Add in error checking
				dup2(out, 1);
				close(out);
			}
			int tmp = 0;	
			for (int i = 0; input[i] != NULL; i++) {
				if(strcmp("<", input[i]) == 0){
					cmd_set[i] = NULL;
					break;
				}
				else if (strcmp(">", input[i]) == 0){
					cmd_set[i] = NULL;
					break;
				}
				else{
					cmd_set[tmp] = input[i];
					tmp++;
					if (input[i+1] == NULL){
						cmd_set[tmp] = NULL;
					}
				}
			}
			
			if (execvp(cmd_set[0], cmd_set) < 0){
				perror("Error");
				exit(1);
			}
		}
		else if (pid < 0){
			perror("fork");
		}
		else {
			//In parent
			while (wait(&status) != pid) {
			}
		}
	}
	else if (numcommands > 1){

		if (numcommands == 2){
			pid_t pid, pid2;
			char **cmdone[10];
			char **cmdtwo[10];
			int tmp = 0;
			int inpos;
			int outpos;
			in_bool = FALSE;
			out_bool = FALSE;
			for (int i = 0; input[i] != NULL; i++){
				if (i < pipe_pos[0] && strcmp("<", input[i]) != 0 && strcmp(">", input[i]) != 0){
					cmdone[tmp] = input[i];
					tmp++;
				}
				else if (i == pipe_pos[0]){
					cmdone[tmp] = NULL;
					tmp = 0;
				}
				else if (i > pipe_pos[0] && strcmp("<", input[i]) != 0 && strcmp(">", input[i]) != 0) {
					cmdtwo[tmp] = input[i];
					tmp++;
				}
				else if (input[i+1] == NULL)
				{
					cmdtwo[tmp] = NULL;
				}
				else if(strcmp(">", input[i]) == 0){
					out_bool = TRUE;
					i++;

				}
				else if(strcmp("<", input[i]) == 0){
					in_bool = TRUE;
					i++;
				}
			}
			cmdtwo[tmp] = NULL;
			if(in_bool == TRUE){
				inpos = Check_Input(input);
				in = open(input[inpos+1], O_RDONLY);
				int in_dp = dup(in);
				if (dup2(in_dp, 0)< 0){
					perror("Error");
					exit(1);
				}

			}
			if (in < 0){
				perror("Error");
				exit(1);
			}

			if(out_bool == TRUE){ 
				outpos = Check_Output(input);
				out = open(input[outpos+1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
				int out_dp = dup(out);
				if (dup2(out_dp, 1) < 0) {
					perror("Error");
				}

			}		

			if (out < 0){
				perror("Error");
				exit(1);
			}
			
			int pipes[2];
				
			if (pipe(pipes) != 0){
				perror("Error");
				exit(1);
			}
			//Create processes	
			pid = fork();
			if (pid == 0) {
				dup2(pipes[1], 1);
				close(pipes[0]);
				close(out);
				
				if(execvp(cmdone[0], cmdone) < 0){
					perror("Error");
					exit(1);
				}
				
			}
			else if (pid < 0){
				perror("fork");
			}
			else{
				//parent
				
				pid2 = fork();

				if (pid2 == 0){
					dup2(pipes[0], 0);
					close(pipes[1]);
					
					if (execvp(cmdtwo[0], cmdtwo) < 0) {
						perror("Error");
						exit(1);
					}
				}
				else if (pid2 < 0){
					perror("fork");
				}
				else{
					if (close(in) < 0)
					{
						perror("error");
					}
					if (close(out) < 0){
						perror("error");
					}
					close(pipes[0]);
					close(pipes[1]);
					
					
				}
				
					
				while(wait(&status) != pid){
				}
			}

		}

		// if commands > 2
		else{

			//Build pipes
			//
			//Dont forget to add in error handling
		
			pid_t pid;
			int pipes[2];

			pid = fork();

			if (pid == 0){
				run_cmd(numcommands, pipe_pos, input);
				//call recursive function
				//dont call wait inside recursive functions when creating forks
				//
				//Make sure to account for error handling inside recursive function.  Call exit() when fork or redirection doesnt work
			}
			else{
				while (wait(&status) != pid) {
				}
			}
		}
	//		
	}
    }
  }
  
  return 0;
}
int Check_Input(char **cmd_list) {
	// Function to get position of input file if any.
	// Return -1 if not found.
	
	//int input_pos = 0;
	for (int i =0; cmd_list[i] != NULL; i++) {
		if (strcmp("<", cmd_list[i]) == 0){
			printf("%d", i);
			return i;
		}
	}
	//return input_pos;
	return -1;
}
int Check_Output(char **cmd_list) {
	//Function to get first output file if any
	// Return -1 if not found
	//int out_pos = 0;
	for (int i = 0; cmd_list[i] != NULL; i++) {
		if (strcmp(">", cmd_list[i]) == 0) {
			return i;
		}
	}
	//return out_pos;
	return -1;
}
void run_cmd(int numcmd, int pipepos[], char **inputline){
	pid_t pid;
	char *command[30];
	int x = 0;
	int start;
	int end;
	int tmp = 0;

	// Count total number of commands just replace with an argument
	for (int i = 0; pipepos[i] != NULL; i++){
		x++;
	}
	if (x - numcmd == 0){
		start = 0;
		end =  pipepos[0];
	}
	else{
		start = pipepos[(x - numcmd) - 1];  // Gets command position to start at.  For example at start if there were 5 commands 5 - 5 would be index 0 of pipepos array
		start = start + 1;
		end = pipepos[(x - numcmd)];
	}

	for(int i = start; i < end; i++){
		command[tmp] = inputline[i];
		tmp++;
	}
	command[tmp] = NULL;

	if(numcmd >  0) {
		//pid = fork();
		//if (pid == 0) {
			//run_cmd(numcmd - 1, pipepos, inputline);
		//}
		//else{			
			run_cmd(numcmd - 1, pipepos, inputline);
		//}		

	}

}

