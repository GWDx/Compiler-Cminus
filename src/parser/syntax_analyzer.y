%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "syntax_tree.h"

// external functions from lex
extern int yylex();
extern int yyparse();
extern int yyrestart();
extern FILE * yyin;

// external variables from lexical_analyzer module
extern int lines;
extern char * yytext;
extern int pos_end;
extern int pos_start;

// Global syntax tree
syntax_tree *gt;

// Error reporting
void yyerror(const char *s);

// Helper functions written for you with love
syntax_tree_node *node(const char *node_name, int children_num, ...);
%}

/* TODO: Complete this definition.
   Hint: See pass_node(), node(), and syntax_tree.h.
         Use forward declaring. */
%union {
    struct _syntax_tree_node *node;     // syntax_tree_node * 会报错

}

/* TODO: Your tokens here. */

%token <node> INT FLOAT VOID
%token <node> IF ELSE WHILE RETURN
%token <node> ID
%token <node> FLOATPOINT INTEGER
%token <node> ADD SUBTRACT MULTIPLY DIVIDE
%token <node> LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token <node> GREATEREQUAL GREATER LESS LESSEQUAL EQUAL UNEQUAL
%token <node> ASSIGN SEMICOLON COMMA
%token <node> ERROR


%type <node> program declaration_list declaration var_declaration type_specifier
%type <node> fun_declaration params param_list param compound_stmt
%type <node> local_declarations statement_list statement expression_stmt selection_stmt
%type <node> iteration_stmt return_stmt expression var simple_expression
%type <node> relop additive_expression addop term mulop
%type <node> factor integer float call args
%type <node> arg_list


%start program


%%
/* TODO: Your rules here. */

// 1
program : declaration_list {$$ = node("program", 1, $1); gt->root = $$;};

declaration_list : declaration_list declaration | declaration;

declaration : var_declaration | fun_declaration;

var_declaration : type_specifier ID SEMICOLON | type_specifier ID LBRACKET INTEGER RBRACKET SEMICOLON;

type_specifier : INT | FLOAT | VOID;

// 6
fun_declaration : type_specifier ID LPAREN params RPAREN compound_stmt;

params : param_list | VOID;

param_list : param_list COMMA param | param;

param : type_specifier ID | type_specifier ID LBRACKET RBRACKET;

compound_stmt : LBRACE local_declarations statement_list RBRACE;

// 11
local_declarations : local_declarations var_declaration | {};

statement_list : statement_list statement | {};

statement : expression_stmt | compound_stmt | selection_stmt | iteration_stmt | return_stmt;

expression_stmt : expression SEMICOLON | SEMICOLON;

selection_stmt : IF LPAREN expression RPAREN statement | IF LPAREN expression RPAREN statement ELSE statement;

// 16
iteration_stmt : WHILE LPAREN expression RPAREN statement;

return_stmt : RETURN SEMICOLON | RETURN expression SEMICOLON;

expression : var ASSIGN expression | simple_expression;

var : ID | ID LBRACKET expression RBRACKET;

simple_expression : additive_expression relop additive_expression | additive_expression;

// 21
relop : GREATEREQUAL | GREATER | LESS | LESSEQUAL | EQUAL | UNEQUAL;

additive_expression : additive_expression addop term | term;

addop : ADD | SUBTRACT;

term : term mulop factor | factor;

mulop : MULTIPLY | DIVIDE;

// 26

factor : LPAREN expression RPAREN | var | call | integer | float;

integer : INTEGER;

float : FLOATPOINT;

call : ID LPAREN args RPAREN;

args : arg_list | {};

// 31
arg_list : arg_list COMMA expression | expression;

%%

/// The error reporting function.
void yyerror(const char * s)
{
    // TO STUDENTS: This is just an example.
    // You can customize it as you like.
    fprintf(stderr, "error at line %d column %d: %s\n", lines, pos_start, s);
}

/// Parse input from file `input_path`, and prints the parsing results
/// to stdout.  If input_path is NULL, read from stdin.
///
/// This function initializes essential states before running yyparse().
syntax_tree *parse(const char *input_path)
{
    if (input_path != NULL) {
        if (!(yyin = fopen(input_path, "r"))) {
            fprintf(stderr, "[ERR] Open input file %s failed.\n", input_path);
            exit(1);
        }
    } else {
        yyin = stdin;
    }

    lines = pos_start = pos_end = 1;
    gt = new_syntax_tree();
    yyrestart(yyin);
    yyparse();
    return gt;
}

/// A helper function to quickly construct a tree node.
///
/// e.g. $$ = node("program", 1, $1);
syntax_tree_node *node(const char *name, int children_num, ...)
{
    syntax_tree_node *p = new_syntax_tree_node(name);
    syntax_tree_node *child;
    if (children_num == 0) {
        child = new_syntax_tree_node("epsilon");
        syntax_tree_add_child(p, child);
    } else {
        va_list ap;
        va_start(ap, children_num);
        for (int i = 0; i < children_num; ++i) {
            child = va_arg(ap, syntax_tree_node *);
            syntax_tree_add_child(p, child);
        }
        va_end(ap);
    }
    return p;
}
