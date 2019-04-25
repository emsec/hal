/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_id = 258,
     T_str = 259,
     T_intval = 260,
     T_true = 261,
     T_false = 262,
     T_initial = 263,
     T_inputs = 264,
     T_actions = 265,
     T_size = 266,
     T_dumpdot = 267,
     T_autoreorder = 268,
     T_reorder = 269,
     T_win2 = 270,
     T_win2ite = 271,
     T_sift = 272,
     T_siftite = 273,
     T_none = 274,
     T_cache = 275,
     T_tautology = 276,
     T_print = 277,
     T_lpar = 278,
     T_rpar = 279,
     T_equal = 280,
     T_semi = 281,
     T_dot = 282,
     T_forall = 283,
     T_exist = 284,
     T_biimp = 285,
     T_imp = 286,
     T_nor = 287,
     T_or = 288,
     T_xor = 289,
     T_and = 290,
     T_nand = 291,
     T_not = 292
   };
#endif
#define T_id 258
#define T_str 259
#define T_intval 260
#define T_true 261
#define T_false 262
#define T_initial 263
#define T_inputs 264
#define T_actions 265
#define T_size 266
#define T_dumpdot 267
#define T_autoreorder 268
#define T_reorder 269
#define T_win2 270
#define T_win2ite 271
#define T_sift 272
#define T_siftite 273
#define T_none 274
#define T_cache 275
#define T_tautology 276
#define T_print 277
#define T_lpar 278
#define T_rpar 279
#define T_equal 280
#define T_semi 281
#define T_dot 282
#define T_forall 283
#define T_exist 284
#define T_biimp 285
#define T_imp 286
#define T_nor 287
#define T_or 288
#define T_xor 289
#define T_and 290
#define T_nand 291
#define T_not 292




/* Copy the first part of user declarations.  */
#line 8 "parser.yxx"

#include <string>
#include <stdarg.h>
#include <fstream>
#include <getopt.h>
#define IMPLEMENTSLIST /* Special for list template handling */
#include "slist.h"
#include "hashtbl.h"
#include "parser_.h"

   /* Definitions for storing and caching of identifiers */
#define inputTag  0
#define exprTag   1

   struct nodeData
   {
      nodeData(const nodeData &d) { tag=d.tag; name=sdup(d.name); val=d.val; }
      nodeData(int t, char *n, bdd v) { tag=t; name=n; val=v; }
      ~nodeData(void) { delete[] name; }
      int tag;
      char *name;
      bdd val;
   };

   typedef SList<nodeData> nodeLst;
   nodeLst inputs;
   hashTable names;

      /* Other */
   int linenum;

   bddgbchandler gbcHandler = bdd_default_gbchandler;

      /* Prototypes */
