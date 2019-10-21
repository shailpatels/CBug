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
   RUN = 0; 
}

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

void repl(int fd, char * fname){
    char buf[MAXBUF]; 
    char cwd[MAXBUF];
    
    if (!getcwd(cwd,MAXBUF-1)){
        fprintf(stderr, "Failed to get current working directory\n");
        return;
    } 

    int len,r,w;
    while(RUN){
        //READ
        printf(">");
        if(! fgets(buf, MAXBUF, stdin))
            break; 
        
        len = strlen(buf); 
        
        lseek(fd, 0, SEEK_END);
        w = write(fd, buf, len);
        if (w != len){
            fprintf(stderr, "Failed to write to file skipping\n");
            continue;
        }  
    

        //EVAL
        write(fd, "}", strlen("}")); 
        char tgt[10];
        strcpy(tgt, fname);
        tgt[strlen(tgt)-2] = '\0';
        //compile
        ADDSPACE = 1;
        char * cmd = concat(6, "cc", fname, "-o", tgt, "2>", ".out");
        system(cmd);
        free(cmd);
        //execute
        ADDSPACE = 0;
        char * exc = concat(2, "./", tgt);
        system(exc);
        free(exc);
        int len = lseek(fd, 0, SEEK_END);
        ftruncate(fd, len-1);
        
    } 
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
}
