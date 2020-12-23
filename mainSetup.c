#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
Variable Identifier Instruction Set


PART A, B
0000 -----> Not I/O Redirection { We have to handle part A or part B }

PART C
0001 -----> Output
0010 -----> Append
0011 -----> Input
0100 -----> Both Input and Output 
0101 -----> Both Input and Append
0110 -----> Both Input and Standard Error
0111 -----> Standard Error
1000 -----> ERROR CASE
1001 -----> ERROR CASE
1010 -----> ERROR CASE
1011 -----> ERROR CASE
1100 -----> ERROR CASE
1101 -----> ERROR CASE
1110 -----> ERROR CASE
1111 -----> ERROR CASE 


*/

#define MAX_LINE 128 /* 128 chars per line, per command, should be enough. */

void setup(char inputBuffer[], char *args[], int *background);
char *getPath(char *arg, char *envPath);
int getCommandType(char *args[]);

struct processNode
{
    char *name;
    pid_t pid;
    pid_t parent_pid;
    int status; // 0 done // 1 working
    struct processNode *next;
};
struct processNode *root;

char *identifier; // Every process has its own unique identifier according to its category
int ct;           /* index of where to place the next parameter into args[] */

int main(void)
{
    char inputBuffer[MAX_LINE];   /*buffer to hold command entered */
    int background;               /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE / 2 + 1]; /*command line arguments */
    char *envPath = getenv("PATH");

    while (1)
    {
        background = 0;
        printf("myshell: ");
        fflush(stdout);
        /*setup() calls exit() when Control-D is entered */
        setup(inputBuffer, args, &background);
        pid_t PID;
        //execv(getPath(args[0], envPath), args);
        printf("Identifier: %s\n", identifier);

        if (strcmp("0000", identifier) == 0)
        {
            if (strcmp(args[0], "ps_all") == 0)
            {
                // PART B
            }
            else if (strcmp(args[0], "^Z") == 0)
            {
                // PART B
            }
            else if (strcmp(args[0], "search") == 0)
            {
                // PART B
            }
            else if (strcmp(args[0], "bookmark") == 0)
            {
                // PART B
            }
            else if (strcmp(args[0], "exit") == 0)
            {
                // PART B
            }
            else
            {
                // PART A
                if (background == 1)
                {
                    args[ct - 1] = NULL;
                }
               
                // HERE WE START PROCESS CREATION
                if ((PID = fork()) == -1)
                {
                    fprintf(stderr, "%s", "Fork Failed!\n");
                }

                if (PID == 0)
                {
                
                    execv(getPath(args[0], envPath), args);
                }
                if (background == 0)
                {
                    if (-1 == waitpid(PID, NULL, 0))
                    {
                        fprintf(stderr, "%s", "Some error occurred while waiting for a foreground application to end.\n");
                    }
                }
            }

            /** the steps are:
            (1) fork a child process using fork() */
            /*
                        (2) the child process will invoke execv()
						(3) if background == 0, the parent will wait,
                        otherwise it will invoke the setup() function again. */
        }
        else
        {
            // DEFINITELY PART C
        }

        
    }
}

/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

void setup(char inputBuffer[], char *args[], int *background)
{

    identifier = strdup("0000");
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start;  /* index where beginning of next command parameter is */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0); /* ^d was entered, end of user command stream */

    /* the signal interrupted the read system call */
    /* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ((length < 0) && (errno != EINTR))
    {
        perror("error reading the command");
        exit(-1); /* terminate with error code of -1 */
    }

    //printf(">>%s<<", inputBuffer);
    for (i = 0; i < length; i++)
    { /* examine every character in the inputBuffer */

        switch (inputBuffer[i])
        {
        case ' ':
        case '\t': /* argument separators */
            if (start != -1)
            {
                args[ct] = &inputBuffer[start]; /* set up pointer */
                ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;

        case '\n': /* should be the final char examined */
            if (start != -1)
            {
                args[ct] = &inputBuffer[start];
                ct++;
            }
            inputBuffer[i] = '\0';
            args[ct] = NULL; /* no more arguments to this command */
            break;
        case '>':
            if (strcmp(identifier, "0011") == 0) // If we have multiple operators like > < , 2>
            {
                if (inputBuffer[i + 1] == '>')
                {

                    identifier = strdup("0101");
                }
                else if (inputBuffer[i - 1] == '2')
                {
                    identifier = strdup("0110");
                }
                else
                {
                    identifier = strdup("0100");
                }
            }
            else if (strcmp(identifier, "0101") != 0 && strcmp(identifier, "0110") != 0 && strcmp(identifier, "0100") != 0)
            {
                if (inputBuffer[i + 1] == '>')
                {

                    identifier = strdup("0010");
                }
                else if (inputBuffer[i - 1] == '2')
                {
                    identifier = strdup("0111");
                }
                else
                {
                    identifier = strdup("0001");
                }
            }
            break;
        case '<':
            identifier = strdup("0011");
            break;

        default: /* some other character */
            if (start == -1)
                start = i;
            if (inputBuffer[i] == '&')
            {
                *background = 1;
                inputBuffer[i - 1] = '\0';
            }
        }            /* end of switch */
    }                /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */

    for (i = 0; i <= ct; i++)
        printf("args %d = %s\n", i, args[i]);
} /* end of setup routine */

char *getPath(char *arg, char *envPath)
{
    char *temp = envPath;
    char *ch;
    ch = strtok(temp, ":");

    while (ch != NULL)
    {
        char tempPath[200];
        strcpy(tempPath, ch);
        tempPath[strlen(ch)] = '/';
        tempPath[strlen(ch) + 1] = '\0';
        strcat(tempPath, arg);
        if (access(tempPath, X_OK) != -1)
        {
            return strdup(tempPath);
        }
        ch = strtok(NULL, ":");
        printf("%s\n", tempPath);
    }
    return "-1";
}