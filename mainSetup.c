#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>

/*
Variable Identifier Instruction Set


PART A, B
0000 -----> Not I/O Redirection { We have to handle part A or part B }

PART C
0001 -----> Output 1
0010 -----> Append 1 
0011 -----> Input 1
0100 -----> Both Input and Output 
0101 -----> Both Input and Append
0110 -----> Both Input and Standard Error
0111 -----> Standard Error 1
1000 -----> ERROR CASE "<< Case"
1001 -----> ERROR CASE
1010 -----> ERROR CASE
1011 -----> ERROR CASE
1100 -----> ERROR CASE
1101 -----> ERROR CASE
1110 -----> ERROR CASE
1111 -----> ERROR CASE 


*/

#define MAX_LINE 128 /* 128 chars per line, per command, should be enough. */
#define PERMISSION_FOR_OUTPUT_FILE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FLAGS_FOR_APPEND (O_WRONLY | O_CREAT | O_APPEND)
#define FLAGS_FOR_OUTPUT (O_WRONLY | O_CREAT | O_TRUNC)
#define FLAGS_FOR_INPUT (O_RDONLY)
char *env_path;

void setup(char inputBuffer[], char *args[], int *background);
char *getPath(char *arg);
int getCommandType(char *args[]);
void insertBookmark(char *command);
void printBookmarkList();
void removeBookmark(int position);
char *getExecutableStringFromBookmarks(int position);
int isArgumentNumber(char *arg);
void removeAllChars(char *str, char c);
int searchInCurrentDirectory(char *input);
void searchRecursiveDirectory(char *path, char *input);
int searchInfile(char *fname, char *str);
void recursiveSearch(char *path, char *str);
char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);

struct bookmarkNode
{
    char *command;
    struct bookmarkNode *next;
};

struct bookmarkNode *head = NULL;
struct bookmarkNode *current = NULL;
struct bookmarkNode *prev = NULL;

struct processNode
{
    char *name;
    pid_t pid;
    pid_t parent_pid;
    int status; // 0 done // 1 working
    struct processNode *next;
    struct processNode *previous;
};
struct processNode *root;

char *identifier; // Every process has its own unique identifier according to its category
int ct;           /* index of where to place the next parameter into args[] */

