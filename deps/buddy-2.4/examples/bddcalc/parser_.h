/*************************************************************************
  FILE:  parser.h
  DESCR: parser defs. for BDD calculator
  AUTH:  Jorn Lind
  DATE:  (C) may 1999
*************************************************************************/

#ifndef _PARSER_H
#define _PARSER_H

#include <stdio.h>
#include "bdd.h"

#define MAXIDLEN 32  /* Max. number of allowed characters in an identifier */

struct token         /* BISON token data */
{
   char id[MAXIDLEN+1];
   char *str;
   int ival;
   bdd *bval;
};

#define YYSTYPE token
#define YY_SKIP_YYWRAP
#define YY_NO_UNPUT
#define yywrap() (1)

extern YYSTYPE yylval;            /* Declare for flex user */
extern void yyerror(char *,...);  /* Declare for flex and bison */
extern FILE *yyin;
extern int yylex(void);           /* Declare for bison */
extern int yyparse(void);         /* Declare for bison user */
extern int linenum;               /* Declare for error handler */

   /* Use this instead of strdup() to avoid malloc() */
inline char *sdup(const char *s) 
{
   return strcpy(new char[strlen(s)+1], s);
}

#endif /* _PARSER_H */

/* EOF */
