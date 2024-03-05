#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 

char cmd[256];
char *delim = " \n";
size_t n = 0;
int argc = 0;
int i = 0;
char *argv[16];
char expName[16];
char expValue[16];

void shell();
void log_handle(int sig);
void parseInput();
void tokens();
int builtIn();
void exeCommands();
void parseExport();
void exeEcho();


int main(void)
{

	signal(SIGCHLD, log_handle);
	
    	while(1)
    	{	
    		shell();
        }	
    
    	return 0;
}

void parseInput()
{
	printf("$ ");
    
    	fgets(cmd, 256, stdin);

    	if((strlen(cmd) > 0) && (cmd[strlen(cmd) - 1] == '\n'))
	    cmd[strlen(cmd) - 1] = '\0';
	    
	int j = 0;
	for (int i = 0; i < strlen(cmd); i ++)
	{
		if (cmd[i] != '"' && cmd[i] != '\\')
            	{ 
                 	cmd[j++] = cmd[i];
            	}
            	else if (cmd[i+1] == '"' && cmd[i] == '\\')
            	{ 
                 	cmd[j++] = '"';
            	}
                else if (cmd[i+1] != '"' && cmd[i] == '\\')
            	{ 
                 	cmd[j++] = '\\';
            	}
	}		
	if(j > 0) 
	   cmd[j] = 0;    
}

void tokens()
{
	char *token = NULL;
    	i = 0;
    	token = strtok(cmd, delim);

    	while(token)
    	{
	    argv[i] = token;
	    token = strtok(NULL, delim);
	    i++;
    	}
    
    	argv[i] = NULL;
    	
    	int x = 1;
	while (argv[x])
	{
		if (argv[x][0] == '$')
		{
			for (int j = 0; strlen(argv[x]) - 1  >= j;  j++)
			{
			        int z = j + 1;
				argv[x][j] = argv[x][z];
			}
			argv[x] = getenv(argv[x]);
		}
		x++;		
	}
}

int builtIn()
{
	int switchOwnArg = 0; 
	char* ListOfOwnCmds[4];

	ListOfOwnCmds[0] = "exit"; 
	ListOfOwnCmds[1] = "cd"; 
	ListOfOwnCmds[2] = "echo"; 
	ListOfOwnCmds[3] = "export"; 

	for (i = 0; i < 4; i++)
	{ 
		if (strcmp(argv[0], ListOfOwnCmds[i]) == 0)
		{ 
			switchOwnArg = i + 1; 
			break; 
		} 
	} 

	switch (switchOwnArg)
	{
	 
		case 1: 
			exit(0); 
		case 2: 
			chdir(argv[1]); 
			return 1; 
		case 3:	
			int i = 1;
			while (argv[i])
			{
			        
				printf("%s", argv[i]);
				
				i++;
				if (argv[i] == NULL)
					printf("\n");
			}			
			return 1; 
		case 4: 
			parseExport();
			setenv(expName, expValue, 1);
			return 1; 
		default: 
			break; 
	} 

	return 0; 
}

void exeEcho()
{
        int i = 1;
	while (argv[i])
	{
		if (argv[i][0] == '$')
		{
			for (int j = 0; strlen(argv[i]) - 1  >= j;  j++)
			{
			        int z = j + 1;
				argv[i][j] = argv[i][z];
			}
			argv[i] = getenv(argv[i]);
		}
		
		printf("%s\n", argv[i]);
		
		i++;	
		
	}	
}

void parseExport()
{
        int i,j;
	for (i = 0, j = 0; argv[1][i] != '\0'; i++)
	{
		if (argv[1][i] != '=')
			argv[1][j++] = argv[1][i];
		else
			argv[1][j++] = ' ';	
	}
	argv[1][j] = '\0';
	
	char *token = strtok(argv[1], " ");
	
	if (token != NULL)
	{
		strcpy(expName, token);
		token = strtok(NULL, " ");
		
		if (token != NULL)
		{
			strcpy(expValue, token);
		}
		else
		{
			expValue[0] = '\0';
		}
	}
	else
	{
		strcpy(argv[1], argv[2]);
		argv[2][0] = '\0';
	}		
}

void exeCommands()
{
	pid_t pid = fork();

    	if (pid == -1)
    	{ 
		printf("\nFailed forking child..\n"); 
		return; 
	} 
	else if (pid == 0)
	{ 
		if (execvp(argv[0], argv) < 0)
		{ 
			printf("\nCould not execute command..\n"); 
		} 
		exit(0); 
	}
	else
	{
		wait(NULL); 
		return; 
	}
}  

void log_handle(int sig)
{
	FILE *pFile;
        pFile = fopen("log.txt", "a");
        
        if(pFile==NULL)
            perror("Error opening file.");
        else
            fprintf(pFile, "[LOG] child proccess terminated.\n");
            
        fclose(pFile);
}  			

void shell()
{   
        
        parseInput();
        tokens();
        
	if (builtIn()) 
		return; 
	else
		exeCommands();
		return;	
}    