int main(void)
{
    char inputBuffer[MAX_LINE];   /*buffer to hold command entered */
    int background;               /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE / 2 + 1]; /*command line arguments */
    env_path = getenv("PATH");

    while (1)
    {
        background = 0;
        printf("myshell: ");
        fflush(stdout);
        /*setup() calls exit() when Control-D is entered */
        setup(inputBuffer, args, &background);
        pid_t PID;
        //execv(getPath(args[0], envPath), args);
        //printf("Identifier: %s\n", identifier);
        if (strcmp("0000", identifier) == 0)
        {

            if (strcmp(args[0], "ps_all") == 0)
            {
                // PART B
            }
            else if (strcmp(args[0], "^Z") == 0)
            {
                printf("muucukkk envera bi");
                // PART B
            }
            else if (strcmp(args[0], "search") == 0)
            {
                // PART B
                if (args[1] == NULL)
                {
                    fprintf(stderr, "%s", "Input argument is invalid.");
                }
                else if (strcmp(args[1], "-r") == 0)
                {
                    char searchKeyword[1024];
                    strcpy(searchKeyword, "");
                    for (int i = 0; i < ct - 1; i++)
                    {
                        strcpy(args[i], args[i + 1]);
                    }
                    int index = ct - 1;
                    for (int i = 1; i < index; i++)
                    {
                        strcat(searchKeyword, args[i]);
                        if (i != index - 1)
                        {
                            strcat(searchKeyword, " ");
                        }
                    }
                    removeAllChars(searchKeyword, '"');
                    char path[1024];
                    getcwd(path, sizeof(path));
                    recursiveSearch(path, searchKeyword);
                }
                else
                {
                    char searchKeyword[1024];
                    strcpy(searchKeyword, "");
                    for (int i = 0; i < ct - 1; i++)
                    {
                        strcpy(args[i], args[i + 1]);
                    }
                    int index = ct - 1;
                    for (int i = 0; i < index; i++)
                    {
                        strcat(searchKeyword, args[i]);
                        if (i != index - 1)
                        {
                            strcat(searchKeyword, " ");
                        }
                    }
                    removeAllChars(searchKeyword, '"');
                    searchInCurrentDirectory(searchKeyword);
                }
            }
            else if (strstr(inputBuffer, "bookmark") != NULL)
            {
                if (args[1] == NULL)
                {
                    fprintf(stderr, "%s", "Input is invalid.");
                    continue;
                }
                else if (strcmp(args[1], "-i") == 0)
                {
                    if (args[2] == NULL)
                    {
                        fprintf(stderr, "%s", "Input is invalid.");
                        continue;
                    }
                    else if (isArgumentNumber(args[2]) == 0)
                    {
                        fprintf(stderr, "%s", "Please specify a numerical value for execution.");
                        continue;
                    }
                    else
                    {
                        char *executeCommand = getExecutableStringFromBookmarks(atoi(args[2]));
                        system(executeCommand);
                    }
                }
                else if (strcmp(args[1], "-d") == 0)
                {
                    if (args[2] == NULL)
                    {
                        fprintf(stderr, "%s", "Input is invalid.");
                        continue;
                    }
                    else if (isArgumentNumber(args[2]) == 0)
                    {
                        fprintf(stderr, "%s", "Please specify a numerical value for execution.");
                        continue;
                    }
                    else
                    {
                        removeBookmark(atoi(args[2]));
                    }
                }
                else if (strcmp(args[1], "-l") == 0)
                {
                    printBookmarkList();
                }
                else
                {
                    char bookmarkString[128];
                    strcpy(bookmarkString, "");

                    for (int i = 0; i < ct - 1; i++)
                    {
                        strcpy(args[i], args[i + 1]);
                    }

                    int index = ct - 1;
                    for (int i = 0; i < index; i++)
                    {
                        strcat(bookmarkString, args[i]);
                        if (i != index - 1)
                        {
                            strcat(bookmarkString, " ");
                        }
                    }
                    insertBookmark(bookmarkString);
                    //printf("%s", bookmarkString);
                }
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

                    execv(getPath(args[0]), args);
                }
                if (background == 0)
                {
                    if (PID != waitpid(PID, NULL, 0))
                    {
                        fprintf(stderr, "%s", "Some error occurred while waiting for a foreground application to end.\n");
                        EXIT_FAILURE;
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

            if (strcmp("0001", identifier) == 0)
            {
                //printf("merhaba ben output\n");

                //printf("%s\n", args[1]);
                //printf("%c\n", inputBuffer[3]);
                int indexOfOperator = 0;
                char outputFileName[128];
                int indexOfOutputFileName = 0;

                for (int i = 0; i < 128; i++)
                {
                    if (inputBuffer[i] == '>')
                    {
                        indexOfOperator = i + 1;
                        i++;
                        continue;
                    }
                    else if (indexOfOperator > 0)
                    {
                        outputFileName[indexOfOutputFileName] = inputBuffer[i];
                        indexOfOutputFileName++;
                        if (inputBuffer[i] == '\0')
                        {
                            outputFileName[indexOfOutputFileName] = '\0';
                            break;
                        }
                    }
                }

                int fileDescription = open(outputFileName, FLAGS_FOR_OUTPUT, PERMISSION_FOR_OUTPUT_FILE);
                if (fileDescription == -1)
                {
                    fprintf(stderr, "%s", "Opening file process has given an error.");
                }

                PID = fork();
                if (PID == -1)
                {
                    fprintf(stderr, "%s", "An unexpected error occurred while creating new process");
                }

                if (PID == 0)
                {
                    dup2(fileDescription, STDOUT_FILENO);
                    execv(getPath(args[0]), &args[0]);
                }

                //This instruction will always be foreground, so we have to wait
                if (PID != waitpid(PID, NULL, 0))
                {
                    fprintf(stderr, "%s", "Some error occurred while waiting for a foreground application to end.");
                }
                close(fileDescription);
            }
            else if (strcmp("0010", identifier) == 0)
            {
                //printf("%s\n", args[1]);
                //printf("%c\n", inputBuffer[3]);
                int indexOfOperator = 0;
                char outputFileName[128];
                int indexOfOutputFileName = 0;

                for (int i = 0; i < 128; i++)
                {
                    if (inputBuffer[i] == '>')
                    {
                        indexOfOperator = i + 1;
                        i = i + 2;
                        continue;
                    }
                    else if (indexOfOperator > 0)
                    {
                        outputFileName[indexOfOutputFileName] = inputBuffer[i];
                        indexOfOutputFileName++;
                        if (inputBuffer[i] == '\0')
                        {
                            outputFileName[indexOfOutputFileName] = '\0';
                            break;
                        }
                    }
                }

                int fileDescription = open(outputFileName, FLAGS_FOR_APPEND, PERMISSION_FOR_OUTPUT_FILE);
                if (fileDescription == -1)
                {
                    fprintf(stderr, "%s", "Opening file process has given an error.");
                }

                PID = fork();
                if (PID == -1)
                {
                    fprintf(stderr, "%s", "An unexpected error occurred while creating new process");
                }

                if (PID == 0)
                {
                    dup2(fileDescription, 1);
                    execv(getPath(args[0]), &args[0]);
                }

                //This instruction will always be foreground, so we have to wait
                if (PID != waitpid(PID, NULL, 0))
                {
                    fprintf(stderr, "%s", "Some error occurred while waiting for a foreground application to end.");
                }
                close(fileDescription);
            }
            else if (strcmp("0011", identifier) == 0)
            {
                //printf("%s\n", args[1]);
                //printf("%c\n", inputBuffer[3]);
                int indexOfOperator = 0;
                char outputFileName[128];
                int indexOfOutputFileName = 0;

                for (int i = 0; i < 128; i++)
                {
                    if (inputBuffer[i] == '<')
                    {
                        indexOfOperator = i + 1;
                        i++;
                        continue;
                    }
                    else if (indexOfOperator > 0)
                    {
                        outputFileName[indexOfOutputFileName] = inputBuffer[i];
                        indexOfOutputFileName++;
                        if (inputBuffer[i] == '\0')
                        {
                            outputFileName[indexOfOutputFileName] = '\0';
                            break;
                        }
                    }
                }

                int fileDescription = open(outputFileName, FLAGS_FOR_INPUT);
                if (fileDescription == -1)
                {
                    fprintf(stderr, "%s", "Opening file process has given an error.");
                }

                PID = fork();
                if (PID == -1)
                {
                    fprintf(stderr, "%s", "An unexpected error occurred while creating new process");
                }

                if (PID == 0)
                {
                    dup2(fileDescription, STDIN_FILENO);
                    execv(getPath(args[0]), &args[0]);
                }

                //This instruction will always be foreground, so we have to wait
                if (PID != waitpid(PID, NULL, 0))
                {
                    fprintf(stderr, "%s", "Some error occurred while waiting for a foreground application to end.");
                }
                close(fileDescription);
            }
            else if (strcmp("0100", identifier) == 0)
            {
                //printf("merhaba ben both input and output\n");
                int indexOfOperator = 0;
                char inputFileName[128];
                char outputFileName[128];
                int indexOfOutputFileName = 0;
                int indexOfInputFileName = 0;

                for (int i = 0; i < 128; i++)
                {
                    if (inputBuffer[i] == '<')
                    {
                        indexOfOperator = i + 1;
                        i++;
                        continue;
                    }
                    else if (indexOfOperator > 0)
                    {
                        inputFileName[indexOfInputFileName] = inputBuffer[i];
                        indexOfInputFileName++;
                        if (inputBuffer[i] == '>')
                        {
                            inputFileName[indexOfInputFileName - 1] = '\0';

                            break;
                        }
                    }
                }
                indexOfOperator = 0;

                for (int i = 0; i < 128; i++)
                {
                    if (inputBuffer[i] == '>')
                    {
                        indexOfOperator = i + 1;
                        i++;
                        continue;
                    }
                    else if (indexOfOperator > 0)
                    {
                        outputFileName[indexOfOutputFileName] = inputBuffer[i];
                        indexOfOutputFileName++;
                        if (inputBuffer[i] == '\0')
                        {
                            outputFileName[indexOfOutputFileName] = '\0';

                            break;
                        }
                    }
                }

                //printf("%s ---- %s\n", inputFileName, outputFileName);

                int inputFileDescription = open(inputFileName, FLAGS_FOR_INPUT);
                if (inputFileDescription == -1)
                {
                    fprintf(stderr, "%s", "Opening file process has given an error.");
                }

                int outputFileDescription = open(outputFileName, FLAGS_FOR_OUTPUT, PERMISSION_FOR_OUTPUT_FILE);
                if (inputFileDescription == -1)
                {
                    fprintf(stderr, "%s", "Opening file process has given an error.");
                }

                PID = fork();
                if (PID == -1)
                {
                    fprintf(stderr, "%s", "An unexpected error occurred while creating new process");
                }

                if (PID == 0)
                {
                    dup2(inputFileDescription, STDIN_FILENO);
                    dup2(outputFileDescription, STDOUT_FILENO);

                    execv(getPath(args[0]), &args[0]);
                }

                //This instruction will always be foreground, so we have to wait
                if (PID != waitpid(PID, NULL, 0))
                {
                    fprintf(stderr, "%s", "Some error occurred while waiting for a foreground application to end.");
                }
                close(inputFileDescription);
                close(outputFileDescription);
            }
            else if (strcmp("0101", identifier) == 0)
            {
                printf("merhaba ben both input and append\n");
            }
            else if (strcmp("0110", identifier) == 0)
            {
                printf("merhaba ben both input and std error\n");
            }
            else if (strcmp("0111", identifier) == 0)
            {
                //printf("merhaba ben std error\n");
                //printf("%s\n", args[1]);
                //printf("%c\n", inputBuffer[3]);
                int indexOfOperator = 0;
                char outputFileName[128];
                int indexOfOutputFileName = 0;

                for (int i = 0; i < 128; i++)
                {
                    if (inputBuffer[i] == '2' && inputBuffer[i + 1] == '>')
                    {
                        indexOfOperator = i + 1;
                        i = i + 2;
                        continue;
                    }
                    else if (indexOfOperator > 0)
                    {
                        outputFileName[indexOfOutputFileName] = inputBuffer[i];
                        indexOfOutputFileName++;
                        if (inputBuffer[i] == '\0')
                        {
                            outputFileName[indexOfOutputFileName] = '\0';
                            break;
                        }
                    }
                }

                int fileDescription = open(outputFileName, FLAGS_FOR_OUTPUT, PERMISSION_FOR_OUTPUT_FILE);
                if (fileDescription == -1)
                {
                    fprintf(stderr, "%s", "Opening file process has given an error.");
                }

                PID = fork();
                if (PID == -1)
                {
                    fprintf(stderr, "%s", "An unexpected error occurred while creating new process");
                }

                if (PID == 0)
                {
                    dup2(fileDescription, STDERR_FILENO);
                    execv(getPath(args[0]), &args[0]);
                }

                //This instruction will always be foreground, so we have to wait
                if (PID != waitpid(PID, NULL, 0))
                {
                    fprintf(stderr, "%s", "Some error occurred while waiting for a foreground application to end.");
                }
                close(fileDescription);
            }
            else
            {
                fprintf(stderr, "%s", "Your input format is invalid in our architecture.\n");
            }
        }
        printf("\n");
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
    int countOfOperators = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);
    //printf("%d\n",length);
    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0); /* ^d was entered, end of user command stream */

    if (length == 1)
    {
        fprintf(stderr, "%s\n", "Please execute program again and specify input arguments.");
        exit(-1);
    }
    /* the signal interrupted the read system call */
    /* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ((length < 0) && (errno != EINTR))
    {
        perror("error reading the command");
        exit(-1); /* terminate with error code of -1 */
    }

    // if (strstr(inputBuffer, "bookmark") != NULL || strstr(inputBuffer, "-l") != NULL || strstr(inputBuffer, "-d") != NULL || strstr(inputBuffer, "-i") != NULL)
    // {
    //     return;
    // }
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
            if (strcmp(identifier, "0011") == 0) // If we have multiple operators like > < , 2 >
            {
                if (inputBuffer[i + 1] == '>')
                {
                    countOfOperators++;
                    identifier = strdup("0101");
                }
                else if (inputBuffer[i - 1] == '2')
                {
                    countOfOperators++;
                    identifier = strdup("0110");
                }
                else
                {
                    countOfOperators++;
                    identifier = strdup("0100");
                }
            }
            else if (strcmp(identifier, "0101") != 0 && strcmp(identifier, "0110") != 0 && strcmp(identifier, "0100") != 0 && strcmp(identifier, "0010") != 0 && strcmp(identifier, "0111") != 0) // Means that input does not contain multiple operators
            {
                if (inputBuffer[i + 1] == '>')
                {
                    countOfOperators++;
                    identifier = strdup("0010");
                }
                else if (inputBuffer[i - 1] == '2')
                {
                    countOfOperators++;
                    identifier = strdup("0111");
                }
                else
                {
                    countOfOperators++;
                    identifier = strdup("0001");
                }
            }
            break;
        case '<':
            if (strcmp(identifier, "0011") == 0)
            {
                countOfOperators++;
                identifier = strdup("1000"); // ERROR CASE
                return;
            }
            else
            {
                countOfOperators++;
                identifier = strdup("0011");
            }

            break;

        default: /* some other character */
            if (start == -1)
                start = i;
            if (inputBuffer[i] == '&')
            {
                *background = 1;
                inputBuffer[i - 1] = '\0';
            }
        } /* end of switch */
    }
    /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */

    // if (args[1] == NULL && strcmp(identifier, "0001") == 0)
    // {
    //     fprintf(stderr, "%s", "Argument count is less than expected.\n");
    //     return;
    // }

    if (strcmp(identifier, "0001") == 0 || strcmp(identifier, "0010") == 0 || strcmp(identifier, "0111") == 0 || strcmp(identifier, "0100") == 0)
    {
        args[ct - 1] = NULL;
    }

    //args[ct-1] = NULL;
    //     for (i = 0; i <= ct; i++)
    //     {
    //         // argumanın command olup olmadığını kontrol eden fonksiyon yazacağız

    //         int j = 0;
    //         int redirectionCount = 0;
    //         while(1){
    //             if(args[i][j] != '\0')
    //                 break;
    //             if((args[i][j] != '<' || args[i][j] != '<') && i == 0)
    //             {
    //                 printf("hata var");
    //             }
    //             else if ( args[i][j] != '<' )
    //             {
    //                 // input
    //                 redirectionCount++;
    //                 if (redirectionCount > 1) {
    //                     // hata
    //                 }
    //             } else if ( args[i][j] != '>' )
    //             {
    //                 // input
    //                 redirectionCount++;
    //                 if (redirectionCount > 2) {
    //                     // hata
    //                     return;
    //                 } else if (redirectionCount == 2) {
    //                     // append
    //                 } else {
    //                     // output
    //                 }
    //             }

    //             j++;
    //         }
    //     }
    //     printf("args %d = %s\n", i, args[i]);
    // }
    /* end of setup routine */
}
char *getPath(char *arg)