void actInit(token *nodes, token *cache);
void actInputs(void);
void actAddInput(token *id);
void actAssign(token *id, token *expr);
void actOpr2(token *res, token *left, token *right, int opr);
void actNot(token *res, token *right);
void actId(token *res, token *id);
void actConst(token *res, int);
void actSize(token *id);
void actDot(token *fname, token *id);
void actAutoreorder(token *times, token *method);
void actCache(void);
void actTautology(token *id);
void actExist(token *res, token *var, token *expr);
void actForall(token *res, token *var, token *expr);
void actQuantVar2(token *res, token *id, token *list);
void actQuantVar1(token *res, token *id);
void actPrint(token *id);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 215 "parser.cxx"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   88

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  38
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  19
/* YYNRULES -- Number of rules. */
#define YYNRULES  46
/* YYNRULES -- Number of states. */
#define YYNSTATES  84

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   292

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     7,    12,    16,    19,    21,    24,    28,
      31,    33,    35,    37,    39,    41,    43,    45,    49,    53,
      57,    61,    65,    69,    73,    77,    80,    84,    86,    88,
      90,    92,    97,   102,   105,   107,   110,   114,   117,   121,
     123,   125,   127,   129,   131,   133,   136
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      39,     0,    -1,    40,    41,    43,    -1,     8,     5,     5,
      26,    -1,     9,    42,    26,    -1,    42,     3,    -1,     3,
      -1,    10,    44,    -1,    44,    45,    26,    -1,    45,    26,
      -1,    46,    -1,    50,    -1,    51,    -1,    52,    -1,    54,
      -1,    55,    -1,    56,    -1,     3,    25,    47,    -1,    47,
      35,    47,    -1,    47,    36,    47,    -1,    47,    34,    47,
      -1,    47,    33,    47,    -1,    47,    32,    47,    -1,    47,
      31,    47,    -1,    47,    30,    47,    -1,    37,    47,    -1,
      23,    47,    24,    -1,     3,    -1,     6,    -1,     7,    -1,
      48,    -1,    29,    49,    27,    47,    -1,    28,    49,    27,
      47,    -1,    49,     3,    -1,     3,    -1,    11,     3,    -1,
      12,     4,     3,    -1,    14,    53,    -1,    13,     5,    53,
      -1,    15,    -1,    16,    -1,    17,    -1,    18,    -1,    19,
      -1,    20,    -1,    21,     3,    -1,    22,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    94,    94,   100,   104,   108,   109,   116,   120,   121,
     125,   126,   127,   128,   129,   130,   131,   135,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   155,   156,   160,   161,   166,   170,   174,   175,   179,
     180,   181,   182,   183,   187,   191,   195
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_id", "T_str", "T_intval", "T_true", 
  "T_false", "T_initial", "T_inputs", "T_actions", "T_size", "T_dumpdot", 
  "T_autoreorder", "T_reorder", "T_win2", "T_win2ite", "T_sift", 
  "T_siftite", "T_none", "T_cache", "T_tautology", "T_print", "T_lpar", 
  "T_rpar", "T_equal", "T_semi", "T_dot", "T_forall", "T_exist", 
  "T_biimp", "T_imp", "T_nor", "T_or", "T_xor", "T_and", "T_nand", 
  "T_not", "$accept", "calc", "initial", "inputs", "inputSeq", "actions", 
  "actionSeq", "action", "assign", "expr", "quantifier", "varlist", 
  "size", "dot", "reorder", "method", "cache", "tautology", "print", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    38,    39,    40,    41,    42,    42,    43,    44,    44,
      45,    45,    45,    45,    45,    45,    45,    46,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    48,    48,    49,    49,    50,    51,    52,    52,    53,
      53,    53,    53,    53,    54,    55,    56
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     3,     4,     3,     2,     1,     2,     3,     2,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     1,     1,     1,
       1,     4,     4,     2,     1,     2,     3,     2,     3,     1,
       1,     1,     1,     1,     1,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,     0,     0,     0,     1,     0,     0,     0,     6,
       0,     0,     2,     3,     5,     4,     0,     0,     0,     0,
       0,    44,     0,     0,     7,     0,    10,    11,    12,    13,
      14,    15,    16,     0,    35,     0,     0,    39,    40,    41,
      42,    43,    37,    45,    46,     0,     9,    27,    28,    29,
       0,     0,     0,     0,    17,    30,    36,    38,     8,     0,
      34,     0,     0,    25,     0,     0,     0,     0,     0,     0,
       0,    26,    33,     0,     0,    24,    23,    22,    21,    20,
      18,    19,    32,    31
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     2,     3,     7,    10,    12,    24,    25,    26,    54,
      55,    61,    27,    28,    29,    42,    30,    31,    32
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -38
static const yysigned_char yypact[] =
{
      -6,     7,    15,    14,    46,   -38,    49,    44,    50,   -38,
      -2,    36,   -38,   -38,   -38,   -38,    30,    74,    75,    73,
      51,   -38,    77,    78,    36,    56,   -38,   -38,   -38,   -38,
     -38,   -38,   -38,    -3,   -38,    80,    51,   -38,   -38,   -38,
     -38,   -38,   -38,   -38,   -38,    58,   -38,   -38,   -38,   -38,
      -3,    82,    82,    -3,    10,   -38,   -38,   -38,   -38,    29,
     -38,     8,    11,   -38,    -3,    -3,    -3,    -3,    -3,    -3,
      -3,   -38,   -38,    -3,    -3,   -26,    39,   -17,   -17,   -14,
     -38,   -38,    10,    10
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -38,   -38,   -38,   -38,   -38,   -38,   -38,    62,   -38,   -37,
     -38,    35,   -38,   -38,   -38,    52,   -38,   -38,   -38
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      47,    14,     1,    48,    49,    65,    66,    67,    68,    69,
      70,    72,     4,    59,    72,     5,    63,    68,    69,    70,
      50,    69,    70,     6,    15,    51,    52,    75,    76,    77,
      78,    79,    80,    81,    53,    73,    82,    83,    74,    16,
      64,    65,    66,    67,    68,    69,    70,    17,    18,    19,
      20,     8,     9,    71,    11,    33,    21,    22,    23,    64,
      65,    66,    67,    68,    69,    70,    37,    38,    39,    40,
      41,    66,    67,    68,    69,    70,    13,    34,    36,    35,
      43,    44,    46,    56,    58,    60,    45,    62,    57
};

