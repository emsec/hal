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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