{
    char buffer[100];
    char qwz[100];
    getcwd(buffer, 100);
    strcpy(qwz, ":");
    strcat(qwz, buffer);

    strcat(env_path, qwz);

    char *temp = env_path;

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
    }
    return "-1";
}

void insertBookmark(char *command)
{
    if (head == NULL)
    {
        struct bookmarkNode *temp = (struct bookmarkNode *)malloc(sizeof(struct bookmarkNode));
        temp->command = strdup(command);
        temp->next = NULL;
        head = temp;
        return;
    }
    else
    {
        struct bookmarkNode *iter = head;
        int previousIndex = 0;

        while (iter->next != NULL)
        {
            iter = iter->next;
        }

        struct bookmarkNode *link = (struct bookmarkNode *)malloc(sizeof(struct bookmarkNode));
        iter->next = link;
        link->command = strdup(command);
        link->next = NULL;
    }
    // //create a link
    // struct bookmarkNode *link = (struct bookmarkNode *)malloc(sizeof(struct bookmarkNode));

    // //link->key = key;
    // link->command = strdup(command);

    // //point it to old first node
    // link->next = head;

    // //point first to new first node
    // head = link;
}

void printBookmarkList()
{

    struct bookmarkNode *ptr = head;

    //start from the beginning
    int i = 0;
    while (ptr != NULL)
    {
        printf(" %d %s\n", i, ptr->command);
        ptr = ptr->next;
        i++;
    }
}