static const unsigned char yycheck[] =
{
       3,     3,     8,     6,     7,    31,    32,    33,    34,    35,
      36,     3,     5,    50,     3,     0,    53,    34,    35,    36,
      23,    35,    36,     9,    26,    28,    29,    64,    65,    66,
      67,    68,    69,    70,    37,    27,    73,    74,    27,     3,
      30,    31,    32,    33,    34,    35,    36,    11,    12,    13,
      14,     5,     3,    24,    10,    25,    20,    21,    22,    30,
      31,    32,    33,    34,    35,    36,    15,    16,    17,    18,
      19,    32,    33,    34,    35,    36,    26,     3,     5,     4,
       3,     3,    26,     3,    26,     3,    24,    52,    36
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     8,    39,    40,     5,     0,     9,    41,     5,     3,
      42,    10,    43,    26,     3,    26,     3,    11,    12,    13,
      14,    20,    21,    22,    44,    45,    46,    50,    51,    52,
      54,    55,    56,    25,     3,     4,     5,    15,    16,    17,
      18,    19,    53,     3,     3,    45,    26,     3,     6,     7,
      23,    28,    29,    37,    47,    48,     3,    53,    26,    47,
       3,    49,    49,    47,    30,    31,    32,    33,    34,    35,
      36,    24,     3,    27,    27,    47,    47,    47,    47,    47,
      47,    47,    47,    47
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 100 "parser.yxx"
    { actInit(&yyvsp[-2],&yyvsp[-1]); }
    break;

  case 4:
#line 104 "parser.yxx"
    { actInputs(); }
    break;

  case 5:
#line 108 "parser.yxx"
    { actAddInput(&yyvsp[0]); }
    break;

  case 6:
#line 109 "parser.yxx"
    { actAddInput(&yyvsp[0]); }
    break;

  case 17:
#line 135 "parser.yxx"
    { actAssign(&yyvsp[-2],&yyvsp[0]); }
    break;

  case 18:
#line 139 "parser.yxx"
    { actOpr2(&yyval,&yyvsp[-2],&yyvsp[0],bddop_and); }
    break;

  case 19:
#line 140 "parser.yxx"
    { actOpr2(&yyval,&yyvsp[-2],&yyvsp[0],bddop_nand); }
    break;

  case 20:
#line 141 "parser.yxx"
    { actOpr2(&yyval,&yyvsp[-2],&yyvsp[0],bddop_xor); }
    break;

  case 21:
#line 142 "parser.yxx"
    { actOpr2(&yyval,&yyvsp[-2],&yyvsp[0],bddop_or); }
    break;

  case 22:
#line 143 "parser.yxx"
    { actOpr2(&yyval,&yyvsp[-2],&yyvsp[0],bddop_nor); }
    break;

  case 23:
#line 144 "parser.yxx"
    { actOpr2(&yyval,&yyvsp[-2],&yyvsp[0],bddop_imp); }
    break;

  case 24:
#line 145 "parser.yxx"
    { actOpr2(&yyval,&yyvsp[-2],&yyvsp[0],bddop_biimp); }
    break;

  case 25:
#line 146 "parser.yxx"
    { actNot(&yyval,&yyvsp[0]); }
    break;

  case 26:
#line 147 "parser.yxx"
    { yyval.bval = yyvsp[-1].bval; }
    break;

  case 27:
#line 148 "parser.yxx"
    { actId(&yyval,&yyvsp[0]); }
    break;

  case 28:
#line 149 "parser.yxx"
    { yyval.bval = new bdd(bddtrue); }
    break;

  case 29:
#line 150 "parser.yxx"
    { yyval.bval = new bdd(bddfalse); }
    break;

  case 30:
#line 151 "parser.yxx"
    { yyval.bval = yyvsp[0].bval; }
    break;

  case 31:
#line 155 "parser.yxx"
    { actExist(&yyval,&yyvsp[-2],&yyvsp[0]); }
    break;

  case 32:
#line 156 "parser.yxx"
    { actForall(&yyval,&yyvsp[-2],&yyvsp[0]); }
    break;

  case 33:
#line 160 "parser.yxx"
    { actQuantVar2(&yyval,&yyvsp[0],&yyvsp[-1]); }
    break;

  case 34:
#line 161 "parser.yxx"
    { actQuantVar1(&yyval,&yyvsp[0]); }
    break;

  case 35:
#line 166 "parser.yxx"
    { actSize(&yyvsp[0]); }
    break;

  case 36:
#line 170 "parser.yxx"
    { actDot(&yyvsp[-1],&yyvsp[0]); }
    break;

  case 37:
#line 174 "parser.yxx"
    { bdd_reorder(yyvsp[0].ival); }
    break;

  case 38:
#line 175 "parser.yxx"
    { actAutoreorder(&yyvsp[-1],&yyvsp[0]); }
    break;

  case 39:
#line 179 "parser.yxx"
    { yyval.ival = BDD_REORDER_WIN2; }
    break;

  case 40:
#line 180 "parser.yxx"
    { yyval.ival = BDD_REORDER_WIN2ITE; }
    break;

  case 41:
#line 181 "parser.yxx"
    { yyval.ival = BDD_REORDER_SIFT; }
    break;

  case 42:
#line 182 "parser.yxx"
    { yyval.ival = BDD_REORDER_SIFTITE; }
    break;

  case 43:
#line 183 "parser.yxx"
    { yyval.ival = BDD_REORDER_NONE; }
    break;

  case 44:
#line 187 "parser.yxx"
    { actCache(); }
    break;

  case 45:
#line 191 "parser.yxx"
    { actTautology(&yyvsp[0]); }
    break;

  case 46:
#line 195 "parser.yxx"
    { actPrint(&yyvsp[0]); }
    break;


    }

