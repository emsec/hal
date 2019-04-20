/*************************************************************************
  FILE:  parser.y
  DESCR: BISON rules and main program for BDD calculator
  AUTH:  Jorn Lind
  DATE:  (C) may 1999
*************************************************************************/

%{
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

%}

/*************************************************************************
   Token definitions
*************************************************************************/

%token T_id T_str T_intval T_true T_false

%token T_initial T_inputs T_actions
%token T_size T_dumpdot
%token T_autoreorder T_reorder T_win2 T_win2ite T_sift T_siftite T_none
%token T_cache T_tautology T_print

%token T_lpar T_rpar
%token T_equal
%token T_semi T_dot

%right T_exist T_forall T_dot
%left T_biimp
%left T_imp
%left T_or T_nor
%left T_xor
%left T_nand T_and
%right T_not

/*************************************************************************
   BISON rules
*************************************************************************/
%%

/*=[ Top ]==============================================================*/

calc:
   initial inputs actions
   ;

/*=[ Initializers ]=====================================================*/

initial:
   T_initial T_intval T_intval T_semi { actInit(&$2,&$3); }
   ;

inputs:
   T_inputs inputSeq T_semi { actInputs(); }
   ;

inputSeq:
   inputSeq T_id { actAddInput(&$2); }
   | T_id        { actAddInput(&$1); }
   ;


/*=[ Actions ]==========================================================*/

actions:
   T_actions actionSeq
   ;

actionSeq:
   actionSeq action T_semi
   | action T_semi
   ;

action:
   assign
   | size
   | dot
   | reorder
   | cache
   | tautology
   | print
   ;

assign:
   T_id T_equal expr { actAssign(&$1,&$3); }
   ;

expr:
   expr T_and expr      { actOpr2(&$$,&$1,&$3,bddop_and); }
   | expr T_nand expr   { actOpr2(&$$,&$1,&$3,bddop_nand); }
   | expr T_xor expr    { actOpr2(&$$,&$1,&$3,bddop_xor); }
   | expr T_or expr     { actOpr2(&$$,&$1,&$3,bddop_or); }
   | expr T_nor expr    { actOpr2(&$$,&$1,&$3,bddop_nor); }
   | expr T_imp expr    { actOpr2(&$$,&$1,&$3,bddop_imp); }
   | expr T_biimp expr  { actOpr2(&$$,&$1,&$3,bddop_biimp); }
   | T_not expr         { actNot(&$$,&$2); }
   | T_lpar expr T_rpar { $$.bval = $2.bval; }
   | T_id               { actId(&$$,&$1); }
   | T_true             { $$.bval = new bdd(bddtrue); }
   | T_false            { $$.bval = new bdd(bddfalse); }
   | quantifier         { $$.bval = $1.bval; }
   ;

quantifier:
   T_exist varlist T_dot expr { actExist(&$$,&$2,&$4); }
   | T_forall varlist T_dot expr { actForall(&$$,&$2,&$4); }
   ;

varlist:
   varlist T_id { actQuantVar2(&$$,&$2,&$1); }
   | T_id       { actQuantVar1(&$$,&$1); }
   ;


size:
   T_size T_id { actSize(&$2); }
   ;

dot:
   T_dumpdot T_str T_id { actDot(&$2,&$3); }
   ;

reorder:
   T_reorder method                { bdd_reorder($2.ival); }
   | T_autoreorder T_intval method { actAutoreorder(&$2,&$3); }
   ;

method:
   T_win2       { $$.ival = BDD_REORDER_WIN2; }
   | T_win2ite  { $$.ival = BDD_REORDER_WIN2ITE; }
   | T_sift     { $$.ival = BDD_REORDER_SIFT; }
   | T_siftite  { $$.ival = BDD_REORDER_SIFTITE; }
   | T_none     { $$.ival = BDD_REORDER_NONE; }
   ;

cache:
   T_cache { actCache(); }
   ;

tautology:
   T_tautology T_id { actTautology(&$2); }
   ;

print:
   T_print T_id { actPrint(&$2); }

%%
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