void removeBookmark(int position)
{
    int pos = 0;

    if (head == NULL)
    {
        printf("Bookmark List not initialized");
        return;
    }

    current = head;

    if (pos == position)
    {
        if (head->next != NULL)
        {
            head = head->next;
            free(current);
            return;
        }
        else
        {
            head = NULL;
            printf("Book list is empty now");
            return;
        }
    }
    else if (pos != position && head->next == NULL)
    {
        printf("Entered index %d does not exists in the bookmark\n", position);
        return;
    }

    prev = head;

    while (current->next != NULL && pos != position)
    {
        prev = current;
        current = current->next;
        pos++;
    }

    if (pos == position)
    {
        prev->next = prev->next->next;
        free(current);
    }
}

char *getExecutableStringFromBookmarks(int position)
{
    int pos = 0;
    char *returnExecutableCommand;
    if (head == NULL)
    {
        printf("Bookmark List not initialized");
        return NULL;
    }

    current = head;
    while (current != NULL)
    {
        if (pos == position)
        {
            returnExecutableCommand = strdup(current->command);
            //strcpy(returnExecutableCommand,current->command);
            removeAllChars(returnExecutableCommand, '"');
            return returnExecutableCommand;
        }

        current = current->next;
        pos++;
    }

    return NULL;
}