/* Line 999 of yacc.c.  */
#line 1331 "parser.cxx"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 93 "parser.yxx"

/*************************************************************************
   Main and more
*************************************************************************/

void usage(void)
{
   using namespace std ;
   cerr << "USAGE: bddcalc [-hg] file\n";
   cerr << " -h : print this message\n";
   cerr << " -g : disable garbage collection info\n";
}


int main(int ac, char **av)
{
   using namespace std ;
   int c;

   while ((c=getopt(ac, av, "hg")) != EOF)
   {
      switch (c)
      {
      case 'h':
	 usage();
	 break;
      case 'g':
	 gbcHandler = bdd_default_gbchandler;
	 break;
      }
   }

   if (optind >= ac)
      usage();

   yyin = fopen(av[optind],"r");
   if (!yyin)
   {
      cerr << "Could not open file: " << av[optind] << endl;
      exit(2);
   }

   linenum = 1;
   bdd_setcacheratio(2);
   yyparse();

   bdd_printstat();
   bdd_done();

   return 0;
}


void yyerror(char *fmt, ...)
{
   va_list argp;
   va_start(argp,fmt);
   fprintf(stderr, "Parse error in (or before) line %d: ", linenum);
   vfprintf(stderr, fmt, argp);
   va_end(argp);
   exit(3);
}


/*************************************************************************
   Semantic actions
*************************************************************************/

void actInit(token *nodes, token *cache)
{
   bdd_init(nodes->ival, cache->ival);
   bdd_gbc_hook(gbcHandler);
   bdd_reorder_verbose(0);
}


