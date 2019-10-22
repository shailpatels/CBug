#ifndef MAIN_H_H
#define MAIN_H_H

#define STDLIBS "#include <stdio.h>\n"\
                "#include <stdlib.h>\n"

#define MAIN "int main(){\n"

#define STREQL(x,y) strcmp(x,y) == 0
#define MAXBUF 1024

int SAVEFD = 0;
//symmetric check
int LPAREN = 0;
int RPAREN = 0;
int LCURLY = 0;
int RCURLY = 0;

#endif
