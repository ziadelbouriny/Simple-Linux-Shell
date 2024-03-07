#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<signal.h>
#include<sys/resource.h>

#define clear() printf("\033[H\033[J") 

char cmd[256];
char *delim = " \n";
size_t n = 0;
int argc = 0;
int i = 0;
char *argv[16];
char expName[16];
char expValue[256];

void shell();
void Setup_Environment();
void Reap_Child_Zombie();
void on_child_exist();
void Write_To_Log_File();
void parseInput();
void tokens();
int builtIn();
void exeCommands();
void parseExport();
void printDir();

int main(void)
{

	signal(SIGCHLD , on_child_exist);
	
	Setup_Environment();
	
	clear(); 
	
    	while(1)
    	{	
    		printDir();
        	parseInput();
        	if(!strcmp("", cmd)) continue;
        	tokens();
    		shell();
        }	
    
    	return 0;
}

void parseInput()
{   
    	fgets(cmd, 256, stdin);

    	if((strlen(cmd) > 0) && (cmd[strlen(cmd) - 1] == '\n'))
	    cmd[strlen(cmd) - 1] = '\0'; 
	    
	int j = 0;
	for (int i = 0; i < strlen(cmd); i++)
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
			char *token = NULL;
			token = strtok(getenv(argv[x]), " ");
			i = x;
			
			while(token)
    			{
			    	argv[i] = token;
			    	token = strtok(NULL, " ");
			    	i++;
    			}
    			break;
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
				printf("%s ", argv[i]);
				
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
	strcpy(expName, token);
	token = strtok(NULL, " ");
	strcpy(expValue, token);
		
	i = 2;	
	while (argv[i])
	{
		strcat(expValue, " ");
		strcat(expValue, argv[i]);
		i++;
		if(argv[i])
			continue;
	}		
}

void exeCommands()
{
	pid_t pid = fork(); 

	if (pid == -1)
	{ 
		printf("\nFailed forking child.."); 
		return; 
	}
	else if (pid == 0)
	{ 
		if(argv[1]  && (!strcmp(argv[1], "&")))
                {
                        argv[1] = NULL;
                }
                		
                execvp(argv[0], argv);	
                exit(0);
	}
	else
	{ 
		if (argv[1])
		{
			if(strcmp(argv[1], "&") == 0)
			{
				printf("Process : %d\n", getpid());
            			return;
            		}		
		}
		
		waitpid(pid , NULL, 0);
	}
}  

void Write_To_Log_File()
{
    	FILE * file = fopen("log.text" , "a");
    	
    	if(file == NULL)
    	{
        	printf("Error in file\n");
        	exit(EXIT_FAILURE);
    	}
    	else
    	{
        	fprintf(file , "%s" , "Child process terminated\n");
    	}
    	fclose(file);
}


void Reap_Child_Zombie()
{
	int status;
        pid_t id = wait(&status);
	    
	if(id == 0 || id == -1)
	 	return;
        else
        	Write_To_Log_File();
}

void on_child_exist()
{
	Reap_Child_Zombie();
} 

void printDir() 
{ 
	char cwd[1024]; 
	char* username = getenv("USER"); 
	getcwd(cwd, sizeof(cwd)); 
	printf("%s | %s $ ", username, cwd); 
} 	

void Setup_Environment()
{
    char arr[100];
    chdir(getcwd(arr , 100));
}

void shell()
{   	    
	if (builtIn()) 
		return; 
	else
	{
		exeCommands();
		return;	
	}
}    