void actInputs(void)
{
   bdd_setvarnum(inputs.size());

   int vnum=0;
   for (nodeLst::ite i=inputs.first() ; i.more() ; i++, vnum++)
   {
      if (names.exists((*i).name))
	 yyerror("Redefinition of input %s", (*i).name);

      (*i).val = bdd_ithvar(vnum);
      hashData hd((*i).name, 0, &(*i));
      names.add(hd);
   }

   bdd_varblockall();
}


void actAddInput(token *id)
{
   inputs.append( nodeData(inputTag,sdup(id->id),bddtrue) );
}


void actAssign(token *id, token *expr)
{
   if (names.exists(id->id))
      yyerror("Redefinition of %s", id->id);

   nodeData *d = new nodeData(exprTag, sdup(id->id), *expr->bval);
   hashData hd(d->name, 0, d);
   names.add(hd);
   delete expr->bval;
}


void actOpr2(token *res, token *left, token *right, int opr)
{
   res->bval = new bdd( bdd_apply(*left->bval, *right->bval, opr) );
   delete left->bval;
   delete right->bval;
}


void actNot(token *res, token *right)
{
   res->bval = new bdd( bdd_not(*right->bval) );
   delete right->bval;
   //printf("%5d -> %f\n", fixme, bdd_satcount(*res->bval));
}


void actId(token *res, token *id)
{
   hashData hd;

   if (names.lookup(id->id,hd) == 0)
   {
      res->bval = new bdd( ((nodeData*)hd.def)->val );
   }
   else
      yyerror("Unknown variable %s", id->id);
}


void actExist(token *res, token *var, token *expr)
{
   res->bval = new bdd( bdd_exist(*expr->bval, *var->bval) );
   delete var->bval;
   delete expr->bval;
}


void actForall(token *res, token *var, token *expr)
{
   res->bval = new bdd( bdd_forall(*expr->bval, *var->bval) );
   delete var->bval;
   delete expr->bval;
}


void actQuantVar2(token *res, token *id, token *list)
{
   hashData hd;

   if (names.lookup(id->id,hd) == 0)
   {
      if (hd.type == inputTag)
      {
	 res->bval = list->bval;
	 *res->bval &= ((nodeData*)hd.def)->val;
      }
      else
	 yyerror("%s is not a variable", id->id);
   }
   else
      yyerror("Unknown variable %s", id->id);
}


void actQuantVar1(token *res, token *id)
{
   hashData hd;

   if (names.lookup(id->id,hd) == 0)
   {
      if (hd.type == inputTag)
	 res->bval = new bdd( ((nodeData*)hd.def)->val );
      else
	 yyerror("%s is not a variable", id->id);
   }
   else
      yyerror("Unknown variable %s", id->id);
}


void actSize(token *id)
{
   using namespace std ;
   hashData hd;

   if (names.lookup(id->id,hd) == 0)
   {
      cout << "Number of nodes used for " << id->id << " = "
	   << bdd_nodecount(((nodeData*)hd.def)->val) << endl;
   }
   else
      yyerror("Unknown variable %s", id->id);
}


void actDot(token *fname, token *id)
{
   using namespace std ;
   hashData hd;

   if (names.lookup(id->id,hd) == 0)
   {
      if (bdd_fnprintdot(fname->str, ((nodeData*)hd.def)->val) < 0)
	 cout << "Could not open file: " << fname->str << endl;
   }
   else
      yyerror("Unknown variable %s", id->id);
}


void actAutoreorder(token *times, token *method)
{
   if (times->ival == 0)
      bdd_autoreorder(method->ival);
   else
      bdd_autoreorder_times(method->ival, times->ival);
}


void actCache(void)
{
   bdd_printstat();
}


void actTautology(token *id)
{
   using namespace std ;
   hashData hd;

   if (names.lookup(id->id,hd) == 0)
   {
      if (((nodeData*)hd.def)->val == bddtrue)
	 cout << "Formula " << id->id << " is a tautology!\n";
      else
	 cout << "Formula " << id->id << " is NOT a tautology!\n";
   }
   else
      yyerror("Unknown variable %s", id->id);
}


void actPrint(token *id)
{
   using namespace std ;
   hashData hd;

   if (names.lookup(id->id,hd) == 0)
      cout << id->id << " = " << bddset << ((nodeData*)hd.def)->val << endl;
   else
      yyerror("Unknown variable %s", id->id);
}

/* EOF */

