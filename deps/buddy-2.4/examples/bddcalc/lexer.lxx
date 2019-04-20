/*************************************************************************
  FILE:  lexer.l
  DESCR: FLEX rules for BDD calculator
  AUTH:  Jorn Lind
  DATE:  (C) may 1999
*************************************************************************/
%{
#include <string.h>
#include <stdlib.h>
#include "parser_.h"
#include "parser.h"
%}


/*************************************************************************
   Macros and sub-lexers
*************************************************************************/

DIGIT	[0-9]
ID	[a-zA-Z_][a-zA-Z0-9_]*
LINE	[^\n]

%x	COMM

%%

 /**************************************************************************
   Tokens
 **************************************************************************/

   /* Keywords */

"initial"	return T_initial;
"inputs"	return T_inputs;
"actions"	return T_actions;
"exist"		return T_exist;
"forall"	return T_forall;
"size"		return T_size;
"dot"		return T_dot;
"autoreorder"	return T_autoreorder;
"reorder"	return T_reorder;
"win2"		return T_win2;
"win2ite"	return T_win2ite;
"sift"		return T_sift;
"siftite"	return T_siftite;
"none"		return T_none;
"cache"		return T_cache;
"tautology"	return T_tautology;
"true"		return T_true;
"false"		return T_false;
"print"		return T_print;

   /* Symbols and operators */

"."		return T_dot;
"("		return T_lpar;
")"		return T_rpar;
";"		return T_semi;
"="		return T_equal;
"<>"		return T_biimp;
"=>"		return T_imp;
"|"		return T_or;
"^"		return T_xor;
"&"		return T_and;
"~"		return T_not;
"biimp"		return T_biimp;
"imp"		return T_imp;
"or"		return T_or;
"xor"		return T_xor;
"and"		return T_and;
"not"		return T_not;
"nand"		return T_nand;
"nor"		return T_nor;

   /* Identifiers and constant values */
{ID}		{ strncpy(yylval.id, yytext, MAXIDLEN); return T_id; }
{DIGIT}+	{ yylval.ival = atol(yytext); return T_intval; }

   /* Strings */
\"[^\"]*\"	{ yylval.str=sdup(yytext+1);
		  yylval.str[strlen(yylval.str)-1]=0; return T_str; }

   /* Whitespace */
[\n]		{ linenum++; }
[\r\t\f ]	/* ignore blanks */


"//"{LINE}*	/* Remove one line comments */


 /**************************************************************************
   Remove multi line comments
 **************************************************************************/

"/*"			BEGIN COMM;
<COMM>[^*\n]*		/* ignore */
<COMM>"*"[^*/\n]*	/* ignore */
<COMM>\n		{ linenum++; }
<COMM>"*/"		BEGIN INITIAL;

.			{ yyerror("Unknown symbol"); }