int isArgumentNumber(char *arg)
{

    int i = 0;
    while (arg[i] != '\0')
    {

        if (!(arg[i] >= '0' && arg[i] <= '9'))
        {
            return 0;
        }
        i++;
    }
    return 1;
}

void removeAllChars(char *str, char c)
{
    char *pr = str, *pw = str;
    while (*pr)
    {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}

int searchInCurrentDirectory(char *input)
{
    char curWorkingDir[255];
    getcwd(curWorkingDir, 255);

    char *fileName;
    DIR *dir;
    struct dirent *entry;

    int occurrenceCount = 0;

    if ((dir = opendir(curWorkingDir)) != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            fileName = entry->d_name;

            if (strstr(fileName, ".c") || strstr(fileName, ".C") || strstr(fileName, ".h") || strstr(fileName, ".H"))
            {
                FILE *fp;
                int lineNumber = 1;

                char buffer[1024];
                fp = fopen(fileName, "r");
                if (fp == NULL)
                {
                    return -1;
                }

                while (fgets(buffer, 1024, fp) != NULL)
                {
                    if ((strstr(buffer, strdup(input))) != NULL)
                    {
                        char *temp = trim(buffer);

                        printf("%d: %s/%s -> %s\n", lineNumber, curWorkingDir, fileName, temp);
                        occurrenceCount++;
                    }
                    lineNumber++;
                }
                fclose(fp);
            }
        }
        if (occurrenceCount == 0)
        {
            printf("Could not find any occurrence under current working directory.");
        }
    }
    closedir(dir);
    return 1;
}

