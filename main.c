#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include "main.h"

/* 
    CBug is an read evaluate print loop (REPL) program for C
    programming. 
    
    Shail Patel 2019
*/

void parseArgs(int argc, char ** argv){
    for(int i = 0; i < argc; ++i){
        char * arg = argv[i];
        if ( STREQL(arg, "-save") )
            SAVEFD = 1;
    }
}

static volatile int RUN = 1;
void sigcapt(int sig){
   printf("got signal %d\n", sig);
   RUN = 0; 
}

//given n strings, glue them together
//if ADDSPACE is true a space will be added
//between each string
int ADDSPACE = 1;
char * concat(int n, ...){
    if (n <= 0)
        return NULL;

    va_list args;
    va_start(args, n);

    int i = 0;
    int len = 0;
    while(i < n){
        char * nxt = va_arg(args, char *);
        len += strlen(nxt); 
        ++i;
    } 
    
    if (ADDSPACE)
        len += (1 + n);

    char * ret = malloc(len);
    
    va_start(args, n);
    i = 0;
    len = 0;
    while(i < n){
        char * nxt = va_arg(args, char *);
        
        strcat(ret,nxt);
        if (ADDSPACE)
            strcat(ret," ");
        
        ++i;
    } 
    
    va_end(args);
    return ret;
}

//is a command symmetric, ie complete
//this can usually be done by checking 
//if theres a semi-colon at the end

//for things like loops and structs check if the parens and 
//curly brackets line up
int isSymmetric(char * str){
    //start at the back and see if we hit a ';'
    char * c = str + strlen(str) - 1;
    while(*c){
        if (*c == ';' && LCURLY == RCURLY && LPAREN == RPAREN)
            return 1;
        
        if (isalnum(*c))
            break;
        
        c--;
    } 
    
    //update the count of parens and brackets
    c = str;
    while(*c){
        switch(*c){
            case '{': RCURLY++; break;
            case '}': LCURLY++; break;
            case '(': LPAREN++; break;
            case ')': RPAREN++; break;
        } 
        c++;
    }
    
    int sym = RCURLY == LCURLY && LPAREN == RPAREN;
    
    if (!sym)
        return 0; 
    
    //if they are equal check if Parens are greater than zero
    //if Curlys then we can finish
    
    if (LCURLY > 0)
        return 1;
    
    if (LPAREN > 0)
        return 0;
    
    return 0;
}

void readErr(void){
    system("cat .out");
}

//if the .out file is empty no stderr, otherwise
//rollback
int didErr(void){
    //check file, we can't use O_APPEND since
    //we need to file offset to check the size 
    int fd = open(".out", O_RDONLY); 
    int len = lseek(fd, 0, SEEK_END);
    close(fd);

    return len > 0; 
}

void repl(int fd, char * fname){
    char buf[MAXBUF]; 
    char tgt[10];
    
    int len,w = 0;
    while(RUN){
        //READ
        printf(">");
        if(! fgets(buf, MAXBUF, stdin))
            break; 
        
        if(!RUN)
            break;

        len = strlen(buf); 
        
        lseek(fd, 0, SEEK_END);
        w += write(fd, buf, len);

        if (!isSymmetric(buf)){
            //multiline code, continue for now 
            continue;
        }

        //EVAL
        w += write(fd, "}", strlen("}")); 
        strcpy(tgt, fname);
        tgt[strlen(tgt)-2] = '\0';

        //compile
        ADDSPACE = 1;
        char * cmd = concat(6, "cc", fname, "-o", tgt, "2>", ".out");
        system(cmd);
        free(cmd);
        
        int len = lseek(fd, 0, SEEK_END);

        //did anything go wrong?
        if (didErr()){
            //undo w bytes and skip execution
            ftruncate(fd, len-w);
            readErr();
            w = 0;
            continue;
        }         
        
        w = 0; 
        //execute
        ADDSPACE = 0;
        char * exc = concat(2, "./", tgt);
        system(exc);
        free(exc);

        ftruncate(fd, len-1);
        
    } 

    //cleanup the compiled target
    int ret = unlink(tgt);
    if (ret < 0)
        printf("Warning failed to cleanup %s compiled file\n", tgt);
}

int main(int argc, char ** argv){
    printf("CBug : press CTRL+C+ENTER to exit\n");
    //prevent ctrl-c from killing before we clean up
    parseArgs(argc, argv); 
    signal(SIGINT, sigcapt);
    
    //create our buffer file first
    //O_TMPFILE isn't supported on all fs and doesn't
    //take in a mode for somereason? so use this

    srand(time(NULL));
    char fname[10];

    //create a random number from 0-999
    sprintf(fname, ".tmp%d.c", rand() % 1000 );
    int fd = open(fname, O_CREAT | O_RDWR, S_IRUSR );
    
    if ( fd < 0 ){
        perror("Failed to create tmp file");
        return EXIT_FAILURE;
    } 
    
    //give some stuff for free
    write(fd, STDLIBS, strlen(STDLIBS));    
    write(fd, MAIN, strlen(MAIN));
    repl(fd, fname);
    
    close(fd);
    int ret = 0;
    
    if (!SAVEFD)
        ret = unlink(fname);

    if (ret < 0){
        fprintf(stderr,"Failed to cleanup tmp file: \"%s\" ", fname);
        perror("");
        return EXIT_FAILURE;
    }
    
    ret = unlink(".out");
    if (ret < 0){
        fprintf(stderr,"Failed to cleanup tmp file: \".out\"");
        perror("");
        return EXIT_FAILURE;
    }
}