char *ltrim(char *s)
{
    while (isspace(*s))
        s++;
    return s;
}

char *rtrim(char *s)
{
    char *back = s + strlen(s);
    while (isspace(*--back))
        ;
    *(back + 1) = '\0';
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s));
}

int searchInfile(char *fname, char *str)
{
    FILE *fp;
    int line_num = 1;
    int find_result = 0;
    char temp[1024];

    if ((fp = fopen(fname, "r")) == NULL)
    {
        return (-1);
    }

    while (fgets(temp, 1024, fp) != NULL)
    {
        if ((strstr(temp, strdup(str))) != NULL)
        {
            printf("%d: %s -> %s\n", line_num, fname, temp);

            find_result++;
        }
        line_num++;
    }

    if (find_result == 0)
    {
        fprintf(stderr, "\nSorry, couldn't find a match.\n");
    }

    if (fp)
    {
        fclose(fp);
    }

    return (0);
}

void recursiveSearch(char *path, char *str)
{
    char *filearray;
    char *directory;
    int i = 0;

    DIR *d = opendir(path);

    if (d == NULL)
        return;
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL)
    {
        if (dir->d_type != DT_DIR)
        {
            filearray = dir->d_name;
            char d_path[1024];
            sprintf(d_path, "%s/%s", path, dir->d_name);
            if (strstr(filearray, ".c") || strstr(filearray, ".C") || strstr(filearray, ".h") || strstr(filearray, ".H"))
            {
                if (searchInfile(d_path, str) == -1)
                {
                    fprintf(stderr, "COULDNT READ THE FILE !");
                }
            }
        }

        else if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
        {
            char d_path[1024];
            sprintf(d_path, "%s/%s", path, dir->d_name);
            recursiveSearch(d_path, str);
        }
    }
    closedir(d);
}
